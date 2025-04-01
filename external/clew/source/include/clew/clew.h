#pragma once


//////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2009-2011 Organic Vectory B.V.
//  Written by George van Venrooij
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file license.txt)
//////////////////////////////////////////////////////////////////////////

//! \file clew.h
//! \brief OpenCL run-time loader header
//!
//! This file contains a copy of the contents of CL.H and CL_PLATFORM.H from the
//! official OpenCL spec. The purpose of this code is to load the OpenCL dynamic
//! library at run-time and thus allow the executable to function on many
//! platforms regardless of the vendor of the OpenCL driver actually installed.
//! Some of the techniques used here were inspired by work done in the GLEW
//! library (http://glew.sourceforge.net/)

//  Run-time dynamic linking functionality based on concepts used in GLEW
#ifdef  __OPENCL_CL_H
#error cl.h included before clew.h
#endif

#ifdef  __OPENCL_CL_PLATFORM_H
#error cl_platform.h included before clew.h
#endif

#pragma once
//  Prevent cl.h inclusion

//  Prevent cl_platform.h inclusion



/*******************************************************************************
* Copyright (c) 2008-2009 The Khronos Group Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and/or associated documentation files (the
* "Materials"), to deal in the Materials without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Materials, and to
* permit persons to whom the Materials are furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Materials.
*
* THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
******************************************************************************/
#ifdef __APPLE__
/* Contains #defines for AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER below */
#include <AvailabilityMacros.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#if defined(_WIN32)

 __stdcall
#else


#endif

#if defined(__APPLE__)
   AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
       __attribute__((weak_import))
#else


#endif

#include <stdint.h>

#if defined(_WIN32) && defined(_MSC_VER)

/* scalar types  */
typedef signed   __int8         cl_char;
typedef unsigned __int8         cl_uchar;
typedef signed   __int16        cl_short;
typedef unsigned __int16        cl_ushort;
typedef signed   __int32        cl_int;
typedef unsigned __int32        cl_uint;
typedef signed   __int64        cl_long;
typedef unsigned __int64        cl_ulong;

typedef unsigned __int16        cl_half;
typedef float                   cl_float;
typedef double                  cl_double;


/*
* Vector types
*
*  Note:   OpenCL requires that all types be naturally aligned.
*          This means that vector types must be naturally aligned.
*          For example, a vector of four floats must be aligned to
*          a 16 byte boundary (calculated as 4 * the natural 4-byte
*          alignment of the float).  The alignment qualifiers here
*          will only function properly if your compiler supports them
*          and if you don't actively work to defeat them.  For example,
*          in order for a cl_float4 to be 16 byte aligned in a struct,
*          the start of the struct must itself be 16-byte aligned.
*
*          Maintaining proper alignment is the user's responsibility.
*/
typedef signed   __int8          cl_char2[2];
typedef signed   __int8          cl_char4[4];
typedef signed   __int8          cl_char8[8];
typedef signed   __int8          cl_char16[16];
typedef unsigned __int8         cl_uchar2[2];
typedef unsigned __int8         cl_uchar4[4];
typedef unsigned __int8         cl_uchar8[8];
typedef unsigned __int8         cl_uchar16[16];

typedef signed   __int16         cl_short2[2];
typedef signed   __int16         cl_short4[4];
typedef signed   __int16         cl_short8[8];
typedef signed   __int16         cl_short16[16];
typedef unsigned __int16        cl_ushort2[2];
typedef unsigned __int16        cl_ushort4[4];
typedef unsigned __int16        cl_ushort8[8];
typedef unsigned __int16        cl_ushort16[16];

typedef signed   __int32         cl_int2[2];
typedef signed   __int32         cl_int4[4];
typedef signed   __int32         cl_int8[8];
typedef signed   __int32         cl_int16[16];
typedef unsigned __int32        cl_uint2[2];
typedef unsigned __int32        cl_uint4[4];
typedef unsigned __int32        cl_uint8[8];
typedef unsigned __int32        cl_uint16[16];

typedef signed   __int64         cl_long2[2];
typedef signed   __int64         cl_long4[4];
typedef signed   __int64         cl_long8[8];
typedef signed   __int64         cl_long16[16];
typedef unsigned __int64        cl_ulong2[2];
typedef unsigned __int64        cl_ulong4[4];
typedef unsigned __int64        cl_ulong8[8];
typedef unsigned __int64        cl_ulong16[16];

typedef float           cl_float2[2];
typedef float           cl_float4[4];
typedef float           cl_float8[8];
typedef float           cl_float16[16];

typedef double          cl_double2[2];
typedef double          cl_double4[4];
typedef double          cl_double8[8];
typedef double          cl_double16[16];
/* There are no vector types for half */

#else

/* scalar types  */
typedef int8_t          cl_char;
typedef uint8_t         cl_uchar;
typedef int16_t         cl_short    __attribute__((aligned(2)));
typedef uint16_t        cl_ushort   __attribute__((aligned(2)));
typedef int32_t         cl_int      __attribute__((aligned(4)));
typedef uint32_t        cl_uint     __attribute__((aligned(4)));
typedef int64_t         cl_long     __attribute__((aligned(8)));
typedef uint64_t        cl_ulong    __attribute__((aligned(8)));

typedef uint16_t        cl_half     __attribute__((aligned(2)));
typedef float           cl_float    __attribute__((aligned(4)));
typedef double          cl_double   __attribute__((aligned(8)));

