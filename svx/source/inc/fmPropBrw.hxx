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
#ifndef SVX_FMPROPBRW_HXX
#define SVX_FMPROPBRW_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
/** === end UNO includes === **/

#include <sfx2/basedlgs.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/childwin.hxx>
#include "svx/fmtools.hxx"

//========================================================================
class FmPropBrwMgr : public SfxChildWindow
{
public:
    FmPropBrwMgr(Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmPropBrwMgr);
};

class FmPropControl;
class SfxBindings;
class FmFormShell;
//========================================================================
class FmPropBrw : public SfxFloatingWindow, public SfxControllerItem
{
    sal_Bool        m_bInitialStateChange;
    bool            m_bInStateChange;
    ::rtl::OUString m_sLastActivePage;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xInspectorContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xLastKnownDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel >
                    m_xInspectorModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                    m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xFrameContainerWindow;

protected:
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECL_LINK( OnAsyncGetFocus, void* );

    void implSetNewSelection( const InterfaceBag& _rSelection );
    void implDetachController();
    bool implIsReadOnlyModel() const;
    ::rtl::OUString getCurrentPage() const;

public:
    FmPropBrw(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
        SfxBindings* pBindings,
        SfxChildWindow* pMgr,
        Window* pParent,
        const SfxChildWinInfo* _pInfo
    );
    virtual ~FmPropBrw();

    using SfxFloatingWindow::StateChanged;

protected:
    virtual void        Resize();

private:
    /** creates the PropertyBrowser (aka ObjectInspector) and plugs it into our frame

        This method ensures that a new component is created every time the XModel which
        we're working for changed. This is necessary since this model is part of the
        ComponentContext we use to create the ObjectInspector.
    */
    void    impl_ensurePropertyBrowser_nothrow( FmFormShell* _pFormShell );

    /** creates a property browser

        After this method returns, m_xBrowserController and m_xBrowserComponentWindow are
        not <NULL/>.

    @precond
        we don't have an ObjectInspector, yet, i.e. m_xBrowserController and m_xBrowserComponentWindow
        are <NULL/>.
    */
    void    impl_createPropertyBrowser_throw( FmFormShell* _pFormShell );
};
#endif //SVX_FMPROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
