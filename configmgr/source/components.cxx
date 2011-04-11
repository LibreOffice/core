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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <algorithm>
#include <cstddef>
#include <list>

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/logfile.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

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

namespace configmgr {

namespace {

namespace css = com::sun::star;

struct UnresolvedListItem {
    rtl::OUString name;
    rtl::Reference< ParseManager > manager;

    UnresolvedListItem(
        rtl::OUString const & theName,
        rtl::Reference< ParseManager > theManager):
        name(theName), manager(theManager) {}
};

typedef std::list< UnresolvedListItem > UnresolvedList;

void parseXcsFile(
    rtl::OUString const & url, int layer, Data & data, Partial const * partial,
    Modifications * modifications, Additions * additions)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::RuntimeException))
{
    OSL_ASSERT(partial == 0 && modifications == 0 && additions == 0);
    (void) partial; (void) modifications; (void) additions;
    OSL_VERIFY(
        rtl::Reference< ParseManager >(
            new ParseManager(url, new XcsParser(layer, data)))->parse());
}

void parseXcuFile(
    rtl::OUString const & url, int layer, Data & data, Partial const * partial,
    Modifications * modifications, Additions * additions)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::RuntimeException))
{
    OSL_VERIFY(
        rtl::Reference< ParseManager >(
            new ParseManager(
                url,
                new XcuParser(
                    layer, data, partial, modifications, additions)))->
        parse());
}

rtl::OUString expand(rtl::OUString const & str) {
    rtl::OUString s(str);
    rtl::Bootstrap::expandMacros(s); //TODO: detect failure
    return s;
}

bool canRemoveFromLayer(int layer, rtl::Reference< Node > const & node) {
    OSL_ASSERT(node.is());
    if (node->getLayer() > layer && node->getLayer() < Data::NO_LAYER) {
        return false;
    }
    switch (node->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
    case Node::KIND_GROUP:
        for (NodeMap::iterator i(node->getMembers().begin());
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

static bool singletonCreated = false;
static Components * singleton = 0;

}

class Components::WriteThread:
    public osl::Thread, public salhelper::SimpleReferenceObject
{
public:
    static void * operator new(std::size_t size)
    { return Thread::operator new(size); }

    static void operator delete(void * pointer)
    { Thread::operator delete(pointer); }

    WriteThread(
        rtl::Reference< WriteThread > * reference, Components & components,
        rtl::OUString const & url, Data const & data);

    void flush() { delay_.set(); }

private:
    virtual ~WriteThread() {}

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated() { release(); }

    rtl::Reference< WriteThread > * reference_;
    Components & components_;
    rtl::OUString url_;
    Data const & data_;
    osl::Condition delay_;
    boost::shared_ptr<osl::Mutex> lock_;
};

Components::WriteThread::WriteThread(
    rtl::Reference< WriteThread > * reference, Components & components,
    rtl::OUString const & url, Data const & data):
    reference_(reference), components_(components), url_(url), data_(data)
{
    lock_ = lock();
    OSL_ASSERT(reference != 0);
    acquire();
}

void Components::WriteThread::run() {
    TimeValue t = { 1, 0 }; // 1 sec
    delay_.wait(&t); // must not throw; result_error is harmless and ignored
    osl::MutexGuard g(*lock_); // must not throw
    try {
        try {
            writeModFile(components_, url_, data_);
        } catch (css::uno::RuntimeException & e) {
            // Silently ignore write errors, instead of aborting:
            OSL_TRACE(
                "configmgr error writing modifications: %s",
                rtl::OUStringToOString(
                    e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    } catch (...) {
        reference_->clear();
        throw;
    }
    reference_->clear();
}

Components & Components::getSingleton(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    OSL_ASSERT(context.is());
    if (!singletonCreated) {
        static Components theSingleton(context);
        singleton = &theSingleton;
        singletonCreated = true;
    }
    if (singleton == 0) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr no Components singleton")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return *singleton;
}

bool Components::allLocales(rtl::OUString const & locale) {
    return locale.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"));
}

rtl::Reference< Node > Components::resolvePathRepresentation(
    rtl::OUString const & pathRepresentation,
    rtl::OUString * canonicRepresentation, Path * path, int * finalizedLayer)
    const
{
    return data_.resolvePathRepresentation(
        pathRepresentation, canonicRepresentation, path, finalizedLayer);
}

rtl::Reference< Node > Components::getTemplate(
    int layer, rtl::OUString const & fullName) const
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
                        mods = 0;
                        break;
                    }
                    mods = &k->second;
                }
                //TODO: If the complete tree of which root is a part is deleted,
                // or replaced, mods will be null, but some of the listeners
                // from within root should probably fire nonetheless:
                if (mods != 0) {
                    root->initBroadcaster(*mods, broadcaster);
                }
            }
        }
    }
}