/*
* Vector types
*
*  Note:   OpenCL requires that all types be naturally aligned.
*          This means that vector types must be naturally aligned.
*          For example, a vector of four floats must be aligned to
*          a 16 byte boundary (calculated as 4 * the natural 4-byte
*          alignment of the float).  The alignment qualifiers here
*          will only function properly if your compiler supports them
*          and if you don't actively work to defeat them.  For example,
*          in order for a cl_float4 to be 16 byte aligned in a struct,
*          the start of the struct must itself be 16-byte aligned.
*
*          Maintaining proper alignment is the user's responsibility.
*/
typedef int8_t          cl_char2[2]     __attribute__((aligned(2)));
typedef int8_t          cl_char4[4]     __attribute__((aligned(4)));
typedef int8_t          cl_char8[8]     __attribute__((aligned(8)));
typedef int8_t          cl_char16[16]   __attribute__((aligned(16)));
typedef uint8_t         cl_uchar2[2]    __attribute__((aligned(2)));
typedef uint8_t         cl_uchar4[4]    __attribute__((aligned(4)));
typedef uint8_t         cl_uchar8[8]    __attribute__((aligned(8)));
typedef uint8_t         cl_uchar16[16]  __attribute__((aligned(16)));

typedef int16_t         cl_short2[2]     __attribute__((aligned(4)));
typedef int16_t         cl_short4[4]     __attribute__((aligned(8)));
typedef int16_t         cl_short8[8]     __attribute__((aligned(16)));
typedef int16_t         cl_short16[16]   __attribute__((aligned(32)));
typedef uint16_t        cl_ushort2[2]    __attribute__((aligned(4)));
typedef uint16_t        cl_ushort4[4]    __attribute__((aligned(8)));
typedef uint16_t        cl_ushort8[8]    __attribute__((aligned(16)));
typedef uint16_t        cl_ushort16[16]  __attribute__((aligned(32)));

typedef int32_t         cl_int2[2]      __attribute__((aligned(8)));
typedef int32_t         cl_int4[4]      __attribute__((aligned(16)));
typedef int32_t         cl_int8[8]      __attribute__((aligned(32)));
typedef int32_t         cl_int16[16]    __attribute__((aligned(64)));
typedef uint32_t        cl_uint2[2]     __attribute__((aligned(8)));
typedef uint32_t        cl_uint4[4]     __attribute__((aligned(16)));
typedef uint32_t        cl_uint8[8]     __attribute__((aligned(32)));
typedef uint32_t        cl_uint16[16]   __attribute__((aligned(64)));

typedef int64_t         cl_long2[2]     __attribute__((aligned(16)));
typedef int64_t         cl_long4[4]     __attribute__((aligned(32)));
typedef int64_t         cl_long8[8]     __attribute__((aligned(64)));
typedef int64_t         cl_long16[16]   __attribute__((aligned(128)));
typedef uint64_t        cl_ulong2[2]    __attribute__((aligned(16)));
typedef uint64_t        cl_ulong4[4]    __attribute__((aligned(32)));
typedef uint64_t        cl_ulong8[8]    __attribute__((aligned(64)));
typedef uint64_t        cl_ulong16[16]  __attribute__((aligned(128)));

typedef float           cl_float2[2]    __attribute__((aligned(8)));
typedef float           cl_float4[4]    __attribute__((aligned(16)));
typedef float           cl_float8[8]    __attribute__((aligned(32)));
typedef float           cl_float16[16]  __attribute__((aligned(64)));

typedef double          cl_double2[2]   __attribute__((aligned(16)));
typedef double          cl_double4[4]   __attribute__((aligned(32)));
typedef double          cl_double8[8]   __attribute__((aligned(64)));
typedef double          cl_double16[16] __attribute__((aligned(128)));

/* There are no vector types for half */

#endif

/******************************************************************************/

// Macro names and corresponding values defined by OpenCL

        127
        (-127-1)
         (-32767-1)
          (-2147483647-1)
         ((cl_long) 0x7FFFFFFFFFFFFFFFLL)
         ((cl_long) -0x7FFFFFFFFFFFFFFFLL - 1LL)
        ((cl_ulong) 0xFFFFFFFFFFFFFFFFULL)

   +38
      +128
   -37
      -125
#if defined(_MSC_VER)
// MSVC doesn't understand hex floats
          3.402823466e+38F
          1.175494351e-38F
      1.192092896e-07F
#else
          0x1.fffffep127f
          0x1.0p-126f
      0x1.0p-23f
#endif

   +308
      +1024
   -307
      -1021
#if defined(_MSC_VER)
// MSVC doesn't understand hex floats
          1.7976931348623158e+308
          2.2250738585072014e-308
      2.2204460492503131e-016
#else
          0x1.fffffffffffffp1023
          0x1.0p-1022
      0x1.0p-52
#endif

#include <stddef.h>


//  CL.h contents
/******************************************************************************/

typedef struct _cl_platform_id *    cl_platform_id;
typedef struct _cl_device_id *      cl_device_id;
typedef struct _cl_context *        cl_context;
typedef struct _cl_command_queue *  cl_command_queue;
typedef struct _cl_mem *            cl_mem;
typedef struct _cl_program *        cl_program;
typedef struct _cl_kernel *         cl_kernel;
typedef struct _cl_event *          cl_event;
typedef struct _cl_sampler *        cl_sampler;

