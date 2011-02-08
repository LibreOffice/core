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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <tools/svwin.h>
#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#ifndef _SV_WORKWIN
#include <vcl/wrkwin.hxx>
#endif
#include <vcl/window.hxx>

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#elif defined ( QUARTZ )
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif
#include <vcl/sysdata.hxx>

/// helper method to set a window handle into a SystemParentData struct
void VCLXWindow::SetSystemParent_Impl( const com::sun::star::uno::Any& rHandle )
{
    // does only work for WorkWindows
    Window *pWindow = GetWindow();
    if ( pWindow->GetType() != WINDOW_WORKWINDOW )
    {
        ::com::sun::star::uno::Exception *pException =
            new ::com::sun::star::uno::RuntimeException;
        pException->Message = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("not a work window"));
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
                if( pProps[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WINDOW" ) ) )
                    pProps[i].Value >>= nHandle;
                else if( pProps[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "XEMBED" ) ) )
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
        pException->Message = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("incorrect window handle type"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
