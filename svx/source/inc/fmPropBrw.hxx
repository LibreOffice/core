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
#ifndef INCLUDED_SVX_SOURCE_INC_FMPROPBRW_HXX
#define INCLUDED_SVX_SOURCE_INC_FMPROPBRW_HXX

#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <sfx2/basedlgs.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/childwin.hxx>
#include <svx/fmtools.hxx>


class FmPropBrwMgr : public SfxChildWindow
{
public:
    FmPropBrwMgr(vcl::Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, const SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmPropBrwMgr);
};

class SfxBindings;
class FmFormShell;

class FmPropBrw final : public SfxModelessDialogController, public SfxControllerItem
{
    bool            m_bInitialStateChange;
    weld::Window*   m_pParent;
    ImplSVEvent*    m_nAsyncGetFocusId;
    OUString        m_sLastActivePage;
    std::unique_ptr<weld::Container> m_xContainer;
    css::uno::Reference< css::uno::XComponentContext >
                    m_xInspectorContext;
    css::uno::Reference< css::uno::XComponentContext >
                    m_xORB;
    css::uno::Reference< css::frame::XFrame2 >
                    m_xMeAsFrame;
    css::uno::Reference< css::uno::XInterface >
                    m_xLastKnownDocument;
    css::uno::Reference< css::inspection::XObjectInspectorModel >
                    m_xInspectorModel;
    css::uno::Reference< css::frame::XController >
                    m_xBrowserController;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState) override;
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const override;
    virtual void Close() override;

    DECL_LINK( OnAsyncGetFocus, void*, void );

    void implSetNewSelection( const InterfaceBag& _rSelection );
    void implDetachController();
    bool implIsReadOnlyModel() const;
    OUString getCurrentPage() const;

public:
    FmPropBrw(
        const css::uno::Reference< css::uno::XComponentContext >& _xORB,
        SfxBindings* pBindings,
        SfxChildWindow* pMgr,
        weld::Window* pParent,
        const SfxChildWinInfo* _pInfo
    );
    virtual ~FmPropBrw() override;

private:

    /** creates the PropertyBrowser (aka ObjectInspector) and plugs it into our frame

        This method ensures that a new component is created every time the XModel which
        we're working for changed. This is necessary since this model is part of the
        ComponentContext we use to create the ObjectInspector.
    */
    void    impl_ensurePropertyBrowser_nothrow( FmFormShell* _pFormShell );

    /** creates a property browser

        After this method returns, m_xBrowserController is not <NULL/>.

    @precond
        we don't have an ObjectInspector, yet, i.e. m_xBrowserController is <NULL/>.
    */
    void    impl_createPropertyBrowser_throw( FmFormShell* _pFormShell );
};
#endif // INCLUDED_SVX_SOURCE_INC_FMPROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