typedef cl_uint             cl_bool;                     /* WARNING!  Unlike cl_ types in cl_platform.h, cl_bool is not guaranteed to be the same size as the bool in kernels. */
typedef cl_ulong            cl_bitfield;
typedef cl_bitfield         cl_device_type;
typedef cl_uint             cl_platform_info;
typedef cl_uint             cl_device_info;
typedef cl_bitfield         cl_device_address_info;
typedef cl_bitfield         cl_device_fp_config;
typedef cl_uint             cl_device_mem_cache_type;
typedef cl_uint             cl_device_local_mem_type;
typedef cl_bitfield         cl_device_exec_capabilities;
typedef cl_bitfield         cl_command_queue_properties;

typedef intptr_t            cl_context_properties;
typedef cl_uint             cl_context_info;
typedef cl_uint             cl_command_queue_info;
typedef cl_uint             cl_channel_order;
typedef cl_uint             cl_channel_type;
typedef cl_bitfield         cl_mem_flags;
typedef cl_uint             cl_mem_object_type;
typedef cl_uint             cl_mem_info;
typedef cl_uint             cl_image_info;
typedef cl_uint             cl_addressing_mode;
typedef cl_uint             cl_filter_mode;
typedef cl_uint             cl_sampler_info;
typedef cl_bitfield         cl_map_flags;
typedef cl_uint             cl_program_info;
typedef cl_uint             cl_program_build_info;
typedef cl_int              cl_build_status;
typedef cl_uint             cl_kernel_info;
typedef cl_uint             cl_kernel_work_group_info;
typedef cl_uint             cl_event_info;
typedef cl_uint             cl_command_type;
typedef cl_uint             cl_profiling_info;

typedef struct {
    cl_channel_order        image_channel_order;
    cl_channel_type         image_channel_data_type;
} cl_image_format;



/******************************************************************************/

// Error Codes
                                  0
                         -1
                     -2
                   -3
            -4
                         -5
                       -6
             -7
                         -8
                    -9
               -10
                    -11
                              -12

                            -30
                      -31
                         -32
                           -33
                          -34
                 -35
                    -36
                         -37
                       -38
          -39
                       -40
                          -41
                           -42
                    -43
                          -44
               -45
                      -46
                -47
                           -48
                        -49
                        -50
                         -51
                      -52
                   -53
                  -54
                   -55
                    -56
                  -57
                            -58
                        -59
                        -60
                      -61
                        -62
                 -63

// cl_bool
                                     1

// cl_platform_info
                         0x0900
                         0x0901
                            0x0902
                          0x0903
                      0x0904

// cl_device_type - bitfield
                      (1 << 0)
                          (1 << 1)
                          (1 << 2)
                  (1 << 3)
                       (1 << 4)
                          0xFFFFFFFF

// cl_device_info
                              0x1000
                         0x1001
                 0x1002
          0x1003
               0x1004
               0x1005
       0x1006
      0x1007
        0x1008
       0x1009
      0x100A
     0x100B
               0x100C
                      0x100D
               0x100E
              0x100F
                0x1010
                 0x1011
                0x1012
                 0x1013
                0x1014
                 0x1015
                     0x1016
                0x1017
                      0x1018
               0x1019
          0x101A
                  0x101B
             0x101C
         0x101D
             0x101E
                   0x101F
          0x1020
                 0x1021
                    0x1022
                    0x1023
          0x1024
        0x1025
                     0x1026
                         0x1027
                0x1028
            0x1029
                  0x102A
                              0x102B
                            0x102C
                           0x102D
                           0x102E
                           0x102F
                        0x1030
                          0x1031
                  0x1032
/* 0x1033 reserved for CL_DEVICE_HALF_FP_CONFIG */
       0x1034
               0x1035
          0x1036
         0x1037
           0x1038
          0x1039
         0x103A
        0x103B
          0x103C
                  0x103D
                  0x103E
                  0x103F
             0x1040
              0x1041
                     0x1042
         0x1043
              0x1044
         0x1045
                    0x1046
                   0x1047
       0x1048
                0x1049
             0x104A
      0x104B

// cl_device_fp_config - bitfield
                                (1 << 0)
                               (1 << 1)
                      (1 << 2)
                         (1 << 3)
                          (1 << 4)
                                   (1 << 5)

// cl_device_exec_capabilities - bitfield
                              (1 << 0)
                       (1 << 1)

// cl_command_queue_properties - bitfield
      (1 << 0)
                   (1 << 1)

// cl_context_info
                          0x1081

// cl_context_properties
                         0x1084

// cl_mem_flags - bitfield
                           (1 << 0)
                           (1 << 1)
                            (1 << 2)
                         (1 << 3)
                       (1 << 4)
                        (1 << 5)

// cl_map_flags - bitfield
                                 (1 << 0)
                                (1 << 1)

// cl_program_info
                      0x1162
                          0x1163
                     0x1165
                         0x1166

// cl_program_build_info
                     0x1181
                        0x1183

// cl_build_status
                               -1
                              -2
                        -3

/********************************************************************************************************/

/********************************************************************************************************/

//  Function signature typedef's

