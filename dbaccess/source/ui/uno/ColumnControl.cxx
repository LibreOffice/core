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
#include "dbustrings.hrc"
#include "apitools.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include "dbu_reghelper.hxx"
#include <comphelper/processfactory.hxx>

extern "C" void SAL_CALL createRegistryInfo_OColumnControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OColumnControl> aAutoRegistration;
}

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

OColumnControl::OColumnControl(const Reference<XComponentContext>& rxContext)
    :UnoControl(), m_xContext(rxContext)
{
}

IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(OColumnControl, SERVICE_CONTROLDEFAULT)
IMPLEMENT_SERVICE_INFO_SUPPORTS(OColumnControl)
IMPLEMENT_SERVICE_INFO_GETSUPPORTED2_STATIC(OColumnControl, "com.sun.star.awt.UnoControl","com.sun.star.sdb.ColumnDescriptorControl")

Reference< XInterface > SAL_CALL OColumnControl::Create(const Reference< XMultiServiceFactory >& _rxORB)
{
    return static_cast< XServiceInfo* >(new OColumnControl(comphelper::getComponentContext(_rxORB)));
}

OUString OColumnControl::GetComponentServiceName()
{
    return OUString("com.sun.star.sdb.ColumnDescriptorControl");
}

void SAL_CALL OColumnControl::createPeer(const Reference< XToolkit >& /*rToolkit*/, const Reference< XWindowPeer >& rParentPeer) throw( RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    if ( !getPeer().is() )
    {
        mbCreatingPeer = sal_True;

        Window* pParentWin = NULL;
        if (rParentPeer.is())
        {
            VCLXWindow* pParent = VCLXWindow::GetImplementation(rParentPeer);
            if (pParent)
                pParentWin = pParent->GetWindow();
        }

        OColumnPeer* pPeer = new OColumnPeer( pParentWin, m_xContext );
        OSL_ENSURE(pPeer != NULL, "FmXGridControl::createPeer : imp_CreatePeer didn't return a peer !");
        setPeer( pPeer );

        UnoControlComponentInfos aComponentInfos(maComponentInfos);
        Reference< XGraphics > xGraphics( mxGraphics );
        Reference< XView >  xV(getPeer(), UNO_QUERY);
        Reference< XWindow >    xW(getPeer(), UNO_QUERY);

        aGuard.clear();

        updateFromModel();

        xV->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );
        setPosSize( aComponentInfos.nX, aComponentInfos.nY, aComponentInfos.nWidth, aComponentInfos.nHeight, ::com::sun::star::awt::PosSize::POSSIZE );

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
            xW->setVisible(sal_True);

        if (!aComponentInfos.bEnable)
            xW->setEnable(sal_False);

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

        Reference< ::com::sun::star::awt::XView >  xPeerView(getPeer(), UNO_QUERY);
        xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
        xPeerView->setGraphics( xGraphics );

        mbCreatingPeer = sal_False;
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
