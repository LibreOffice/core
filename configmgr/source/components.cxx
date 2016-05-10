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

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <list>
#include <set>

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <config_dconf.h>
#include <config_folders.h>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>

#include "additions.hxx"
#include "components.hxx"
#include "data.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "partial.hxx"
#include "rootaccess.hxx"
#include "writemodfile.hxx"
#include "xcdparser.hxx"
#include "xcuparser.hxx"
#include "xcsparser.hxx"

#if ENABLE_DCONF
#include <dconf.hxx>
#endif

#if defined WNT
#include "winreg.hxx"
#endif

namespace configmgr {

namespace {

struct UnresolvedListItem {
    OUString name;
    rtl::Reference< ParseManager > manager;

    UnresolvedListItem(
        OUString const & theName,
        rtl::Reference< ParseManager > theManager):
        name(theName), manager(theManager) {}
};

typedef std::list< UnresolvedListItem > UnresolvedList;

void parseXcsFile(
    OUString const & url, int layer, Data & data, Partial const * partial,
    Modifications * modifications, Additions * additions)
{
    assert(partial == nullptr && modifications == nullptr && additions == nullptr);
    (void) partial; (void) modifications; (void) additions;
    bool ok = rtl::Reference< ParseManager >(
        new ParseManager(url, new XcsParser(layer, data)))->parse(nullptr);
    assert(ok);
    (void) ok; // avoid warnings
}

void parseXcuFile(
    OUString const & url, int layer, Data & data, Partial const * partial,
    Modifications * modifications, Additions * additions)
{
    bool ok = rtl::Reference< ParseManager >(
        new ParseManager(
            url,
            new XcuParser(layer, data, partial, modifications, additions)))->
        parse(nullptr);
    assert(ok);
    (void) ok; // avoid warnings
}

OUString expand(OUString const & str) {
    OUString s(str);
    rtl::Bootstrap::expandMacros(s); //TODO: detect failure
    return s;
}

bool canRemoveFromLayer(int layer, rtl::Reference< Node > const & node) {
    assert(node.is());
    if (node->getLayer() > layer && node->getLayer() < Data::NO_LAYER) {
        return false;
    }
    switch (node->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
    case Node::KIND_GROUP:
        for (NodeMap::const_iterator i(node->getMembers().begin());
             i != node->getMembers().end(); ++i)
        {
            if (!canRemoveFromLayer(layer, i->second)) {
                return false;
            }
        }
        return true;
    case Node::KIND_SET:
        return node->getMembers().empty();
    default: // Node::KIND_PROPERTY, Node::KIND_LOCALIZED_VALUE
        return true;
    }
}

}

class Components::WriteThread: public salhelper::Thread {
public:
    WriteThread(
        rtl::Reference< WriteThread > * reference, Components & components,
        OUString const & url, Data const & data);

    void flush() { delay_.set(); }

private:
    virtual ~WriteThread() {}

    virtual void execute() override;

