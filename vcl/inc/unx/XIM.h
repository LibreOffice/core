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
#ifndef INCLUDED_VCL_INC_UNX_XIM_H
#define INCLUDED_VCL_INC_UNX_XIM_H

#include <X11/Xlib.h>

#ifndef XIMCallback1
typedef int (*XIMProc1)(XIC, XPointer, XPointer);
typedef struct {
  XPointer client_data;
  XIMProc1 callback;
} XIMCallback1;
#endif

typedef struct {
  int start_position;
  int end_position;
  XPointer data;
} XIMAnnotation;

/*
  XIMUText: XIMText extension for UTF16
 */
typedef struct {
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

typedef struct {
  int       choice_per_window;  /* Number of choices can be display
                     * in the region
                     */
  int       nrows;
  int       ncolumns;
  XIMDrawUpDirection draw_up_direction;
} XIMLookupStartCallbackStruct;

typedef struct {
  XIMUnicodeText *label;
  XIMUnicodeText *value;
} XIMUnicodeChoiceObject;

typedef struct {
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

typedef struct {
  XIMUnicodeCharacterSubsetID   index;
  XIMUnicodeCharacterSubsetID   subset_id;
  char          *name;
  Bool          is_active;
} XIMUnicodeCharacterSubset;

typedef struct {
  unsigned short count_subsets;
  XIMUnicodeCharacterSubset *supported_subsets;
} XIMUnicodeCharacterSubsets;

typedef struct {
  XIMUnicodeCharacterSubset *from;
  XIMUnicodeCharacterSubset *to;
} XIMSwitchIMNotifyCallbackStruct;

/* XIC attributes for multilingual IM extension */

#define XNUnicodeCharacterSubset "UnicodeCharacterSubset"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
