/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoSlideView.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-21 17:26:46 $
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

#ifndef SD_UNO_SLIDE_VIEW_HXX
#define SD_UNO_SLIDE_VIEW_HXX

#ifndef SD_DRAW_SUB_CONTROLLER_HXX
#include "DrawSubController.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace controller {
class PageSelector;
} } }

namespace sd {

class DrawController;
class SlideViewShell;
class View;


/** This class implements the SlideSorterViewShell specific part of the
    controller.
 */
class SdUnoSlideView
    : public DrawSubController
{
public:
    SdUnoSlideView (
        DrawController& rController,
        slidesorter::SlideSorterViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoSlideView (void) throw();

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select (const ::com::sun::star::uno::Any& aSelection)
        throw(::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getSelection (void)
        throw(::com::sun::star::uno::RuntimeException);


    // XDrawView
    virtual void SAL_CALL setCurrentPage (
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage)
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
        getCurrentPage (void)
        throw(::com::sun::star::uno::RuntimeException);

private:
    DrawController& mrController;
    slidesorter::SlideSorterViewShell& mrSlideSorterViewShell;
    sd::View& mrView;

    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

};

} // end of namespace sd

#endif