void Components::addModification(Path const & path) {
    data_.modifications.add(path);
}

bool Components::hasModifications() const
{
    return data_.modifications.getRoot().children.begin() !=
        data_.modifications.getRoot().children.end();
}

void Components::writeModifications() {

    if (!hasModifications())
        return;

    if (!writeThread_.is()) {
        writeThread_ = new WriteThread(
            &writeThread_, *this, getModificationFileUrl(), data_);
        writeThread_->create();
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
    bool shared, rtl::OUString const & fileUri)
{
    try {
        parseXcsFile(fileUri, shared ? 9 : 13, data_, 0, 0, 0);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "insertExtensionXcsFile does not exist: ")) +
             e.Message),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void Components::insertExtensionXcuFile(
    bool shared, rtl::OUString const & fileUri, Modifications * modifications)
{
    OSL_ASSERT(modifications != 0);
    int layer = shared ? 10 : 14;
    Additions * adds = data_.addExtensionXcuAdditions(fileUri, layer);
    try {
        parseXcuFile(fileUri, layer, data_, 0, modifications, adds);
    } catch (css::container::NoSuchElementException & e) {
        data_.removeExtensionXcuAdditions(fileUri);
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "insertExtensionXcuFile does not exist: ")) +
             e.Message),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void Components::removeExtensionXcuFile(
    rtl::OUString const & fileUri, Modifications * modifications)
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
    OSL_ASSERT(modifications != 0);
    rtl::Reference< Data::ExtensionXcu > item(
        data_.removeExtensionXcuAdditions(fileUri));
    if (item.is()) {
        for (Additions::reverse_iterator i(item->additions.rbegin());
             i != item->additions.rend(); ++i)
        {
            rtl::Reference< Node > parent;
            NodeMap const * map = &data_.components;
            rtl::Reference< Node > node;
            for (Path::const_iterator j(i->begin()); j != i->end(); ++j) {
                parent = node;
                node = Data::findNode(Data::NO_LAYER, *map, *j);
                if (!node.is()) {
                    break;
                }
                map = &node->getMembers();
            }
            if (node.is()) {
                OSL_ASSERT(parent.is());
                if (parent->kind() == Node::KIND_SET) {
                    OSL_ASSERT(
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
    rtl::OUString const & fileUri,
    std::set< rtl::OUString > const & includedPaths,
    std::set< rtl::OUString > const & excludedPaths,
    Modifications * modifications)
{
    OSL_ASSERT(modifications != 0);
    Partial part(includedPaths, excludedPaths);
    try {
        parseFileLeniently(
            &parseXcuFile, fileUri, Data::NO_LAYER, data_, &part, modifications,
            0);
    } catch (css::container::NoSuchElementException & e) {
        OSL_TRACE(
            "configmgr error inserting non-existing %s: %s",
            rtl::OUStringToOString(fileUri, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}

css::beans::Optional< css::uno::Any > Components::getExternalValue(
    rtl::OUString const & descriptor)
{
    sal_Int32 i = descriptor.indexOf(' ');
    if (i <= 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("bad external value descriptor ")) +
             descriptor),
            css::uno::Reference< css::uno::XInterface >());
    }
    //TODO: Do not make calls with mutex locked:
    rtl::OUString name(descriptor.copy(0, i));
    ExternalServices::iterator j(externalServices_.find(name));
    if (j == externalServices_.end()) {
        css::uno::Reference< css::uno::XInterface > service;
        try {
            service = css::uno::Reference< css::lang::XMultiComponentFactory >(
                context_->getServiceManager(), css::uno::UNO_SET_THROW)->
                createInstanceWithContext(name, context_);
        } catch (css::uno::RuntimeException &) {
            // Assuming these exceptions are real errors:
            throw;
        } catch (css::uno::Exception & e) {
            // Assuming these exceptions indicate that the service is not
            // installed:
            OSL_TRACE(
                "createInstance(%s) failed with %s",
                rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr(),
                rtl::OUStringToOString(
                    e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
        css::uno::Reference< css::beans::XPropertySet > propset;
        if (service.is()) {
            propset = css::uno::Reference< css::beans::XPropertySet >(
                service, css::uno::UNO_QUERY_THROW);
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
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "cannot obtain external value through ")) +
                     descriptor),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } catch (css::beans::UnknownPropertyException & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "unknwon external value descriptor ID: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >());
        } catch (css::lang::WrappedTargetException & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "cannot obtain external value: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    return value;
}

Components::Components(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    context_(context)
{
    lock_ = lock();

    OSL_ASSERT(context.is());
    RTL_LOGFILE_TRACE_AUTHOR("configmgr", "sb", "begin parsing");
    parseXcsXcuLayer(
        0,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))));
    parseModuleLayer(
        2,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$OOO_BASE_DIR/share/registry/modules"))));
    parseResLayer(
        3,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))));
    parseXcsXcuLayer(
        4,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$BRAND_BASE_DIR/share/registry"))));
    parseModuleLayer(
        6,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$BRAND_BASE_DIR/share/registry/modules"))));
    parseXcsXcuIniLayer(
        7,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":BUNDLED_EXTENSIONS_USER}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/configmgr.ini"))),
        false);
    parseXcsXcuIniLayer(
        9,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":SHARED_EXTENSIONS_USER}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/configmgr.ini"))),
        true);
    parseXcsXcuLayer(
        11,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_USER_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))));
        // can be dropped once old UserInstallation format can no longer exist
        // (probably OOo 4)
    parseXcsXcuIniLayer(
        13,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_USER_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/configmgr.ini"))),
        true);
    parseModificationLayer();
    RTL_LOGFILE_TRACE_AUTHOR("configmgr", "sb", "end parsing");
}

