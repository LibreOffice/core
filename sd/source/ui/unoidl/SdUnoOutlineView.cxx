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
#include "precompiled_sd.hxx"

#include <comphelper/serviceinfohelper.hxx>

#include "SdUnoOutlineView.hxx"

#include "DrawController.hxx"
#include "OutlineViewShell.hxx"
#include "sdpage.hxx"
#include "unopage.hxx"

#include <cppuhelper/proptypehlp.hxx>
#include <svx/unopage.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using ::rtl::OUString;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



namespace sd {

SdUnoOutlineView::SdUnoOutlineView(
    DrawController& rController,
    OutlineViewShell& rViewShell,
    View& rView) throw()
    :   DrawSubControllerInterfaceBase(m_aMutex),
        mrController(rController),
        mrOutlineViewShell(rViewShell),
        mrView(rView)
{
}




SdUnoOutlineView::~SdUnoOutlineView (void) throw()
{
}




void SAL_CALL SdUnoOutlineView::disposing (void)
{
}




//----- XSelectionSupplier ----------------------------------------------------

sal_Bool SAL_CALL SdUnoOutlineView::select( const Any&  )
    throw(lang::IllegalArgumentException, RuntimeException)
{
    // todo: add selections for text ranges
    return sal_False;
}



Any SAL_CALL SdUnoOutlineView::getSelection()
    throw(RuntimeException)
{
    Any aAny;
    return aAny;
}



void SAL_CALL SdUnoOutlineView::addSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




void SAL_CALL SdUnoOutlineView::removeSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




//----- XDrawView -------------------------------------------------------------


void SAL_CALL SdUnoOutlineView::setCurrentPage (
    const Reference< drawing::XDrawPage >& xPage)
    throw(RuntimeException)
{
    SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
    SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

    if (pSdrPage != NULL)
        mrOutlineViewShell.SetCurrentPage(dynamic_cast<SdPage*>(pSdrPage));
}




Reference< drawing::XDrawPage > SAL_CALL SdUnoOutlineView::getCurrentPage (void)
    throw(RuntimeException)
{
    Reference<drawing::XDrawPage>  xPage;

    SdPage* pPage = mrOutlineViewShell.getCurrentPage();
    if (pPage != NULL)
        xPage = Reference<drawing::XDrawPage>::query(pPage->getUnoPage());

    return xPage;
}



/*
// Return sal_True, value change
sal_Bool SdUnoOutlineView::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bResult = sal_False;

    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
            {
                Reference< drawing::XDrawPage > xOldPage( getCurrentPage() );
                Reference< drawing::XDrawPage > xNewPage;
                ::cppu::convertPropertyValue( xNewPage, rValue );
                if( xOldPage != xNewPage )
                {
                    rConvertedValue <<= xNewPage;
                    rOldValue <<= xOldPage;
                    bResult = sal_True;
                }
            }
            break;

        default:
            break;
    }

    return bResult;
}
*/


void SdUnoOutlineView::setFastPropertyValue (
    sal_Int32 nHandle,
        const Any& rValue)
    throw(css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
        {
            Reference< drawing::XDrawPage > xPage;
            rValue >>= xPage;
            setCurrentPage( xPage );
        }
        break;

        default:
            throw beans::UnknownPropertyException();
    }
}




void SAL_CALL SdUnoOutlineView::disposing (const ::com::sun::star::lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException)
{
}




Any SAL_CALL SdUnoOutlineView::getFastPropertyValue (
    sal_Int32 nHandle)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    Any aValue;

    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
        {
            SdPage* pPage = const_cast<OutlineViewShell&>(mrOutlineViewShell).GetActualPage();
            if (pPage != NULL)
                aValue <<= pPage->getUnoPage();
        }
        break;
        case DrawController::PROPERTY_VIEWOFFSET:
            break;

        default:
            throw beans::UnknownPropertyException();
    }

    return aValue;
}


// XServiceInfo
OUString SAL_CALL SdUnoOutlineView::getImplementationName(  ) throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sd.SdUnoOutlineView") );
}

sal_Bool SAL_CALL SdUnoOutlineView::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

Sequence< OUString > SAL_CALL SdUnoOutlineView::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString aSN( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OutlineView") );
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

} // end of namespace sd
