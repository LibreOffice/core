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

#ifndef INCLUDED_SVTOOLS_INETTBC_HXX
#define INCLUDED_SVTOOLS_INETTBC_HXX

#include <rtl/ref.hxx>
#include <svtools/svtdllapi.h>
#include <tools/urlobj.hxx>

#include <vcl/combobox.hxx>

class SvtMatchContext_Impl;
class SvtURLBox_Impl;
class SVT_DLLPUBLIC SvtURLBox : public ComboBox
{
friend class SvtMatchContext_Impl;
friend class SvtURLBox_Impl;
    Link<SvtURLBox*,void>           aOpenHdl;
    OUString                        aBaseURL;
    OUString                        aPlaceHolder;
    rtl::Reference< SvtMatchContext_Impl > pCtx;
    SvtURLBox_Impl*                 pImp;
    INetProtocol                    eSmartProtocol;
    bool                            bAutoCompleteMode   : 1;
    bool                            bOnlyDirectories    : 1;
    bool                            bCtrlClick          : 1;
    bool                            bHistoryDisabled    : 1;
    bool                            bNoSelection        : 1;
    bool                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE bool             ProcessKey( const vcl::KeyCode& rCode );
    SVT_DLLPRIVATE void             TryAutoComplete();
    SVT_DLLPRIVATE void             UpdatePicklistForSmartProtocol_Impl();
    SVT_DLLPRIVATE void             Init(bool bSetDefaultHelpID);

    SAL_DLLPRIVATE void AutoCompleteHandler( Edit* );

protected:
    virtual bool                    Notify( NotifyEvent& rNEvt ) override;
    virtual void                    Select() override;
    virtual void                    Modify() override;
    virtual bool                    PreNotify( NotifyEvent& rNEvt ) override;

public:
                                    SvtURLBox( vcl::Window* pParent, INetProtocol eSmart = INetProtocol::NotValid, bool bSetDefaultHelpID = true );
                                    SvtURLBox( vcl::Window* pParent, WinBits _nStyle, INetProtocol eSmart = INetProtocol::NotValid, bool bSetDefaultHelpID = true );
                                    SvtURLBox( vcl::Window* pParent, const ResId& _rResId, INetProtocol eSmart = INetProtocol::NotValid, bool bSetDefaultHelpID = true );
                                    virtual ~SvtURLBox();
    virtual void                    dispose() override;

    void                            SetBaseURL( const OUString& rURL );
    const OUString&                 GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link<SvtURLBox*,void>& rLink ) { aOpenHdl = rLink; }
    const Link<SvtURLBox*,void>&    GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( bool bDir = true );
    void                            SetNoURLSelection( bool bSet = true );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    bool                            IsCtrlOpen()
                                        { return bCtrlClick; }
    OUString                        GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static OUString                 ParseSmart( const OUString& aText, const OUString& aBaseURL );

    void                            SetFilter(const OUString& _sFilter);

    inline void                     EnableAutocompletion( bool _bEnable = true )
                                        { bIsAutoCompleteEnabled = _bEnable; }
    void                            SetPlaceHolder( const OUString& sPlaceHolder )
                                        { aPlaceHolder = sPlaceHolder; }
    OUString                        GetPlaceHolder() { return aPlaceHolder; }
    bool                            MatchesPlaceHolder( const OUString& sToMatch ) const
                                        { return ( !aPlaceHolder.isEmpty() ) && ( aPlaceHolder == sToMatch ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
