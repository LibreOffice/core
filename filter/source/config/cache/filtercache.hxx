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

#ifndef INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FILTERCACHE_HXX
#define INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FILTERCACHE_HXX

#include <memory>

#include "cacheitem.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/ChangesEvent.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <comphelper/documentconstants.hxx>
#include <rtl/instance.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>


namespace filter{
    namespace config{

class CacheUpdateListener;


/** @short      implements a cache, which contains all
                elements of our filter and type detection
                configuration.

    @descr      The cache itself is threadsafe implemented.
                Because it should be used as a singleton only.
                To do so please use reference mechanism as wrapper
                around this FilterCache class.

    @attention  Because we use a trick to get a full initialized
                mutex lock during initialization time (means during
                the constructor runs), the base class FilterCacheLock
                must be the first of all declared one!
                Further we make it public. So any user of this class
                can lock us from outside too.
 */
class FilterCache : public BaseLock
{

    // public types

    public:


        /** @short  identify the type of a container item.

            @descr  Because the cache interface is a generic one
                    every group of container items must be specified.
         */
        enum EItemType
        {
            E_TYPE          ,
            E_FILTER        ,
            E_FRAMELOADER   ,
            E_CONTENTHANDLER
        };


        /** @short      indicates, which items already exists inside this cache
                        and which not.

            @descr      This cache supports a 2-step load mechanism.
                        First only types (and only some special properties of every type!)
                        but no filters/frame loaders/content handlers will be readed.
                        That should be enough to work with this cache e.g. for loading
                        the first document. After this first document was loaded successfully,
                        a special "load-on-demand-thread" will be started to fill this cache
                        with ALL other information, which was not readed before.
                        Thats the second step. All operations on top of this cache will be
                        blocked then.
         */
        enum EFillState
        {
            E_CONTAINS_NOTHING          = 0,
            E_CONTAINS_STANDARD         = 1,
            E_CONTAINS_TYPES            = 2,
            E_CONTAINS_FILTERS          = 4,
            E_CONTAINS_FRAMELOADERS     = 8,
            E_CONTAINS_CONTENTHANDLERS  = 16
        };


    // private types

    private:


        /** @short      regulate, which properties of a configured item
                        will be readed.

            @descr      To perform reading of all configuration items,
                        only standard properties will be handled. At a second
                        step all optional properties will be read and added to
                        our internal structures. Of course the combination of
                        both options can be used too, to get all properties
                        at the same time.
         */
        enum EReadOption
        {
            E_READ_STANDARD = 1,
            E_READ_UPDATE   = 2,
            E_READ_ALL      = 3
        };


        /** @short      indicates the state of a configuration set item.

            @descr      Inside method flush we check:
                        <ul>
                            <li>if the item exists inside config layer but not inside our cache => REMOVED</li>
                            <li>if the item exists inside config layer and inside our cache => CHANGED</li>
                            <li>if the item does not exists inside config layer but inside our cache => ADDED.</li>
                        </ul>
         */
        enum EItemFlushState
        {
            /// indicates an unchanged item (can occur e.g. if an item was added and(!) removed before it was flushed ...
            E_ITEM_UNCHANGED = 0,
            /// indicates an item, which exists inside config layer but not inside our own cache
            E_ITEM_REMOVED = 1,
            /// indicates an item, which exists inside config layer and inside our own cache
            E_ITEM_CHANGED = 2,
            /// indicates an item, which does not exists inside config layer but inside our own cache
            E_ITEM_ADDED = 3
        };


        /** TODO document me */
        enum EConfigProvider
        {
            E_PROVIDER_TYPES = 0,
            E_PROVIDER_FILTERS = 1,
            E_PROVIDER_OTHERS = 2,
            E_PROVIDER_OLD = 3
        };


    // member

    private:


        /** @short  holds the used configuration provider alive, which
                    provides access to the list of types. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigTypes;


        /** @short  holds the used configuration provider alive, which
                    provides access to the list of filters. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigFilters;


        /** @short  holds the used configuration provider alive, which
                    provides access to the list of other values needed
                    by our type detection framework. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigOthers;


        /** @short  contains all loaded types with its properties. */
        mutable CacheItemList m_lTypes;


