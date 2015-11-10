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
#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_FWKTABWINDOW_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_FWKTABWINDOW_HXX

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
    FwkTabControl(vcl::Window* pParent);

    void BroadcastEvent( sal_uLong nEvent );
};

class FwkTabPage : public TabPage
{
private:
    OUString                                                   m_sPageURL;
    css::uno::Reference< css::awt::XWindow >                        m_xPage;
    css::uno::Reference< css::awt::XContainerWindowEventHandler >   m_xEventHdl;
    css::uno::Reference< css::awt::XContainerWindowProvider >       m_xWinProvider;

    void            CreateDialog();
    bool        CallMethod( const OUString& rMethod );

public:
    FwkTabPage(
        vcl::Window* pParent,
        const OUString& rPageURL,
    const css::uno::Reference< css::awt::XContainerWindowEventHandler >& rEventHdl,
        const css::uno::Reference< css::awt::XContainerWindowProvider >& rProvider );

    virtual ~FwkTabPage();
    virtual void    dispose() override;

    virtual void    ActivatePage() override;
    virtual void    DeactivatePage() override;
    virtual void    Resize() override;
};

struct TabEntry
{
    sal_Int32           m_nIndex;
    ScopedVclPtr<FwkTabPage>  m_pPage;
    OUString            m_sPageURL;
    css::uno::Reference< css::awt::XContainerWindowEventHandler > m_xEventHdl;

    TabEntry() :
        m_nIndex( -1 ), m_pPage( nullptr ) {}

    TabEntry( sal_Int32 nIndex, const OUString& sURL, const css::uno::Reference< css::awt::XContainerWindowEventHandler > & rEventHdl ) :
        m_nIndex( nIndex ), m_pPage( nullptr ), m_sPageURL( sURL ), m_xEventHdl( rEventHdl ) {}
    ~TabEntry() { m_pPage.disposeAndClear(); }
};

typedef std::vector< TabEntry* > TabEntryList;

class FwkTabWindow : public vcl::Window
{
private:
    VclPtr<FwkTabControl>  m_aTabCtrl;
    TabEntryList           m_TabList;

    css::uno::Reference< css::awt::XContainerWindowProvider >   m_xWinProvider;

    void            ClearEntryList();
    TabEntry*       FindEntry( sal_Int32 nIndex ) const;
    bool            RemoveEntry( sal_Int32 nIndex );

    DECL_DLLPRIVATE_LINK_TYPED(ActivatePageHdl,   TabControl *, void);
    DECL_DLLPRIVATE_LINK_TYPED(DeactivatePageHdl, TabControl *, bool);

public:
    FwkTabWindow( vcl::Window* pParent );
    virtual ~FwkTabWindow();
    virtual void    dispose() override;

    void            AddEventListener( const Link<VclWindowEvent&,void>& rEventListener );
    void            RemoveEventListener( const Link<VclWindowEvent&,void>& rEventListener );
    FwkTabPage*     AddTabPage( sal_Int32 nIndex, const css::uno::Sequence< css::beans::NamedValue >& rProperties );
    void            ActivatePage( sal_Int32 nIndex );
    void            RemovePage( sal_Int32 nIndex );
    virtual void        Resize() override;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
