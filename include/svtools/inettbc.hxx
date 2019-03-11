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

#include <memory>
#include <rtl/ref.hxx>
#include <svtools/svtdllapi.h>
#include <tools/urlobj.hxx>

#include <vcl/combobox.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

class MatchContext_Impl;
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
    std::unique_ptr<SvtURLBox_Impl> pImpl;
    INetProtocol                    eSmartProtocol;
    bool                            bAutoCompleteMode   : 1;
    bool                            bOnlyDirectories    : 1;
    bool                            bHistoryDisabled    : 1;
    bool                            bNoSelection        : 1;
    bool                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE bool             ProcessKey( const vcl::KeyCode& rCode );
    SVT_DLLPRIVATE void             TryAutoComplete();
    SVT_DLLPRIVATE void             UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(     AutoCompleteHdl_Impl, Edit&, void );
    SVT_DLLPRIVATE void             Init(bool bSetDefaultHelpID);

protected:
    virtual bool                    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void                    Select() override;
    virtual bool                    PreNotify( NotifyEvent& rNEvt ) override;

public:
                                    SvtURLBox( vcl::Window* pParent, INetProtocol eSmart = INetProtocol::NotValid, bool bSetDefaultHelpID = true );
                                    SvtURLBox( vcl::Window* pParent, WinBits _nStyle, INetProtocol eSmart = INetProtocol::NotValid, bool bSetDefaultHelpID = true );
                                    virtual ~SvtURLBox() override;
    virtual void                    dispose() override;

    void                            SetBaseURL( const OUString& rURL );
    const OUString&                 GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link<SvtURLBox*,void>& rLink ) { aOpenHdl = rLink; }
    const Link<SvtURLBox*,void>&    GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( bool bDir );
    void                            SetNoURLSelection( bool bSet );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    OUString                        GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static OUString                 ParseSmart( const OUString& aText, const OUString& aBaseURL );

    void                            SetFilter(const OUString& _sFilter);

    void                            EnableAutocompletion( bool _bEnable )
                                        { bIsAutoCompleteEnabled = _bEnable; }
    void                            SetPlaceHolder( const OUString& sPlaceHolder )
                                        { aPlaceHolder = sPlaceHolder; }
    const OUString&                 GetPlaceHolder() { return aPlaceHolder; }
    bool                            MatchesPlaceHolder( const OUString& sToMatch ) const
                                        { return ( !aPlaceHolder.isEmpty() ) && ( aPlaceHolder == sToMatch ); }
};

class SVT_DLLPUBLIC URLBox
{
    friend class MatchContext_Impl;
    friend class SvtURLBox_Impl;

    Idle                            aChangedIdle;
    OUString                        aBaseURL;
    rtl::Reference< MatchContext_Impl > pCtx;
    std::unique_ptr<SvtURLBox_Impl> pImpl;
    INetProtocol                    eSmartProtocol;
    bool                            bHistoryDisabled    : 1;

    Link<weld::ComboBox&, void>     aChangeHdl;

    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_DLLPRIVATE_LINK(           TryAutoComplete, Timer*, void);
    SVT_DLLPRIVATE void             UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(           ChangedHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(           FocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(           FocusOutHdl, weld::Widget&, void);
    SVT_DLLPRIVATE void             Init();

public:
    URLBox(std::unique_ptr<weld::ComboBox> pWidget);
    ~URLBox();

    void                            set_entry_text(const OUString& rStr) { m_xWidget->set_entry_text(rStr); }
    void                            Clear() { m_xWidget->clear(); }
    void connect_entry_activate(const Link<weld::ComboBox&, bool>& rLink) { m_xWidget->connect_entry_activate(rLink); }
    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { aChangeHdl = rLink; }
    void                            append_text(const OUString& rStr) { m_xWidget->append_text(rStr); }
    OUString                        get_active_text() const { return m_xWidget->get_active_text(); }
    void                            grab_focus() { m_xWidget->grab_focus(); }
    void                            EnableAutocomplete() { m_xWidget->set_entry_completion(true); }

    void                            SetBaseURL( const OUString& rURL );
    void                            SetSmartProtocol( INetProtocol eProt );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    OUString                        GetURL();
    void                            DisableHistory();

    weld::Widget*                   getWidget() { return m_xWidget.get(); }

    static OUString                 ParseSmart( const OUString& aText, const OUString& aBaseURL );

    void                            SetFilter(const OUString& _sFilter);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