        /** @short  contains all loaded filters with its properties. */
        mutable CacheItemList m_lFilters;


        /** @short  contains all loaded frame loader with its properties. */
        mutable CacheItemList m_lFrameLoaders;


        /** @short  contains all loaded content handler with its properties. */
        mutable CacheItemList m_lContentHandlers;


        /** @short  optimize mapping of URL extensions to a type representation,
                    by using extensions as key and a list of internal
                    type names as value. */
        mutable CacheItemRegistration m_lExtensions2Types;


        /** @short  optimize mapping of URL pattern to a type representation,
                    by using patterns as key and a list of internal
                    type names as value. */
        mutable CacheItemRegistration m_lURLPattern2Types;


        /** @short  contains the current locale of the office and will be
                    used to work with localized configuration values. */
        OUString m_sActLocale;


        /** @short  contains status, which cache items/properties
                    was already loaded from the underlying configuration.

            @descr  This information can be used to detect missing
                    information and load it on demand.

            @see    EFillState
            @see    load()
         */
        EFillState m_eFillState;


        /** TODO document me ... */
        OUStringList m_lChangedTypes;
        OUStringList m_lChangedFilters;
        OUStringList m_lChangedFrameLoaders;
        OUStringList m_lChangedContentHandlers;

        /// standard property names for filter config keyed by EReadOption
        css::uno::Sequence< OUString > m_aStandardProps[4];

        /// type property names for filter config keyed by EReadOption
        css::uno::Sequence< OUString > m_aTypeProps[4];

        /// readonly access to the module configuration of OOo
        css::uno::Reference< css::container::XNameAccess > m_xModuleCfg;

        rtl::Reference< CacheUpdateListener > m_xTypesChglisteners;
        rtl::Reference< CacheUpdateListener > m_xFiltersChgListener;


    // interface

    public:


        // ctor/dtor

        /** @short  standard ctor

            @descr  It's not allowed to do anything here...
                    especially is forbidden to start operations,
                    which needs a FilterCache instance too!
                    Why? Because this FilterCache instance will be
                    used as a singleton! And if during this ctor any
                    action related to this FilterCache singleton is
                    started... a race will be the result.

                    The first method after construction of a new
                    singleton reference should be "load()". There
                    a special fill state of this cache can be forced.
         */
        FilterCache();


        /** @short  standard dtor.
         */
        ~FilterCache();


        /** @short  creates a copy of this container.

            @descr  Such copy can be used then to modify items (add/change/remove)
                    without the risk to damage the original container.
                    After its changed data was flushed to the configuration it can be
                    removed.

                    The original container will get these new data automatically
                    because it listen for changes on the internal used configuration layer.
                    If the new data are needed immediately inside the original container,
                    the method takeOver() can be used to copy all changes back.
                    The may be following notifications of the configuration will be superfluous then.
                    But they can't be stopped...

                    All internal structures will be copied here. But the internal used
                    configuration (update) access won't be copied. The cloned instance contains
                    a different one.
         */
        std::unique_ptr<FilterCache> clone() const;


        /** @short  copy the cache content or rClone back to this instance.
         */
        void takeOver(const FilterCache& rClone);


        /** @short      force special fill state of this cache.

            @descr      This method checks, if all requested items/properties already
                        exist. Only missing information will be readed.
                        Otherwise this method does nothing!

                        This method must be called from every user of this cache
                        every time it needs a filled cache. Normally we load
                        only standard information into this cache on startup.

            @throw      An exception if the cache could not be filled really
                        or seems to be invalid afterwards. But there is no reaction
                        at all if this method does nothing inside, because the cache
                        is already fully filled!
         */
        void load(EFillState eRequired);


        /** @short      return the current fill state of this cache.

            @descr      This information can be used e.g. to start
                        a search on top of this cache with a minimum on
                        information ... and do it again, if some other
                        cache items seems to be available after calling of "loadAll()"
                        on this cache and first search does not had any valid results.

            @return     sal_True if the required fill state exists for this cache; FALSE
                        otherwise.

            @throws css::uno::Exception
         */
        bool isFillState(EFillState eRequired) const;


