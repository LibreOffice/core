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
#ifndef FRAMEWORK_TABWINDOW_HXX
#define FRAMEWORK_TABWINDOW_HXX

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

    DECL_DLLPRIVATE_LINK( ActivatePageHdl, TabControl * );
    DECL_DLLPRIVATE_LINK( DeactivatePageHdl, TabControl * );
    DECL_DLLPRIVATE_LINK( CloseHdl, PushButton * );

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

