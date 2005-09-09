/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XIM.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:38:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _XIM_h
#define _XIM_h

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#ifdef __cplusplus
extern "C"
#endif
XIM XvaOpenIM( Display*, XrmDatabase, char*, char*, ... );

extern Status XCloseIM(XIM);

#ifndef XIMCallback1
typedef int (*XIMProc1)(XIC, XPointer, XPointer);
typedef struct {
  XPointer client_data;
  XIMProc1 callback;
} XIMCallback1;
#endif

typedef struct _XIMAnnotation {
  int start_position;
  int end_position;
  XPointer data;
} XIMAnnotation;

/*
  XIMUText: XIMText extention for UTF16
 */
typedef struct _XIMUnicodeText {
  unsigned short length;
  XIMFeedback *feedback;
  Bool encoding_is_wchar;
  union {
    char *multi_byte;
    wchar_t *wide_char;
    unsigned short *utf16_char;
  } string;
  unsigned int   count_annotations;
  XIMAnnotation  *annotations;
} XIMUnicodeText;

/* lookup choice */
typedef enum {
  XIMDrawUpHorizontally = 0 ,
  XIMDrawUpVertically = 1
} XIMDrawUpDirection ;

typedef struct _XIMLookupStartCallbackStruct {
  int       choice_per_window;  /* Number of choices can be display
                     * in the region
                     */
  int       nrows;
  int       ncolumns;
  XIMDrawUpDirection draw_up_direction;
} XIMLookupStartCallbackStruct;

typedef struct _XIMUnicodeChoiceObject {
  XIMUnicodeText *label;
  XIMUnicodeText *value;
} XIMUnicodeChoiceObject;

typedef struct _XIMLookupDrawCallbackStruct {
  XIMUnicodeChoiceObject *choices;       /* the lookup choices */
  int              n_choices;      /* Total number of lookup choices */
  int              first_index;
  int              last_index;
  int              current_index;
  XIMUnicodeText   *title;
} XIMLookupDrawCallbackStruct;

/* Unicode Subset */
typedef enum {
  XIMKatakana, XIMHanzi
} XIMUnicodeCharacterSubsetID;

typedef struct  _XIMUncodeSubset {
  XIMUnicodeCharacterSubsetID   index;
  XIMUnicodeCharacterSubsetID   subset_id;
  char          *name;
  Bool          is_active;
} XIMUnicodeCharacterSubset;

typedef struct  _XIMUncodeSubsets {
  unsigned short count_subsets;
  XIMUnicodeCharacterSubset *supported_subsets;
} XIMUnicodeCharacterSubsets;

typedef struct  _XIMSwitchIMNotifyCallbackStruct {
  XIMUnicodeCharacterSubset *from;
  XIMUnicodeCharacterSubset *to;
} XIMSwitchIMNotifyCallbackStruct;

/* XIM attributes for multilingual IM extension */
#define XNMultiLingualInput "multiLingualInput"
#define XNQueryUnicodeCharacterSubset "unicodeCharacterSubset"

/* XIC attributes for multilingual IM extension */

#define XNUnicodeCharacterSubset "UnicodeChararcterSubset"

#define XNSwitchIMNotifyCallback "switchIMNotifyCallback"
#define XNCommitStringCallback "commitStringCallback"
#define XNForwardEventCallback "forwardEventCallback"

#define XNLookupStartCallback "lookupStartCallback"
#define XNLookupDrawCallback "lookupDrawCallback"
#define XNLookupDoneCallback "lookupDoneCallback"

#endif