        /** @short      return a list of key names for items, which match
                        the specified criteria.

            @descr      The returned key names can be used at another method "getItem()"
                        of this cache to get further information about this item.

            @attention  Please note: because this cache can be used inside multithreaded
                        environments, such returned key name can point to an already removed
                        item! Please be aware of some "NoSuchElementExceptions" if you try to
                        call any other method of this cache in relation to this key name.

            @param      eType
                        specify the sub container of this cache, which should be used for
                        searching. see also EItemType.

            @param      lIProps
                        specify the property set, which must exist at the searched items
                        as minimum.

            @param      lEProps
                        specify the property set, which must not(!) exist at the searched items
                        as minimum.

            @return     [OUStringList]
                        a list of key names, which identify items of the queried sub container.
                        May be an empty list.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        OUStringList getMatchingItemsByProps(      EItemType  eType                ,
                                                     const CacheItem& lIProps              ,
                                                     const CacheItem& lEProps = CacheItem()) const;


        /** @short      indicates if the requested sub container
                        contains some items.

            @descr      We don't provide any information about the count
                        of such items. Because we don't implement any index
                        based interface! The information "we have items or not"
                        must be enough for the outside code ... till somewhere
                        give us a good reason. :-)

            @param      eType
                        specify the sub container of this cache, which should be used.
                        see also EItemType.

            @return     [sal_Bool]
                        True, if the requested sub container contains some items;
                        False otherwise.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        bool hasItems(EItemType eType) const;


        /** @short      return a list of all key names, which represent
                        an item inside the specified sub container.

            @attention  Please note: because this cache can be used inside multithreaded
                        environments, such returned key names can point to some already removed
                        items! Please be aware of some "NoSuchElementExceptions" if you try to
                        call any other method of this cache in relation to this key names.

            @param      eType
                        specify the sub container of this cache, which should be used for
                        searching. see also EItemType.

            @return     [OUStringList]
                        a list of key names, which can be used to access the item properties
                        using some other methods of this cache.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        OUStringList getItemNames(EItemType eType) const;


        /** @short      check if the required item exist inside this container.

            @attention  This method exists to supports some UNO container interfaces
                        only. (e.g. XNameAccess.hasByName()). But inside multithreaded
                        environments there is no guarantee, that this item still exists, if
                        its really requested e.g. by calling getItem()!
                        Be aware of some NoSuchElementExistExceptions ...

            @param      eType
                        specify the sub container of this cache, which should be used.
                        see also EItemType.

            @param      sItem
                        the key name of the requested item inside the specified sub container.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        bool hasItem(      EItemType        eType,
                                 const OUString& sItem);


        /** @short      return an item, which match the specified type and name.

            @descr      Because this cache can be used inside multithreaded environments
                        the caller must be aware of some exceptions - especially a "NoSuchElementExcepotion".
                        May another thread already removed the required item before ...

            @param      eType
                        specify the sub container of this cache, which should be used for
                        searching. see also EItemType.

            @param      sItem
                        specify the requested item by its key name.

            @return     [CacheItem]
                        the required item if it could be located ...
                        But we throw an exception if the required item does not exist!

            @throw      [css::container::NoSuchElementException]
                        if the required item does not still exist.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        CacheItem getItem(      EItemType        eType,
                                  const OUString& sItem);


        /** TODO document me ...

            @throws css::uno::Exception
         */
        void removeItem(      EItemType        eType,
                                const OUString& sItem);


        /** TODO document me ...

            @throws css::uno::Exception
         */
        void setItem(      EItemType        eType ,
                             const OUString& sItem ,
                             const CacheItem&       aValue);


        /** TODO document me ...

            @throws css::uno::Exception
         */
        void refreshItem(      EItemType        eType,
                                 const OUString& sItem);


