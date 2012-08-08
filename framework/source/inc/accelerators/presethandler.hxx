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

#ifndef __FRAMEWORK_ACCELERATORS_PRESETHANDLER_HXX_
#define __FRAMEWORK_ACCELERATORS_PRESETHANDLER_HXX_

#include <accelerators/storageholder.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/embed/XStorage.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <salhelper/singletonref.hxx>
#include <comphelper/locale.hxx>

namespace framework
{
/**
    TODO document me

        <layer>/global/<resourcetype>/<preset>.xml
        <layer>/modules/<moduleid>/<resourcetype>/<preset>.xml

        RESOURCETYPE        PRESET        TARGET
                            (share)       (user)
        "accelerator"       "default"     "current"
                            "word"
                            "excel"

        "menubar"           "default"     "menubar"

 */
class PresetHandler : private ThreadHelpBase // attention! Must be the first base class to guarentee right initialize lock ...
{


    public:

        static ::rtl::OUString PRESET_DEFAULT();
        static ::rtl::OUString TARGET_CURRENT();

        static ::rtl::OUString RESOURCETYPE_MENUBAR();
        static ::rtl::OUString RESOURCETYPE_TOOLBAR();
        static ::rtl::OUString RESOURCETYPE_ACCELERATOR();
        static ::rtl::OUString RESOURCETYPE_STATUSBAR();

    //-------------------------------------------
    // types

    public:

        //---------------------------------------
        /** @short  this handler can provide different
                    types of configuration.

            @descr  Means: a global or a module dependend
                    or ... configuration.
         */
        enum EConfigType
        {
            E_GLOBAL,
            E_MODULES,
            E_DOCUMENT
        };

    private:

        //---------------------------------------
        /** @short  because a concurrent access to the same storage from different implementations
                    isnt supported, we have to share it with others.

            @descr  This struct makes it possible to use any shared storage
                    in combination with a SingletonRef<> template ...

                    Attention: Because these struct is shared it must be
                    used within a synchronized section. Thats why this struct
                    uses a base class ThreadHelpBase and can be locked
                    from outside doing so!
         */
        struct TSharedStorages : public ThreadHelpBase
        {
            public:

                StorageHolder m_lStoragesShare;
                StorageHolder m_lStoragesUser;

                TSharedStorages()
                    : m_lStoragesShare(::comphelper::getProcessServiceFactory())
                    , m_lStoragesUser (::comphelper::getProcessServiceFactory())
                {};

                virtual ~TSharedStorages() {};
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  can be used to create on needed uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  knows the type of provided configuration.

            @descr  e.g. global, modules, ...
         */
        EConfigType m_eConfigType;

        //---------------------------------------
        /** @short  specify the type of resource, which configuration sets
                    must be provided here.

            @descr  e.g. menubars, toolbars, accelerators
         */
        ::rtl::OUString m_sResourceType;

        //---------------------------------------
        /** @short  specify the application module for a module
                    dependend configuration.

            @descr  Will be used only, if m_sResourceType is set to
                    "module". Further it must be a valid module identifier
                    then ...
         */
        ::rtl::OUString m_sModule;

        //---------------------------------------
        /** @short  provides access to the:
                    a) shared root storages
                    b) shared "inbetween" storages
                    of the share and user layer. */
        ::salhelper::SingletonRef< TSharedStorages > m_aSharedStorages;

        //---------------------------------------
        /** @short  if we run in document mode, we cant use the global root storages!
                    We have to use a special document storage explicitly. */
        StorageHolder m_lDocumentStorages;

        //---------------------------------------
        /** @short  holds the folder storage of the share layer alive,
                    where the current configuration set exists.

            @descr  Note: If this preset handler works in document mode
                    this member is meaned relative to the document root ...
                    not to the share layer root!

                    Further is defined, that m_xWorkingStorageUser
                    is equals to m_xWorkingStorageShare then!
         */
        css::uno::Reference< css::embed::XStorage > m_xWorkingStorageShare;

        //---------------------------------------
        /** @short  global language-independent storage
         */
        css::uno::Reference< css::embed::XStorage > m_xWorkingStorageNoLang;

        //---------------------------------------
        /** @short  holds the folder storage of the user layer alive,
                    where the current configuration set exists.

            @descr  Note: If this preset handler works in document mode
                    this member is meaned relative to the document root ...
                    not to the user layer root!

                    Further is defined, that m_xWorkingStorageUser
                    is equals to m_xWorkingStorageShare then!
         */
        css::uno::Reference< css::embed::XStorage > m_xWorkingStorageUser;

        //---------------------------------------
        /** @short  knows the names of all presets inside the current
                    working storage of the share layer. */
        OUStringList m_lPresets;

        //---------------------------------------
        /** @short  knows the names of all targets inside the current
                    working storage of the user layer. */
        OUStringList m_lTargets;

