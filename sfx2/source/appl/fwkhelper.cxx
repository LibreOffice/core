/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fwkhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 10:24:33 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include "workwin.hxx"
#include "frame.hxx"

void SAL_CALL RefreshToolbars( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xFrame.is() )
    {
        SfxFrame* pFrame=0;
        for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
        {
            if ( pFrame->GetFrameInterface() == xFrame )
                break;
        }

        if ( pFrame )
        {
            SfxWorkWindow* pWrkWin = pFrame->GetWorkWindow_Impl();
            if ( pWrkWin )
                pWrkWin->UpdateObjectBars_Impl();
        }
    }
}