    rtl::Reference< WriteThread > * reference_;
    Components & components_;
    OUString url_;
    Data const & data_;
    osl::Condition delay_;
    std::shared_ptr<osl::Mutex> lock_;
};

Components::WriteThread::WriteThread(
    rtl::Reference< WriteThread > * reference, Components & components,
    OUString const & url, Data const & data):
    Thread("configmgrWriter"), reference_(reference), components_(components),
    url_(url), data_(data),
    lock_( lock() )
{
    assert(reference != nullptr);
}

void Components::WriteThread::execute() {
    TimeValue t = { 1, 0 }; // 1 sec
    delay_.wait(&t); // must not throw; result_error is harmless and ignored
    osl::MutexGuard g(*lock_); // must not throw
    try {
        try {
            writeModFile(components_, url_, data_);
        } catch (css::uno::RuntimeException & e) {
            // Ignore write errors, instead of aborting:
            SAL_WARN(
                "configmgr",
                "error writing modifications: \"" << e.Message << '"');
        }
    } catch (...) {
        reference_->clear();
        throw;
    }
    reference_->clear();
}

class theComponentsSingleton :
    public rtl::StaticWithArg<
        Components,
        css::uno::Reference< css::uno::XComponentContext >,
        theComponentsSingleton>
{
};

Components & Components::getSingleton(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    assert(context.is());
    return theComponentsSingleton::get(context);
}

bool Components::allLocales(OUString const & locale) {
    return locale == "*";
}

rtl::Reference< Node > Components::resolvePathRepresentation(
    OUString const & pathRepresentation,
    OUString * canonicRepresentation, Path * path, int * finalizedLayer)
    const
{
    return data_.resolvePathRepresentation(
        pathRepresentation, canonicRepresentation, path, finalizedLayer);
}

rtl::Reference< Node > Components::getTemplate(
    int layer, OUString const & fullName) const
{
    return data_.getTemplate(layer, fullName);
}

void Components::addRootAccess(rtl::Reference< RootAccess > const & access) {
    roots_.insert(access.get());
}

void Components::removeRootAccess(RootAccess * access) {
    roots_.erase(access);
}

void Components::initGlobalBroadcaster(
    Modifications const & modifications,
    rtl::Reference< RootAccess > const & exclude, Broadcaster * broadcaster)
{
    //TODO: Iterate only over roots w/ listeners:
    for (WeakRootSet::iterator i(roots_.begin()); i != roots_.end(); ++i) {
        rtl::Reference< RootAccess > root;
        if ((*i)->acquireCounting() > 1) {
            root.set(*i); // must not throw
        }
        (*i)->releaseNondeleting();
        if (root.is()) {
            if (root != exclude) {
                Path path(root->getAbsolutePath());
                Modifications::Node const * mods = &modifications.getRoot();
                for (Path::iterator j(path.begin()); j != path.end(); ++j) {
                    Modifications::Node::Children::const_iterator k(
                        mods->children.find(*j));
                    if (k == mods->children.end()) {
                        mods = nullptr;
                        break;
                    }
                    mods = &k->second;
                }
                //TODO: If the complete tree of which root is a part is deleted,
                // or replaced, mods will be null, but some of the listeners
                // from within root should probably fire nonetheless:
                if (mods != nullptr) {
                    root->initBroadcaster(*mods, broadcaster);
                }
            }
        }
    }
}

void Components::addModification(Path const & path) {
    data_.modifications.add(path);
}

void Components::writeModifications() {

    if (!data_.modifications.empty()) {
        switch (modificationTarget_) {
        case ModificationTarget::None:
            break;
        case ModificationTarget::File:
            if (!writeThread_.is()) {
                writeThread_ = new WriteThread(
                    &writeThread_, *this, modificationFileUrl_, data_);
                writeThread_->launch();
            }
            break;
        case ModificationTarget::Dconf:
#if ENABLE_DCONF
            dconf::writeModifications(*this, data_);
#endif
            break;
        }
    }
}

void Components::flushModifications() {
    rtl::Reference< WriteThread > thread;
    {
        osl::MutexGuard g(*lock_);
        thread = writeThread_;
    }
    if (thread.is()) {
        thread->flush();
        thread->join();
    }
}

void Components::insertExtensionXcsFile(
    bool shared, OUString const & fileUri)
{
    int layer = getExtensionLayer(shared);
    try {
        parseXcsFile(fileUri, layer, data_, nullptr, nullptr, nullptr);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::RuntimeException(
            "insertExtensionXcsFile does not exist: " + e.Message);
    }
}

void Components::insertExtensionXcuFile(
    bool shared, OUString const & fileUri, Modifications * modifications)
{
    assert(modifications != nullptr);
    int layer = getExtensionLayer(shared) + 1;
    Additions * adds = data_.addExtensionXcuAdditions(fileUri, layer);
    try {
        parseXcuFile(fileUri, layer, data_, nullptr, modifications, adds);
    } catch (css::container::NoSuchElementException & e) {
        data_.removeExtensionXcuAdditions(fileUri);
        throw css::uno::RuntimeException(
            "insertExtensionXcuFile does not exist: " + e.Message);
    }
}

void Components::removeExtensionXcuFile(
    OUString const & fileUri, Modifications * modifications)
{
    //TODO: Ideally, exactly the data coming from the specified xcu file would
    // be removed.  However, not enough information is recorded in the in-memory
    // data structures to do so.  So, as a workaround, all those set elements
    // that were freshly added by the xcu and have afterwards been left
    // unchanged or have only had their properties changed in the user layer are
    // removed (and nothing else).  The heuristic to determine
    // whether a node has been left unchanged is to check the layer ID (as
    // usual) and additionally to check that the node does not recursively
    // contain any non-empty sets (multiple extension xcu files are merged into
    // one layer, so checking layer ID alone is not enough).  Since
    // item->additions records all additions of set members in textual order,
    // the latter check works well when iterating through item->additions in
    // reverse order.
    assert(modifications != nullptr);
    rtl::Reference< Data::ExtensionXcu > item(
        data_.removeExtensionXcuAdditions(fileUri));
    if (item.is()) {
        for (Additions::reverse_iterator i(item->additions.rbegin());
             i != item->additions.rend(); ++i)
        {
            rtl::Reference< Node > parent;
            NodeMap const * map = &data_.getComponents();
            rtl::Reference< Node > node;
            for (Path::const_iterator j(i->begin()); j != i->end(); ++j) {
                parent = node;
                node = map->findNode(Data::NO_LAYER, *j);
                if (!node.is()) {
                    break;
                }
                map = &node->getMembers();
            }
            if (node.is()) {
                assert(parent.is());
                if (parent->kind() == Node::KIND_SET) {
                    assert(
                        node->kind() == Node::KIND_GROUP ||
                        node->kind() == Node::KIND_SET);
                    if (canRemoveFromLayer(item->layer, node)) {
                        parent->getMembers().erase(i->back());
                        data_.modifications.remove(*i);
                        modifications->add(*i);
                    }
                }
            }
        }
        writeModifications();
    }
}

void Components::insertModificationXcuFile(
    OUString const & fileUri,
    std::set< OUString > const & includedPaths,
    std::set< OUString > const & excludedPaths,
    Modifications * modifications)
{
    assert(modifications != nullptr);
    Partial part(includedPaths, excludedPaths);
    try {
        parseFileLeniently(
            &parseXcuFile, fileUri, Data::NO_LAYER, &part, modifications, nullptr);
    } catch (css::container::NoSuchElementException & e) {
        SAL_WARN(
            "configmgr",
            "error inserting non-existing \"" << fileUri << "\": \""
                << e.Message << '"');
    }
}

css::beans::Optional< css::uno::Any > Components::getExternalValue(
    OUString const & descriptor)
{
    sal_Int32 i = descriptor.indexOf(' ');
    if (i <= 0) {
        throw css::uno::RuntimeException(
            "bad external value descriptor " + descriptor);
    }
    //TODO: Do not make calls with mutex locked:
    OUString name(descriptor.copy(0, i));
    ExternalServices::iterator j(externalServices_.find(name));
    if (j == externalServices_.end()) {
        css::uno::Reference< css::uno::XInterface > service;
        try {
            service = context_->getServiceManager()->createInstanceWithContext(
                name, context_);
        } catch (css::uno::RuntimeException &) {
            // Assuming these exceptions are real errors:
            throw;
        } catch (css::uno::Exception & e) {
            // Assuming these exceptions indicate that the service is not
            // installed:
            SAL_WARN(
                "configmgr",
                "createInstance(" << name << ") failed with \"" << e.Message
                    << '"');
        }
        css::uno::Reference< css::beans::XPropertySet > propset;
        if (service.is()) {
            propset.set( service, css::uno::UNO_QUERY_THROW);
        }
        j = externalServices_.insert(
            ExternalServices::value_type(name, propset)).first;
    }
    css::beans::Optional< css::uno::Any > value;
    if (j->second.is()) {
        try {
            if (!(j->second->getPropertyValue(descriptor.copy(i + 1)) >>=
                  value))
            {
                throw css::uno::RuntimeException(
                    "cannot obtain external value through " + descriptor);
            }
        } catch (css::beans::UnknownPropertyException & e) {
            throw css::uno::RuntimeException(
                "unknown external value descriptor ID: " + e.Message);
        } catch (css::lang::WrappedTargetException & e) {
            throw css::uno::RuntimeException(
                "cannot obtain external value: " + e.Message);
        }
    }
    return value;
}

Components::Components(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    context_(context), sharedExtensionLayer_(-1), userExtensionLayer_(-1),
    modificationTarget_(ModificationTarget::None)
{
    assert(context.is());
    lock_ = lock();
    OUString conf(expand("${CONFIGURATION_LAYERS}"));
    int layer = 0;
    for (sal_Int32 i = 0;;) {
        while (i != conf.getLength() && conf[i] == ' ') {
            ++i;
        }
        if (i == conf.getLength()) {
            break;
        }
        if (modificationTarget_ != ModificationTarget::None) {
            throw css::uno::RuntimeException(
                "CONFIGURATION_LAYERS: modification target layer followed by"
                " further layers");
        }
        sal_Int32 c = i;
        for (;; ++c) {
            if (c == conf.getLength() || conf[c] == ' ') {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: missing \":\"");
            }
            if (conf[c] == ':') {
                break;
            }
        }
        sal_Int32 n = conf.indexOf(' ', c + 1);
        if (n == -1) {
            n = conf.getLength();
        }
        OUString type(conf.copy(i, c - i));
        OUString url(conf.copy(c + 1, n - c - 1));
        if (type == "xcsxcu") {
            sal_uInt32 nStartTime = osl_getGlobalTimer();
            parseXcsXcuLayer(layer, url);
            SAL_INFO("configmgr", "parseXcsXcuLayer() took " << (osl_getGlobalTimer() - nStartTime) << " ms");
            layer += 2; //TODO: overflow
        } else if (type == "bundledext") {
            parseXcsXcuIniLayer(layer, url, false);
            layer += 2; //TODO: overflow
        } else if (type == "sharedext") {
            if (sharedExtensionLayer_ != -1) {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: multiple \"sharedext\" layers");
            }
            sharedExtensionLayer_ = layer;
            parseXcsXcuIniLayer(layer, url, true);
            layer += 2; //TODO: overflow
        } else if (type == "userext") {
            if (userExtensionLayer_ != -1) {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: multiple \"userext\" layers");
            }
            userExtensionLayer_ = layer;
            parseXcsXcuIniLayer(layer, url, true);
            layer += 2; //TODO: overflow
        } else if (type == "module") {
            parseModuleLayer(layer, url);
            ++layer; //TODO: overflow
        } else if (type == "res") {
            sal_uInt32 nStartTime = osl_getGlobalTimer();
            parseResLayer(layer, url);
            SAL_INFO("configmgr", "parseResLayer() took " << (osl_getGlobalTimer() - nStartTime) << " ms");
            ++layer; //TODO: overflow
#if ENABLE_DCONF
        } else if (type == "dconf") {
            if (url == "!") {
                modificationTarget_ = ModificationTarget::Dconf;
                dconf::readLayer(data_, Data::NO_LAYER);
            } else if (url == "*") {
                dconf::readLayer(data_, layer);
            } else {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: unknown \"dconf\" kind \"" + url
                    + "\"");
            }
            ++layer; //TODO: overflow
#endif
#if defined WNT
        } else if (type == "winreg") {
            WinRegType eType;
            if (url == "LOCAL_MACHINE" || url.isEmpty()/*backwards comp.*/) {
                eType = WinRegType::LOCAL_MACHINE;
            } else if (url == "CURRENT_USER") {
                eType = WinRegType::CURRENT_USER;
            } else {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: unknown \"winreg\" kind \"" + url
                    + "\"");
            }
            OUString aTempFileURL;
            if (dumpWindowsRegistry(&aTempFileURL, eType)) {
                parseFileLeniently(&parseXcuFile, aTempFileURL, layer, 0, 0, 0);
                osl::File::remove(aTempFileURL);
            }
            ++layer; //TODO: overflow