// Platform API
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETPLATFORMIDS)(cl_uint          /* num_entries */,
                 cl_platform_id * /* platforms */,
                 cl_uint *        /* num_platforms */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETPLATFORMINFO)(cl_platform_id   /* platform */,
                  cl_platform_info /* param_name */,
                  size_t           /* param_value_size */,
                  void *           /* param_value */,
                  size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Device APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETDEVICEIDS)(cl_platform_id   /* platform */,
               cl_device_type   /* device_type */,
               cl_uint          /* num_entries */,
               cl_device_id *   /* devices */,
               cl_uint *        /* num_devices */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETDEVICEINFO)(cl_device_id    /* device */,
                cl_device_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Context APIs
typedef CL_API_ENTRY cl_context (CL_API_CALL *
PFNCLCREATECONTEXT)(const cl_context_properties * /* properties */,
                cl_uint                       /* num_devices */,
                const cl_device_id *          /* devices */,
                void (*pfn_notify)(const char *, const void *, size_t, void *) /* pfn_notify */,
                void *                        /* user_data */,
                cl_int *                      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_context (CL_API_CALL *
PFNCLCREATECONTEXTFROMTYPE)(const cl_context_properties * /* properties */,
                        cl_device_type                /* device_type */,
                        void (*pfn_notify)(const char *, const void *, size_t, void *) /* pfn_notify */,
                        void *                        /* user_data */,
                        cl_int *                      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINCONTEXT)(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASECONTEXT)(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETCONTEXTINFO)(cl_context         /* context */,
                 cl_context_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Command Queue APIs
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *
PFNCLCREATECOMMANDQUEUE)(cl_context                     /* context */,
                     cl_device_id                   /* device */,
                     cl_command_queue_properties    /* properties */,
                     cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINCOMMANDQUEUE)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASECOMMANDQUEUE)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETCOMMANDQUEUEINFO)(cl_command_queue      /* command_queue */,
                      cl_command_queue_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLSETCOMMANDQUEUEPROPERTY)(cl_command_queue              /* command_queue */,
                          cl_command_queue_properties   /* properties */,
                          cl_bool                        /* enable */,
                          cl_command_queue_properties * /* old_properties */) CL_API_SUFFIX__VERSION_1_0;

