/*************************************************************************
 *
 *  $RCSfile: frameloaderfactory.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
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
