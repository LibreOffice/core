/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FILTER_CONFIG_FILTERCACHE_HXX_
#define __FILTER_CONFIG_FILTERCACHE_HXX_

//_______________________________________________
// includes

#include "cacheitem.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/ChangesEvent.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

class CacheUpdateListener;

//_______________________________________________
// definitions

//_______________________________________________

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
    //-------------------------------------------
    // public types

    public:

        //---------------------------------------

        /** @short  identify the type of a container item.

            @descr  Because the cache interface is a generic one
                    every group of container items must be specified.
         */
        enum EItemType
        {
            E_TYPE          ,
            E_FILTER        ,
            E_FRAMELOADER   ,
            E_CONTENTHANDLER,
            E_DETECTSERVICE
        };

        //---------------------------------------

        /** @short      indicates, which items already exists inside this cache
                        and which not.

            @descr      This cache supports a 2-step load mechanism.
                        First only types (and only some special properties of every type!)
                        but no filters/frame loaders/content handlers will be readed.
                        That should be enough to work with this cache e.g. for loading
                        the first document. After this first document was loaded successfully,
                        a special "load-on-demand-thread" will be started to fill this cache
                        with ALL other informations, which was not readed before.
                        Thats the second step. All operations on top of this cache will be
                        blocked then.
         */
        enum EFillState
        {
            E_CONTAINS_NOTHING          = 0,
            E_CONTAINS_STANDARD         = 1,
            E_CONTAINS_TYPES            = 2,
            E_CONTAINS_FILTERS          = 4,
            E_CONTAINS_DETECTSERVICES   = 8,
            E_CONTAINS_FRAMELOADERS     = 16,
            E_CONTAINS_CONTENTHANDLERS  = 32,
            E_CONTAINS_ALL              = 63 // must be a combination of all excepting E_CONTAINS_NOTHING! Please update if items will be added or removed ...
        };

    //-------------------------------------------
    // private types

    private:

        //---------------------------------------
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
            E_READ_NOTHING  = 0,
            E_READ_STANDARD = 1,
            E_READ_UPDATE   = 2,
            E_READ_ALL      = 3
        };

        //---------------------------------------
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

        //---------------------------------------
        /** TODO document me */
        enum EConfigProvider
        {
            E_PROVIDER_TYPES = 0,
            E_PROVIDER_FILTERS = 1,
            E_PROVIDER_OTHERS = 2,
            E_PROVIDER_OLD = 3
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        mutable css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  holds the used configuration provider alive, which
                    provides access to the list of types. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigTypes;

        //---------------------------------------
        /** @short  holds the used configuration provider alive, which
                    provides access to the list of filters. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigFilters;

        //---------------------------------------
        /** @short  holds the used configuration provider alive, which
                    provides access to the list of other values needed
                    by our type detection framework. */
        mutable css::uno::Reference< css::uno::XInterface > m_xConfigOthers;

        //---------------------------------------
        /** @short  contains all loaded types with its properties. */
        mutable CacheItemList m_lTypes;

        //---------------------------------------
        /** @short  contains all well known detect service with its properties. */
        mutable CacheItemList m_lDetectServices;

        //---------------------------------------
        /** @short  contains all loaded filters with its properties. */
        mutable CacheItemList m_lFilters;

        //---------------------------------------
        /** @short  contains all loaded frame loader with its properties. */
        mutable CacheItemList m_lFrameLoaders;

        //---------------------------------------
        /** @short  contains all loaded content handler with its properties. */
        mutable CacheItemList m_lContentHandlers;

        //---------------------------------------
        /** @short  optimize mapping of URL extensions to a type representation,
                    by using extensions as key and a list of internal
                    type names as value. */
        mutable CacheItemRegistration m_lExtensions2Types;

        //---------------------------------------
        /** @short  optimize mapping of URL pattern to a type representation,
                    by using patterns as key and a list of internal
                    type names as value. */
        mutable CacheItemRegistration m_lURLPattern2Types;

        //---------------------------------------
        /** @short  contains the current locale of the office and will be
                    used to work with localized configuration values. */
        ::rtl::OUString m_sActLocale;

        //---------------------------------------
        /** TODO */
        ::rtl::OUString m_sFormatName;

        //---------------------------------------
        /** TODO */
        ::rtl::OUString m_sFormatVersion;

        //---------------------------------------
        /** @short  contains status, which cache items/properties
                    was already loaded from the underlying configuration.

            @descr  This information can be used to detect missing
                    informations and load it on demand.

            @see    EFillState
            @see    load()
         */
        EFillState m_eFillState;

        //---------------------------------------
        /** TODO document me ... */
        OUStringList m_lChangedTypes;
        OUStringList m_lChangedFilters;
        OUStringList m_lChangedDetectServices;
        OUStringList m_lChangedFrameLoaders;
        OUStringList m_lChangedContentHandlers;

        //---------------------------------------
        /// readonly acccess to the module configuration of OOo
        css::uno::Reference< css::container::XNameAccess > m_xModuleCfg;

        rtl::Reference< CacheUpdateListener > m_xTypesChglisteners;
        rtl::Reference< CacheUpdateListener > m_xFiltersChgListener;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor

            @descr  Its not allowed to do anything here ...
                    especialy is forbidden to start operations,
                    which needs a FilterCache instance too!
                    Why? Because thie FilterCache instance will be
                    used as a singleton! And if during this ctor any
                    action related to this FilterCache singleton is
                    started ... a race will be the result.

                    The first method after construction of a new
                    singleton reference should be "load()". There
                    a special fill state of this cache can be forced.
         */
        FilterCache();

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~FilterCache();

        //---------------------------------------

        /** @short  creates a copy of this container.

            @descr  Such copy can be used then to modify items (add/change/remove)
                    without the risk to damage the original container.
                    After its changed data was flushed to the configuration it can be
                    removed.

                    The original container will get these new data automaticly
                    because it listen for changes on the internal used configuration layer.
                    If the new data are needed immediately inside the original container,
                    the method takeOver() can be used to copy all changes back.
                    The may be following notifications of the configuration will be superflous then.
                    But they cant be stopped ...

                    All internal structures will be copied here. But the internal used
                    configuration (update) access wont be copied. The cloned instance contains
                    a different one.

            @note   The cloned instance is created on the heap. The user of this instance
                    has to remove it later.
         */
        virtual FilterCache* clone() const;

        //---------------------------------------

        /** @short  copy the cache content or rClone back to this instance.
         */
        virtual void takeOver(const FilterCache& rClone);

        //---------------------------------------

        /** @short      force special fill state of this cache.

            @descr      This method check if all requested items/properties already
                        exists. Only missing informations will be readed.
                        Otherwise this method does nothing!

                        This method must be called from every user of this cache
                        everytimes it need a filled cache. Normaly we load
                        only standard informations into this cache on startup.
                        After a few seconds we start a special thread, which
                        may fill this cache completely. But if somehwere outside
                        need a filled cache before ... it can run into trouble,
                        if this "load-on-demand" thread does not finished its work before.
                        This method "load(xxx)" synchronize such load-on-demand requests.

                        Of course it would be possible to supress this special load thread
                        in general and start it manualy inside this load() request.
                        The outside code decide then, if and when this cache will be filled
                        with all available informations ...

            @param      bByThread
                        indicates using of this method by our global "load-on-demand-thread".
                        Its an implementation detail! We use it to check, if this "load()"
                        request was forced e.g. by one of our derived service container (which need
                        it to full fill its own operations) or if it was forced by our own
                        "load-on-demand-thread", which tries to optimize our startup performance
                        and start this load() only in case the office startup was already finished!

            @throw      An exception if the cache could not be filled realy
                        or seems to be invalid afterwards. But there is no reaction
                        at all if this method does nothing inside, because the cache
                        is already full filled!
         */
        virtual void load(EFillState eRequired ,
                          sal_Bool   bByThread = sal_False)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      return the current fill state of this cache.

            @descr      This information can be used e.g. to start
                        a search on top of this cache with a minimum on
                        informations ... and do it again, if some other
                        cache items seems to be available after calling of "loadAll()"
                        on this cache and first search does not had any valid results.

            @return     sal_True if the required fill state exists for this cache; FALSE
                        otherwise.
         */
        virtual sal_Bool isFillState(EFillState eRequired) const
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      return a list of key names for items, which match
                        the specified criteria.

            @descr      The returned key names can be used at another method "getItem()"
                        of this cache to get further informations about this item.

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
        virtual OUStringList getMatchingItemsByProps(      EItemType  eType                ,
                                                     const CacheItem& lIProps              ,
                                                     const CacheItem& lEProps = CacheItem()) const
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      indicates if the requested sub container
                        contains some items.

            @descr      We dont provide any information about the count
                        of such items. Because we dont implement any index
                        based interface! The information "we have items or not"
                        must be enough for the outside code ... till somewhere
                        give us a good reason. :-)

            @param      eType
                        specify the sub container of this cache, which should be used.
                        see also EItemType.

            @return     [sal_Bool]
                        True, if the requested sub container contains some items;
                        False otherwhise.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        virtual sal_Bool hasItems(EItemType eType) const
            throw(css::uno::Exception);

        //---------------------------------------

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
        virtual OUStringList getItemNames(EItemType eType) const
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      check if the required item exist inside this container.

            @attention  This method exists to supports some UNO container interfaces
                        only. (e.g. XNameAccess.hasByName()). But inside multithreaded
                        environments there is no guarantee, that this item still exists, if
                        its realy requested e.g. by calling getItem()!
                        Be aware of some NoSuchElementExistExceptions ...

            @param      eType
                        specify the sub container of this cache, which should be used.
                        see also EItemType.

            @param      sItem
                        the key name of the requested item inside the pecified sub container.

            @throw      [css::uno::Exception]
                        if some input parameter are wrong or the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        virtual sal_Bool hasItem(      EItemType        eType,
                                 const ::rtl::OUString& sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      return an item, which match the specified type and name.

            @descr      Because this cache can be used inside multithreaded environments
                        the caller must be aware of some exceptions - especialy a "NoSuchElementExcepotion".
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
        virtual CacheItem getItem(      EItemType        eType,
                                  const ::rtl::OUString& sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO document me ...
         */
        virtual void removeItem(      EItemType        eType,
                                const ::rtl::OUString& sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO document me ...
         */
        virtual void setItem(      EItemType        eType ,
                             const ::rtl::OUString& sItem ,
                             const CacheItem&       aValue)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO document me ...
         */
        virtual void refreshItem(      EItemType        eType,
                                 const ::rtl::OUString& sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      add some implicit properties to the given
                        cache item reference.

            @descr      Such properties can e.g. finalized or mandatory.
                        They are not persistent  and not realy part of e.g. a
                        filter not. But they are attributes of a configuration
                        entry and can influence our container interface.

            @attention  These properties are not part of the normal CacheItem
                        returned by the method getItem(). Because getItem() is
                        used internaly too but these specialized properties
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
                        Note: If the item is missing inside the underlying configuration
                        no exception will be thrown. In such case the item is marked as
                        finalized/mandatory automaticly
                        Reason: May be the item cames from the old configuration package and
                        was not migrated to the new one. So we cant provide write access
                        to such items ...
         */
        virtual void addStatePropsToItem(      EItemType        eType,
                                         const ::rtl::OUString& sItem,
                                               CacheItem&       rItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO document me
         */
        virtual void removeStatePropsFromItem(CacheItem& aValue)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      force writing of all changes (which was made after
                        last flush was called) back to the configuration.

            @descr      TODO

            @throw      [css::uno::Exception]
                        if the cache itself is not valid
                        any longer, because any operation before damage it.
         */
        virtual void flush()
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      supports a flat type detection for given URL.

            @descr      Because such detection works on our optimized internal
                        structures (e.g. mapping from extensions/pattern to type names),
                        it should be made inside this cache.

            @param      aURL
                        URL of the content, which type should be detected.
                        Its already parsed and splitted into its differnt parts,
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
        virtual void detectFlatForURL(const css::util::URL& aURL      ,
                                            FlatDetection&  rFlatTypes) const
            throw(css::uno::Exception);

    //-------------------------------------------
    // private helper

    private:

        //---------------------------------------

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

        //---------------------------------------

        /** @short      return a valid configuration update access
                        to the underlying configuration package, which
                        is fix for this cache.

            @descr      It checks first, if the internal member m_xConfig already
                        points to an open update access. If not - it opens a new one.
                        Doing so this method can be called everytimes a configuration
                        access is needed.

            @param      eProvider
                        specify the needed configuration provider.
                        see EConfigProvider for further informations ...

            @attention  If a configuration access was opened successfully
                        all neccessary listener connections will be established
                        too. So this cache will be informed about outside updates.
         */
        css::uno::Reference< css::uno::XInterface > impl_openConfig(EConfigProvider eProvide)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      tries to open the requested configuration root
                        using the specified modi.

            @param      sRoot
                        specify the configuration root, which should be opened.

            @param      bReadOnly
                        enable/disable write access on the returned configuration
                        object.

            @param      bLocalesMode
                        enable/disable special handling of localized configuratiom
                        items by the returned configuration object.

            @return     A valid reference, if the configuration access could be opened
                        and initialized within the requested modes successfully;
                        a NULL reference otherwhise.
         */
        css::uno::Reference< css::uno::XInterface > impl_createConfigAccess(const ::rtl::OUString& sRoot       ,
                                                                                  sal_Bool         bReadOnly   ,
                                                                                  sal_Bool         bLocalesMode);

        //---------------------------------------

        /** @short      reads the specified configuration key
                        and return its value.

            @descr      The specified key must be an absolute configuration path,
                        which can be splitted into its package and relative path tokens.

            @attention  Because this function might opens a new configuration
                        read access for reading one key value only, it should
                        be used in rare cases only. Its an easy way ... but an
                        expensive one.

            @param      sDirectKey
                        the absolute configuration path, which should be readed.

            @return     [css::uno::Any]
                        the value of the requested key.
                        Can be empty if an internal error occurred or if the requested
                        key does not exists!
         */
        css::uno::Any impl_getDirectCFGValue(const ::rtl::OUString& sDirectKey);

        //---------------------------------------

        /** @short      load the underlying configuration into this cache.

            @descr      Which items should be readed can be regulate by the
                        parameter eRequiredState. That provides the possibility
                        to load standard values on startup only and update this
                        cache later on demand with all available informations.

            @param      eRequiredState
                        indicates, which fill state this cache should have afterwards.
         */
        void impl_load(EFillState eRequiredState)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      validate the whole cache and create
                        structures for optimized items access.

            @descr      Wrong cache items will be removed automaticly.
                        Wrong dependencies will be corrected automaticly.
                        If something could not be repaired - an exception
                        is thrown.
                        Further some optmized structures will be created.
                        E.g.: a hash to map extensions to her types.

            @attention  There is no exception, if the cache could be repaired
                        but contained wrong elements before!

            @throw      [css::uno::Exception]
                        if cache is invalid and could not be repaired.
         */
        void impl_validateAndOptimize()
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short      register the specified item for the given type.

            @descr      Because detect services, frame loader or content handler
                        are not listed inside the xml configuration as seperated
                        items (they are properties of any type entry!), this method update
                        the internal lists of such items. Thats neccessary to have
                        it accessible for our container interfaces of detect, frame loader
                        and content handler services.

            @param      pList
                        points to a CacheItemList of this filter cache, where
                        this item registration should be updated or added.

            @param      sItem
                        specify the detect service, frame loader or content handler,
                        which should be registered for the given type.

            @param      sType
                        contains the internal type name, where the item should be registered for.

            @throw      [css::uno::Exception]
                        If registration failed by any reason.
                        That does not include double registrations!
         */
        void impl_resolveItem4TypeRegistration(      CacheItemList*   pList,
                                               const ::rtl::OUString& sItem,
                                               const ::rtl::OUString& sType)
            throw(css::uno::Exception);

    //-------------------------------------------
    // static helper

    private:

        //---------------------------------------

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
                                CacheItemList*                                      pCache )
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short  read the specified container item from the given configuration set.

            @descr  Its not added to any internal structures here. That must be done
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
                    See defintion of EReadOption for further informations.

            @throw  [css::uno::Exception]
                    if an unrecoverable error occurs inside this operation.
         */
        CacheItem impl_loadItem(const css::uno::Reference< css::container::XNameAccess >& xSet   ,
                                      EItemType                                           eType  ,
                                const ::rtl::OUString&                                    sItem  ,
                                      EReadOption                                         eOption)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short  try to load the requested item on demand from the underlying configuration
                    layer.

            @descr  The outside code has to be shure, that the item does not already exists
                    inside this cachse. Otherwise it will be loaded twice. This method
                    doesnt check such constellations!

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
                                                      const ::rtl::OUString& sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_saveItem(const css::uno::Reference< css::container::XNameReplace >& xSet  ,
                                 EItemType                                            eType ,
                           const CacheItem&                                           aValue)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_addItem2FlushList(      EItemType        eType,
                                    const ::rtl::OUString& sItem)
            throw(css::uno::Exception);


        //---------------------------------------

        /** TODO */
        void impl_flushByList(const css::uno::Reference< css::container::XNameAccess >& xSet  ,
                                    EItemType                                           eType ,
                              const CacheItemList&                                      rCache,
                              const OUStringList&                                       lItems)
            throw(css::uno::Exception);

        //---------------------------------------

        /** @short  specify, which save operation is neccessary for the specified item.

            @desrc  If an item of this cache will be added/removed or modified it will
                    be changed inside memory only first. But we save its name inside a special
                    list of changed items. If at least the method flush() is called, we use
                    this list to check if the item was changed/added or removed. This method
                    checks the exist state of the requested item inside our own cache
                    and inside the underlying configuration layer to find out, if the item
                    must be removed/added or modified inside the configuratuion layer.

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
        EItemFlushState impl_specifyFlushOperation(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                                   const CacheItemList&                                      rList,
                                                   const ::rtl::OUString&                                    sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_readPatchUINames(const css::uno::Reference< css::container::XNameAccess >& xNode,
                                         CacheItem&                                          rItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_savePatchUINames(const css::uno::Reference< css::container::XNameReplace >& xNode,
                                   const CacheItem&                                           rItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_readOldFormat()
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        CacheItem impl_readOldItem(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                         EItemType                                           eType,
                                   const ::rtl::OUString&                                    sItem)
            throw(css::uno::Exception);

        //---------------------------------------

        /** TODO */
        void impl_interpretDataVal4Type(const ::rtl::OUString& sValue,
                                              sal_Int32        nProp ,
                                              CacheItem&       rItem );

        //---------------------------------------

        /** TODO */
        void impl_interpretDataVal4Filter(const ::rtl::OUString& sValue,
                                                sal_Int32        nProp ,
                                                CacheItem&       rItem );

        //---------------------------------------

        /** TODO */
        OUStringList impl_tokenizeString(const ::rtl::OUString& sData     ,
                                               sal_Unicode      cSeperator);

        //---------------------------------------

#if OSL_DEBUG_LEVEL > 0
        /** TODO */
        ::rtl::OUString impl_searchFrameLoaderForType(const ::rtl::OUString& sType) const;
        ::rtl::OUString impl_searchContentHandlerForType(const ::rtl::OUString& sType) const;
#endif

        //---------------------------------------
        /** @short check if the specified OOo module is installed.

            @param  sModule
                    the long name of the module (e.g. "com.sun.star.text.TextDocument").

            @return sal_True if the requested module is installed; sal_False otherwise.
         */
        sal_Bool impl_isModuleInstalled(const ::rtl::OUString& sModule);

        //---------------------------------------

        /** @short  convert a list of flag names to its int representation.

            @param  lNames
                    the list of flag names.

            @return [sal_Int32]
                    the converted flag field.
         */
        static sal_Int32 impl_convertFlagNames2FlagField(const css::uno::Sequence< ::rtl::OUString >& lNames);

        //---------------------------------------

        /** @short  convert a flag field value to its list representation of flag names.

            @param  nFlags
                    the flag field value

            @return [seq< string >]
                    the converted flag name list.
         */
        static css::uno::Sequence< ::rtl::OUString > impl_convertFlagField2FlagNames(sal_Int32 nFlags);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_FILTERCACHE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
