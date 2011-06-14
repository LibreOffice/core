/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#define _SAL_I18N_INPUTCONTEXT_HXX

#include <i18npool/lang.h>
#include "i18n_cb.hxx"

class SalI18N_InputContext
{

private:

    Bool    mbUseable; // system supports current locale ?
    Bool    mbMultiLingual; // system supports iiimp ?
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

    Bool         SupportInputMethodStyle( XIMStyles *pIMStyles );
    unsigned int GetWeightingOfIMStyle(   XIMStyle n_style ) const ;
    Bool         IsSupportedIMStyle(      XIMStyle n_style ) const ;

public:

    Bool UseContext()       { return mbUseable; }
    Bool IsMultiLingual()   { return mbMultiLingual; }
    Bool IsPreeditMode()    { return maClientData.eState == ePreeditStatusActive; }
    XIC  GetContext()       { return maContext; }

    void ExtendEventMask(  XLIB_Window aFocusWindow );
    void SetICFocus( SalFrame* pFocusFrame );
    void UnsetICFocus( SalFrame* pFrame );
    void HandleDestroyIM();

    int  HandleKeyEvent( XKeyEvent *pEvent, SalFrame *pFrame ); // unused
    void EndExtTextInput( sal_uInt16 nFlags );                      // unused
    int  CommitStringCallback( sal_Unicode* pText, sal_Size nLength );
    int  CommitKeyEvent( sal_Unicode* pText, sal_Size nLength );
    int  UpdateSpotLocation();

    void Map( SalFrame *pFrame );
    void Unmap( SalFrame* pFrame );

    void SetPreeditState(Bool aPreeditState);
    void SetLanguage(LanguageType aInputLanguage);

    SalI18N_InputContext( SalFrame *aFrame );
    ~SalI18N_InputContext();

private:

    SalI18N_InputContext(); // do not use this

};

#endif // _SAL_I18N_INPUTCONTEXT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
