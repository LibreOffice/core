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

#pragma once

#include "i18n_cb.hxx"

#include <unx/salframe.h>

enum class EndExtTextInputFlags;

class SalI18N_InputContext
{

private:

    Bool    mbUseable; // system supports current locale ?
    XIC     maContext;

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

    bool         SupportInputMethodStyle( XIMStyles const *pIMStyles );
    static unsigned int GetWeightingOfIMStyle(   XIMStyle n_style );
    bool         IsSupportedIMStyle(      XIMStyle n_style ) const;

public:

    Bool UseContext() const { return mbUseable; }
    bool IsPreeditMode() const { return maClientData.eState == PreeditStatus::Active; }
    XIC  GetContext() const { return maContext; }

    void ExtendEventMask(  ::Window aFocusWindow );
    void SetICFocus(X11SalFrame* pFocusFrame);
    void UnsetICFocus();
    void HandleDestroyIM();

    void EndExtTextInput();
    void CommitKeyEvent( sal_Unicode const * pText, std::size_t nLength );
    int  UpdateSpotLocation();

    void Map(X11SalFrame* pFrame);
    void Unmap();

    SalI18N_InputContext( SalFrame *aFrame );
    ~SalI18N_InputContext();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
