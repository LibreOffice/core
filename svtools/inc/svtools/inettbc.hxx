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

#ifndef _SVTOOLS_INETTBC_HXX
#define _SVTOOLS_INETTBC_HXX

#include "rtl/ref.hxx"
#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

#include <vcl/combobox.hxx>

class IUrlFilter;
class SvtMatchContext_Impl;
class SvtURLBox_Impl;
class SVT_DLLPUBLIC SvtURLBox : public ComboBox
{
friend class SvtMatchContext_Impl;
friend class SvtURLBox_Impl;
    Link                            aOpenHdl;
    String                          aBaseURL;
    String                          aPlaceHolder;
    rtl::Reference< SvtMatchContext_Impl > pCtx;
    SvtURLBox_Impl*                 pImp;
    INetProtocol                    eSmartProtocol;
    sal_Bool                            bAutoCompleteMode   : 1;
    sal_Bool                            bOnlyDirectories    : 1;
    sal_Bool                            bModified           : 1;
    sal_Bool                            bTryAutoComplete    : 1;
    sal_Bool                            bCtrlClick          : 1;
    sal_Bool                            bHistoryDisabled    : 1;
    sal_Bool                            bNoSelection        : 1;
    sal_Bool                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE sal_Bool                         ProcessKey( const KeyCode& rCode );
    SVT_DLLPRIVATE void                         TryAutoComplete( sal_Bool bForce );
    SVT_DLLPRIVATE void                            UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(                      AutoCompleteHdl_Impl, void* );
    using Window::ImplInit;
    SVT_DLLPRIVATE void                            ImplInit();

protected:
    virtual long                    Notify( NotifyEvent& rNEvt );
    virtual void                    Select();
    virtual void                    Modify();
    virtual long                    PreNotify( NotifyEvent& rNEvt );

public:
                                    SvtURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    SvtURLBox( Window* pParent, WinBits _nStyle, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    SvtURLBox( Window* pParent, const ResId& _rResId, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    ~SvtURLBox();

    void                            SetBaseURL( const String& rURL );
    const String&                   GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link& rLink ) { aOpenHdl = rLink; }
    const Link&                     GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( sal_Bool bDir = sal_True );
    void                            SetNoURLSelection( sal_Bool bSet = sal_True );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    sal_Bool                            IsCtrlOpen()
                                        { return bCtrlClick; }
    String                          GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static String                   ParseSmart( String aText, String aBaseURL, String aWorkDir );

    void                            SetFilter(const String& _sFilter);
    void                            SetUrlFilter( const IUrlFilter* _pFilter );

    inline void                     EnableAutocompletion( sal_Bool _bEnable = sal_True )
                                        { bIsAutoCompleteEnabled = _bEnable; }
    void SetPlaceHolder( const String& sPlaceHolder ) { aPlaceHolder = sPlaceHolder; }
    String GetPlaceHolder() { return aPlaceHolder; }
    bool MatchesPlaceHolder( const String& sToMatch ) { return ( ( aPlaceHolder.Len() > 0 ) && ( aPlaceHolder == sToMatch ) ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
