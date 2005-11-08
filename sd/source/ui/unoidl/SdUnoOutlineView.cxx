/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoOutlineView.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 09:05:39 $
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

#include "SdUnoOutlineView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "unohelp.hxx"
#include "unopage.hxx"
#include "unomodel.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#include <cppuhelper/proptypehlp.hxx>

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



namespace sd {

//----------------------------------------------------------------------
//------ SdUnoOutlineView--------------------------------------------------
//----------------------------------------------------------------------

SdUnoOutlineView::SdUnoOutlineView(
    ViewShellBase& rBase,
    OutlineViewShell& rViewShell,
    View& rView) throw()
    :   DrawController (rBase, rViewShell, rView),
        mxCurrentPage(NULL)
{
}




SdUnoOutlineView::~SdUnoOutlineView() throw()
{
    try
    {
        Reference<XComponent> xComponent (mxCurrentPage, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(this);
    }
    catch (Exception aException)
    {
    }
}




OutlineViewShell* SdUnoOutlineView::GetDrawViewShell (void) const
{
    return static_cast<OutlineViewShell*>(mpViewShell);
}




// XComponent
void SAL_CALL SdUnoOutlineView::dispose (void)
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XComponent> xComponent (mxCurrentPage, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(this);
    mxCurrentPage = NULL;

    DrawController::dispose();
}




// XTypeProvider

IMPLEMENT_GET_IMPLEMENTATION_ID(SdUnoOutlineView);





// XServiceInfo


sal_Char pImplSdUnoOutlineViewService[sizeof("com.sun.star.presentation.OutlineView")] = "com.sun.star.presentation.OutlineView";

OUString SAL_CALL SdUnoOutlineView::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoOutlineView" ) );
}




sal_Bool SAL_CALL SdUnoOutlineView::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return ServiceName.equalsAscii( pImplSdUnoOutlineViewService );
}




Sequence< OUString > SAL_CALL SdUnoOutlineView::getSupportedServiceNames(  ) throw(RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoOutlineViewService ) );
    Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}





// XSelectionSupplier


sal_Bool SAL_CALL SdUnoOutlineView::select( const Any& aSelection )
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



// XDrawView


void SAL_CALL SdUnoOutlineView::setCurrentPage (
    const Reference< drawing::XDrawPage >& xPage)
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT (mpView!=NULL, "View is NULL in SdUnoDrawView::setCurrentPage");

    if (mpViewShell != NULL)
    {
        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
        SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

        OutlineViewShell* pOutlineViewShell = GetDrawViewShell();
        if (pSdrPage != NULL && pOutlineViewShell != NULL)
            pOutlineViewShell->SetCurrentPage(dynamic_cast<SdPage*>(pSdrPage));
    }
}




Reference< drawing::XDrawPage > SAL_CALL SdUnoOutlineView::getCurrentPage (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    Reference<drawing::XDrawPage>  xPage;

    if (mpViewShell != NULL)
    {
        SdPage* pPage = mpViewShell->getCurrentPage();
         if (pPage != NULL)
             xPage = Reference<drawing::XDrawPage>::query(pPage->getUnoPage());
    }

    return xPage;
}




/**
 * All Properties of this implementation. Must be sorted by name.
 */
void SdUnoOutlineView::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    DrawController::FillPropertyTable (rProperties);

    rProperties.push_back (
        beans::Property( OUString(
            RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),
            PROPERTY_CURRENTPAGE,
            ::getCppuType((const Reference< drawing::XDrawPage > *)0),
            beans::PropertyAttribute::BOUND));
}




// Return sal_True, value change
sal_Bool SdUnoOutlineView::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bResult = sal_False;

    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
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
            bResult = DrawController::convertFastPropertyValue
                (rConvertedValue, rOldValue, nHandle, rValue);
            break;
    }

    return bResult;
}



/**
 * only set the value.
 */
void SdUnoOutlineView::setFastPropertyValue_NoBroadcast (
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::uno::Exception)
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
        {
            Reference< drawing::XDrawPage > xPage;
            rValue >>= xPage;
            setCurrentPage( xPage );
        }
        break;

        default:
            DrawController::setFastPropertyValue_NoBroadcast (nHandle, rValue);
            break;
    }
}




void SAL_CALL SdUnoOutlineView::disposing (const ::com::sun::star::lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (rEventObject.Source == mxCurrentPage)
        FireSwitchCurrentPage(NULL);
}




void SdUnoOutlineView::getFastPropertyValue(
    Any & rRet,
    sal_Int32 nHandle ) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    if (mpViewShell != NULL)
    {
        switch( nHandle )
        {
            case PROPERTY_CURRENTPAGE:
            {
                SdPage* pPage = const_cast<OutlineViewShell*>(
                    static_cast<const OutlineViewShell*>(mpViewShell)
                    )->GetActualPage();

                if (pPage != NULL)
                    rRet <<= pPage->getUnoPage();
            }
            break;

            default:
                DrawController::getFastPropertyValue (rRet, nHandle);
        }
    }
}




void SdUnoOutlineView::FireSwitchCurrentPage (SdPage* pCurrentPage) throw()
{

    Reference<drawing::XDrawPage> xNewPage;
    if (pCurrentPage != NULL)
        xNewPage = Reference<drawing::XDrawPage>(pCurrentPage->getUnoPage(), UNO_QUERY);

    if (xNewPage != mxCurrentPage)
    {
        Any aNewValue (makeAny(xNewPage));
        Any aOldValue (makeAny(mxCurrentPage));
        FirePropertyChange (PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

        Reference<XComponent> xComponent (mxCurrentPage, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(this);

        mxCurrentPage = xNewPage;

        xComponent = Reference<XComponent>(mxCurrentPage, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);
    }
}




} // end of namespace sd
