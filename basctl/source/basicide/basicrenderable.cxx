/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basidesh.hxx,v $
 * $Revision: 1.10 $
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


#include "basicrenderable.hxx"
#include "bastypes.hxx"

#include "com/sun/star/awt/XDevice.hpp"
#include "toolkit/awt/vclxdevice.hxx"
#include "vcl/print.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace basicide;

BasicRenderable::~BasicRenderable()
{
}

Printer* BasicRenderable::getPrinter ( const Sequence<beans::PropertyValue >& i_xOptions )
{
    Printer* pPrinter = NULL;
    sal_Int32 nProps = i_xOptions.getLength();
    const beans::PropertyValue* pProps = i_xOptions.getConstArray();
    for( sal_Int32 i = 0; i < nProps; i++ )
    {
        if( pProps[i].Name.equalsAscii( "RenderDevice" ) )
        {
            Reference<awt::XDevice> xRenderDevice;

            if( pProps[i].Value >>= xRenderDevice )
            {
                VCLXDevice* pDevice = VCLXDevice::GetImplementation(xRenderDevice);
                OutputDevice* pOut = pDevice ? pDevice->GetOutputDevice() : NULL;
                pPrinter = dynamic_cast<Printer*>(pOut);
                break;
            }
        }
    }
    return pPrinter;
}

sal_Int32 SAL_CALL BasicRenderable::getRendererCount (
        const Any& i_rSelection, const Sequence<beans::PropertyValue >& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    sal_Int32 nCount = 0;
    if( mpWindow )
    {
        Printer* pPrinter = getPrinter( i_xOptions );
        if( pPrinter )
            nCount = mpWindow->countPages( pPrinter );
        else
            throw lang::IllegalArgumentException();
    }

    return nCount;
}

Sequence<beans::PropertyValue> SAL_CALL BasicRenderable::getRenderer (
        sal_Int32 nRenderer, const Any& rSelection, const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    Sequence< beans::PropertyValue > aVals;
    // insert page size here
    Printer* pPrinter = getPrinter( i_xOptions );
    // no renderdevice is legal; the first call is to get our print ui options
    if( pPrinter )
    {
        Size aPageSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );

        aVals.realloc( 1 );
        aVals[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) );
        awt::Size aSize;
        aSize.Width  = aPageSize.Width();
        aSize.Height = aPageSize.Height();
        aVals[0].Value <<= aSize;
    }

    return aVals;
}

void SAL_CALL BasicRenderable::render (
        sal_Int32 nRenderer, const Any& rSelection,
        const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    if( mpWindow )
    {
        Printer* pPrinter = getPrinter( i_xOptions );
        if( pPrinter )
            mpWindow->printPage( nRenderer, pPrinter );
        else
            throw lang::IllegalArgumentException();
    }
}