        //---------------------------------------
        /** @short  its the current office locale and will be used
                    to handle localized presets.

            @descr  Default is "x-notranslate" which disable any
                    localized handling inside this class! */
        ::comphelper::Locale m_aLocale;

        //---------------------------------------
        /** @short  knows the relative path from the root. */
        ::rtl::OUString m_sRelPathShare;
        ::rtl::OUString m_sRelPathNoLang;
        ::rtl::OUString m_sRelPathUser;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        /** @short  does nothing real.

            @descr  Because this class should be useable in combination
                    with ::salhelper::SingletonRef template this ctor
                    cant have any special parameters!

            @param  xSMGR
                    points to an uno service manager, which is used internaly
                    to create own needed uno resources.
         */
        PresetHandler(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** @short  copy ctor */
        PresetHandler(const PresetHandler& rCopy);

        //---------------------------------------
        /** @short  closes all open storages ... if user forgot that .-) */
        virtual ~PresetHandler();

        //---------------------------------------
        /** @short  free all currently cache(!) storages. */
        void forgetCachedStorages();

        //---------------------------------------
        /** @short  return access to the internaly used and cached root storage.

            @descr  These root storages are the base of all further opened
                    presets and targets. They are provided here only, to support
                    older implementations, which base on them ...

                    getOrCreate...() - What does it mean?
                    Such root storage will be created one times only and
                    cached then internaly till the last instance of such PresetHandler
                    dies.

            @return com::sun::star::embed::XStorage
                    which represent a root storage.
         */
        css::uno::Reference< css::embed::XStorage > getOrCreateRootStorageShare();
        css::uno::Reference< css::embed::XStorage > getOrCreateRootStorageUser();

        //---------------------------------------
        /** @short  provides access to the current working storages.

            @descr  Working storages are the "lowest" storages, where the
                    preset and target files exists.

            @return com::sun::star::embed::XStorage
                    which the current working storage.
         */
        css::uno::Reference< css::embed::XStorage > getWorkingStorageShare();
        css::uno::Reference< css::embed::XStorage > getWorkingStorageUser();

        //---------------------------------------
        /** @short  check if there is a parent storage well known for
                    the specified child storage and return it.

            @param  xChild
                    the child storage where a paranet storage should be searched for.

            @return com::sun::star::embed::XStorage
                    A valid storage if a paranet exists. NULL otherwise.
         */
        css::uno::Reference< css::embed::XStorage > getParentStorageShare(const css::uno::Reference< css::embed::XStorage >& xChild);
        css::uno::Reference< css::embed::XStorage > getParentStorageUser (const css::uno::Reference< css::embed::XStorage >& xChild);

        //---------------------------------------
        /** @short  free all internal structures and let this handler
                    work on a new type of configuration sets.

            @param  eConfigType
                    differ between global or module dependend configuration.

            @param  sResourceType
                    differ between menubar/toolbar/accelerator/... configuration.

            @param  sModule
                    if sResourceType is set to a module dependend configuration,
                    it address the current application module.

            @param  xDocumentRoot
                    if sResourceType is set to E_DOCUMENT, this value points to the
                    root storage inside the document, where we can save our
                    configuration files. Note: Thats not the real root of the document ...
                    its only a sub storage. But we interpret it as our root storage.

            @param  aLocale
                    in case this configuration supports localized entries,
                    the current locale must be set.

                    Localzation will be represented as directory structure
                    of provided presets. Means: you call us with a preset name "default";
                    and we use e.g. "/en-US/default.xml" internaly.

                    If no localization exists for this preset set, this class
                    will work in default mode - means "no locale" - automaticly.
                    e.g. "/default.xml"

            @throw  com::sun::star::uno::RuntimeException(!)
                    if the specified resource couldn't be located.
         */
        void connectToResource(      EConfigType                                  eConfigType   ,
                               const ::rtl::OUString&                             sResourceType ,
                               const ::rtl::OUString&                             sModule       ,
                               const css::uno::Reference< css::embed::XStorage >& xDocumentRoot ,
                               const ::comphelper::Locale&                        aLocale       = ::comphelper::Locale(::comphelper::Locale::X_NOTRANSLATE()));

        //---------------------------------------
        /** @short  try to copy the specified preset from the share
                    layer to the user layer and establish it as the
                    specified target.

            @descr  Means: copy share/.../<preset>.xml user/.../<target>.xml
                    Note: The target will be overwritten completly or
                    created as new by this operation!

            @param  sPreset
                    the ALIAS name of an existing preset.

            @param  sTarget
                    the ALIAS name of the target.

            @throw  com::sun::star::container::NoSuchElementException
                    if the specified preset does not exists.

            @throw  com::sun::star::io::IOException
                    if copying failed.
         */
        void copyPresetToTarget(const ::rtl::OUString& sPreset,
                                const ::rtl::OUString& sTarget);

        //---------------------------------------
        /** @short  open the specified preset as stream object
                    and return it.

            @descr  Note: Because presets resist inside the share
                    layer, they will be opened readonly everytimes.

            @param  sPreset
                    the ALIAS name of an existing preset.

            @param  bNoLangGlobal
                    access the global language-independent storage instead of the preset storage

            @return The opened preset stream ... or NULL if the preset does not exists.
         */
        css::uno::Reference< css::io::XStream > openPreset(const ::rtl::OUString& sPreset,
                                                           sal_Bool bUseNoLangGlobal = sal_False);

        //---------------------------------------
        /** @short  open the specified target as stream object
                    and return it.

            @descr  Note: Targets resist inside the user
                    layer. Normaly they are opened in read/write mode.
                    But it will be opened readonly automaticly if that isnt possible
                    (may be the file is write protected on the system ...).

            @param  sTarget
                    the ALIAS name of the target.

            @param  bCreateIfMissing
                    create target file, if it does not still exists.
                    Note: That does not means reseting of an existing file!

            @return The opened target stream ... or NULL if the target does not exists
                    or couldnt be created as new one.
         */
        css::uno::Reference< css::io::XStream > openTarget(const ::rtl::OUString& sTarget         ,
                                                                 sal_Bool         bCreateIfMissing);

        //---------------------------------------
        /** @short  do anything which is neccessary to flush all changes
                    back to disk.

            @descr  We have to call commit on all cached sub storages on the
                    path from the root storage upside down to the working storage
                    (which are not realy used, but required to be holded alive!).
         */
        void commitUserChanges();

        //---------------------------------------
        /** TODO */
        void addStorageListener(IStorageListener* pListener);
        void removeStorageListener(IStorageListener* pListener);

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** @short  open a config path ignoring errors (catching exceptions).

            @descr  We catch only normal exceptions here - no runtime exceptions.

            @param  sPath
                    the configuration path, which should be opened.

            @param  eMode
                    the open mode (READ/READWRITE)

            @param  bShare
                    force using of the share layer instead of the user layer.

            @return An opened storage in case method was successfully - null otherwise.
         */
        css::uno::Reference< css::embed::XStorage > impl_openPathIgnoringErrors(const ::rtl::OUString& sPath ,
                                                                                      sal_Int32        eMode ,
                                                                                      sal_Bool         bShare);

        //---------------------------------------
        /** @short  try to find the specified locale inside list of possible ones.

            @descr  The lits of possible locale values was e.g. retrieved from the system
                    (configuration, directory listing etcpp). The locale normaly represent
                    the current office locale. This method search for a suitable item by using
                    different algorithm.
                    a) exact search
                    b) search with using fallbacks

            @param  lLocalizedValues
                    list of ISO locale codes

            @param  aLocale
                    [IN ] the current office locale, which should be searched inside lLocalizedValues.
                    [OUT] in case fallbacks was allowed, it contains afterwards the fallback locale.

            @param  bAllowFallbacks
                    enable/disable using of fallbacks

            @return An iterator, which points directly into lLocalizedValue list.
                    As a negative result the special iterator lLocalizedValues.end() will be returned.
         */
        ::std::vector< ::rtl::OUString >::const_iterator impl_findMatchingLocalizedValue(const ::std::vector< ::rtl::OUString >& lLocalizedValues,
                                                                                               ::comphelper::Locale&             aLocale         ,
                                                                                               sal_Bool                          bAllowFallbacks );

        //---------------------------------------
        /** @short  open a config path ignoring errors (catching exceptions).

            @descr  We catch only normal exceptions here - no runtime exceptions.
                    Further the path itself is tries in different versions (using locale
                    specific attributes).
                    e.g. "path/e-US" => "path/en" => "path/de"

            @param  sPath
                    the configuration path, which should be opened.
                    Its further used as out parameter too, so we can return the localized
                    path to the calli!

            @param  eMode
                    the open mode (READ/READWRITE)

            @param  bShare
                    force using of the share layer instead of the user layer.

            @param  aLocale
                    [IN ] contains the start locale for searching localized sub dirs.
                    [OUT] contains the locale of a found localized sub dir

            @param  bAllowFallback
                    enable/disable fallback handling for locales

            @return An opened storage in case method was successfully - null otherwise.
         */
        css::uno::Reference< css::embed::XStorage > impl_openLocalizedPathIgnoringErrors(::rtl::OUString&      sPath         ,
                                                                                         sal_Int32             eMode         ,
                                                                                         sal_Bool              bShare        ,
                                                                                         ::comphelper::Locale& aLocale       ,
                                                                                         sal_Bool              bAllowFallback);

        //---------------------------------------
        /** @short  returns the names of all sub storages of specified storage.

            @param  xFolder
                    the base storage for this operation.

            @return [vector< string >]
                    a list of folder names.
         */
        ::std::vector< ::rtl::OUString > impl_getSubFolderNames(const css::uno::Reference< css::embed::XStorage >& xFolder);
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_PRESETHANDLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
