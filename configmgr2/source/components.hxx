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

#ifndef INCLUDED_CONFIGMGR_SOURCE_COMPONENTS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_COMPONENTS_HXX

#include "sal/config.h"

#include <set>

#include "boost/noncopyable.hpp"
#include "rtl/ref.hxx"

#include "data.hxx"
#include "path.hxx"

namespace rtl {
    class Bootstrap;
    class OUString;
}

namespace configmgr {

class Broadcaster;
class Node;
class RootAccess;
struct Modifications;

class Components: private boost::noncopyable {
public:
    static Components & singleton();

    static bool allLocales(rtl::OUString const & locale);

    rtl::Reference< Node > resolvePathRepresentation(
        rtl::OUString const & pathRepresentation, Path * path,
        int * finalizedLayer) const;

    rtl::Reference< Node > getTemplate(
        int layer, rtl::OUString const & fullName) const;

    void addRootAccess(rtl::Reference< RootAccess > const & access);

    void removeRootAccess(RootAccess * access);

    void initGlobalBroadcaster(
        Modifications const & globalModifications,
        rtl::Reference< RootAccess > const & exclude,
        Broadcaster * broadcaster);

    void addModification(Path const & path);

    void writeModifications();

    void insertXcsFile(int layer, rtl::OUString const & fileUri);

    void insertXcuFile(int layer, rtl::OUString const & fileUri);

private:
    Components();

    ~Components();

    void parseFiles(
        int layer, rtl::OUString const & extension,
        void (* parseFile)(rtl::OUString const &, int, Data *),
        rtl::OUString const & url, bool recursive);

    void parseFileList(
        int layer, void (* parseFile)(rtl::OUString const &, int, Data *),
        rtl::OUString const & urls, rtl::Bootstrap const & ini);

    void parseXcdFiles(int layer, rtl::OUString const & url);

    void parseXcsXcuLayer(int layer, rtl::OUString const & url);

    void parseXcsXcuIniLayer(int layer, rtl::OUString const & url);

    void parseModuleLayer(int layer, rtl::OUString const & url);

    void parseResLayer(int layer, rtl::OUString const & url);

    rtl::OUString getModificationFileUrl() const;

    void parseModificationLayer();

    typedef std::set< RootAccess * > WeakRootSet;

    Data data_;
    WeakRootSet roots_;
};

}

#endif
