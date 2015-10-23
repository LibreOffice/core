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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PROPBRW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PROPBRW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <vcl/dockwin.hxx>
#include <svx/svdmark.hxx>
#include "ModuleHelper.hxx"

namespace rptui
{

class OSectionView;
class ODesignView;
class OObjectBase;

// PropBrw


class PropBrw : public DockingWindow , public SfxListener, public SfxBroadcaster
{
private:
    OModuleClient       m_aModuleClient;
    css::uno::Reference< css::uno::XComponentContext >
                        m_xInspectorContext;
    css::uno::Reference< css::uno::XComponentContext >
                        m_xORB;
    css::uno::Reference< css::frame::XFrame2 >
                        m_xMeAsFrame;
    css::uno::Reference< css::inspection::XObjectInspector >
                        m_xBrowserController;
    css::uno::Reference< css::awt::XWindow >
                        m_xBrowserComponentWindow;
    css::uno::Reference< css::uno::XInterface>
                        m_xLastSection; /// is the previously displayed section
    OUString            m_sLastActivePage;
    VclPtr<ODesignView>        m_pDesignView;
    OSectionView*       m_pView;
    bool                m_bInitialStateChange;

    PropBrw(PropBrw&) = delete;
    void operator =(PropBrw&) = delete;
protected:

    virtual void Resize() override;
    virtual bool Close() override;

    css::uno::Sequence< css::uno::Reference< css::uno::XInterface> >
        CreateCompPropSet(const SdrMarkList& rMarkList);

    void implSetNewObject(
        const css::uno::Sequence< css::uno::Reference< css::uno::XInterface> >& _aObjects = css::uno::Sequence< css::uno::Reference< css::uno::XInterface> >());

    static OUString GetHeadlineName(
        const css::uno::Sequence< css::uno::Reference< css::uno::XInterface> >& _aObjects);

    void implDetachController();
    css::uno::Reference< css::uno::XInterface> CreateComponentPair(OObjectBase* _pObj);
    css::uno::Reference< css::uno::XInterface> CreateComponentPair(
             const css::uno::Reference< css::uno::XInterface>& _xFormComponent
            ,const css::uno::Reference< css::uno::XInterface>& _xReportComponent);
    DECL_LINK_TYPED( OnAsyncGetFocus, void*, void );

public:
    PropBrw(const css::uno::Reference< css::uno::XComponentContext >& _xORB
            ,Window* pParent
            ,ODesignView*  _pDesignView);
    virtual ~PropBrw();
    virtual void dispose() override;

    virtual void LoseFocus() override;

    void    Update( OSectionView* m_pView );
    void    Update( const css::uno::Reference< css::uno::XInterface>& _xReportComponent);
    OUString         getCurrentPage() const;
    void                    setCurrentPage(const OUString& _sLastActivePage);

    ::Size getMinimumSize() const;
private:
    using Window::Update;
};

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
