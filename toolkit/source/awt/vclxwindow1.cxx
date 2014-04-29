/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#elif defined ( OS2 )
#include <svpm.h>
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
        pException->Message = ::rtl::OUString::createFromAscii( "not a work window" );
        throw pException;
    }

    // use sal_Int64 here to accommodate all int types
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
#if defined( WNT ) || defined ( OS2 )
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

