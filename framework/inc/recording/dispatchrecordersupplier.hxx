/*************************************************************************
 *
 *  $RCSfile: dispatchrecordersupplier.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-04-22 13:49:10 $
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

#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_
#define __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_

//_________________________________________________________________________________________________________________
// include own things

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
// include interfaces

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDERSUPPLIER_HPP_
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.HPP>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
// include other projects

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const

//_________________________________________________________________________________________________________________
//  exported definitions

//_______________________________________________

/** @short          implement a supplier for dispatch recorder
    @descr          This supplier can be set on property "DispatchRecorderSupplier" on a frame.
                    By using of this supplier and his internal XDispatchRecorder it's possible to
                    record XDispatch::dispatch() requests.

    @threadsafe     yes
 */
class DispatchRecorderSupplier  :   // interfaces
                                    public  css::lang::XTypeProvider                ,
                                    public  css::lang::XServiceInfo                 ,
                                    public  css::frame::XDispatchRecorderSupplier   ,
                                    // baseclasses
                                    // Order is neccessary for right initialization!
                                    private ThreadHelpBase                          ,
                                    public  ::cppu::OWeakObject
{
    //___________________________________________
    // member

    private:

        //_______________________________________
        /** provided dispatch recorder of this supplier instance

            @life   Is controled from outside. Because this variable is setted
                    from there and not created internaly. But we release our
                    reference to it if we die.
         */
        css::uno::Reference< css::frame::XDispatchRecorder > m_xDispatchRecorder;

        //_______________________________________
        /** reference to the global uno service manager
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    //___________________________________________
    // uno interface

    public:

        //_______________________________________
        // XInterface, XTypeProvider, XServiceInfo

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //_______________________________________
        // XDispatchRecorderSupplier

        virtual void                                                 SAL_CALL setDispatchRecorder( const css::uno::Reference< css::frame::XDispatchRecorder >& xRecorder   ) throw (css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XDispatchRecorder > SAL_CALL getDispatchRecorder(                                                                         ) throw (css::uno::RuntimeException);
        virtual void                                                 SAL_CALL dispatchAndRecord  ( const css::util::URL&                                       aURL        ,
                                                                                                   const css::uno::Sequence< css::beans::PropertyValue >&      lArguments  ,
                                                                                                   const css::uno::Reference< css::frame::XDispatch >&         xDispatcher ) throw (css::uno::RuntimeException);

    //___________________________________________
    // native interface

    public:

         DispatchRecorderSupplier( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        ~DispatchRecorderSupplier();

};      //  class DispatchRecorderSupplier

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_