Components::~Components()
{
    flushModifications();
}

void Components::parseFileLeniently(
    FileParser * parseFile, rtl::OUString const & url, int layer, Data & data,
    Partial const * partial, Modifications * modifications,
    Additions * additions)
{
    OSL_ASSERT(parseFile != 0);
    try {
        (*parseFile)(url, layer, data, partial, modifications, additions);
    } catch (css::container::NoSuchElementException &) {
        throw;
    } catch (css::uno::Exception & e) { //TODO: more specific exception catching
        // Silently ignore invalid XML files, instead of completely preventing
        // OOo from starting:
        OSL_TRACE(
            "configmgr error reading %s: %s",
            rtl::OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}

void Components::parseFiles(
    int layer, rtl::OUString const & extension, FileParser * parseFile,
    rtl::OUString const & url, bool recursive)
{
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (!recursive) {
            return;
        }
        // fall through
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot open directory ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot iterate directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            FileStatusMask_Type | FileStatusMask_FileName |
            FileStatusMask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot stat in directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            parseFiles(layer, extension, parseFile, stat.getFileURL(), true);
        } else {
            rtl::OUString file(stat.getFileName());
            if (file.getLength() >= extension.getLength() &&
                file.match(extension, file.getLength() - extension.getLength()))
            {
                try {
                    parseFileLeniently(
                        parseFile, stat.getFileURL(), layer, data_, 0, 0, 0);
                } catch (css::container::NoSuchElementException & e) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "stat'ed file does not exist: ")) +
                         e.Message),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        }
    }
}

void Components::parseFileList(
    int layer, FileParser * parseFile, rtl::OUString const & urls,
    rtl::Bootstrap const & ini, bool recordAdditions)
{
    for (sal_Int32 i = 0;;) {
        rtl::OUString url(urls.getToken(0, ' ', i));
        if (url.getLength() != 0) {
            ini.expandMacrosFrom(url); //TODO: detect failure
            Additions * adds = 0;
            if (recordAdditions) {
                adds = data_.addExtensionXcuAdditions(url, layer);
            }
            try {
                parseFileLeniently(parseFile, url, layer, data_, 0, 0, adds);
            } catch (css::container::NoSuchElementException & e) {
                OSL_TRACE(
                    "configmgr file does not exist: %s",
                    rtl::OUStringToOString(
                        e.Message, RTL_TEXTENCODING_UTF8).getStr());
                if (adds != 0) {
                    data_.removeExtensionXcuAdditions(url);
                }
            }
        }
        if (i == -1) {
            break;
        }
    }
}

