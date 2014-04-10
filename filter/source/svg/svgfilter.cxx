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
#include "precompiled_filter.hxx"

#include <cstdio>
#include "svgfilter.hxx"
#include <vos/mutex.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XDesktop.hdl>
#include <com/sun/star/frame/XController.hdl>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#define SVG_FILTER_SERVICE_NAME         "com.sun.star.comp.Draw.SVGFilter"
#define SVG_FILTER_IMPLEMENTATION_NAME  SVG_FILTER_SERVICE_NAME

using ::rtl::OUString;
using namespace ::com::sun::star;

// -------------
// - SVGFilter -
// -------------

SVGFilter::SVGFilter( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF ),
    mpSVGDoc( NULL ),
    mpSVGExport( NULL ),
    mpSVGFontExport( NULL ),
    mpSVGWriter( NULL ),
    mpDefaultSdrPage( NULL ),
    mpSdrModel( NULL ),
    mbPresentation( sal_False ),
    mpObjects( NULL ),
    mxSrcDoc(),
#ifdef SOLAR_JAVA
    mxDstDoc(),
#endif
    mxDefaultPage(),
    maFilterData(),
    maShapeSelection(),
    mbExportSelection(false),
    maUniqueIdVector(),
    mnMasterSlideId(0),
    mnSlideId(0),
    mnDrawingGroupId(0),
    mnDrawingId(0),
    maOldFieldHdl()
{
}

// -----------------------------------------------------------------------------

SVGFilter::~SVGFilter()
{
    DBG_ASSERT( mpSVGDoc == NULL, "mpSVGDoc not destroyed" );
    DBG_ASSERT( mpSVGExport == NULL, "mpSVGExport not destroyed" );
    DBG_ASSERT( mpSVGFontExport == NULL, "mpSVGFontExport not destroyed" );
    DBG_ASSERT( mpSVGWriter == NULL, "mpSVGWriter not destroyed" );
    DBG_ASSERT( mpObjects == NULL, "mpObjects not destroyed" );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Window*     pFocusWindow = Application::GetFocusWindow();
    sal_Int16   nCurrentPageNumber = -1;
    sal_Bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

#ifdef SOLAR_JAVA
    if( mxDstDoc.is() )
        bRet = sal_False;//implImport( rDescriptor );
    else
#endif
    if( mxSrcDoc.is() )
    {
        // #124608# detext selection
        sal_Bool bSelectionOnly = sal_False;
        bool bGotSelection(false);
        Reference< drawing::XShapes > xShapes;

        // #124608# extract Single selection wanted from dialog return values
        for ( sal_Int32 nInd = 0; nInd < rDescriptor.getLength(); nInd++ )
        {
            if ( rDescriptor[nInd].Name.equalsAscii( "SelectionOnly" ) )
            {
                rDescriptor[nInd].Value >>= bSelectionOnly;
            }
        }

        uno::Reference< frame::XDesktop > xDesktop( mxMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                                                    uno::UNO_QUERY);
        if( xDesktop.is() )
        {
            uno::Reference< frame::XFrame > xFrame( xDesktop->getCurrentFrame() );

            if( xFrame.is() )
            {
                uno::Reference< frame::XController > xController( xFrame->getController() );

                if( xController.is() )
                {
                    uno::Reference< drawing::XDrawView > xDrawView( xController, uno::UNO_QUERY );

                    if( xDrawView.is() )
                    {
                        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawView->getCurrentPage() );

                        if( xDrawPage.is() )
                        {
                            uno::Reference< beans::XPropertySet >( xDrawPage, uno::UNO_QUERY )->
                                getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) >>= nCurrentPageNumber;
                        }
                    }

                    if(bSelectionOnly)
                    {
                        // #124608# when selection only is wanted, get the current object selection
                        // from the DrawView
                        Reference< view::XSelectionSupplier > xSelection (xController, UNO_QUERY);

                        if (xSelection.is())
                        {
                            uno::Any aSelection;

                            if(xSelection->getSelection() >>= aSelection)
                            {
                                bGotSelection = (sal_True == ( aSelection >>= xShapes ));
                            }
                        }
                    }
                }
            }
        }

        if(bSelectionOnly && bGotSelection && 0 == xShapes->getCount())
        {
            // #124608# export selection, got xShapes but no shape selected -> nothing
            // to export, we are done (maybe return true, but a hint that nothing was done
            // may be useful; it may have happened by error)
            bRet = sal_False;
        }
        else
        {
            Sequence< PropertyValue > aNewDescriptor(rDescriptor);
            const bool bSinglePage(nCurrentPageNumber > 0);

            if(bSinglePage || bGotSelection)
            {
                const sal_uInt32 nOldLength = rDescriptor.getLength();
                sal_uInt32 nInsert(nOldLength);

                aNewDescriptor.realloc(nOldLength + (bSinglePage ? 1 : 0) + (bGotSelection ? 1 : 0));

                if(bSinglePage)
                {
                    aNewDescriptor[nInsert].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PagePos"));
                    aNewDescriptor[nInsert].Value <<= static_cast<sal_Int16>(nCurrentPageNumber - 1);
                    nInsert++;
                }

                if(bGotSelection)
                {
                    // #124608# add retrieved ShapeSelection if export of only selected shapes is wanted
                    // so that the filter implementation can use it
                    aNewDescriptor[nInsert].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShapeSelection"));
                    aNewDescriptor[nInsert].Value <<= xShapes;
                    // reactivate this when you add other properties to aNewDescriptor
                    // nInsert++;
                }
            }

            bRet = implExport(aNewDescriptor);
        }
    }
    else
        bRet = sal_False;

    if( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::cancel( ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

// -----------------------------------------------------------------------------

#ifdef SOLAR_JAVA
void SAL_CALL SVGFilter::setTargetDocument( const Reference< XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDstDoc = xDoc;
}
#endif

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& /* aArguments */ )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString SVGFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( SVG_FILTER_IMPLEMENTATION_NAME ) );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter_supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return( rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SVG_FILTER_SERVICE_NAME ) ) );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SVGFilter_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SVG_FILTER_SERVICE_NAME ) );
    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL SVGFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*) new SVGFilter( rSMgr );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL SVGFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return SVGFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return SVGFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL SVGFilter::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return SVGFilter_getSupportedServiceNames();
}