        /** @short      add some implicit properties to the given
                        cache item reference.

            @descr      Such properties can e.g. finalized or mandatory.
                        They are not persistent  and not really part of e.g. a
                        filter not. But they are attributes of a configuration
                        entry and can influence our container interface.

            @attention  These properties are not part of the normal CacheItem
                        returned by the method getItem(). Because getItem() is
                        used internally too but these specialized properties
                        are needed at our container services only. So these
                        function sets are different to allow different handling.

            @param      eType
                        specify the sub container of this cache, which should be used for
                        searching. see also EItemType.

            @param      sItem
                        specify the requested item by its key name.

            @param      rItem
                        contains already the normal properties of this item,
                        and will be used as out parameter to add the implicit
                        attributes there.

            @throw      [css::uno::Exception]
                        if an internal error occurred.
                        Note: if the item is missing inside the underlying configuration
                        no exception will be thrown. In such case the item is marked as
                        finalized/mandatory automatically
                        Reason: maybe the item comes from the old configuration package and
                        was not migrated to the new one. So we can't provide write access
                        to such items...
         */
        void addStatePropsToItem(      EItemType        eType,
                                         const OUString& sItem,
                                               CacheItem&       rItem);


        /** TODO document me

            @throws css::uno::Exception
         */
        static void removeStatePropsFromItem(CacheItem& aValue);


        /** @short      force writing of all changes (which was made after
                        last flush was called) back to the configuration.

            @descr      TODO

            @throw      [css::uno::Exception]
                        if the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        void flush();


        /** @short      supports a flat type detection for given URL.

            @descr      Because such detection works on our optimized internal
                        structures (e.g. mapping from extensions/pattern to type names),
                        it should be made inside this cache.

            @param      aURL
                        URL of the content, which type should be detected.
                        Its already parsed and splitted into its different parts,
                        like e.g.: main, jump marks etcpp.

            @param      rFlatTypes
                        used as [out] parameter to add all types, which match to the given
                        URL. Further an information is added for every type. It indicates, how
                        this type is related to the specified URL (means e.g. if it matches
                        by extension or URLPattern ...).

            @attention  Please note: because this cache can be used inside multithreaded
                        environments, such returned key names can point to some already removed
                        items! Please be aware of some "NoSuchElementExceptions" if you try to
                        call any other method of this cache in relation to this key names.

            @throw      [css::uno::Exception]
                        if the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        void detectFlatForURL(const css::util::URL& aURL      ,
                                            FlatDetection&  rFlatTypes) const;


    // private helper

    private:


        /** @short      return a reference to one of our internal
                        sub container, which contains items of the
                        requested type.

            @param      eType
                        specify, which sub container is needed outside.

            @return     [CacheItemList&]
                        a reference(!) to the right sub container member.

            @throw      [css::uno::Exception]
                        if the required list does not exist.
         */
        const CacheItemList& impl_getItemList(EItemType eType) const;

        CacheItemList& impl_getItemList(EItemType eType);


        /** @short      return a valid configuration update access
                        to the underlying configuration package, which
                        is fix for this cache.

            @descr      It checks first, if the internal member m_xConfig already
                        points to an open update access. If not - it opens a new one.
                        Doing so this method can be called every time a configuration
                        access is needed.

            @param      eProvider
                        specify the needed configuration provider.
                        see EConfigProvider for further information ...

            @throws css::uno::Exception

            @attention  If a configuration access was opened successfully
                        all necessary listener connections will be established
                        too. So this cache will be informed about outside updates.
         */
        css::uno::Reference< css::uno::XInterface > impl_openConfig(EConfigProvider eProvide);


        /** @short      tries to open the requested configuration root
                        using the specified modi.

            @param      sRoot
                        specify the configuration root, which should be opened.

            @param      bReadOnly
                        enable/disable write access on the returned configuration
                        object.

            @param      bLocalesMode
                        enable/disable special handling of localized configuration
                        items by the returned configuration object.

            @return     A valid reference, if the configuration access could be opened
                        and initialized within the requested modes successfully;
                        a NULL reference otherwise.
         */
        css::uno::Reference< css::uno::XInterface > impl_createConfigAccess(const OUString& sRoot       ,
                                                                                  bool         bReadOnly   ,
                                                                                  bool         bLocalesMode);


