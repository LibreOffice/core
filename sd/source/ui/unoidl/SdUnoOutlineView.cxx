/*************************************************************************
 *
 *  $RCSfile: SdUnoOutlineView.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:49:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef SD_PREVIEW_VIEW_SHELL_HXX
#include "PreviewViewShell.hxx"
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
        mpCurrentPage(NULL)
{
}




SdUnoOutlineView::~SdUnoOutlineView() throw()
{
}




OutlineViewShell& SdUnoOutlineView::GetDrawViewShell (void) const
{
    return static_cast<OutlineViewShell&>(mrViewShell);
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
        /*      case PROPERTY_CURRENTPAGE:
            {
                Reference< drawing::XDrawPage > xPage;
                rValue >>= xPage;
                setCurrentPage( xPage );
            }
            break;
        */
        default:
            DrawController::setFastPropertyValue_NoBroadcast (nHandle, rValue);
            break;
    }
}




void SdUnoOutlineView::getFastPropertyValue(
    Any & rRet,
    sal_Int32 nHandle ) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
        {
            SdPage* pPage = const_cast<OutlineViewShell&>(
                static_cast<const OutlineViewShell&>(mrViewShell)
                ).GetActualPage();

            if (pPage != NULL)
                rRet <<= pPage->getUnoPage();
        }
        break;

        default:
            DrawController::getFastPropertyValue (rRet, nHandle);
    }
}




void SdUnoOutlineView::FireSwitchCurrentPage (SdPage* pCurrentPage) throw()
{
    if (pCurrentPage != mpCurrentPage )
    {
        Reference< drawing::XDrawPage > xNewPage( pCurrentPage->getUnoPage(), UNO_QUERY );
        Any aNewValue( makeAny( xNewPage ) );

        Any aOldValue;
        if( mpCurrentPage )
        {
            Reference< drawing::XDrawPage > xOldPage( mpCurrentPage->getUnoPage(), UNO_QUERY );
            aOldValue <<= xOldPage;
        }

        FirePropertyChange (PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

        mpCurrentPage = pCurrentPage;
    }
}




} // end of namespace sd