#endif
        } else if (type == "user") {
            bool write;
            if (url.startsWith("!", &url)) {
                write = true;
            } else if (url.startsWith("*", &url)) {
                write = false;
            } else {
                write = true; // for backwards compatibility
            }
            if (url.isEmpty()) {
                throw css::uno::RuntimeException(
                    "CONFIGURATION_LAYERS: empty \"user\" URL");
            }
            bool ignore = false;
#if ENABLE_DCONF
            if (write) {
                OUString token(
                    expand("${SYSUSERCONFIG}/libreoffice/dconfwrite"));
                osl::DirectoryItem it;
                osl::FileBase::RC e = osl::DirectoryItem::get(token, it);
                ignore = e == osl::FileBase::E_None;
                SAL_INFO(
                    "configmgr",
                    "dconf write (<" << token << "> " << +e << "): "
                        << int(ignore));
                if (ignore) {
                    modificationTarget_ = ModificationTarget::Dconf;
                }
            }
#endif
            if (!ignore) {
                if (write) {
                    modificationTarget_ = ModificationTarget::File;
                    modificationFileUrl_ = url;
                }
                parseModificationLayer(write ? Data::NO_LAYER : layer, url);
            }
            ++layer; //TODO: overflow
        } else {
            throw css::uno::RuntimeException(
                "CONFIGURATION_LAYERS: unknown layer type \"" + type + "\"");
        }
        i = n;
    }
}

