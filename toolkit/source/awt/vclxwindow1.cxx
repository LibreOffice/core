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
#elif defined ( IOS )
#include "premac.h"
#include <UIKit/UIKit.h>
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
        pException->Message = ::rtl::OUString("not a work window");
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
                if ( pProps[i].Name == "WINDOW" )
                    pProps[i].Value >>= nHandle;
                else if ( pProps[i].Name == "XEMBED" )
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
        pException->Message = ::rtl::OUString("incorrect window handle type");
        throw pException;
    }
    // create system parent data
    SystemParentData aSysParentData;
    aSysParentData.nSize = sizeof ( SystemParentData );
#if defined( WNT )
    aSysParentData.hWnd = (HWND) nHandle;
#elif defined( QUARTZ )
    aSysParentData.pView = reinterpret_cast<NSView*>(nHandle);
#elif defined( IOS )
    aSysParentData.pView = reinterpret_cast<UIView*>(nHandle);
#elif defined( UNX )
    aSysParentData.aWindow = (long)nHandle;
    aSysParentData.bXEmbedSupport = bXEmbed;
#endif

    // set system parent
    ((WorkWindow*)pWindow)->SetPluginParent( &aSysParentData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