        /** @short      reads the specified configuration key
                        and return its value.

            @descr      The specified key must be an absolute configuration path,
                        which can be splitted into its package and relative path tokens.

            @attention  Because this function might opens a new configuration
                        read access for reading one key value only, it should
                        be used in rare cases only. It's an easy way... but an
                        expensive one.

            @param      sDirectKey
                        the absolute configuration path, which should be readed.

            @return     [css::uno::Any]
                        the value of the requested key.
                        Can be empty if an internal error occurred or if the requested
                        key does not exists!
         */
        css::uno::Any impl_getDirectCFGValue(const OUString& sDirectKey);


        /** @short      load the underlying configuration into this cache.

            @descr      Which items should be readed can be regulate by the
                        parameter eRequiredState. That provides the possibility
                        to load standard values on startup only and update this
                        cache later on demand with all available information.

            @param      eRequiredState
                        indicates, which fill state this cache should have afterwards.

            @throws css::uno::Exception
         */
        void impl_load(EFillState eRequiredState);


        /** @short      validate the whole cache and create
                        structures for optimized items access.

            @descr      Wrong cache items will be removed automatically.
                        Wrong dependencies will be corrected automatically.
                        If something could not be repaired - an exception
                        is thrown.
                        Further some optimized structures will be created.
                        E.g.: a hash to map extensions to her types.

            @attention  There is no exception, if the cache could be repaired
                        but contained wrong elements before!

            @throw      [css::uno::Exception]
                        if cache is invalid and could not be repaired.
         */
        void impl_validateAndOptimize();

    private:


        /** @short  read the specified config set into the cache.

            @descr  This method provides the following mechanism for reading:
                    a) read only standard properties of set items
                    b) read anything
                    c) read only optional properties and update already existing
                       items of the specified cache

            @param  xConfig
                    API which provides access to the required configuration set.

            @param  eType
                    specify the type of config item, which must be interpreted.
                    Of course this information can be used to locate the right set
                    at the given xConfig API object.

            @param  eOption
                    regulate reading of standard/optional or all properties.

            @param  pCache
                    points to the cache member, which should be filled or updated.

            @throw  [css::uno::Exception]
                    if an unrecoverable error occurs inside this operation.
         */
        void impl_loadSet(const css::uno::Reference< css::container::XNameAccess >& xConfig,
                                EItemType                                           eType  ,
                                EReadOption                                         eOption,
                                CacheItemList*                                      pCache );


        /** @short  read the specified container item from the given configuration set.

            @descr  It's not added to any internal structures here. That must be done
                    outside this method.

            @param  xSet
                    provides access to the configuration set, which includes all items.

            @param  eType
                    specify, which container item type must be readed.

            @param  sItem
                    means the internal name, which can be used to address the item
                    properties relative to the given configuration set.

            @param  eOption
                    regulate, which properties of the requested item should be read.
                    See definition of EReadOption for further information.

            @throw  [css::uno::Exception]
                    if an unrecoverable error occurs inside this operation.
         */
        CacheItem impl_loadItem(const css::uno::Reference< css::container::XNameAccess >& xSet   ,
                                      EItemType                                           eType  ,
                                const OUString&                                    sItem  ,
                                      EReadOption                                         eOption);


        /** @short  try to load the requested item on demand from the underlying configuration
                    layer.

            @descr  The outside code has to be sure, that the item does not already exists
                    inside this cache. Otherwise it will be loaded twice. This method
                    doesn't check such constellations!

            @param  eType
                    specify the type of config item, which must be interpreted.
                    Of course this information can be used to locate the right set
                    at the given xConfig API object.

            @param  sItem
                    the set node name of the requested item.

            @return An iterator, which points directly to the new cached item.
                    Is a valid iterator if no exception occurred here!
                    But to improve robustness - it should be checked :-)

            @throw  [css::container::NoSuchElementException]
                    if the item does not exists inside the configuration layer too!

            @throw  [css::uno::Exception]
                    if an unrecoverable error occurs inside this operation.
         */
        CacheItemList::iterator impl_loadItemOnDemand(      EItemType        eType,
                                                      const OUString& sItem);