Components::~Components()
{
    flushModifications();
    for (WeakRootSet::iterator i(roots_.begin()); i != roots_.end(); ++i) {
        (*i)->setAlive(false);
    }
}

void Components::parseFileLeniently(
    FileParser * parseFile, OUString const & url, int layer,
    Partial const * partial, Modifications * modifications,
    Additions * additions)
{
    assert(parseFile != nullptr);
    try {
        (*parseFile)(url, layer, data_, partial, modifications, additions);
    } catch (css::container::NoSuchElementException &) {
        throw;
    } catch (css::uno::Exception & e) { //TODO: more specific exception catching
        // Ignore invalid XML files, instead of completely preventing OOo from
        // starting:
        SAL_WARN(
            "configmgr",
            "error reading \"" << url << "\": \"" << e.Message << '"');
    }
}

void Components::parseFiles(
    int layer, OUString const & extension, FileParser * parseFile,
    OUString const & url, bool recursive)
{
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (!recursive) {
            return;
        }
        SAL_FALLTHROUGH;
    default:
        throw css::uno::RuntimeException(
            "cannot open directory " + url);
    }
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                "cannot iterate directory " + url);
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                "cannot stat in directory " + url);
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            parseFiles(layer, extension, parseFile, stat.getFileURL(), true);
        } else {
            OUString file(stat.getFileName());
            if (file.endsWith(extension)) {
                try {
                    parseFileLeniently(
                        parseFile, stat.getFileURL(), layer, nullptr, nullptr, nullptr);
                } catch (css::container::NoSuchElementException & e) {
                    throw css::uno::RuntimeException(
                        "stat'ed file does not exist: " + e.Message);
                }
            }
        }
    }
}

