/* sane - Scanner Access Now Easy.
   Copyright (C) 1997 David Mosberger-Tang and Andreas Beck
   This file is part of the SANE package.

   This file is in the public domain.  You may use and modify it as
   you see fit, as long as this copyright message is included and
   that there is an indication as to what modifications have been
   made (if any).

   SANE is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   This file declares SANE application interface.  See the SANE
   standard for a detailed explanation of the interface.  */
#pragma once


  0

(major, minor, build)  \
  (  (((SANE_Word) (major) &   0xff) << 24) \
   | (((SANE_Word) (minor) &   0xff) << 16) \
   | (((SANE_Word) (build) & 0xffff) <<  0))

(code)    ((((SANE_Word)(code)) >> 24) &   0xff)
(code)    ((((SANE_Word)(code)) >> 16) &   0xff)
(code)    ((((SANE_Word)(code)) >>  0) & 0xffff)

  0
   1

typedef unsigned char SANE_Byte;
typedef int SANE_Word;
typedef SANE_Word SANE_Bool;
typedef SANE_Word SANE_Int;
typedef char SANE_Char;
typedef SANE_Char *SANE_String;
typedef const SANE_Char *SANE_String_Const;
typedef void *SANE_Handle;
typedef SANE_Word SANE_Fixed;

  16
(v) ((SANE_Word) ((v) * (1 << SANE_FIXED_SCALE_SHIFT)))
(v)   ((double)(v) / (1 << SANE_FIXED_SCALE_SHIFT))

typedef enum
  {
    SANE_STATUS_GOOD = 0,   /* everything A-OK */
    SANE_STATUS_UNSUPPORTED,    /* operation is not supported */
    SANE_STATUS_CANCELLED,  /* operation was cancelled */
    SANE_STATUS_DEVICE_BUSY,    /* device is busy; try again later */
    SANE_STATUS_INVAL,      /* data is invalid (includes no dev at open) */
    SANE_STATUS_EOF,        /* no more data available (end-of-file) */
    SANE_STATUS_JAMMED,     /* document feeder jammed */
    SANE_STATUS_NO_DOCS,    /* document feeder out of documents */
    SANE_STATUS_COVER_OPEN, /* scanner cover is open */
    SANE_STATUS_IO_ERROR,   /* error during device I/O */
    SANE_STATUS_NO_MEM,     /* out of memory */
    SANE_STATUS_ACCESS_DENIED   /* access to resource has been denied */
  }
SANE_Status;

typedef enum
  {
    SANE_TYPE_BOOL = 0,
    SANE_TYPE_INT,
    SANE_TYPE_FIXED,
    SANE_TYPE_STRING,
    SANE_TYPE_BUTTON,
    SANE_TYPE_GROUP
  }
SANE_Value_Type;

typedef enum
  {
    SANE_UNIT_NONE = 0,     /* the value is unit-less (e.g., # of scans) */
    SANE_UNIT_PIXEL,        /* value is number of pixels */
    SANE_UNIT_BIT,      /* value is number of bits */
    SANE_UNIT_MM,       /* value is millimeters */
    SANE_UNIT_DPI,      /* value is resolution in dots/inch */
    SANE_UNIT_PERCENT,      /* value is a percentage */
    SANE_UNIT_MICROSECOND   /* value is micro seconds */
  }
SANE_Unit;

typedef struct
  {
    SANE_String_Const name; /* unique device name */
    SANE_String_Const vendor;   /* device vendor string */
    SANE_String_Const model;    /* device model name */
    SANE_String_Const type; /* device type (e.g., "flatbed scanner") */
  }
SANE_Device;

        (1 << 0)
        (1 << 1)
        (1 << 2)
       (1 << 3)
      (1 << 4)
       (1 << 5)
       (1 << 6)
    (1 << 7)

(cap)  (((cap) & SANE_CAP_INACTIVE) == 0)
(cap)    (((cap) & SANE_CAP_SOFT_SELECT) != 0)

       (1 << 0)
    (1 << 1)
     (1 << 2)

typedef enum
  {
    SANE_CONSTRAINT_NONE = 0,
    SANE_CONSTRAINT_RANGE,
    SANE_CONSTRAINT_WORD_LIST,
    SANE_CONSTRAINT_STRING_LIST
  }
SANE_Constraint_Type;

typedef struct
  {
    SANE_Word min;      /* minimum (element) value */
    SANE_Word max;      /* maximum (element) value */
    SANE_Word quant;        /* quantization value (0 if none) */
  }
SANE_Range;

typedef struct
  {
    SANE_String_Const name; /* name of this option (command-line name) */
    SANE_String_Const title;    /* title of this option (single-line) */
    SANE_String_Const desc; /* description of this option (multi-line) */
    SANE_Value_Type type;   /* how are values interpreted? */
    SANE_Unit unit;     /* what is the (physical) unit? */
    SANE_Int size;
    SANE_Int cap;       /* capabilities */

    SANE_Constraint_Type constraint_type;
    union
      {
    const SANE_String_Const *string_list;   /* NULL-terminated list */
    const SANE_Word *word_list; /* first element is list-length */
    const SANE_Range *range;
      }
    constraint;
  }
SANE_Option_Descriptor;

typedef enum
  {
    SANE_ACTION_GET_VALUE = 0,
    SANE_ACTION_SET_VALUE,
    SANE_ACTION_SET_AUTO
  }
SANE_Action;

typedef enum
  {
    SANE_FRAME_GRAY,        /* band covering human visual range */
    SANE_FRAME_RGB,     /* pixel-interleaved red/green/blue bands */
    SANE_FRAME_RED,     /* red band only */
    SANE_FRAME_GREEN,       /* green band only */
    SANE_FRAME_BLUE     /* blue band only */
  }
SANE_Frame;

typedef struct
  {
    SANE_Frame format;
    SANE_Bool last_frame;
    SANE_Int bytes_per_line;
    SANE_Int pixels_per_line;
    SANE_Int lines;
    SANE_Int depth;
  }
SANE_Parameters;

struct SANE_Auth_Data;

   256
   256

typedef void (*SANE_Auth_Callback) (SANE_String_Const resource,
                    SANE_Char username[SANE_MAX_USERNAME_LEN],
                    SANE_Char password[SANE_MAX_PASSWORD_LEN]);

extern SANE_Status sane_init (SANE_Int * version_code,
                  SANE_Auth_Callback authorize);
extern void sane_exit (void);
extern SANE_Status sane_get_devices (const SANE_Device *** device_list,
                     SANE_Bool local_only);
extern SANE_Status sane_open (SANE_String_Const devicename,
                  SANE_Handle * handle);
extern void sane_close (SANE_Handle handle);
extern const SANE_Option_Descriptor *
  sane_get_option_descriptor (SANE_Handle handle, SANE_Int option);
extern SANE_Status sane_control_option (SANE_Handle handle, SANE_Int option,
                    SANE_Action action, void *value,
                    SANE_Int * info);
extern SANE_Status sane_get_parameters (SANE_Handle handle,
                    SANE_Parameters * params);
extern SANE_Status sane_start (SANE_Handle handle);
extern SANE_Status sane_read (SANE_Handle handle, SANE_Byte * data,
                  SANE_Int max_length, SANE_Int * length);
extern void sane_cancel (SANE_Handle handle);
extern SANE_Status sane_set_io_mode (SANE_Handle handle,
                     SANE_Bool non_blocking);
extern SANE_Status sane_get_select_fd (SANE_Handle handle,
                       SANE_Int * fd);
extern SANE_String_Const sane_strstatus (SANE_Status status);