void Components::parseXcdFiles(int layer, rtl::OUString const & url) {
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot open directory ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    UnresolvedList unres;
    XcdParser::Dependencies deps;
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot iterate directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            FileStatusMask_Type | FileStatusMask_FileName |
            FileStatusMask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot stat in directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { //TODO: symlinks
            rtl::OUString file(stat.getFileName());
            if (file.getLength() >= RTL_CONSTASCII_LENGTH(".xcd") &&
                file.matchAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".xcd"),
                    file.getLength() - RTL_CONSTASCII_LENGTH(".xcd")))
            {
                rtl::OUString name(
                    file.copy(
                        0, file.getLength() - RTL_CONSTASCII_LENGTH(".xcd")));
                rtl::Reference< ParseManager > manager;
                try {
                    manager = new ParseManager(
                        stat.getFileURL(), new XcdParser(layer, deps, data_));
                } catch (css::container::NoSuchElementException & e) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "stat'ed file does not exist: ")) +
                         e.Message),
                        css::uno::Reference< css::uno::XInterface >());
                }
                if (manager->parse()) {
                    deps.insert(name);
                } else {
                    unres.push_back(UnresolvedListItem(name, manager));
                }
            }
        }
    }
    while (!unres.empty()) {
        bool resolved = false;
        for (UnresolvedList::iterator i(unres.begin()); i != unres.end();) {
            if (i->manager->parse()) {
                deps.insert(i->name);
                unres.erase(i++);
                resolved = true;
            } else {
                ++i;
            }
        }
        if (!resolved) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcd: unresolved dependencies in ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void Components::parseXcsXcuLayer(int layer, rtl::OUString const & url) {
    parseXcdFiles(layer, url);
    parseFiles(
        layer, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcs")),
        &parseXcsFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/schema")), false);
    parseFiles(
        layer + 1, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")),
        &parseXcuFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/data")), false);
}

void Components::parseXcsXcuIniLayer(
    int layer, rtl::OUString const & url, bool recordAdditions)
{
    //TODO: rtl::Bootstrap::getFrom "first trie[s] to retrieve the value via the
    // global function"
    rtl::Bootstrap ini(url);
    rtl::OUString urls;
    if (ini.getFrom(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SCHEMA")), urls))
    {
        parseFileList(layer, &parseXcsFile, urls, ini, false);
    }
    if (ini.getFrom(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATA")), urls))
    {
        parseFileList(layer + 1, &parseXcuFile, urls, ini, recordAdditions);
    }
}

void Components::parseModuleLayer(int layer, rtl::OUString const & url) {
    parseFiles(
        layer, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")),
        &parseXcuFile, url, false);
}

void Components::parseResLayer(int layer, rtl::OUString const & url) {
    rtl::OUString resUrl(
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/res")));
    parseXcdFiles(layer, resUrl);
    parseFiles(
        layer, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")),
        &parseXcuFile, resUrl, false);
}

rtl::OUString Components::getModificationFileUrl() const {
    return expand(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap")
                ":UserInstallation}/user/registrymodifications.xcu")));
}

void Components::parseModificationLayer() {
    try {
        parseFileLeniently(
            &parseXcuFile, getModificationFileUrl(), Data::NO_LAYER, data_, 0,
            0, 0);
    } catch (css::container::NoSuchElementException &) {
        OSL_TRACE(
            "configmgr user registrymodifications.xcu does not (yet) exist");
        // Migrate old user layer data (can be removed once migration is no
        // longer relevant, probably OOo 4; also see hack for xsi namespace in
        // xmlreader::XmlReader::registerNamespaceIri):
        parseFiles(
            Data::NO_LAYER, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")),
            &parseXcuFile,
            expand(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap")
                        ":UserInstallation}/user/registry/data"))),
            false);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
