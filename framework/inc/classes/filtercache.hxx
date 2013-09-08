/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/*TODO
    - late init
    - order by number!
    - insert default detector and loader as last ones in hashes ... don't hold it as an extra member!
      => CheckedIterator will be obsolete!
 */

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#define __FRAMEWORK_CLASSES_FILTERCACHE_HXX_

#include <classes/filtercachedata.hxx>
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <general.h>
#include <queries.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>

#include <rtl/ustring.hxx>

#ifdef ENABLE_GENERATEFILTERCACHE
    #include <rtl/ustrbuf.hxx>
#endif

namespace framework{

/*-************************************************************************************************************//**
    @short          cache for all filter and type information
    @descr          Frameloader - and filterfactory need some information about our current registered filters and types.
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
            @descr      Mostly we search for a type first and get all information about filter, detector and loader
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

        sal_Bool searchType                     (   const   OUString&            sURL                ,
                                                    const   OUString&            sMediaType          ,
                                                    const   OUString&            sClipboardFormat    ,
                                                            CheckedTypeIterator&        aStartEntry         ,
                                                            OUString&            sResult             ) const;

        sal_Bool searchFilterForType            (   const   OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            OUString&            sResult             ) const;

        sal_Bool searchDetectorForType          (   const   OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            OUString&            sResult             ) const;

        sal_Bool searchLoaderForType            (   const   OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            OUString&            sResult             ) const;

        sal_Bool searchContentHandlerForType    (   const   OUString&            sInternalTypeName   ,
                                                            CheckedStringListIterator&  aStartEntry         ,
                                                            OUString&            sResult             ) const;

        /*-****************************************************************************************************//**
            @short      get all properties of a cache entry by given name
            @descr      If you need additional information about our internal cache values
                        you can use these methods to get a list of all cached config values
                        and subkeys of specified entry.

            @seealso    -

            @param      "sName", name of suspected entry in cache
            @return     A structure with valid information if item exists! An empty Any otherwise.

            @onerror    We return an empty Any.
        *//*-*****************************************************************************************************/

        css::uno::Sequence< OUString >               getAllTypeNames                 () const;
        css::uno::Sequence< OUString >               getAllFilterNames               () const;
        css::uno::Sequence< OUString >               getAllDetectorNames             () const;   // without default detector!
        css::uno::Sequence< OUString >               getAllLoaderNames               () const;   // without default loader!
        css::uno::Sequence< OUString >               getAllContentHandlerNames       () const;
        css::uno::Sequence< OUString >               getAllDetectorNamesWithDefault  () const;   // default detector is last one!
        css::uno::Sequence< OUString >               getAllLoaderNamesWithDefault    () const;   // default loader is last one!
        OUString                                     getDefaultLoader                () const;

        css::uno::Sequence< css::beans::PropertyValue >     getTypeProperties               (   const   OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getFilterProperties             (   const   OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getDetectorProperties           (   const   OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getLoaderProperties             (   const   OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getContentHandlerProperties     (   const   OUString&    sName   ) const;

        FileType                                            getType                         (   const   OUString&    sName   ) const;
        Filter                                              getFilter                       (   const   OUString&    sName   ) const;
        Detector                                            getDetector                     (   const   OUString&    sName   ) const;
        Loader                                              getLoader                       (   const   OUString&    sName   ) const;
        ContentHandler                                      getContentHandler               (   const   OUString&    sName   ) const;

        sal_Bool                                            existsType                      (   const   OUString&    sName   ) const;
        sal_Bool                                            existsFilter                    (   const   OUString&    sName   ) const;
        sal_Bool                                            existsDetector                  (   const   OUString&    sName   ) const;
        sal_Bool                                            existsLoader                    (   const   OUString&    sName   ) const;
        sal_Bool                                            existsContentHandler            (   const   OUString&    sName   ) const;

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

        css::uno::Any queryFilters( const OUString& sQuery ) const;

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

        sal_Bool addFilter    ( const OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool replaceFilter( const OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool removeFilter ( const OUString&                                 sName       ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);

        sal_Bool addType      ( const OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool replaceType  ( const OUString&                                 sName       ,
                                const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);
        sal_Bool removeType   ( const OUString&                                 sName       ,
                                      sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                           css::registry::InvalidRegistryException);

        sal_Bool addDetector    ( const OUString&                                 sName       ,
                                  const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                        sal_Bool                                         bException  ) throw(css::container::ElementExistException  ,
                                                                                                             css::registry::InvalidRegistryException);
        sal_Bool replaceDetector( const OUString&                                 sName       ,
                                  const css::uno::Sequence< css::beans::PropertyValue >& lProperties ,
                                        sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                             css::registry::InvalidRegistryException);
        sal_Bool removeDetector ( const OUString&                                 sName       ,
                                        sal_Bool                                         bException  ) throw(css::container::NoSuchElementException  ,
                                                                                                             css::registry::InvalidRegistryException);

        sal_Bool validateAndRepair();
        sal_Bool validateAndRepairTypes();
        sal_Bool validateAndRepairFilter();
        sal_Bool validateAndRepairDetectors();
        sal_Bool validateAndRepairLoader();
        sal_Bool validateAndRepairHandler();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
