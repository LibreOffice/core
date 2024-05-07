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

#include "ColumnControl.hxx"
#include "ColumnPeer.hxx"
#include <strings.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/awt/PosSize.hpp>

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dbu_OColumnControl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::OColumnControl(context));
}

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;

OColumnControl::OColumnControl(const Reference<XComponentContext>& rxContext)
    : m_xContext(rxContext)
{
}

OUString SAL_CALL OColumnControl::getImplementationName()
{
        return SERVICE_CONTROLDEFAULT;
}
sal_Bool SAL_CALL OColumnControl::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
css::uno::Sequence< OUString > SAL_CALL OColumnControl::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.UnoControl"_ustr,u"com.sun.star.sdb.ColumnDescriptorControl"_ustr };
}

OUString OColumnControl::GetComponentServiceName() const
{
    return u"com.sun.star.sdb.ColumnDescriptorControl"_ustr;
}

void SAL_CALL OColumnControl::createPeer(const Reference< XToolkit >& /*rToolkit*/, const Reference< XWindowPeer >& rParentPeer)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    if ( getPeer().is() )
        return;

    mbCreatingPeer = true;

    vcl::Window* pParentWin = nullptr;
    if (rParentPeer.is())
    {
        VCLXWindow* pParent = dynamic_cast<VCLXWindow*>(rParentPeer.get());
        if (pParent)
            pParentWin = pParent->GetWindow();
    }

    rtl::Reference<OColumnPeer> pPeer = new OColumnPeer( pParentWin, m_xContext );
    OSL_ENSURE(pPeer != nullptr, "FmXGridControl::createPeer : imp_CreatePeer didn't return a peer !");
    setPeer( pPeer );

    UnoControlComponentInfos aComponentInfos(maComponentInfos);
    Reference< XGraphics > xGraphics( mxGraphics );
    Reference< XView >  xV(getPeer(), UNO_QUERY);
    Reference< XWindow >    xW(getPeer(), UNO_QUERY);

    aGuard.clear();

    updateFromModel();

    xV->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );
    setPosSize( aComponentInfos.nX, aComponentInfos.nY, aComponentInfos.nWidth, aComponentInfos.nHeight, css::awt::PosSize::POSSIZE );

    Reference<XPropertySet> xProp(getModel(), UNO_QUERY);
    if ( xProp.is() )
    {
        Reference<XConnection> xCon(xProp->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);
        pPeer->setConnection(xCon);
        Reference<XPropertySet> xColumn(xProp->getPropertyValue(PROPERTY_COLUMN),UNO_QUERY);
        pPeer->setColumn(xColumn);
        sal_Int32 nWidth = 50;
        xProp->getPropertyValue(PROPERTY_EDIT_WIDTH) >>= nWidth;
        pPeer->setEditWidth(nWidth);
    }

    if (aComponentInfos.bVisible)
        xW->setVisible(true);

    if (!aComponentInfos.bEnable)
        xW->setEnable(false);

    if (maWindowListeners.getLength())
        xW->addWindowListener( &maWindowListeners );

    if (maFocusListeners.getLength())
        xW->addFocusListener( &maFocusListeners );

    if (maKeyListeners.getLength())
        xW->addKeyListener( &maKeyListeners );

    if (maMouseListeners.getLength())
        xW->addMouseListener( &maMouseListeners );

    if (maMouseMotionListeners.getLength())
        xW->addMouseMotionListener( &maMouseMotionListeners );

    if (maPaintListeners.getLength())
        xW->addPaintListener( &maPaintListeners );

    Reference< css::awt::XView >  xPeerView(getPeer(), UNO_QUERY);
    xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
    xPeerView->setGraphics( xGraphics );

    mbCreatingPeer = false;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
