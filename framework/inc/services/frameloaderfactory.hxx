/*************************************************************************
 *
 *  $RCSfile: frameloaderfactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-01-26 08:41:06 $
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

#ifdef TF_FILTER

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

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
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

#define ANY                     ::com::sun::star::uno::Any
#define OWEAKOBJECT             ::cppu::OWeakObject
#define PROPERTYVALUE           ::com::sun::star::beans::PropertyValue
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define NOSUCHELEMENTEXCEPTION  ::com::sun::star::container::NoSuchElementException
#define WRAPPEDTARGETEXCEPTION  ::com::sun::star::lang::WrappedTargetException
#define XINPUTSTREAM            ::com::sun::star::io::XInputStream
#define XMULTISERVICEFACTORY    ::com::sun::star::lang::XMultiServiceFactory
#define XPROPERTYSET            ::com::sun::star::beans::XPropertySet
#define XSERVICEINFO            ::com::sun::star::lang::XServiceInfo
#define XTYPEPROVIDER           ::com::sun::star::lang::XTypeProvider
#define XINTERFACE              ::com::sun::star::uno::XInterface
#define XNAMEACCESS             ::com::sun::star::container::XNameAccess
#define UNOTYPE                 ::com::sun::star::uno::Type

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct tIMPLExtractedArguments
{
    OUSTRING                    sMimeType           ;
    sal_Int32                   nFlags              ;
    OUSTRING                    sFilterName         ;
    sal_Int32                   nClipboardFormat    ;
    OUSTRING                    sDetectService      ;
    REFERENCE< XINPUTSTREAM >   xInputStream        ;

    sal_Int32                   nValidMask          ;
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

class FrameLoaderFactory    :   public XTYPEPROVIDER                ,
                                public XSERVICEINFO                 ,
                                public XMULTISERVICEFACTORY         ,
                                public XNAMEACCESS                  ,       // => XElementAccess
                                public OMutexMember                 ,       // Struct for right initalization of mutex member! Mst first of baseclasses
                                public OWEAKOBJECT
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

         FrameLoaderFactory( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

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

        virtual REFERENCE< XINTERFACE > SAL_CALL createInstance( const OUSTRING& sTypeName ) throw( EXCEPTION       ,
                                                                                                      RUNTIMEEXCEPTION);

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

        virtual REFERENCE< XINTERFACE > SAL_CALL createInstanceWithArguments(   const   OUSTRING&           sTypeName   ,
                                                                                const   SEQUENCE< ANY >&    seqArguments) throw(    EXCEPTION       ,
                                                                                                                                    RUNTIMEEXCEPTION);

        /*-****************************************************************************************************//**
            @short      not supported
            @descr      Please use XNameAcces instead of these!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< OUSTRING > SAL_CALL getAvailableServiceNames() throw( RUNTIMEEXCEPTION );

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

        virtual ANY SAL_CALL getByName( const OUSTRING& sName ) throw(  NOSUCHELEMENTEXCEPTION  ,
                                                                        WRAPPEDTARGETEXCEPTION  ,
                                                                        RUNTIMEEXCEPTION        );

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

        virtual SEQUENCE< OUSTRING > SAL_CALL getElementNames() throw( RUNTIMEEXCEPTION );

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

        virtual sal_Bool SAL_CALL hasByName( const OUSTRING& sName ) throw( RUNTIMEEXCEPTION );

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

        virtual UNOTYPE SAL_CALL getElementType() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      return state if informations about frame loader available
            @descr      If these method return false - no information could'nt read from configuration ...
                        I think nothing will work then. Normaly we return TRUE!

            @seealso    class FilterCache!

            @param      -
            @return     sal_True if information available, sal_False otherwise.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasElements() throw( RUNTIMEEXCEPTION );

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

        void impl_initializeLoader( REFERENCE< XINTERFACE >& xLoader, const TLoader* pLoaderInfo );

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

         static sal_Bool impldbg_checkParameter_FrameLoaderFactory          (   const   REFERENCE< XMULTISERVICEFACTORY >&  xFactory    );
        static sal_Bool impldbg_checkParameter_createInstance               (   const   OUSTRING&                           sTypeName   );
        static sal_Bool impldbg_checkParameter_createInstanceWithArguments  (   const   OUSTRING&                           sTypeName   ,
                                                                                const   SEQUENCE< ANY >&                    seqArguments);
        static sal_Bool impldbg_checkParameter_getByName                    (   const   OUSTRING&                           sName       );
        static sal_Bool impldbg_checkParameter_hasByName                    (   const   OUSTRING&                           sName       );

    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >           m_xFactory          ;
        FilterCache                                 m_aCache            ;

};      //  class FrameLoaderFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_

#else   //  #ifdef TF_FILTER

#ifndef __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_
#define __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_REGISTRYCACHE_HXX_
#include <classes/registrycache.hxx>
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

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADERQUERY_HPP_
#include <com/sun/star/frame/XFrameLoaderQuery.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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

#define ANY                     ::com::sun::star::uno::Any
#define OWEAKOBJECT             ::cppu::OWeakObject
#define PROPERTYVALUE           ::com::sun::star::beans::PropertyValue
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define XFRAMELOADERQUERY       ::com::sun::star::frame::XFrameLoaderQuery
#define XINPUTSTREAM            ::com::sun::star::io::XInputStream
#define XMULTISERVICEFACTORY    ::com::sun::star::lang::XMultiServiceFactory
#define XPROPERTYSET            ::com::sun::star::beans::XPropertySet
#define XSERVICEINFO            ::com::sun::star::lang::XServiceInfo
#define XTYPEPROVIDER           ::com::sun::star::lang::XTypeProvider
#define XINTERFACE              ::com::sun::star::uno::XInterface

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct tIMPLExtractedArguments
{
    OUSTRING                    sMimeType           ;
    sal_Int32                   nFlags              ;
    OUSTRING                    sFilterName         ;
    sal_Int32                   nClipboardFormat    ;
    OUSTRING                    sDetectService      ;
    REFERENCE< XINPUTSTREAM >   xInputStream        ;

    sal_Int32                   nValidMask          ;
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
                XFrameLoaderQuery
                XMultiServiceFactory
                XDebugging [Exist in debug version only, if ENABLE_SERVICEDEBUG is set!]
    @base       OMutexMember
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/

//class FrameLoaderFactory  :   DERIVE_FROM_XSPECIALDEBUGINTERFACE          // => These macro will expand to nothing, if no testmode is set in debug.h!
class FrameLoaderFactory    :   public XTYPEPROVIDER                ,
                                public XSERVICEINFO                 ,
                                public XFRAMELOADERQUERY            ,
                                public XMULTISERVICEFACTORY         ,
                                public OMutexMember                 ,       // Struct for right initalization of mutex member!
                                public OWEAKOBJECT
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

         FrameLoaderFactory( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

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
//      DECLARE_XSPECIALDEBUGINTERFACE  // => These macro will expand to nothing, if no testmode is set in debug.h!

        //---------------------------------------------------------------------------------------------------------
        //  XFrameLoaderQuery
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< OUSTRING > SAL_CALL getAvailableFilterNames() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getLoaderProperties( const OUSTRING& sFilterName ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual OUSTRING SAL_CALL searchFilter( const OUSTRING& sURL, const SEQUENCE< PROPERTYVALUE >& seqArguments ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XMultiServiceFactory
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new frameloder (without arguments!)
            @descr      We search for an agreement between given servicespecifier and cache-content.
                        If we found some information, who match the specifier, we create the registered frameloader.
                        Otherwise - we return NULL.

            @seealso    method impl_createFrameLoader()
            @seealso    method createInstanceWithArguments()

            @param      "sServiceSpecifier", specifier/URL or name of searched frameloader.
            @return     A reference to a new created frameloader.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XINTERFACE > SAL_CALL createInstance( const OUSTRING& sServiceSpecifier ) throw( EXCEPTION       ,
                                                                                                            RUNTIMEEXCEPTION);

        /*-****************************************************************************************************//**
            @short      create a new frameloder
            @descr      We search for an agreement between given servicespecifier and cache-content.
                        If we found some information, who match the specifier, we create the registered frameloader.
                        Otherwise - we return NULL.
                        You can give us some optional arguments to initialize the new frameloader.

            @seealso    method impl_createFrameLoader()
            @seealso    method createInstance()

            @param      "sServiceSpecifier", specifier/URL or name of searched frameloader.
            @param      "seqArguments", list of optional arguments for initializing of new frameloader.
            @return     A reference to a new created frameloader.

            @onerror    A null reference is returned.

            @ATTENTION  Because of missing filterdetection, we HACK this method and support a SPECIALMODE!
                        If you call this method with an empty "sServiceSpecifier" and ONE string-argument - a FilterName -
                        we return a propertyset for access to all informations of these specified filter.
                        You can get alist of all supported filternames by calling of "getAvailableServiceNames()".
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XINTERFACE > SAL_CALL createInstanceWithArguments(   const   OUSTRING&           sServiceSpecifier   ,
                                                                                const   SEQUENCE< ANY >&    seqArguments        ) throw(    EXCEPTION       ,
                                                                                                                                            RUNTIMEEXCEPTION);

        /*-****************************************************************************************************//**
            @short      not supported yet
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -

            @ATTENTION  Because of missing filterdetection, we HACK this method and support a SPECIALMODE!
                        You can call these method to get names of all supported filternames for creation of new
                        frameloader! see "createInstanceWithArguments()" for further informations.
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< OUSTRING > SAL_CALL getAvailableServiceNames() throw( RUNTIMEEXCEPTION );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      search for a frame loader which match given parameter
            @descr      These method is a helper for interface methods to search a frame loader.
                        We search in ouer registrycache for matching of different parameters of a registered loader
                        with given URL. If someone found - we return the index in ouer cache of founded item!
                        Caller can use these index for faster access. If we return the name of found loader, caller
                        must search this name again if he wish to have full access to other properties of it!

            @seealso    method searchFilter()
            @seealso    method createInstanceWithArguments()
            @seealso    class RegistryCache

            @param      "sURL", servicespecifier for new frameloader
            @param      "seqArguments", list of optional parameter for new frameloader
            @return     The index of found item in cache or INVALID_CACHE_POSITION if no item was found.

            @onerror    INVALID_CACHE_POSITION is returned.
        *//*-*****************************************************************************************************/

        sal_uInt32 impl_searchFrameLoader(  const   OUSTRING&                   sURL            ,
                                            const   SEQUENCE< PROPERTYVALUE >&  seqArguments    );

        /*-****************************************************************************************************//**
            @short      extract all arguments from given property-sequence, which are important for us
            @descr      Ouer methods are called with an optional list of arguments - sometimes.
                        And we must have informations about existing arguments.
                        This is the reason for this method. They will extract all neccessary arguments from sequence
                        and collect informations about his values and his valid-state! (exist, non exist, valid ...)
                        Unknown arguments are ignored!

            @seealso    method impl_createFrameLoader()

            @param      "seqArguments", list of optional parameter to extract informations.
            @return     A structure with all informatons about existing arguments in sequence.

            @onerror    State flags of values are set to INVALID!
        *//*-*****************************************************************************************************/

        tIMPLExtractedArguments impl_extractArguments( const SEQUENCE< PROPERTYVALUE >& seqArguments );

        /*-****************************************************************************************************//**
            @short      ask special detect service of a frameloader for matching with given document parameter
            @descr      A frameloader CAN register a detect service [with an interface XExtendedFilterDetection]
                        to check given URL in a special way. He can parse the URL; look in stream or something else.
                        If he can handle these document, the return value is a valid filtername; an empty name
                        otherwise.

            @seealso    interface XExtendedFilterDetection
            @seealso    const group DetectState

            @param      "sDetectService", implementationname of detect service of frameloader
            @param      "sURL", URL for loading.
            @param      "sFilterName", possible filtername of frameloader
            @param      "seqArguments", list of optional parameter for given URL.
            @return     A valid filtername is returned.

            @onerror    An empty name is returned.
        *//*-*****************************************************************************************************/
        OUSTRING impl_extendedDetect(   const   OUSTRING&                   sDetectService  ,
                                        const   OUSTRING&                   sFilterName     ,
                                         const  OUSTRING&                   sURL            ,
                                         const  SEQUENCE< PROPERTYVALUE >&  seqArguments    );

        /*-****************************************************************************************************//**
            @short      extract the extension of a given URL
            @descr      We are a uno service and can't use special parse objects to extract informations from an URL.
                        We make itself with normal string functions! The extension is all below last "." in string
                        with no "/" before!

            @seealso    class InetURLObject (!)

            @param      "sURL", URL to extract extension.
            @return     A extracted extension.

            @onerror    An empty string is returned.
        *//*-*****************************************************************************************************/

        OUSTRING impl_getExtensionFromURL( const OUSTRING& sURL );

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

        sal_Bool impldbg_checkParameter_getLoaderProperties         (   const   OUSTRING&                           sFilterName         );
        sal_Bool impldbg_checkParameter_searchFilter                (   const   OUSTRING&                           sURL                ,
                                                                        const   SEQUENCE< PROPERTYVALUE >&          seqArguments        );
         sal_Bool impldbg_checkParameter_FrameLoaderFactoryCtor     (   const   REFERENCE< XMULTISERVICEFACTORY >&  xFactory            );
        sal_Bool impldbg_checkParameter_createInstanceWithArguments (   const   OUSTRING&                           sServiceSpecifier   ,
                                                                        const   SEQUENCE< ANY >&                    seqArguments        );

    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >           m_xFactory                  ;
        RegistryCache                               m_aRegistryCache            ;

};      //  class FrameLoaderFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_FRAMELOADERFACTORY_HXX_

#endif  //  #ifdef ... #else TF_FILTER
