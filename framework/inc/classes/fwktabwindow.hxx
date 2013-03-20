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
#if 1

#include <general.h>

#include <vector>

#include <com/sun/star/uno/Reference.h>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/button.hxx>

namespace com { namespace sun { namespace star {
    namespace awt {
        class XWindow;
        class XContainerWindowProvider;
        class XContainerWindowEventHandler; }
    namespace beans {
        struct NamedValue; }
} } }

namespace framework
{

class FwkTabControl : public TabControl
{
public:
    FwkTabControl( Window* pParent, const ResId& rResId );

    void    BroadcastEvent( sal_uLong nEvent );
};

class FwkTabPage : public TabPage
{
private:
    rtl::OUString                                                   m_sPageURL;
    rtl::OUString                                                   m_sEventHdl;
    css::uno::Reference< css::awt::XWindow >                        m_xPage;
    css::uno::Reference< css::awt::XContainerWindowEventHandler >   m_xEventHdl;
    css::uno::Reference< css::awt::XContainerWindowProvider >       m_xWinProvider;

    void            CreateDialog();
    sal_Bool        CallMethod( const rtl::OUString& rMethod );

public:
    FwkTabPage(
        Window* pParent,
        const rtl::OUString& rPageURL,
    const css::uno::Reference< css::awt::XContainerWindowEventHandler >& rEventHdl,
        const css::uno::Reference< css::awt::XContainerWindowProvider >& rProvider );

    virtual ~FwkTabPage();

    virtual void    ActivatePage();
    virtual void    DeactivatePage();
    virtual void    Resize();
};

struct TabEntry
{
    sal_Int32           m_nIndex;
    FwkTabPage*         m_pPage;
    ::rtl::OUString     m_sPageURL;
    css::uno::Reference< css::awt::XContainerWindowEventHandler > m_xEventHdl;

    TabEntry() :
        m_nIndex( -1 ), m_pPage( NULL ) {}

    TabEntry( sal_Int32 nIndex, ::rtl::OUString sURL, const css::uno::Reference< css::awt::XContainerWindowEventHandler > & rEventHdl ) :
        m_nIndex( nIndex ), m_pPage( NULL ), m_sPageURL( sURL ), m_xEventHdl( rEventHdl ) {}

    ~TabEntry() { delete m_pPage; }
};

typedef std::vector< TabEntry* > TabEntryList;

class FwkTabWindow : public Window
{
private:
    FwkTabControl   m_aTabCtrl;
    TabEntryList    m_TabList;

    css::uno::Reference< css::awt::XContainerWindowProvider >   m_xWinProvider;

    void            ClearEntryList();
    TabEntry*       FindEntry( sal_Int32 nIndex ) const;
    bool            RemoveEntry( sal_Int32 nIndex );

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, void *);
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, void *);

public:
    FwkTabWindow( Window* pParent );
    ~FwkTabWindow();

    void            AddEventListener( const Link& rEventListener );
    void            RemoveEventListener( const Link& rEventListener );
    FwkTabPage*     AddTabPage( sal_Int32 nIndex, const css::uno::Sequence< css::beans::NamedValue >& rProperties );
    void            ActivatePage( sal_Int32 nIndex );
    void            RemovePage( sal_Int32 nIndex );
    virtual void        Resize();
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
