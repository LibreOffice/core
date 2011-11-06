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


#include "precompiled_basctl.hxx"

#include "basicrenderable.hxx"
#include "bastypes.hxx"
#include "basidesh.hrc"

#include "com/sun/star/awt/XDevice.hpp"
#include "toolkit/awt/vclxdevice.hxx"
#include "vcl/print.hxx"
#include "tools/multisel.hxx"
#include "tools/resary.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace basicide;

BasicRenderable::BasicRenderable( IDEBaseWindow* pWin )
: cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >( maMutex )
, mpWindow( pWin )
{
    ResStringArray aStrings( IDEResId( RID_PRINTDLG_STRLIST )  );
    DBG_ASSERT( aStrings.Count() >= 3, "resource incomplete" );
    if( aStrings.Count() < 3 ) // bad resource ?
        return;

    m_aUIProperties.realloc( 3 );

    // create Subgroup for print range
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    aPrintRangeOpt.mbInternalOnly = sal_True;
    m_aUIProperties[0].Value = getSubgroupControlOpt( rtl::OUString( aStrings.GetString( 0 ) ),
                                                      rtl::OUString(),
                                                      aPrintRangeOpt
                                                      );

    // create a choice for the range to print
    rtl::OUString aPrintContentName( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) );
    Sequence< rtl::OUString > aChoices( 2 );
    Sequence< rtl::OUString > aHelpIds( 2 );
    aChoices[0] = aStrings.GetString( 1 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ) );
    aChoices[1] = aStrings.GetString( 2 );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ) );
    m_aUIProperties[1].Value = getChoiceControlOpt( rtl::OUString(),
                                                    aHelpIds,
                                                    aPrintContentName,
                                                    aChoices,
                                                    0 );

    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintContentName, 1, sal_True );
    m_aUIProperties[2].Value = getEditControlOpt( rtl::OUString(),
                                                  rtl::OUString(),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ),
                                                  rtl::OUString(),
                                                  aPageRangeOpt
                                                  );
}

BasicRenderable::~BasicRenderable()
{
}

Printer* BasicRenderable::getPrinter()
{
    Printer* pPrinter = NULL;
    Any aValue( getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) ) );
    Reference<awt::XDevice> xRenderDevice;

    if( aValue >>= xRenderDevice )
    {
        VCLXDevice* pDevice = VCLXDevice::GetImplementation(xRenderDevice);
        OutputDevice* pOut = pDevice ? pDevice->GetOutputDevice() : NULL;
        pPrinter = dynamic_cast<Printer*>(pOut);
    }
    return pPrinter;
}

sal_Int32 SAL_CALL BasicRenderable::getRendererCount (
        const Any&, const Sequence<beans::PropertyValue >& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    processProperties( i_xOptions );

    sal_Int32 nCount = 0;
    if( mpWindow )
    {
        Printer* pPrinter = getPrinter();
        if( pPrinter )
        {
            nCount = mpWindow->countPages( pPrinter );
            sal_Int64 nContent = getIntValue( "PrintContent", -1 );
            if( nContent == 1 )
            {
                rtl::OUString aPageRange( getStringValue( "PageRange" ) );
                MultiSelection aSel( aPageRange );
                long nSelCount = aSel.GetSelectCount();
                if( nSelCount >= 0 && nSelCount < nCount )
                    nCount = nSelCount;
            }
        }
        else
            throw lang::IllegalArgumentException();
    }

    return nCount;
}

Sequence<beans::PropertyValue> SAL_CALL BasicRenderable::getRenderer (
        sal_Int32, const Any&, const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    processProperties( i_xOptions );

    Sequence< beans::PropertyValue > aVals;
    // insert page size here
    Printer* pPrinter = getPrinter();
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

    appendPrintUIOptions( aVals );

    return aVals;
}

void SAL_CALL BasicRenderable::render (
        sal_Int32 nRenderer, const Any&,
        const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    processProperties( i_xOptions );

    if( mpWindow )
    {
        Printer* pPrinter = getPrinter();
        if( pPrinter )
        {
            sal_Int64 nContent = getIntValue( "PrintContent", -1 );
            if( nContent == 1 )
            {
                rtl::OUString aPageRange( getStringValue( "PageRange" ) );
                MultiSelection aSel( aPageRange );
                long nSelect = aSel.FirstSelected();
                while( nSelect != long(SFX_ENDOFSELECTION) && nRenderer-- )
                    nSelect = aSel.NextSelected();
                if( nSelect != long(SFX_ENDOFSELECTION) )
                    mpWindow->printPage( sal_Int32(nSelect-1), pPrinter );
            }
            else
                mpWindow->printPage( nRenderer, pPrinter );
        }
        else
            throw lang::IllegalArgumentException();
    }
}