// Memory Object APIs
typedef CL_API_ENTRY cl_mem (CL_API_CALL *
PFNCLCREATEBUFFER)(cl_context   /* context */,
               cl_mem_flags /* flags */,
               size_t       /* size */,
               void *       /* host_ptr */,
               cl_int *     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *
PFNCLCREATEIMAGE2D)(cl_context              /* context */,
                cl_mem_flags            /* flags */,
                const cl_image_format * /* image_format */,
                size_t                  /* image_width */,
                size_t                  /* image_height */,
                size_t                  /* image_row_pitch */,
                void *                  /* host_ptr */,
                cl_int *                /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *
PFNCLCREATEIMAGE3D)(cl_context              /* context */,
                cl_mem_flags            /* flags */,
                const cl_image_format * /* image_format */,
                size_t                  /* image_width */,
                size_t                  /* image_height */,
                size_t                  /* image_depth */,
                size_t                  /* image_row_pitch */,
                size_t                  /* image_slice_pitch */,
                void *                  /* host_ptr */,
                cl_int *                /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINMEMOBJECT)(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASEMEMOBJECT)(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETSUPPORTEDIMAGEFORMATS)(cl_context           /* context */,
                           cl_mem_flags         /* flags */,
                           cl_mem_object_type   /* image_type */,
                           cl_uint              /* num_entries */,
                           cl_image_format *    /* image_formats */,
                           cl_uint *            /* num_image_formats */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETMEMOBJECTINFO)(cl_mem           /* memobj */,
                   cl_mem_info      /* param_name */,
                   size_t           /* param_value_size */,
                   void *           /* param_value */,
                   size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETIMAGEINFO)(cl_mem           /* image */,
               cl_image_info    /* param_name */,
               size_t           /* param_value_size */,
               void *           /* param_value */,
               size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Sampler APIs
typedef CL_API_ENTRY cl_sampler (CL_API_CALL *
PFNCLCREATESAMPLER)(cl_context          /* context */,
                cl_bool             /* normalized_coords */,
                cl_addressing_mode  /* addressing_mode */,
                cl_filter_mode      /* filter_mode */,
                cl_int *            /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINSAMPLER)(cl_sampler /* sampler */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASESAMPLER)(cl_sampler /* sampler */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETSAMPLERINFO)(cl_sampler         /* sampler */,
                 cl_sampler_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Program Object APIs
typedef CL_API_ENTRY cl_program (CL_API_CALL *
PFNCLCREATEPROGRAMWITHSOURCE)(cl_context        /* context */,
                          cl_uint           /* count */,
                          const char **     /* strings */,
                          const size_t *    /* lengths */,
                          cl_int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_program (CL_API_CALL *
PFNCLCREATEPROGRAMWITHBINARY)(cl_context                     /* context */,
                          cl_uint                        /* num_devices */,
                          const cl_device_id *           /* device_list */,
                          const size_t *                 /* lengths */,
                          const unsigned char **         /* binaries */,
                          cl_int *                       /* binary_status */,
                          cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINPROGRAM)(cl_program /* program */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASEPROGRAM)(cl_program /* program */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLBUILDPROGRAM)(cl_program           /* program */,
               cl_uint              /* num_devices */,
               const cl_device_id * /* device_list */,
               const char *         /* options */,
               void (*pfn_notify)(cl_program /* program */, void * /* user_data */),
               void *               /* user_data */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLUNLOADCOMPILER)(void) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETPROGRAMINFO)(cl_program         /* program */,
                 cl_program_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETPROGRAMBUILDINFO)(cl_program            /* program */,
                      cl_device_id          /* device */,
                      cl_program_build_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Kernel Object APIs
typedef CL_API_ENTRY cl_kernel (CL_API_CALL *
PFNCLCREATEKERNEL)(cl_program      /* program */,
               const char *    /* kernel_name */,
               cl_int *        /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLCREATEKERNELSINPROGRAM)(cl_program     /* program */,
                         cl_uint        /* num_kernels */,
                         cl_kernel *    /* kernels */,
                         cl_uint *      /* num_kernels_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINKERNEL)(cl_kernel    /* kernel */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASEKERNEL)(cl_kernel   /* kernel */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLSETKERNELARG)(cl_kernel    /* kernel */,
               cl_uint      /* arg_index */,
               size_t       /* arg_size */,
               const void * /* arg_value */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETKERNELINFO)(cl_kernel       /* kernel */,
                cl_kernel_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETKERNELWORKGROUPINFO)(cl_kernel                  /* kernel */,
                         cl_device_id               /* device */,
                         cl_kernel_work_group_info  /* param_name */,
                         size_t                     /* param_value_size */,
                         void *                     /* param_value */,
                         size_t *                   /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Event Object APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLWAITFOREVENTS)(cl_uint             /* num_events */,
                const cl_event *    /* event_list */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETEVENTINFO)(cl_event         /* event */,
               cl_event_info    /* param_name */,
               size_t           /* param_value_size */,
               void *           /* param_value */,
               size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRETAINEVENT)(cl_event /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLRELEASEEVENT)(cl_event /* event */) CL_API_SUFFIX__VERSION_1_0;

// Profiling APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLGETEVENTPROFILINGINFO)(cl_event            /* event */,
                        cl_profiling_info   /* param_name */,
                        size_t              /* param_value_size */,
                        void *              /* param_value */,
                        size_t *            /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

// Flush and Finish APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLFLUSH)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLFINISH)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

// Enqueued Commands APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEREADBUFFER)(cl_command_queue    /* command_queue */,
                    cl_mem              /* buffer */,
                    cl_bool             /* blocking_read */,
                    size_t              /* offset */,
                    size_t              /* cb */,
                    void *              /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEWRITEBUFFER)(cl_command_queue   /* command_queue */,
                     cl_mem             /* buffer */,
                     cl_bool            /* blocking_write */,
                     size_t             /* offset */,
                     size_t             /* cb */,
                     const void *       /* ptr */,
                     cl_uint            /* num_events_in_wait_list */,
                     const cl_event *   /* event_wait_list */,
                     cl_event *         /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUECOPYBUFFER)(cl_command_queue    /* command_queue */,
                    cl_mem              /* src_buffer */,
                    cl_mem              /* dst_buffer */,
                    size_t              /* src_offset */,
                    size_t              /* dst_offset */,
                    size_t              /* cb */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEREADIMAGE)(cl_command_queue     /* command_queue */,
                   cl_mem               /* image */,
                   cl_bool              /* blocking_read */,
                   const size_t *       /* origin[3] */,
                   const size_t *       /* region[3] */,
                   size_t               /* row_pitch */,
                   size_t               /* slice_pitch */,
                   void *               /* ptr */,
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEWRITEIMAGE)(cl_command_queue    /* command_queue */,
                    cl_mem              /* image */,
                    cl_bool             /* blocking_write */,
                    const size_t *      /* origin[3] */,
                    const size_t *      /* region[3] */,
                    size_t              /* input_row_pitch */,
                    size_t              /* input_slice_pitch */,
                    const void *        /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUECOPYIMAGE)(cl_command_queue     /* command_queue */,
                   cl_mem               /* src_image */,
                   cl_mem               /* dst_image */,
                   const size_t *       /* src_origin[3] */,
                   const size_t *       /* dst_origin[3] */,
                   const size_t *       /* region[3] */,
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUECOPYIMAGETOBUFFER)(cl_command_queue /* command_queue */,
                           cl_mem           /* src_image */,
                           cl_mem           /* dst_buffer */,
                           const size_t *   /* src_origin[3] */,
                           const size_t *   /* region[3] */,
                           size_t           /* dst_offset */,
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUECOPYBUFFERTOIMAGE)(cl_command_queue /* command_queue */,
                           cl_mem           /* src_buffer */,
                           cl_mem           /* dst_image */,
                           size_t           /* src_offset */,
                           const size_t *   /* dst_origin[3] */,
                           const size_t *   /* region[3] */,
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *
PFNCLENQUEUEMAPBUFFER)(cl_command_queue /* command_queue */,
                   cl_mem           /* buffer */,
                   cl_bool          /* blocking_map */,
                   cl_map_flags     /* map_flags */,
                   size_t           /* offset */,
                   size_t           /* cb */,
                   cl_uint          /* num_events_in_wait_list */,
                   const cl_event * /* event_wait_list */,
                   cl_event *       /* event */,
                   cl_int *         /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *
PFNCLENQUEUEMAPIMAGE)(cl_command_queue  /* command_queue */,
                  cl_mem            /* image */,
                  cl_bool           /* blocking_map */,
                  cl_map_flags      /* map_flags */,
                  const size_t *    /* origin[3] */,
                  const size_t *    /* region[3] */,
                  size_t *          /* image_row_pitch */,
                  size_t *          /* image_slice_pitch */,
                  cl_uint           /* num_events_in_wait_list */,
                  const cl_event *  /* event_wait_list */,
                  cl_event *        /* event */,
                  cl_int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEUNMAPMEMOBJECT)(cl_command_queue /* command_queue */,
                        cl_mem           /* memobj */,
                        void *           /* mapped_ptr */,
                        cl_uint          /* num_events_in_wait_list */,
                        const cl_event *  /* event_wait_list */,
                        cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUENDRANGEKERNEL)(cl_command_queue /* command_queue */,
                       cl_kernel        /* kernel */,
                       cl_uint          /* work_dim */,
                       const size_t *   /* global_work_offset */,
                       const size_t *   /* global_work_size */,
                       const size_t *   /* local_work_size */,
                       cl_uint          /* num_events_in_wait_list */,
                       const cl_event * /* event_wait_list */,
                       cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUETASK)(cl_command_queue  /* command_queue */,
              cl_kernel         /* kernel */,
              cl_uint           /* num_events_in_wait_list */,
              const cl_event *  /* event_wait_list */,
              cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUENATIVEKERNEL)(cl_command_queue  /* command_queue */,
                      void (*user_func)(void *),
                      void *            /* args */,
                      size_t            /* cb_args */,
                      cl_uint           /* num_mem_objects */,
                      const cl_mem *    /* mem_list */,
                      const void **     /* args_mem_loc */,
                      cl_uint           /* num_events_in_wait_list */,
                      const cl_event *  /* event_wait_list */,
                      cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEMARKER)(cl_command_queue    /* command_queue */,
                cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEWAITFOREVENTS)(cl_command_queue /* command_queue */,
                       cl_uint          /* num_events */,
                       const cl_event * /* event_list */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *
PFNCLENQUEUEBARRIER)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

// Extension function access
//
// Returns the extension function address for the given function name,
// or NULL if a valid function can not be found.  The client must
// check to make sure the address is not NULL, before using or
// calling the returned function address.
//
typedef CL_API_ENTRY void * (CL_API_CALL * PFNCLGETEXTENSIONFUNCTIONADDRESS)(const char * /* func_name */) CL_API_SUFFIX__VERSION_1_0;

#ifdef CLEW_STATIC
#  define CLEWAPI extern
#else
#  ifdef CLEW_BUILD
#    if defined(_WIN32)
#      define CLEWAPI extern __declspec(dllexport)
#    else
#      define CLEWAPI extern __attribute__ ((visibility("default")))
#    endif
#  else
#    if defined(_WIN32)
#      define CLEWAPI extern __declspec(dllimport)
#    else
#      define CLEWAPI extern
#    endif
#  endif
#endif

 CLEWAPI

(x) x


//  Variables holding function entry points
CLEW_FUN_EXPORT     PFNCLGETPLATFORMIDS                 __clewGetPlatformIDs                ;
CLEW_FUN_EXPORT     PFNCLGETPLATFORMINFO                __clewGetPlatformInfo               ;
CLEW_FUN_EXPORT     PFNCLGETDEVICEIDS                   __clewGetDeviceIDs                  ;
CLEW_FUN_EXPORT     PFNCLGETDEVICEINFO                  __clewGetDeviceInfo                 ;
CLEW_FUN_EXPORT     PFNCLCREATECONTEXT                  __clewCreateContext                 ;
CLEW_FUN_EXPORT     PFNCLCREATECONTEXTFROMTYPE          __clewCreateContextFromType         ;
CLEW_FUN_EXPORT     PFNCLRETAINCONTEXT                  __clewRetainContext                 ;
CLEW_FUN_EXPORT     PFNCLRELEASECONTEXT                 __clewReleaseContext                ;
CLEW_FUN_EXPORT     PFNCLGETCONTEXTINFO                 __clewGetContextInfo                ;
CLEW_FUN_EXPORT     PFNCLCREATECOMMANDQUEUE             __clewCreateCommandQueue            ;
CLEW_FUN_EXPORT     PFNCLRETAINCOMMANDQUEUE             __clewRetainCommandQueue            ;
CLEW_FUN_EXPORT     PFNCLRELEASECOMMANDQUEUE            __clewReleaseCommandQueue           ;
CLEW_FUN_EXPORT     PFNCLGETCOMMANDQUEUEINFO            __clewGetCommandQueueInfo           ;
CLEW_FUN_EXPORT     PFNCLSETCOMMANDQUEUEPROPERTY        __clewSetCommandQueueProperty       ;
CLEW_FUN_EXPORT     PFNCLCREATEBUFFER                   __clewCreateBuffer                  ;
CLEW_FUN_EXPORT     PFNCLCREATEIMAGE2D                  __clewCreateImage2D                 ;
CLEW_FUN_EXPORT     PFNCLCREATEIMAGE3D                  __clewCreateImage3D                 ;
CLEW_FUN_EXPORT     PFNCLRETAINMEMOBJECT                __clewRetainMemObject               ;
CLEW_FUN_EXPORT     PFNCLRELEASEMEMOBJECT               __clewReleaseMemObject              ;
CLEW_FUN_EXPORT     PFNCLGETSUPPORTEDIMAGEFORMATS       __clewGetSupportedImageFormats      ;
CLEW_FUN_EXPORT     PFNCLGETMEMOBJECTINFO               __clewGetMemObjectInfo              ;
CLEW_FUN_EXPORT     PFNCLGETIMAGEINFO                   __clewGetImageInfo                  ;
CLEW_FUN_EXPORT     PFNCLCREATESAMPLER                  __clewCreateSampler                 ;
CLEW_FUN_EXPORT     PFNCLRETAINSAMPLER                  __clewRetainSampler                 ;
CLEW_FUN_EXPORT     PFNCLRELEASESAMPLER                 __clewReleaseSampler                ;
CLEW_FUN_EXPORT     PFNCLGETSAMPLERINFO                 __clewGetSamplerInfo                ;
CLEW_FUN_EXPORT     PFNCLCREATEPROGRAMWITHSOURCE        __clewCreateProgramWithSource       ;
CLEW_FUN_EXPORT     PFNCLCREATEPROGRAMWITHBINARY        __clewCreateProgramWithBinary       ;
CLEW_FUN_EXPORT     PFNCLRETAINPROGRAM                  __clewRetainProgram                 ;
CLEW_FUN_EXPORT     PFNCLRELEASEPROGRAM                 __clewReleaseProgram                ;
CLEW_FUN_EXPORT     PFNCLBUILDPROGRAM                   __clewBuildProgram                  ;
CLEW_FUN_EXPORT     PFNCLUNLOADCOMPILER                 __clewUnloadCompiler                ;
CLEW_FUN_EXPORT     PFNCLGETPROGRAMINFO                 __clewGetProgramInfo                ;
CLEW_FUN_EXPORT     PFNCLGETPROGRAMBUILDINFO            __clewGetProgramBuildInfo           ;
CLEW_FUN_EXPORT     PFNCLCREATEKERNEL                   __clewCreateKernel                  ;
CLEW_FUN_EXPORT     PFNCLCREATEKERNELSINPROGRAM         __clewCreateKernelsInProgram        ;
CLEW_FUN_EXPORT     PFNCLRETAINKERNEL                   __clewRetainKernel                  ;
CLEW_FUN_EXPORT     PFNCLRELEASEKERNEL                  __clewReleaseKernel                 ;
CLEW_FUN_EXPORT     PFNCLSETKERNELARG                   __clewSetKernelArg                  ;
CLEW_FUN_EXPORT     PFNCLGETKERNELINFO                  __clewGetKernelInfo                 ;
CLEW_FUN_EXPORT     PFNCLGETKERNELWORKGROUPINFO         __clewGetKernelWorkGroupInfo        ;
CLEW_FUN_EXPORT     PFNCLWAITFOREVENTS                  __clewWaitForEvents                 ;
CLEW_FUN_EXPORT     PFNCLGETEVENTINFO                   __clewGetEventInfo                  ;
CLEW_FUN_EXPORT     PFNCLRETAINEVENT                    __clewRetainEvent                   ;
CLEW_FUN_EXPORT     PFNCLRELEASEEVENT                   __clewReleaseEvent                  ;
CLEW_FUN_EXPORT     PFNCLGETEVENTPROFILINGINFO          __clewGetEventProfilingInfo         ;
CLEW_FUN_EXPORT     PFNCLFLUSH                          __clewFlush                         ;
CLEW_FUN_EXPORT     PFNCLFINISH                         __clewFinish                        ;
CLEW_FUN_EXPORT     PFNCLENQUEUEREADBUFFER              __clewEnqueueReadBuffer             ;
CLEW_FUN_EXPORT     PFNCLENQUEUEWRITEBUFFER             __clewEnqueueWriteBuffer            ;
CLEW_FUN_EXPORT     PFNCLENQUEUECOPYBUFFER              __clewEnqueueCopyBuffer             ;
CLEW_FUN_EXPORT     PFNCLENQUEUEREADIMAGE               __clewEnqueueReadImage              ;
CLEW_FUN_EXPORT     PFNCLENQUEUEWRITEIMAGE              __clewEnqueueWriteImage             ;
CLEW_FUN_EXPORT     PFNCLENQUEUECOPYIMAGE               __clewEnqueueCopyImage              ;
CLEW_FUN_EXPORT     PFNCLENQUEUECOPYIMAGETOBUFFER       __clewEnqueueCopyImageToBuffer      ;
CLEW_FUN_EXPORT     PFNCLENQUEUECOPYBUFFERTOIMAGE       __clewEnqueueCopyBufferToImage      ;
CLEW_FUN_EXPORT     PFNCLENQUEUEMAPBUFFER               __clewEnqueueMapBuffer              ;
CLEW_FUN_EXPORT     PFNCLENQUEUEMAPIMAGE                __clewEnqueueMapImage               ;
CLEW_FUN_EXPORT     PFNCLENQUEUEUNMAPMEMOBJECT          __clewEnqueueUnmapMemObject         ;
CLEW_FUN_EXPORT     PFNCLENQUEUENDRANGEKERNEL           __clewEnqueueNDRangeKernel          ;
CLEW_FUN_EXPORT     PFNCLENQUEUETASK                    __clewEnqueueTask                   ;
CLEW_FUN_EXPORT     PFNCLENQUEUENATIVEKERNEL            __clewEnqueueNativeKernel           ;
CLEW_FUN_EXPORT     PFNCLENQUEUEMARKER                  __clewEnqueueMarker                 ;
CLEW_FUN_EXPORT     PFNCLENQUEUEWAITFOREVENTS           __clewEnqueueWaitForEvents          ;
CLEW_FUN_EXPORT     PFNCLENQUEUEBARRIER                 __clewEnqueueBarrier                ;
CLEW_FUN_EXPORT     PFNCLGETEXTENSIONFUNCTIONADDRESS    __clewGetExtensionFunctionAddress   ;


                CLEW_GET_FUN(__clewGetPlatformIDs                )
               CLEW_GET_FUN(__clewGetPlatformInfo               )
                  CLEW_GET_FUN(__clewGetDeviceIDs                  )
                 CLEW_GET_FUN(__clewGetDeviceInfo                 )
                 CLEW_GET_FUN(__clewCreateContext                 )
         CLEW_GET_FUN(__clewCreateContextFromType         )
                 CLEW_GET_FUN(__clewRetainContext                 )
                CLEW_GET_FUN(__clewReleaseContext                )
                CLEW_GET_FUN(__clewGetContextInfo                )
            CLEW_GET_FUN(__clewCreateCommandQueue            )
            CLEW_GET_FUN(__clewRetainCommandQueue            )
           CLEW_GET_FUN(__clewReleaseCommandQueue           )
           CLEW_GET_FUN(__clewGetCommandQueueInfo           )
       CLEW_GET_FUN(__clewSetCommandQueueProperty       )
                  CLEW_GET_FUN(__clewCreateBuffer                  )
                 CLEW_GET_FUN(__clewCreateImage2D                 )
                 CLEW_GET_FUN(__clewCreateImage3D                 )
               CLEW_GET_FUN(__clewRetainMemObject               )
              CLEW_GET_FUN(__clewReleaseMemObject              )
      CLEW_GET_FUN(__clewGetSupportedImageFormats      )
              CLEW_GET_FUN(__clewGetMemObjectInfo              )
                  CLEW_GET_FUN(__clewGetImageInfo                  )
                 CLEW_GET_FUN(__clewCreateSampler                 )
                 CLEW_GET_FUN(__clewRetainSampler                 )
                CLEW_GET_FUN(__clewReleaseSampler                )
                CLEW_GET_FUN(__clewGetSamplerInfo                )
       CLEW_GET_FUN(__clewCreateProgramWithSource       )
       CLEW_GET_FUN(__clewCreateProgramWithBinary       )
                 CLEW_GET_FUN(__clewRetainProgram                 )
                CLEW_GET_FUN(__clewReleaseProgram                )
                  CLEW_GET_FUN(__clewBuildProgram                  )
                CLEW_GET_FUN(__clewUnloadCompiler                )
                CLEW_GET_FUN(__clewGetProgramInfo                )
           CLEW_GET_FUN(__clewGetProgramBuildInfo           )
                  CLEW_GET_FUN(__clewCreateKernel                  )
        CLEW_GET_FUN(__clewCreateKernelsInProgram        )
                  CLEW_GET_FUN(__clewRetainKernel                  )
                 CLEW_GET_FUN(__clewReleaseKernel                 )
                  CLEW_GET_FUN(__clewSetKernelArg                  )
                 CLEW_GET_FUN(__clewGetKernelInfo                 )
        CLEW_GET_FUN(__clewGetKernelWorkGroupInfo        )
                 CLEW_GET_FUN(__clewWaitForEvents                 )
                  CLEW_GET_FUN(__clewGetEventInfo                  )
                   CLEW_GET_FUN(__clewRetainEvent                   )
                  CLEW_GET_FUN(__clewReleaseEvent                  )
         CLEW_GET_FUN(__clewGetEventProfilingInfo         )
                         CLEW_GET_FUN(__clewFlush                         )
                        CLEW_GET_FUN(__clewFinish                        )
             CLEW_GET_FUN(__clewEnqueueReadBuffer             )
            CLEW_GET_FUN(__clewEnqueueWriteBuffer            )
             CLEW_GET_FUN(__clewEnqueueCopyBuffer             )
              CLEW_GET_FUN(__clewEnqueueReadImage              )
             CLEW_GET_FUN(__clewEnqueueWriteImage             )
              CLEW_GET_FUN(__clewEnqueueCopyImage              )
      CLEW_GET_FUN(__clewEnqueueCopyImageToBuffer      )
      CLEW_GET_FUN(__clewEnqueueCopyBufferToImage      )
              CLEW_GET_FUN(__clewEnqueueMapBuffer              )
               CLEW_GET_FUN(__clewEnqueueMapImage               )
         CLEW_GET_FUN(__clewEnqueueUnmapMemObject         )
          CLEW_GET_FUN(__clewEnqueueNDRangeKernel          )
                   CLEW_GET_FUN(__clewEnqueueTask                   )
           CLEW_GET_FUN(__clewEnqueueNativeKernel           )
                 CLEW_GET_FUN(__clewEnqueueMarker                 )
          CLEW_GET_FUN(__clewEnqueueWaitForEvents          )
                CLEW_GET_FUN(__clewEnqueueBarrier                )
   CLEW_GET_FUN(__clewGetExtensionFunctionAddress   )



                0       //!<    Success error code
      -1      //!<    Error code for failing to open the dynamic library
    -2      //!<    Error code for failing to queue the closing of the dynamic library to atexit()
    -3      //!<    Error code for failing to import a named function from the dll

//! \brief Load OpenCL dynamic library and set function entry points
CLEW_FUN_EXPORT int         clewInit        (const char*);
//! \brief Convert an OpenCL error code to its string equivalent
CLEW_FUN_EXPORT const char* clewErrorString (cl_int error);

#ifdef __cplusplus
}
#endif