void Components::parseFileList(
    int layer, FileParser * parseFile, OUString const & urls,
    bool recordAdditions)
{
    for (sal_Int32 i = 0;;) {
        OUString url(urls.getToken(0, ' ', i));
        if (!url.isEmpty()) {
            Additions * adds = nullptr;
            if (recordAdditions) {
                adds = data_.addExtensionXcuAdditions(url, layer);
            }
            try {
                parseFileLeniently(parseFile, url, layer, nullptr, nullptr, adds);
            } catch (css::container::NoSuchElementException & e) {
                SAL_WARN(
                    "configmgr", "file does not exist: \"" << e.Message << '"');
                if (adds != nullptr) {
                    data_.removeExtensionXcuAdditions(url);
                }
            }
        }
        if (i == -1) {
            break;
        }
    }
}

void Components::parseXcdFiles(int layer, OUString const & url) {
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return;
    default:
        throw css::uno::RuntimeException(
            "cannot open directory " + url);
    }
    UnresolvedList unres;
    std::set< OUString > existingDeps;
    std::set< OUString > processedDeps;
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                "cannot iterate directory " + url);
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                "cannot stat in directory " + url);
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { //TODO: symlinks
            OUString file(stat.getFileName());
            OUString name;
            if (file.endsWith(".xcd", &name)) {
                existingDeps.insert(name);
                rtl::Reference< ParseManager > manager;
                try {
                    manager = new ParseManager(
                        stat.getFileURL(),
                        new XcdParser(layer, processedDeps, data_));
                } catch (css::container::NoSuchElementException & e) {
                    throw css::uno::RuntimeException(
                        "stat'ed file does not exist: " + e.Message);
                }
                if (manager->parse(nullptr)) {
                    processedDeps.insert(name);
                } else {
                    unres.push_back(UnresolvedListItem(name, manager));
                }
            }
        }
    }
    while (!unres.empty()) {
        bool isResolved = false;
        for (UnresolvedList::iterator i(unres.begin()); i != unres.end();) {
            if (i->manager->parse(&existingDeps)) {
                processedDeps.insert(i->name);
                unres.erase(i++);
                isResolved = true;
            } else {
                ++i;
            }
        }
        if (!isResolved) {
            throw css::uno::RuntimeException(
                "xcd: unresolved dependencies in " + url);
        }
    }
}

