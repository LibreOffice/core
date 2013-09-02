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

#ifndef _SVTOOLS_INETTBC_HXX
#define _SVTOOLS_INETTBC_HXX

#include "rtl/ref.hxx"
#include "svtools/svtdllapi.h"
#include <tools/urlobj.hxx>

#include <vcl/combobox.hxx>

class SvtMatchContext_Impl;
class SvtURLBox_Impl;
class SVT_DLLPUBLIC SvtURLBox : public ComboBox
{
friend class SvtMatchContext_Impl;
friend class SvtURLBox_Impl;
    Link                            aOpenHdl;
    OUString                        aBaseURL;
    OUString                        aPlaceHolder;
    rtl::Reference< SvtMatchContext_Impl > pCtx;
    SvtURLBox_Impl*                 pImp;
    INetProtocol                    eSmartProtocol;
    sal_Bool                            bAutoCompleteMode   : 1;
    sal_Bool                            bOnlyDirectories    : 1;
    sal_Bool                            bCtrlClick          : 1;
    sal_Bool                            bHistoryDisabled    : 1;
    sal_Bool                            bNoSelection        : 1;
    sal_Bool                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE sal_Bool                         ProcessKey( const KeyCode& rCode );
    SVT_DLLPRIVATE void                         TryAutoComplete();
    SVT_DLLPRIVATE void                            UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(                      AutoCompleteHdl_Impl, void* );
    SVT_DLLPRIVATE void             Init(bool bSetDefaultHelpID);

protected:
    virtual long                    Notify( NotifyEvent& rNEvt );
    virtual void                    Select();
    virtual void                    Modify();
    virtual long                    PreNotify( NotifyEvent& rNEvt );

public:
                                    SvtURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_NOT_VALID, bool bSetDefaultHelpID = true );
                                    SvtURLBox( Window* pParent, WinBits _nStyle, INetProtocol eSmart = INET_PROT_NOT_VALID, bool bSetDefaultHelpID = true );
                                    SvtURLBox( Window* pParent, const ResId& _rResId, INetProtocol eSmart = INET_PROT_NOT_VALID, bool bSetDefaultHelpID = true );
                                    ~SvtURLBox();

    void                            SetBaseURL( const OUString& rURL );
    const OUString&                 GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link& rLink ) { aOpenHdl = rLink; }
    const Link&                     GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( sal_Bool bDir = sal_True );
    void                            SetNoURLSelection( sal_Bool bSet = sal_True );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    sal_Bool                            IsCtrlOpen()
                                        { return bCtrlClick; }
    OUString                        GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static OUString                 ParseSmart( OUString aText, OUString aBaseURL, const OUString& aWorkDir );

    void                            SetFilter(const OUString& _sFilter);

    inline void                     EnableAutocompletion( sal_Bool _bEnable = sal_True )
                                        { bIsAutoCompleteEnabled = _bEnable; }
    void                            SetPlaceHolder( const OUString& sPlaceHolder )
                                        { aPlaceHolder = sPlaceHolder; }
    OUString                        GetPlaceHolder() { return aPlaceHolder; }
    bool                            MatchesPlaceHolder( const OUString& sToMatch ) const
                                        { return ( !aPlaceHolder.isEmpty() ) && ( aPlaceHolder == sToMatch ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
