/*************************************************************************
 *
 *  $RCSfile: contenthandlerfactory.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-07-02 13:34:10 $
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

#ifndef __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_
#define __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
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

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
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
    @short      factory to create handler-objects
    @descr      These class can be used to create new handler for specified contents.
                We use cached values of our configuration to lay down, which handler match
                a given URL or handlername. (use service TypeDetection to do that)
                With a detected type name you can search a registered handler and create it with these factory.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XMultiServiceFactory
                XNameContainer
                XNameReplace
                XNameAccess
                XElementAccess
                XFlushable

    @base       ThreadHelpBase
                OWeakObject

    @devstatus  ready to use
    @threadsafe yes
*//*-*************************************************************************************************************/

class ContentHandlerFactory :   // interfaces
                        public  css::lang::XTypeProvider            ,
                        public  css::lang::XServiceInfo             ,
                        public  css::lang::XMultiServiceFactory     ,
                        public  css::container::XNameContainer      ,       // => XNameReplace => XNameAccess => XElementAccess
                        public  css::util::XFlushable               ,
                        // base classes
                        // Order is neccessary for right initialization of it!
                        private ThreadHelpBase                      ,
                        private TransactionBase                     ,
                        public  ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                 ContentHandlerFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~ContentHandlerFactory(                                                                        );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XMultiServiceFactory
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance             ( const ::rtl::OUString&                        sTypeName   ) throw( css::uno::Exception        ,
                                                                                                                                                                     css::uno::RuntimeException );
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString&                        sTypeName   ,
                                                                                                  const css::uno::Sequence< css::uno::Any >&    lArguments  ) throw( css::uno::Exception        );
        virtual css::uno::Sequence< ::rtl::OUString >       SAL_CALL getAvailableServiceNames   (                                                           ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XNameContainer
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL insertByName( const ::rtl::OUString&  sHandlerName        ,
                                            const css::uno::Any&    aHandlerProperties  ) throw( css::lang::IllegalArgumentException     ,
                                                                                                 css::container::ElementExistException   ,
                                                                                                 css::lang::WrappedTargetException       ,
                                                                                                 css::uno::RuntimeException              );
        virtual void SAL_CALL removeByName( const ::rtl::OUString&  sHandlerName        ) throw( css::container::NoSuchElementException  ,
                                                                                                 css::lang::WrappedTargetException       ,
                                                                                                 css::uno::RuntimeException              );

        //---------------------------------------------------------------------------------------------------------
        //  XNameReplace
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL replaceByName( const ::rtl::OUString& sHandlerName       ,
                                             const css::uno::Any&   aHandlerProperties ) throw( css::lang::IllegalArgumentException    ,
                                                                                                css::container::NoSuchElementException ,
                                                                                                css::lang::WrappedTargetException      ,
                                                                                                css::uno::RuntimeException             );

        //---------------------------------------------------------------------------------------------------------
        //  XNameAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any                         SAL_CALL getByName      ( const ::rtl::OUString& sName ) throw( css::container::NoSuchElementException ,
                                                                                                                      css::lang::WrappedTargetException      ,
                                                                                                                      css::uno::RuntimeException             );
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(                              ) throw( css::uno::RuntimeException             );
        virtual sal_Bool                              SAL_CALL hasByName      ( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException             );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );
        virtual sal_Bool       SAL_CALL hasElements   () throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XFlushable
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL flush              (                                                                   ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL addFlushListener   ( const css::uno::Reference< css::util::XFlushListener >& xListener ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL removeFlushListener( const css::uno::Reference< css::util::XFlushListener >& xListener ) throw ( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool implcp_ContentHandlerFactory        (   const   css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory            );
        static sal_Bool implcp_createInstanceWithArguments  (   const   ::rtl::OUString&                                        sTypeName           ,
                                                                  const css::uno::Sequence< css::uno::Any >&                    lArguments          );
        static sal_Bool implcp_getByName                    (   const   ::rtl::OUString&                                        sName               );
        static sal_Bool implcp_hasByName                    (   const   ::rtl::OUString&                                        sName               );
        static sal_Bool implcp_removeByName                 (   const   ::rtl::OUString&                                        sHandlerName        );

    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory              ;
        FilterCache                                                 m_aCache                ;
//      ::cppu::OMultiTypeInterfaceContainerHelper                  m_aListenerContainer    ;

};      //  class ContentHandlerFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_
