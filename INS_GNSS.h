/*------------------------------------------------------------------------------
* INS_GNSS.h : INS_GNSS constants, types and function prototypes
*
*          Copyright (C) 2015-2018 by Cavalheri E.P. and Mendonca M.A.
*
* version : $Revision: 1.1 $ $Date:  $
* history : 2018/11/01 1.0  INS_GNSS ver.1.0.0
*-----------------------------------------------------------------------------*/
#ifndef INS_GNSS_H
#define INS_GNSS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include "rtklib.h"
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <pthread.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* constants -----------------------------------------------------------------*/

/* Earth parameters */
#define e_2	((2*FE_WGS84)-(FE_WGS84*FE_WGS84)) 				/* second eccentricity */
#define RN(lat) (RE_WGS84/sqrt(1-e_2*(sin(lat)*sin(lat)))) 			/* Prime vertical radii */
#define RM(lat) ( RE_WGS84*(1-e_2) / pow( 1-e_2*( sin(lat)*sin(lat) ),(3/2) ) ) /* Merdidian radius of curvature */
#define reeS(lat) (RN(lat)*sqrt(cos(lat)*cos(lat)+(1-e_2)*(1-e_2)*sin(lat)*sin(lat))) /* Geocentric radius at the surface*/
/* Normal gravity by Schwars and Wei (2000), p.30 from Shin (2001) */
#define	a1gn	9.7803267715
#define	a2gn	0.0052790414
#define	a3gn	0.0000232718
#define	a4gn	-0.0000030876910891
#define	a5gn	0.0000000043977311
#define	a6gn	0.0000000000007211
#define gn(lat,h) ( a1gn*(1+a2gn*(sin(lat)*sin(lat))+a3gn*(sin(lat)*sin(lat)*sin(lat)*sin(lat))) + h*(a4gn+a5gn*(sin(lat)*sin(lat))) + a6gn*h*h   ) /* Normal gravity on the ellipsoidal surface WGS84 - Defense Mapping Agency */

/* Coordinate rotation matrices (Jekeli, 2001; Shin, 2001) */

/* Earth-Centered Inertial to Earth-Centered Fixed (i to e-frame)
   where t is the angle (g.gg) and X is the output Rotation matrix	*/
#define Ci2e(t,X) do { \
 (X)[0]=cos(t); (X)[1]=sin(t); (X)[3]=-sin(t);(X)[4]=cos(t);\
 (X)[2]=(X)[5]=(X)[6]=(X)[7]=0;\
 (X)[8]=1;
} while (0) //In this case t=wie*t, earth rotation and the time since start of navigation

/* Earth-Centered Fixed to navigation frame (NED) (e to n-frame)	*/
#define Ce2n(lat,lon,X) do {\
 (X)[0]=-cos(lon)*sin(lat); (X)[1]=-sin(lon)*sin(lat); (X)[2]= cos(lat);\
 (X)[3]=-sin(lon);          (X)[4]=cos(lon);           (X)[5]=0;\
 (X)[6]=-cos(lon)*cos(lat); (X)[7]=-sin(lon)*cos(lat); (X)[8]=-sin(lat);\
} while (0)
/* n-frame to e-frame	*/
#define Cn2e(lat,lon,X) do {\
 (X)[0]=-cos(lon)*sin(lat); (X)[1]=-sin(lon); (X)[2]= -cos(lon)*cos(lat);\
 (X)[3]=-sin(lon)*sin(lat); (X)[4]=cos(lon);  (X)[5]=-sin(lon)*cos(lat);\
 (X)[6]=cos(lat);           (X)[7]=0;         (X)[8]=-sin(lat);\
} while (0)

/* Body-frame (x(nav dir.),y(dext. sys.), down(vertical)) to navigation frame (NED) (e to n-frame)	*/
#define Cb2n(phi,theta,psi,X) do {\
 (X)[0]=cos(theta)*cos(psi);(X)[1]=-cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi);(X)[2]=sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi);\
 (X)[3]=cos(theta)*sin(psi);(X)[4]=cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi);(X)[5]=-sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi); \
 (X)[6]=-sin(theta);(X)[7]=sin(phi)*cos(theta);(X)[8]=cos(phi)*cos(theta); \
} while (0)
/* n-frame to b-frame */
#define Cn2b(phi,theta,psi,X) do {\
 (X)[0]=cos(theta)*cos(psi);(X)[1]=cos(theta)*sin(psi);(X)[2]=-sin(theta);\
 (X)[3]=-cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi);(X)[4]=cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi);(X)[5]=sin(phi)*cos(theta); \
 (X)[6]=sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi);(X)[7]=-sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi);(X)[8]=cos(phi)*cos(theta); \
} while (0)

