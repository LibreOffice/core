/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoOutlineView.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:55:28 $
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
#include "precompiled_sd.hxx"

#include "SdUnoOutlineView.hxx"

#include "DrawController.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "sdpage.hxx"
#include "unopage.hxx"

#include <cppuhelper/proptypehlp.hxx>
#include <svx/unopage.hxx>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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

        default:
            throw beans::UnknownPropertyException();
    }

    return aValue;
}




} // end of namespace sd
