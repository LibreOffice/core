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
#ifndef INCLUDED_VCL_INC_UNX_I18N_CB_HXX
#define INCLUDED_VCL_INC_UNX_I18N_CB_HXX

#include <salwtype.hxx>
#include <vector>

extern "C" {

// xim callbacks
void PreeditDoneCallback ( XIC ic, XPointer client_data, XPointer call_data);
int  PreeditStartCallback( XIC ic, XPointer client_data, XPointer call_data);
void PreeditDrawCallback ( XIC ic, XPointer client_data,
                           XIMPreeditDrawCallbackStruct *call_data );
void PreeditCaretCallback( XIC ic, XPointer client_data,
                           XIMPreeditCaretCallbackStruct *call_data );
void GetPreeditSpotLocation(XIC ic, XPointer client_data);

void StatusStartCallback (XIC ic, XPointer client_data, XPointer call_data);
void StatusDoneCallback  (XIC ic, XPointer client_data, XPointer call_data);
void StatusDrawCallback  (XIC ic, XPointer client_data,
            XIMStatusDrawCallbackStruct *call_data);

// keep informed if kinput2 crashed again
void IC_IMDestroyCallback (XIM im, XPointer client_data, XPointer call_data);
void IM_IMDestroyCallback (XIM im, XPointer client_data, XPointer call_data);

Bool IsControlCode(sal_Unicode nChar);

} /* extern "C" */

typedef struct {
  sal_Unicode   *pUnicodeBuffer;
  XIMFeedback   *pCharStyle;
  unsigned int   nCursorPos;
  unsigned int   nLength;
  unsigned int   nSize;
} preedit_text_t;

class SalFrame;

enum class PreeditStatus {
    DontKnow = 0,
    Active,
    ActivationRequired,
    StartPending
};

struct preedit_data_t {
    SalFrame*               pFrame;
    PreeditStatus           eState;
    preedit_text_t          aText;
    SalExtTextInputEvent    aInputEv;
    std::vector< ExtTextInputAttr >   aInputFlags;

    preedit_data_t() : eState(PreeditStatus::DontKnow) {}
};

#endif // INCLUDED_VCL_INC_UNX_I18N_CB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
