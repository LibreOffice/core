/*
 * $XFree86: xc/include/extensions/randrproto.h,v 1.6 2002/09/29 23:39:43 keithp Exp $
 *
 * Copyright © 2000 Compaq Computer Corporation
 * Copyright © 2002 Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq or HP not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  Compaq and HP makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL COMPAQ
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, Hewlett-Packard Company, Inc.
 */

/* note that RANDR 1.0 is incompatible with version 0.0, or 0.1 */
/* V1.0 removes depth switching from the protocol */
#ifndef _XRANDRP_H_
#define _XRANDRP_H_

#include <X11/extensions/randr.h>

#define Window CARD32
#define Drawable CARD32
#define Font CARD32
#define Pixmap CARD32
#define Cursor CARD32
#define Colormap CARD32
#define GContext CARD32
#define Atom CARD32
#define Time CARD32
#define KeyCode CARD8
#define KeySym CARD32

#define Rotation CARD16
#define SizeID CARD16
#define SubpixelOrder CARD16

/*
 * data structures
 */

typedef struct {
    CARD16 widthInPixels B16;
    CARD16 heightInPixels B16;
    CARD16 widthInMillimeters B16;
    CARD16 heightInMillimeters B16;
} xScreenSizes;
#define sz_xScreenSizes 8

/* 
 * requests and replies
 */

typedef struct {
    CARD8   reqType;
    CARD8   randrReqType;
    CARD16  length B16;
    CARD32  majorVersion B32;
    CARD32  minorVersion B32;
} xRRQueryVersionReq;
#define sz_xRRQueryVersionReq   12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  majorVersion B32;
    CARD32  minorVersion B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
} xRRQueryVersionReply;
#define sz_xRRQueryVersionReply	32

typedef struct {
    CARD8   reqType;
    CARD8   randrReqType;
    CARD16  length B16;
    Window  window B32;
} xRRGetScreenInfoReq;
#define sz_xRRGetScreenInfoReq   8

/* 
 * the xRRScreenInfoReply structure is followed by:
 *
 * the size information
 */


typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    setOfRotations;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    Window  root B32;
    Time    timestamp B32;
    Time    configTimestamp B32;
    CARD16  nSizes B16;
    SizeID  sizeID B16;
    Rotation  rotation B16;
    CARD16  rate B16;
    CARD16  nrateEnts B16;
    CARD16  pad B16;
} xRRGetScreenInfoReply;
#define sz_xRRGetScreenInfoReply	32

typedef struct {
    CARD8    reqType;
    CARD8    randrReqType;
    CARD16   length B16;
    Drawable drawable B32;
    Time     timestamp B32;
    Time     configTimestamp B32;
    SizeID   sizeID B16;
    Rotation rotation B16;
} xRR1_0SetScreenConfigReq;
#define sz_xRR1_0SetScreenConfigReq   20

typedef struct {
    CARD8    reqType;
    CARD8    randrReqType;
    CARD16   length B16;
    Drawable drawable B32;
    Time     timestamp B32;
    Time     configTimestamp B32;
    SizeID   sizeID B16;
    Rotation rotation B16;
    CARD16   rate B16;
    CARD16   pad B16;
} xRRSetScreenConfigReq;
#define sz_xRRSetScreenConfigReq   24

typedef struct {
    BYTE    type;   /* X_Reply */
    CARD8   status;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    Time    newTimestamp B32;  
    Time    newConfigTimestamp B32;
    Window  root;
    CARD16  subpixelOrder B16;
    CARD16  pad4 B16;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
} xRRSetScreenConfigReply;
#define sz_xRRSetScreenConfigReply 32

typedef struct {
    CARD8   reqType;
    CARD8   randrReqType;
    CARD16  length B16;
    Window  window B32;
    CARD16  enable B16;
    CARD16  pad2 B16;
} xRRSelectInputReq;
#define sz_xRRSelectInputReq   12

#define sz_xRRSelectInputReq   12

/*
 * event
 */
typedef struct {
    CARD8 type;				/* always evBase + ScreenChangeNotify */
    CARD8 rotation;			/* new rotation */
    CARD16 sequenceNumber B16;
    Time timestamp B32;			/* time screen was changed */
    Time configTimestamp B32;		/* time config data was changed */
    Window root B32;			/* root window */
    Window window B32;			/* window requesting notification */
    SizeID sizeID B16;			/* new size ID */
    CARD16 subpixelOrder B16;		/* subpixel order */
    CARD16 widthInPixels B16;		/* new size */
    CARD16 heightInPixels B16;
    CARD16 widthInMillimeters B16;
    CARD16 heightInMillimeters B16;
} xRRScreenChangeNotifyEvent;
#define sz_xRRScreenChangeNotifyEvent	32

#undef Window
#undef Drawable
#undef Font
#undef Pixmap
#undef Cursor
#undef Colormap
#undef GContext
#undef Atom
#undef Time
#undef KeyCode
#undef KeySym
#undef Rotation
#undef SizeID
#undef SubpixelOrder

#endif /* _XRANDRP_H_ */
