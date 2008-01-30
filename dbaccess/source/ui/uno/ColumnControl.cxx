/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnControl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:55:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
: m_xORB(_rxFactory)
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

        OColumnPeer* pPeer = new OColumnPeer(pParentWin,m_xORB);
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
