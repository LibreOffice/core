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

#ifndef INCLUDED_CONFIGMGR_COMPONENTS_HXX
#define INCLUDED_CONFIGMGR_COMPONENTS_HXX

#include "sal/config.h"

#include <list>
#include <set>

#include "boost/noncopyable.hpp"
#include "libxml/parser.h"
#include "rtl/ref.hxx"

#include "nodemap.hxx"

namespace rtl {
    class Bootstrap;
    class OUString;
}

namespace configmgr {

class GroupNode;
class Node;

class Components: private boost::noncopyable {
public:
    static Components & singleton();

    static bool allLocales(rtl::OUString const & locale);

    static rtl::OUString createSegment(
        rtl::OUString const & templateName, rtl::OUString const & name);

    static sal_Int32 parseSegment(
        rtl::OUString const & path, sal_Int32 index, rtl::OUString * name,
        bool * setElement, rtl::OUString * templateName);

    rtl::Reference< Node > resolvePath(
        rtl::OUString const & path, rtl::OUString * firstSegment,
        rtl::OUString * lastSegment, rtl::OUString * canonicalPath,
        rtl::Reference< Node > * parent, int * finalizedLayer);

    rtl::Reference< Node > getTemplate(
        int layer, rtl::OUString const & fullName) const;

    void addModification(rtl::OUString const & path);

    void writeModifications();

    void insertXcsFile(int layer, rtl::OUString const & fileUri);

    void insertXcuFile(int layer, rtl::OUString const & fileUri);

private:
    typedef std::set< rtl::OUString > Dependencies;

    typedef std::list< rtl::OUString > Modifications;

    Components();

    ~Components();

    void parseXcsGroupContent(
        int layer, rtl::OUString const & componentName, xmlDocPtr doc,
        xmlNodePtr node, rtl::Reference< GroupNode > const & group);

    rtl::Reference< Node > parseXcsGroup(
        int layer, rtl::OUString const & componentName, xmlDocPtr doc,
        xmlNodePtr node, rtl::OUString const & templateName);

    xmlNodePtr parseXcsTemplates(
        int layer, rtl::OUString const & componentName, xmlDocPtr doc,
        xmlNodePtr node);

    xmlNodePtr parseXcsComponent(
        int layer, xmlDocPtr doc, rtl::OUString const & component,
        xmlNodePtr node);

    void parseXcsContent(int layer, xmlDocPtr doc, xmlNodePtr root);

    void parseXcsFile(int layer, rtl::OUString const & url);

    void parseXcuNode(
        int layer, rtl::OUString const & componentName, xmlDocPtr doc,
        xmlNodePtr xmlNode, rtl::Reference< Node > const & node,
        bool inheritedFinalized, bool modifications,
        rtl::OUString const & pathPrefix);

    void parseXcuContent(int layer, xmlDocPtr doc, xmlNodePtr root);

    void parseXcuFile(int layer, rtl::OUString const & url);

    void parseFiles(
        int layer, rtl::OUString const & extension,
        void (Components::* parseFile)(int, rtl::OUString const &),
        rtl::OUString const & url, bool recursive);

    void parseFileList(
        int layer, void (Components::* parseFile)(int, rtl::OUString const &),
        rtl::OUString const & urls, rtl::Bootstrap const & ini);

    bool parseDataFile(
        int layer, xmlDocPtr doc, Dependencies const & dependencies);

    void parseDataFiles(int layer, rtl::OUString const & url);

    void parseXcsXcuLayer(int layer, rtl::OUString const & url);

    void parseXcsXcuIniLayer(int layer, rtl::OUString const & url);

    void parseModuleLayer(int layer, rtl::OUString const & url);

    void parseResLayer(int layer, rtl::OUString const & url);

    rtl::OUString getModificationFileUrl() const;

    void parseModificationLayer();

    NodeMap templates_;
    NodeMap components_;
    Modifications modifications_;
};

}

#endif
