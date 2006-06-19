/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dispatchinformationprovider.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:50:12 $
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

#ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHINFORMATIONPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          a helper to merge dispatch informations of different sources together.
*//*-*************************************************************************************************************/
class DispatchInformationProvider : public  css::frame::XDispatchInformationProvider
                                  , private ThreadHelpBase
                                  , public  ::cppu::OWeakObject
{
    //_______________________
    // member
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //_______________________
    // interface
    public:

        DispatchInformationProvider(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame);

        virtual ~DispatchInformationProvider();

        FWK_DECLARE_XINTERFACE

        virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups()
            throw (css::uno::RuntimeException);

        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
            throw (css::uno::RuntimeException);

    //_______________________
    // helper
    private:

        css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > implts_getAllSubProvider();

}; // class DispatchInformationProvider

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_
