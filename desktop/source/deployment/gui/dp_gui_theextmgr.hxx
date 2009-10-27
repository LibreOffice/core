/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_theextmgr.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef INCLUDED_DP_GUI_THEEXTMGR_HXX
#define INCLUDED_DP_GUI_THEEXTMGR_HXX

#include "comphelper/sequence.hxx"

#include "cppuhelper/implbase2.hxx"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XTerminateListener.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/util/XModifyListener.hpp"

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_updatedata.hxx"

//==============================================================================
namespace dp_gui {

//------------------------------------------------------------------------------
class ExtensionCmdQueue;

//------------------------------------------------------------------------------
class TheExtensionManager :
    public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XTerminateListener,
                                    ::com::sun::star::util::XModifyListener >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop >           m_xDesktop;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> > m_sPackageManagers;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xNameAccessNodes;

    ::std::auto_ptr< ExtensionCmdQueue > m_pExecuteCmdQueue;

    Window                  *m_pParent;
    ExtMgrDialog            *m_pExtMgrDialog;
    UpdateRequiredDialog    *m_pUpdReqDialog;

    ::rtl::OUString          m_sGetExtensionsURL;

    // liste der packages ( xpackage?, mit parent manager, ... )

    void createPackageList();
    bool createPackageList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager );

public:
    static ::rtl::Reference<TheExtensionManager> s_ExtMgr;

         TheExtensionManager( Window * pParent,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext );
        ~TheExtensionManager();

    void createDialog( const bool bCreateUpdDlg );
    sal_Int16 execute();

    Dialog* getDialog() { return m_pExtMgrDialog ? (Dialog*) m_pExtMgrDialog : (Dialog*) m_pUpdReqDialog; }
    DialogHelper* getDialogHelper() { return m_pExtMgrDialog ? (DialogHelper*) m_pExtMgrDialog : (DialogHelper*) m_pUpdReqDialog; }

    void SetText( const ::rtl::OUString &rTitle );
    void Show();
    void ToTop( USHORT nFlags );
    bool Close();
    bool isVisible();

    //-----------------
    bool checkUpdates( bool showUpdateOnly, bool parentVisible );
    bool updatePackages( const std::vector< TUpdateListEntry > &vList );

    bool enablePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                        bool bEnable );
    bool removePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool installPackage( const ::rtl::OUString &rPackageURL, bool bWarnUser = false );

    bool queryTermination();
    void terminateDialog();

    // Tools
    bool supportsOptions( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    PackageState getPackageState( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext() const { return m_xContext; }
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > getUserPkgMgr() const { return m_sPackageManagers[0]; }
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > getSharedPkgMgr() const { return m_sPackageManagers[1]; }

    //-----------------
    static ::rtl::Reference<TheExtensionManager> get(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xContext,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> const & xParent = 0,
        ::rtl::OUString const & view = ::rtl::OUString() );

    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);
};

} // namespace dp_gui

#endif

