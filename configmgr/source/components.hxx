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

#ifndef INCLUDED_CONFIGMGR_SOURCE_COMPONENTS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_COMPONENTS_HXX

#include <sal/config.h>

#include <map>
#include <memory>
#include <set>

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

#include "additions.hxx"
#include "data.hxx"
#include "modifications.hxx"
#include "path.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace uno {
        class Any;
        class XComponentContext;
    }
} } }

namespace configmgr {

class Broadcaster;
class Node;
class Partial;
class RootAccess;

class Components {
public:
    static Components & getSingleton(
        css::uno::Reference< css::uno::XComponentContext > const & context);

    static bool allLocales(OUString const & locale);

    rtl::Reference< Node > resolvePathRepresentation(
        OUString const & pathRepresentation,
        OUString * canonicRepresenation, Path * path, int * finalizedLayer)
        const;

    rtl::Reference< Node > getTemplate(
        int layer, OUString const & fullName) const;

    void addRootAccess(rtl::Reference< RootAccess > const & access);

    void removeRootAccess(RootAccess * access);

    void initGlobalBroadcaster(
        Modifications const & modifications,
        rtl::Reference< RootAccess > const & exclude,
        Broadcaster * broadcaster);

    void addModification(Path const & path);

    void writeModifications();

    void flushModifications();
        // must be called with configmgr::lock unaquired; must be called before
        // shutdown if writeModifications has ever been called (probably
        // indirectly, via removeExtensionXcuFile)

    void insertExtensionXcsFile(bool shared, OUString const & fileUri);

    void insertExtensionXcuFile(
        bool shared, OUString const & fileUri,
        Modifications * modifications);

    void removeExtensionXcuFile(
        OUString const & fileUri, Modifications * modifications);

    void insertModificationXcuFile(
        OUString const & fileUri,
        std::set< OUString > const & includedPaths,
        std::set< OUString > const & excludedPaths,
        Modifications * modifications);

    css::beans::Optional< css::uno::Any >
    getExternalValue(OUString const & descriptor);

private:
    Components(const Components&) = delete;
    Components& operator=(const Components&) = delete;

    typedef void FileParser(
        OUString const &, int, Data &, Partial const *, Modifications *,
        Additions *);
public:
    explicit Components(
        css::uno::Reference< css::uno::XComponentContext > const & context);

    ~Components();
private:

    void parseFileLeniently(
        FileParser * parseFile, OUString const & url, int layer,
        Partial const * partial, Modifications * modifications,
        Additions * additions);

    void parseFiles(
        int layer, OUString const & extension, FileParser * parseFile,
        OUString const & url, bool recursive);

    void parseFileList(
        int layer, FileParser * parseFile, OUString const & urls,
        bool recordAdditions);

    void parseXcdFiles(int layer, OUString const & url);

    void parseXcsXcuLayer(int layer, OUString const & url);

    void parseXcsXcuIniLayer(
        int layer, OUString const & url, bool recordAdditions);

    void parseModuleLayer(int layer, OUString const & url);

    void parseResLayer(int layer, OUString const & url);

    void parseModificationLayer(int layer, OUString const & url);

    int getExtensionLayer(bool shared);

    typedef std::set< RootAccess * > WeakRootSet;

    typedef
        config_map<
            css::uno::Reference<
                css::beans::XPropertySet > >
        ExternalServices;

    class WriteThread;

    enum class ModificationTarget { None, File, Dconf };

    css::uno::Reference< css::uno::XComponentContext >
        context_;
    Data data_;
    WeakRootSet roots_;
    ExternalServices externalServices_;
    rtl::Reference< WriteThread > writeThread_;
    int sharedExtensionLayer_;
    int userExtensionLayer_;
    ModificationTarget modificationTarget_;
    OUString modificationFileUrl_;
    std::shared_ptr<osl::Mutex> lock_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