void Components::parseXcsXcuLayer(int layer, OUString const & url) {
    parseXcdFiles(layer, url);
    parseFiles(layer, ".xcs", &parseXcsFile, url + "/schema", false);
    parseFiles(layer + 1, ".xcu", &parseXcuFile, url + "/data", false);
}

void Components::parseXcsXcuIniLayer(
    int layer, OUString const & url, bool recordAdditions)
{
    // Check if ini file exists (otherwise .override would still read global
    // SCHEMA/DATA variables, which could interfere with unrelated environment
    // variables):
    if (rtl::Bootstrap(url).getHandle() != nullptr) {
        OUStringBuffer prefix("${.override:");
        for (sal_Int32 i = 0; i != url.getLength(); ++i) {
            sal_Unicode c = url[i];
            switch (c) {
            case '$':
            case ':':
            case '\\':
                prefix.append('\\');
                SAL_FALLTHROUGH;
            default:
                prefix.append(c);
            }
        }
        prefix.append(':');
        OUString urls(prefix.toString() + "SCHEMA}");
        rtl::Bootstrap::expandMacros(urls);
        if (!urls.isEmpty()) {
            parseFileList(layer, &parseXcsFile, urls, false);
        }
        urls = prefix.makeStringAndClear() + "DATA}";
        rtl::Bootstrap::expandMacros(urls);
        if (!urls.isEmpty()) {
            parseFileList(layer + 1, &parseXcuFile, urls, recordAdditions);
        }
    }
}

void Components::parseModuleLayer(int layer, OUString const & url) {
    parseFiles(layer, ".xcu", &parseXcuFile, url, false);
}

void Components::parseResLayer(int layer, OUString const & url) {
    OUString resUrl(url + "/res");
    parseXcdFiles(layer, resUrl);
    parseFiles(layer, ".xcu", &parseXcuFile, resUrl, false);
}

void Components::parseModificationLayer(int layer, OUString const & url) {
    try {
        parseFileLeniently(&parseXcuFile, url, layer, nullptr, nullptr, nullptr);
    } catch (css::container::NoSuchElementException &) {
        SAL_INFO(
            "configmgr", "user registrymodifications.xcu does not (yet) exist");
        // Migrate old user layer data (can be removed once migration is no
        // longer relevant, probably OOo 4; also see hack for xsi namespace in
        // xmlreader::XmlReader::registerNamespaceIri):
        parseFiles(
            layer, ".xcu", &parseXcuFile,
            expand(
                "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap")
                ":UserInstallation}/user/registry/data"),
            false);
    }
}

int Components::getExtensionLayer(bool shared) {
    int layer = shared ? sharedExtensionLayer_ : userExtensionLayer_;
    if (layer == -1) {
        throw css::uno::RuntimeException(
            "insert extension xcs/xcu file into undefined layer");
    }
    return layer;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