/* R1, R2 and R3 rotation matrices	*/
#define R1(phi,X) do {\
 (X)[0]=1; (X)[1]=0;        (X)[2]=0;\
 (X)[3]=0; (X)[4]=cos(phi); (X)[5]=-sin(phi);\
 (X)[6]=0; (X)[7]=sin(phi); (X)[8]=cos(phi);\
} while (0)

#define R2(theta,X) do {\
 (X)[0]=cos(theta);  (X)[1]=0; (X)[2]=sin(theta);\
 (X)[3]=0;           (X)[4]=1; (X)[5]=0;\
 (X)[6]=-sin(theta); (X)[7]=0; (X)[8]=cos(theta);\
} while (0)

#define R3(psi,X) do {\
 (X)[0]=cos(psi); (X)[1]=-sin(psi);(X)[2]=0;\
 (X)[3]=sin(psi); (X)[4]=cos(psi); (X)[5]=0;\
 (X)[6]=0;        (X)[7]=0;        (X)[8]=0;\
} while (0)

/* type definitions ----------------------------------------------------------*/

typedef struct {        /* initialization of pos., vel. and attitude rrors */
  double  delta_r_eb_n[3];     /* position error resolved along NED (m) */
  double  delta_v_eb_n[3];     /* velocity error resolved along NED (m/s) */
  double  delta_eul_nb_n[3];   /* attitude error as NED Euler angles (rad) */
} initialization_errors;

typedef struct {        /* IMU errors */
  double  delta_r_eb_n[3];     /* position error resolved along NED (m) */
  float   b_a[3];              /* Accelerometer biases (m/s^2) */
  float   b_g[3];              /* Gyro biases (rad/s) */
  double  M_a[9];              /* Accelerometer scale factor and cross coupling errors */
  double  M_g[9];              /* Gyro scale factor and cross coupling errors */
  double  G_g[9];              /* Gyro g-dependent biases (rad-sec/m) */
  float   accel_noise_root_PSD; /* Accelerometer noise root PSD (m s^-1.5) */
  float   gyro_noise_root_PSD; /* Gyro noise root PSD (rad s^-0.5) */
  float   accel_quant_level;   /* Accelerometer quantization level (m/s^2) */
  float   gyro_quant_level;    /* Gyro quantization level (rad/s) */
} IMU_errors;

typedef struct {      /* GNSS configuration */
  int epoch_interval;      /* Interval between GNSS epochs (s) */
  int init_est_r_ea_e;     /* Initial estimated position (m; ECEF) */
  int no_sat;              /* Number of satellites in constellation */
  float r_os;              /* Orbital radius of satellites (m) */
  float inclination;       /* Inclination angle of satellites (deg) */
  double const_delta_lambda; /* Longitude offset of constellation (deg) */
  float const_delta_t;     /* Timing offset of constellation (s) */
  int mask_angle;          /* Mask angle (deg) */
  float SIS_err_SD;        /* Signal in space error SD (m) */
  float zenith_iono_err_SD; /* Zenith ionosphere error SD (m) */
  float zenith_trop_err_SD; /* Zenith troposphere error SD (m) */
  float code_track_err_SD; /* Code tracking error SD (m) */
  float rate_track_err_SD; /* Range rate tracking error SD (m/s) */
  float rx_clock_offset;   /* Receiver clock offset at time=0 (m) */
  float rx_clock_drift;    /* Receiver clock drift at time=0 (m/s) */
} GNSS_config;

typedef struct {      /* Tighlty coupled Kalman Filter configuration */
 init_att_unc           Initial attitude uncertainty per axis (rad)
 init_vel_unc           Initial velocity uncertainty per axis (m/s)
 init_pos_unc           Initial position uncertainty per axis (m)
 init_b_a_unc           Initial accel. bias uncertainty (m/s^2)
 init_b_g_unc           Initial gyro. bias uncertainty (rad/s)
 init_clock_offset_unc  Initial clock offset uncertainty per axis (m)
 init_clock_drift_unc   Initial clock drift uncertainty per axis (m/s)
 gyro_noise_PSD         Gyro noise PSD (rad^2/s)
 accel_noise_PSD        Accelerometer noise PSD (m^2 s^-3)
 accel_bias_PSD         Accelerometer bias random walk PSD (m^2 s^-5)
 gyro_bias_PSD          Gyro bias random walk PSD (rad^2 s^-3)
 clock_freq_PSD         Receiver clock frequency-drift PSD (m^2/s^3)
 clock_phase_PSD        Receiver clock phase-drift PSD (m^2/s)
 pseudo_range_SD        Pseudo-range measurement noise SD (m)
 range_rate_SD          Pseudo-range rate measurement noise SD (m/s)
} TC_KF_config;


/* global variables ----------------------------------------------------------*/

/* function declaration ------------------------------------------------------*/

/* utilities functions */