/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <algorithm>
#include <list>

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "components.hxx"
#include "data.hxx"
#include "node.hxx"
#include "parsemanager.hxx"
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

void parseSystemLayer() {
    //TODO
}

void parseXcsFile(rtl::OUString const & url, int layer, Data * data)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::UnoRuntimeException))
{
    OSL_VERIFY(
        rtl::Reference< ParseManager >(
            new ParseManager(url, new XcsParser(layer, data)))->parse());
}

void parseXcuFile(rtl::OUString const & url, int layer, Data * data)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::UnoRuntimeException))
{
    OSL_VERIFY(
        rtl::Reference< ParseManager >(
            new ParseManager(url, new XcuParser(layer, data)))->parse());
}

rtl::OUString expand(rtl::OUString const & str) {
    rtl::OUString s(str);
    rtl::Bootstrap::expandMacros(s); //TODO: detect failure
    return s;
}

}

Components & Components::singleton() {
    static Components * c = new Components(); // leaks
    return *c;
}

bool Components::allLocales(rtl::OUString const & locale) {
    return locale.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"));
}

rtl::Reference< Node > Components::resolvePath(
    rtl::OUString const & path, rtl::OUString * lastSegment,
    rtl::OUString * canonicalPath, int * finalizedLayer) const
{
    return data_.resolvePath(
        path, 0, lastSegment, canonicalPath, 0, finalizedLayer);
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
    Modifications const & globalModifications,
    rtl::Reference< RootAccess > const & exclude, Broadcaster * broadcaster)
{
    for (WeakRootSet::iterator i(roots_.begin()); i != roots_.end(); ++i) {
        if (*i != exclude.get()) {
            (*i)->initGlobalBroadcaster(globalModifications, broadcaster);
        }
    }
}

void Components::addModification(rtl::OUString const & path) {
    data_.modifications.add(path);
}

void Components::writeModifications() {
    writeModFile(getModificationFileUrl(), data_);
}

void Components::insertXcsFile(int layer, rtl::OUString const & fileUri) {
    try {
        parseXcsFile(fileUri, layer, &data_);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("insertXcsFile does not exist: ")) +
             e.Message),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void Components::insertXcuFile(int layer, rtl::OUString const & fileUri) {
    try {
        parseXcuFile(fileUri, layer + 1, &data_);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("insertXcuFile does not exist: ")) +
             e.Message),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Components::Components() {
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
    parseXcsXcuLayer( //TODO: migrate
        7,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_SHARED_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))));
    parseXcsXcuIniLayer(
        9,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_SHARED_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/configmgrrc"))));
    parseSystemLayer();
    parseXcsXcuLayer( //TODO: migrate
        11,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_USER_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))));
    parseXcsXcuIniLayer(
        13,
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")
                    ":UNO_USER_PACKAGES_CACHE}/registry/"
                    "com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/configmgrrc"))));
    // reading the old user xcs/xcu layer ("${$BRAND_BASE_DIR/program/" +
    // SAL_CONFIGFILE("bootstrap") + ":UserInstallation}/user/registry") would
    // potentially fail, anyway, as xcu files in that layer used the xsi
    // namespace prefix without declaring a corresponding namespace binding (see
    // issue 77174)
    parseModificationLayer();
}

Components::~Components() {}

void Components::parseFiles(
    int layer, rtl::OUString const & extension,
    void (* parseFile)(rtl::OUString const &, int, Data *),
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
                    (*parseFile)(stat.getFileURL(), layer, &data_);
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
    int layer, void (* parseFile)(rtl::OUString const &, int, Data *),
    rtl::OUString const & urls, rtl::Bootstrap const & ini)
{
    for (sal_Int32 i = 0;;) {
        rtl::OUString url(urls.getToken(0, ' ', i));
        if (url.getLength() != 0) {
            ini.expandMacrosFrom(url); //TODO: detect failure
            try {
                (*parseFile)(url, layer, &data_);
            } catch (css::container::NoSuchElementException & e) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "stat'ed file does not exist: ")) +
                     e.Message),
                    css::uno::Reference< css::uno::XInterface >());
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
                        stat.getFileURL(), new XcdParser(layer, deps, &data_));
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

void Components::parseXcsXcuIniLayer(int layer, rtl::OUString const & url) {
    //TODO: rtl::Bootstrap::getFrom "first trie[s] to retrieve the value via the
    // global function"
    rtl::Bootstrap ini(url);
    rtl::OUString urls;
    if (ini.getFrom(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SCHEMA")), urls))
    {
        parseFileList(layer, &parseXcsFile, urls, ini);
    }
    if (ini.getFrom(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATA")), urls))
    {
        parseFileList(layer + 1, &parseXcuFile, urls, ini);
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
        parseXcuFile(getModificationFileUrl(), Data::NO_LAYER, &data_);
    } catch (css::container::NoSuchElementException &) {
        OSL_TRACE(
            "configmgr user registrymodifications.xcu does not (yet) exist");
    }
}

}
