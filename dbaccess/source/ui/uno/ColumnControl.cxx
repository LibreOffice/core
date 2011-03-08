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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_COLUMNCONTROL_HXX
#include "ColumnControl.hxx"
#endif
#ifndef DBAUI_COLUMNPEER_HXX
#include "ColumnPeer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OColumnControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OColumnControl> aAutoRegistration;
}
//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

OColumnControl::OColumnControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :UnoControl( _rxFactory )
{
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2_STATIC(OColumnControl,SERVICE_CONTROLDEFAULT,"com.sun.star.awt.UnoControl","com.sun.star.sdb.ColumnDescriptorControl")
// -----------------------------------------------------------------------------
::rtl::OUString OColumnControl::GetComponentServiceName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.ColumnDescriptorControl"));
}
// -----------------------------------------------------------------------------
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

        OColumnPeer* pPeer = new OColumnPeer( pParentWin, maContext.getLegacyServiceFactory() );
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
//.........................................................................
}   // namespace dbaui
//.........................................................................
