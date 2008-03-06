/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RangeSelectionListener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:47:12 $
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
#ifndef CHART2_RANGESELECTIONLISTENER_HXX
#define CHART2_RANGESELECTIONLISTENER_HXX

#include "ControllerLockGuard.hxx"

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XRANGESELECTIONLISTENER_HPP_
#include <com/sun/star/sheet/XRangeSelectionListener.hpp>
#endif

namespace chart
{

class RangeSelectionListenerParent
{
public:
    virtual void listeningFinished( const ::rtl::OUString & rNewRange ) = 0;
    virtual void disposingRangeSelection() = 0;
};

// ----------------------------------------

class RangeSelectionListener : public
    ::cppu::WeakImplHelper1<
        ::com::sun::star::sheet::XRangeSelectionListener >
{
public:
    explicit RangeSelectionListener(
        RangeSelectionListenerParent & rParent,
        const ::rtl::OUString & rInitialRange,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xModelToLockController );
    virtual ~RangeSelectionListener();

protected:
    // ____ XRangeSelectionListener ____
    virtual void SAL_CALL done( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL aborted( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    RangeSelectionListenerParent & m_rParent;
    ::rtl::OUString m_aRange;
    ControllerLockGuard m_aControllerLockGuard;
};

} //  namespace chart

// CHART2_RANGESELECTIONLISTENER_HXX
#endif
