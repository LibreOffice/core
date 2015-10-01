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

#ifndef INCLUDED_VCL_INC_UNX_I18N_IC_HXX
#define INCLUDED_VCL_INC_UNX_I18N_IC_HXX

#include <i18nlangtag/lang.h>
#include "i18n_cb.hxx"

enum class EndExtTextInputFlags;

class SalI18N_InputContext
{

private:

    Bool    mbUseable; // system supports current locale ?
    XIC     maContext;

    XIMStyle mnSupportedStatusStyle;
    XIMStyle mnSupportedPreeditStyle;
    XIMStyle mnStatusStyle;
    XIMStyle mnPreeditStyle;

    preedit_data_t maClientData;
    XIMCallback maPreeditStartCallback;
    XIMCallback maPreeditDoneCallback;
    XIMCallback maPreeditDrawCallback;
    XIMCallback maPreeditCaretCallback;
    XIMCallback maCommitStringCallback;
    XIMCallback maSwitchIMCallback;
    XIMCallback maDestroyCallback;

    XVaNestedList mpAttributes;
    XVaNestedList mpStatusAttributes;
    XVaNestedList mpPreeditAttributes;

    bool         SupportInputMethodStyle( XIMStyles *pIMStyles );
    static unsigned int GetWeightingOfIMStyle(   XIMStyle n_style );
    Bool         IsSupportedIMStyle(      XIMStyle n_style ) const;

public:

    Bool UseContext()       { return mbUseable; }
    bool IsPreeditMode()    { return maClientData.eState == ePreeditStatusActive; }
    XIC  GetContext()       { return maContext; }

    void ExtendEventMask(  ::Window aFocusWindow );
    void SetICFocus( SalFrame* pFocusFrame );
    void UnsetICFocus( SalFrame* pFrame );
    void HandleDestroyIM();

    void EndExtTextInput( EndExtTextInputFlags nFlags );        // unused
    int  CommitKeyEvent( sal_Unicode* pText, sal_Size nLength );
    int  UpdateSpotLocation();

    void Map( SalFrame *pFrame );
    void Unmap( SalFrame* pFrame );

    SalI18N_InputContext( SalFrame *aFrame );
    ~SalI18N_InputContext();

private:

    SalI18N_InputContext(); // do not use this

};

#endif // INCLUDED_VCL_INC_UNX_I18N_IC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
