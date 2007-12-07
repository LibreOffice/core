/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxwindow1.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:40:19 $
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
#include "precompiled_toolkit.hxx"

#include <tools/svwin.h>

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _SV_WORKWIN
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifdef QUARTZ
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif

/// helper method to set a window handle into a SystemParentData struct
void VCLXWindow::SetSystemParent_Impl( const com::sun::star::uno::Any& rHandle )
{
    // does only work for WorkWindows
    Window *pWindow = GetWindow();
    if ( pWindow->GetType() != WINDOW_WORKWINDOW )
    {
        ::com::sun::star::uno::Exception *pException =
            new ::com::sun::star::uno::RuntimeException;
        pException->Message = ::rtl::OUString::createFromAscii( "not a work window" );
        throw pException;
    }

    // use sal_Int64 here to accomodate all int types
    // uno::Any shift operator whill upcast if necessary
    sal_Int64 nHandle = 0;
    sal_Bool  bXEmbed = sal_False;
    bool bThrow = false;
    if( ! (rHandle >>= nHandle) )
    {
        com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > aProps;
        if( rHandle >>= aProps )
        {
            const int nProps = aProps.getLength();
            const com::sun::star::beans::NamedValue* pProps = aProps.getConstArray();
            for( int i = 0; i < nProps; i++ )
            {
                if( pProps[i].Name.equalsAscii( "WINDOW" ) )
                    pProps[i].Value >>= nHandle;
                else if( pProps[i].Name.equalsAscii( "XEMBED" ) )
                    pProps[i].Value >>= bXEmbed;
            }
        }
        else
            bThrow = true;
    }
    if( bThrow )
    {
        ::com::sun::star::uno::Exception *pException =
            new ::com::sun::star::uno::RuntimeException;
        pException->Message = ::rtl::OUString::createFromAscii( "incorrect window handle type" );
        throw pException;
    }
    // create system parent data
    SystemParentData aSysParentData;
    aSysParentData.nSize = sizeof ( SystemParentData );
#if defined( WIN ) || defined( WNT ) || defined ( OS2 )
    aSysParentData.hWnd = (HWND) nHandle;
#elif defined( QUARTZ )
    aSysParentData.pView = reinterpret_cast<NSView*>(nHandle);
#elif defined( UNX )
    aSysParentData.aWindow = (long)nHandle;
    aSysParentData.bXEmbedSupport = bXEmbed;
#endif

    // set system parent
    ((WorkWindow*)pWindow)->SetPluginParent( &aSysParentData );
}

