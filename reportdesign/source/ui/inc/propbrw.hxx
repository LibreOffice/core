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
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xInspectorContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 >
                        m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspector >
                        m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                        m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                        m_xLastSection; /// is the previously displayed section
    OUString     m_sLastActivePage;
    VclPtr<ODesignView>        m_pDesignView;
    OSectionView*       m_pView;
    bool            m_bInitialStateChange;

    PropBrw(PropBrw&) = delete;
    void operator =(PropBrw&) = delete;
protected:

    virtual void Resize() SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >
        CreateCompPropSet(const SdrMarkList& rMarkList);

    void implSetNewObject(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >& _aObjects = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >());

    static OUString GetHeadlineName(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >& _aObjects);

    void implDetachController();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> CreateComponentPair(OObjectBase* _pObj);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> CreateComponentPair(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xFormComponent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);
    DECL_LINK_TYPED( OnAsyncGetFocus, void*, void );

public:
    PropBrw(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
            ,Window* pParent
            ,ODesignView*  _pDesignView);
    virtual ~PropBrw();
    virtual void dispose() SAL_OVERRIDE;

    virtual void LoseFocus() SAL_OVERRIDE;

    void    Update( OSectionView* m_pView );
    void    Update( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);
    OUString         getCurrentPage() const;
    void                    setCurrentPage(const OUString& _sLastActivePage);

    ::Size getMinimumSize() const;
private:
    using Window::Update;
};

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
