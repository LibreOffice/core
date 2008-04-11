/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propbrw.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _BASCTL_PROPBRW_HXX
#define _BASCTL_PROPBRW_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <comphelper/composedprops.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/brdcst.hxx>
#include <svtools/lstner.hxx>
#include <sfx2/childwin.hxx>
#include <svx/svdmark.hxx>

//============================================================================
// PropBrwMgr
//============================================================================

class PropBrwMgr : public SfxChildWindow
{
public:
    PropBrwMgr(Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(PropBrwMgr);
};

//============================================================================
// PropBrw
//============================================================================

class SfxBindings;
class SdrView;

class PropBrw : public SfxFloatingWindow , public SfxListener, public SfxBroadcaster
{
private:
    sal_Bool        m_bInitialStateChange;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    m_xContextDocument;

protected:
    SdrView*        pView;
    virtual void Resize();
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, InterfaceArray);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
        CreateMultiSelectionSequence( const SdrMarkList& _rMarkList );
    void implSetNewObjectSequence( const ::com::sun::star::uno::Sequence
        < ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rObjectSeq );

    void implSetNewObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

    ::rtl::OUString GetHeadlineName(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

public:
    PropBrw( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
             SfxBindings *pBindings,
             PropBrwMgr* pMgr,
             Window* pParent,
             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
    );
    virtual ~PropBrw();
    using Window::Update;
    // note: changing the Context document to an instance other than the one given in the ctor is not supported
    // currently
    void    Update( const SfxViewShell* _pShell );
    SdrView*        GetCurView() const { return pView; }

private:
    void    ImplUpdate( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument, SdrView* pView );
    void    ImplDestroyController();
    void    ImplReCreateController();
};

#endif // _BASCTL_PROPBRW_HXX