        /** TODO

            @throws css::uno::Exception
        */
        static void impl_saveItem(const css::uno::Reference< css::container::XNameReplace >& xSet  ,
                                 EItemType                                            eType ,
                           const CacheItem & aValue);


        /** TODO

            @throws css::uno::Exception
        */
        void impl_addItem2FlushList(      EItemType        eType,
                                    const OUString& sItem);


        /** TODO

            @throws css::uno::Exception
        */
        static void impl_flushByList(const css::uno::Reference< css::container::XNameAccess >& xSet  ,
                                    EItemType                                           eType ,
                              const CacheItemList&                                      rCache,
                              const OUStringList&                                       lItems);


        /** @short  specify, which save operation is necessary for the specified item.

            @desrc  If an item of this cache will be added/removed or modified it will
                    be changed inside memory only first. But we save its name inside a special
                    list of changed items. If at least the method flush() is called, we use
                    this list to check if the item was changed/added or removed. This method
                    checks the exist state of the requested item inside our own cache
                    and inside the underlying configuration layer to find out, if the item
                    must be removed/added or modified inside the configuration layer.

            @param  xSet
                    points directly to the configuration set, where the item should resist
                    (if it exists!).

            @param  rList
                    points to our internal cache list, where the item should resist
                    (if it exists!).

            @param  sItem
                    the internal name of the item, which should be checked.

            @return An enum value of type EItemFlushState, which indicates the needed
                    API operation for updating the underlying configuration layer.

            @throws An exception if anything failed inside this operation.
                    e.g. the given configuration set was not open.
         */
        static EItemFlushState impl_specifyFlushOperation(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                                   const CacheItemList&                                      rList,
                                                   const OUString&                                    sItem);


        /** TODO

            @throws css::uno::Exception
        */
        void impl_readPatchUINames(const css::uno::Reference< css::container::XNameAccess >& xNode,
                                         CacheItem&                                          rItem);


        /** TODO

            @throws css::uno::Exception
        */
        static void impl_savePatchUINames(const css::uno::Reference< css::container::XNameReplace >& xNode,
                                   const CacheItem&                                           rItem);

        /** TODO */
        void impl_readOldFormat();

        /** TODO

            @throws css::uno::Exception
        */
        CacheItem impl_readOldItem(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                         EItemType                                           eType,
                                   const OUString&                                    sItem);


        /** TODO */
        static void impl_interpretDataVal4Type(const OUString& sValue,
                                              sal_Int32        nProp ,
                                              CacheItem&       rItem );


        /** TODO */
        static void impl_interpretDataVal4Filter(const OUString& sValue,
                                                sal_Int32        nProp ,
                                                CacheItem&       rItem );


        /** TODO */
        static OUStringList impl_tokenizeString(const OUString& sData     ,
                                               sal_Unicode      cSeparator);


#if OSL_DEBUG_LEVEL > 0
        /** TODO */
        OUString impl_searchFrameLoaderForType(const OUString& sType) const;
        OUString impl_searchContentHandlerForType(const OUString& sType) const;
#endif


        /** @short check if the specified OOo module is installed.

            @param  sModule
                    the long name of the module (e.g. "com.sun.star.text.TextDocument").

            @return sal_True if the requested module is installed; sal_False otherwise.
         */
        bool impl_isModuleInstalled(const OUString& sModule);


        /** @short  convert a list of flag names to its int representation.

            @param  lNames
                    the list of flag names.

            @return the converted flag field.
         */
        static SfxFilterFlags impl_convertFlagNames2FlagField(const css::uno::Sequence< OUString >& lNames);


        /** @short  convert a flag field value to its list representation of flag names.

            @param  nFlags
                    the flag field value

            @return [seq< string >]
                    the converted flag name list.
         */
        static css::uno::Sequence< OUString > impl_convertFlagField2FlagNames(SfxFilterFlags nFlags);
};

struct TheFilterCache: public rtl::Static<FilterCache, TheFilterCache> {};

    } // namespace config
} // namespace filter

#endif // INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FILTERCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
