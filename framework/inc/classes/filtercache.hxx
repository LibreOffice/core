/*************************************************************************
 *
 *  $RCSfile: filtercache.hxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:49:46 $
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

/*TODO
    - late init
    - order by number!
    - insert default detector and loader as last ones in hashes ... don't hold it as an extra member!
      => CheckedIterator will be obsolete!
 */

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#define __FRAMEWORK_CLASSES_FILTERCACHE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_
#include <classes/filtercachedata.hxx>
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

#ifndef __FRAMEWORK_QUERIES_H_
#include <queries.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifdef ENABLE_GENERATEFILTERCACHE
    #ifndef _RTL_USTRBUF_HXX_
    #include <rtl/ustrbuf.hxx>
    #endif
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
    @short          cache for all filter and type information
    @descr          Frameloader- and filterfactory need some informations about our current registered filters and types.
                    For better performance its neccessary to cache all needed values.

    @implements     -
    @base           ThreadHelpBase
                    TransactionBase

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/

class FilterCache   :   private ThreadHelpBase
                    ,   private TransactionBase
{
    public:

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        FilterCache( sal_Int32 nVersion = DEFAULT_FILTERCACHE_VERSION,
                     sal_Int16 nMode    = DEFAULT_FILTERCACHE_MODE   );

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      This will clear the cache if last owner release it.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~FilterCache();

        void flush( DataContainer::ECFGType eType );

        /*-****************************************************************************************************//**
            @short      get the current state of the cache
            @descr      Call this methods to get information about the state of the current cache.

            @seealso    -

            @param      -
            @return     -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool isValidOrRepairable() const;
        sal_Bool hasTypes           () const;
        sal_Bool hasFilters         () const;
        sal_Bool hasDetectors       () const;
        sal_Bool hasLoaders         () const;
        sal_Bool hasContentHandlers () const;

        /*-****************************************************************************************************//**
            @short      search routines to find items which match given parameter
            @descr      Mostly we search for a type first and get all informations about filter, detector and loader
                        services from the other configuration tables which are registered for this type.
                        These operations support a FindFirst/Next mechanism.
                        If you call search...( ... nStartEntry=0 ... ) we search for
                        the first entry. If these return a value different from <empty> you can work with these value.
                        If found value isn't the right one - you can call search method again.
                        DONT'T CHANGE THE VALUE OF "rStartEntry" between two search calls!
                        You can use returned value as parameter for getBy...Name() functions of this implementation too!

            @attention  returned type name is an internal name
                        returned filter name is an internal name
                        returned loader name is an implementation name of a service
                        returned detector name is an implementation name of a service

            @seealso    -

            @param      "sResult", name of found type, filter, ...
            @return     true, if search was successful,
                        false, otherwise.

            @onerror    We return false.
        *//*-*****************************************************************************************************/

        sal_Bool searchType                     (   const   ::rtl::OUString&            sURL                ,
                                                    const   ::rtl::OUString&            sMediaType          ,
                                                    const   ::rtl::OUString&            sClipboardFormat    ,
                                                            CheckedTypeIterator&        aStartEntry         ,
                                                            ::rtl::OUString&            sResult             ) const;

        sal_Bool searchFilterForType            (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            ::rtl::OUString&            sResult             ) const;

        sal_Bool searchDetectorForType          (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            ::rtl::OUString&            sResult             ) const;

        sal_Bool searchLoaderForType            (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            ::rtl::OUString&            sResult             ) const;

        sal_Bool searchContentHandlerForType    (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            ::rtl::OUString&            sResult             ) const;

        /*-****************************************************************************************************//**
            @short      get all properties of a cache entry by given name
            @descr      If you need additional informations about our internal cache values
                        you can use these methods to get a list of all cached config values
                        and subkeys of specified entry.

            @seealso    -

            @param      "sName", name of suspected entry in cache
            @return     A structure with valid information if item exists! An empty Any otherwise.

            @onerror    We return an empty Any.
        *//*-*****************************************************************************************************/

        css::uno::Sequence< ::rtl::OUString >               getAllTypeNames                 () const;
        css::uno::Sequence< ::rtl::OUString >               getAllFilterNames               () const;
        css::uno::Sequence< ::rtl::OUString >               getAllDetectorNames             () const;   // without default detector!
        css::uno::Sequence< ::rtl::OUString >               getAllLoaderNames               () const;   // without default loader!
        css::uno::Sequence< ::rtl::OUString >               getAllContentHandlerNames       () const;
        css::uno::Sequence< ::rtl::OUString >               getAllDetectorNamesWithDefault  () const;   // default detector is last one!
        css::uno::Sequence< ::rtl::OUString >               getAllLoaderNamesWithDefault    () const;   // default loader is last one!
        ::rtl::OUString                                     getDefaultLoader                () const;

        css::uno::Sequence< css::beans::PropertyValue >     getTypeProperties               (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getFilterProperties             (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getDetectorProperties           (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getLoaderProperties             (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getContentHandlerProperties     (   const   ::rtl::OUString&    sName   ) const;

        FileType                                            getType                         (   const   ::rtl::OUString&    sName   ) const;
        Filter                                              getFilter                       (   const   ::rtl::OUString&    sName   ) const;
        Detector                                            getDetector                     (   const   ::rtl::OUString&    sName   ) const;
        Loader                                              getLoader                       (   const   ::rtl::OUString&    sName   ) const;
        ContentHandler                                      getContentHandler               (   const   ::rtl::OUString&    sName   ) const;

        sal_Bool                                            existsType                      (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsFilter                    (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsDetector                  (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsLoader                    (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsContentHandler            (   const   ::rtl::OUString&    sName   ) const;

        /*-****************************************************************************************************//**
            @short      support special query modes
            @descr      Our owner services need sometimes a special mode to query for subsets of our configuration!
                        They give us a special query string - we return right values.

            @seealso    file queries.h
            @seealso    class FilterFactory
            @seealso    class FrameLoaderFactory
            @seealso    class TypeDetection

            @param      "sName", name of query
            @return     A structure with valid information!

            @onerror    We return an empty result set.
        *//*-*****************************************************************************************************/

        css::uno::Any queryFilters( const ::rtl::OUString& sQuery ) const;

        /*-****************************************************************************************************//**
            @short      support registration of elements in current configuration
            @descr      Use this methods to add or remove items in our configuration files.
                        We use the globale configuration to do that ... in fat office "share/config/registry/..."!

                        *** structure of type properties **********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

                        *** structure of filter properties ********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            "Name"                              [string]                            internal name
                            "Type"                              [string]                            registered for these type
                            "UIName"                            [string]                            localized name for UI (valid for current locale at runtime!)
                            "UINames"                           [stringlist]                        assignment of all supported localized names to right locales
                            "DocumentService"                   [string]                            uno servicename of document services
                            "FilterService"                     [string]                            uno servicename of filter implementation
                            "Flags"                             [long]                              describe filter
                            "UserData"                          [stringlist]                        additional user data (format not fixed!)
                            "FileFormatVersion"                 [long]                              version numbher of supported files
                            "TemplateName"                      [string]                            name of template

                        *** structure of detector properties ******************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

                        *** structure of loader properties ********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

            @seealso    -

            @param      "sName"         , name of type, filter ...
            @param      "lProperties"   , values of new type, filter
            @return     state of operation as bool

            @onerror    We return false then.
        *//*-*****************************************************************************************************/

        sal_Bool addFilter    ( const ::rtl::OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool replaceFilter( const ::rtl::OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool removeFilter ( const ::rtl::OUString&                                 sName       ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);

        sal_Bool addType      ( const ::rtl::OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool replaceType  ( const ::rtl::OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool removeType   ( const ::rtl::OUString&                                 sName       ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);

        sal_Bool addDetector    ( const ::rtl::OUString&                                 sName       ,
                                  const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                        sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                             css::registry::InvalidRegistryException);
        sal_Bool replaceDetector( const ::rtl::OUString&                                 sName       ,
                                  const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                        sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                             css::registry::InvalidRegistryException);
        sal_Bool removeDetector ( const ::rtl::OUString&                                 sName       ,
                                        sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                             css::registry::InvalidRegistryException);

        sal_Bool validateAndRepair();
        sal_Bool validateAndRepairTypes();
        sal_Bool validateAndRepairFilter();
        sal_Bool validateAndRepairDetectors();
        sal_Bool validateAndRepairLoader();
        sal_Bool validateAndRepairHandler();

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool implcp_searchType                           (   const   ::rtl::OUString&                                    sURL                ,
                                                                        const   ::rtl::OUString*                                    pMediaType          ,
                                                                        const   ::rtl::OUString*                                    pClipboardFormat    ,
                                                                        const   CheckedTypeIterator&                                aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchFilterForType                  (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchDetectorForType                (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchLoaderForType                  (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchContentHandlerForType          (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_getTypeProperties                    (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getFilterProperties                  (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getDetectorProperties                (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getLoaderProperties                  (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getContentHandlerProperties          (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getType                              (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getFilter                            (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getDetector                          (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getLoader                            (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getContentHandler                    (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsType                           (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsFilter                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsDetector                       (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsLoader                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsContentHandler                 (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_addFilter                            (   const   ::rtl::OUString&                                    sName               ,
                                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties         );
        static sal_Bool implcp_replaceFilter                        (   const   ::rtl::OUString&                                    sName               ,
                                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties         );
        static sal_Bool implcp_removeFilter                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_queryFilters                         (   const   ::rtl::OUString&                                    sQuery              );

    #endif  //  #ifdef ENABLE_ASSERTIONS

    #ifdef ENABLE_COMPONENT_SELF_CHECK

    private:

        void impldbg_dumpCache();

    #endif // ENABLE_COMPONENT_SELF_CHECK

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //-------------------------------------------------------------------------------------------------------------
    private:

        static sal_Int32        m_nRefCount         ;
        static DataContainer*   m_pData             ;
        static sal_Int32        m_nVersion          ;
        static sal_Int16        m_nMode             ;

};      //  class FilterCache

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
