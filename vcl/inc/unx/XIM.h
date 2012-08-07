/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XIM_h
#define _XIM_h

#include <X11/Xlib.h>
#include <X11/Xresource.h>

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

/* XIC attributes for multilingual IM extension */

#define XNUnicodeCharacterSubset "UnicodeChararcterSubset"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
