/*************************************************************************
 *
 *  $RCSfile: frameloaderfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:10 $
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

#ifndef __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_
#define __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

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

struct tIMPLExtractedArguments
{
    ::rtl::OUString                                 sMimeType           ;
    sal_Int32                                       nFlags              ;
    ::rtl::OUString                                 sFilterName         ;
    sal_Int32                                       nClipboardFormat    ;
    ::rtl::OUString                                 sDetectService      ;
    css::uno::Reference< css::io::XInputStream >    xInputStream        ;

    sal_Int32                                       nValidMask          ;
};

/*-************************************************************************************************************//**
    @short      factory to create frameloader-objects
    @descr      These class can be used to create new loader for specified contents.
                We use cached values of the registry to lay down, wich frameloader match
                a given URL or filtername. To do this, we use the XMultiServiceFactory-interface.

    @ATTENTION  In a specialmode of these implementation we support a simple filterdetection.
                But there is no special interface. You must call some existing methods in another context!
                see createInstanceWithArguments() fo rfurther informations!

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XMultiServiceFactory
                XNameAccess
                XElementAccess
    @base       OMutexMember
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/

class FrameLoaderFactory    :   public css::lang::XTypeProvider             ,
                                public css::lang::XServiceInfo              ,
                                public css::lang::XMultiServiceFactory      ,
                                public css::container::XNameAccess          ,       // => XElementAccess
                                public OMutexMember                         ,       // Struct for right initalization of mutex member! Mst first of baseclasses
                                public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor
            @descr      Initialize a new instance and fill the registrycache with values.
                        To do this - xFactory must be valid!

            @seealso    class RegistryCache
            @seealso    member m_aRegistryCache

            @param      "xFactory", factory which has created us.
            @return     -

            @onerror    An ASSERTION is thrown in debug version, if xFactory is invalid or cache can't filled.
        *//*-*****************************************************************************************************/

         FrameLoaderFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      We use it to clear ouer cache.

            @seealso    class RegistryCache

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~FrameLoaderFactory();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XMultiServiceFactory
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new frameloder (without arguments!)
            @descr      We search for an agreement between given type name and our cache-content.
                        If we found some information, we create the registered frameloader for these type.
                        Otherwise - we return NULL.
                        We search from begin to end of internal loader list!

            @seealso    method impl_createFrameLoader()
            @seealso    method createInstanceWithArguments()

            @param      "sTypeName", type name of a document to filter, open or save.
            @return     A reference to a new created frameloader.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& sTypeName ) throw(  css::uno::Exception         ,
                                                                                                                                css::uno::RuntimeException  );

        /*-****************************************************************************************************//**
            @short      create a new frameloder
            @descr      We search for an agreement between given type name and our cache-content.
                        If we found some information, we create the registered frameloader for these type.
                        Otherwise - we return NULL.
                        You can give us some optional arguments to influence our search!

            @seealso    method impl_createFrameLoader()
            @seealso    method createInstance()

            @param      "sTypeName", type name of a document to filter, open or save.
            @param      "seqArguments", list of optional arguments for initializing of new frameloader.
            @return     A reference to a new created frameloader.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(   const   ::rtl::OUString&                        sTypeName   ,
                                                                                                    const   css::uno::Sequence< css::uno::Any >&    seqArguments) throw(    css::uno::Exception     ,
                                                                                                                                                                              css::uno::RuntimeException);

        /*-****************************************************************************************************//**
            @short      not supported
            @descr      Please use XNameAcces instead of these!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames() throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XNameAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return properties of registered frame loader specified by his implementation name
            @descr      Use these method to get all informations about our internal loader cache
                        and a special frame loader.
                        We return a Sequence< PropertyValue > in an Any packed as result.

                        structure of return value:
                            [0].Name    =   "Types"
                            [0].Value   =   list of supported types of these loader as [sequence< oustring >]

                            [1].Name    =   "UIName"
                            [1].Value   =   localized name of loader as [string]

            @seealso    method getElementNames()
            @seealso    method hasByName()

            @param      "sName", the name of searched frame loader (use getElementNames() to get it!)
            @return     A Sequence< PropertyValue > packed in an Any.

            @onerror    If given name not exist a NoSuchElementException is thrown.
        *//*-*****************************************************************************************************/

        virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& sName ) throw( css::container::NoSuchElementException  ,
                                                                                        css::lang::WrappedTargetException       ,
                                                                                        css::uno::RuntimeException              );

        /*-****************************************************************************************************//**
            @short      return list of all well known loader names from configuration
            @descr      Use these method to get all names of well known loader.
                        You can use it to get the properties of a loader by calling getByName()!

            @seealso    method getByName()
            @seealso    method hasByName()

            @param      -
            @return     A list of well known loader. Is static at runtime!

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      check if searched frame loader exist in configuration
            @descr      Normaly nobody need these method realy (use getElementNames() in combination with getByName()).
                        We guarantee correctness of these mechanism. There is no reason to check for existing elements then ...
                        but if you have an unknwon name and has no fun to search it in returned sequence ...
                        you can call these ...

            @seealso    method getByName()
            @seealso    method getElementNames()

            @param      "sName", implementation name of searched frame loader
            @return     sal_True if loader exist, sal_False otherwise.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return the type of retrun value of method XNameAccess::getByName()
            @descr      In these implementation its a Sequence< PropertyValue > everytime!

            @seealso    description of interface XNameAccess

            @param      -
            @return     Type of Sequence< PropertyValue >.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      return state if informations about frame loader available
            @descr      If these method return false - no information could'nt read from configuration ...
                        I think nothing will work then. Normaly we return TRUE!

            @seealso    class FilterCache!

            @param      -
            @return     sal_True if information available, sal_False otherwise.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      try to set configuration properties at created loader
            @descr      We support the old async. loader interface ( XFrameLoader ) and the new one XSynchronousFrameLoader.
                        The new one should implement a property set on which we can set his configuration values!
                        We try to cast given loader to these interface - if it's exist we set the values - otherwise not!

            @seealso    service FrameLoader
            @seealso    service SynchronousFrameLoader

            @param      "xLoader" loader with generic XInterface! (We don't know before which service type is used!)
            @param      "pLoaderInfo" configuration structure of these loader.
            @return     -

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        void impl_initializeLoader( css::uno::Reference< css::uno::XInterface >& xLoader, const TLoader* pLoaderInfo );

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

         static sal_Bool impldbg_checkParameter_FrameLoaderFactory          (   const   css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory    );
        static sal_Bool impldbg_checkParameter_createInstance               (   const   ::rtl::OUString&                                        sTypeName   );
        static sal_Bool impldbg_checkParameter_createInstanceWithArguments  (   const   ::rtl::OUString&                                        sTypeName   ,
                                                                                const   css::uno::Sequence< css::uno::Any >&                    seqArguments);
        static sal_Bool impldbg_checkParameter_getByName                    (   const   ::rtl::OUString&                                        sName       );
        static sal_Bool impldbg_checkParameter_hasByName                    (   const   ::rtl::OUString&                                        sName       );

    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory          ;
        FilterCache                                                 m_aCache            ;

};      //  class FrameLoaderFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_
