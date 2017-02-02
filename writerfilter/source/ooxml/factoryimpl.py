#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from __future__ import print_function
from xml.dom import minidom
import sys


def getElementsByTagNamesNS(parent, ns, names, ret=minidom.NodeList()):
    for node in parent.childNodes:
        if node.nodeType == minidom.Node.ELEMENT_NODE and node.namespaceURI == ns and node.tagName in names:
            ret.append(node)
        getElementsByTagNamesNS(node, ns, names, ret)
    return ret


def createFastChildContextFromFactory(model):
    print("""uno::Reference<xml::sax::XFastContextHandler> OOXMLFactory::createFastChildContextFromFactory
(OOXMLFastContextHandler* pHandler, OOXMLFactory_ns::Pointer_t pFactory, Token_t Element)
{
    uno::Reference <xml::sax::XFastContextHandler> aResult;
    Id nDefine = pHandler->getDefine();

    if (pFactory.get() != NULL)
    {
        ResourceType_t nResource;
        Id nElementId;
        if (pFactory->getElementId(nDefine, Element, nResource, nElementId))
        {
            Id nId = pFactory->getResourceId(nDefine, Element);

            switch (nResource)
            {""")
    resources = ["List", "Integer", "Hex", "String", "UniversalMeasure", "Boolean"]
    for resource in [r.getAttribute("resource") for r in model.getElementsByTagName("resource")]:
        if resource not in resources:
            resources.append(resource)
            print("""            case RT_%s:
                aResult.set(OOXMLFastHelper<OOXMLFastContextHandler%s>::createAndSetParentAndDefine(pHandler, Element, nId, nElementId));
                break;""" % (resource, resource))
    print("""            case RT_Any:
                aResult.set(createFastChildContextFromStart(pHandler, Element));
                break;
            default:
                break;
            }

        }
    }

    return aResult;
}
""")


def getFactoryForNamespace(model):
    print("""OOXMLFactory_ns::Pointer_t OOXMLFactory::getFactoryForNamespace(Id nId)
{
    OOXMLFactory_ns::Pointer_t pResult;

    switch (oox::getNamespace(nId))
    {""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        id = namespace.replace('-', '_')
        print("""    case NN_%s:
        pResult = OOXMLFactory_%s::getInstance();
        break;""" % (id, id))
    print("""    default:
        break;
    }

    return pResult;
}
""")


def createFastChildContextFromStart(model):
    print("""uno::Reference<xml::sax::XFastContextHandler> OOXMLFactory::createFastChildContextFromStart
(OOXMLFastContextHandler* pHandler, Token_t Element)
{
    uno::Reference<xml::sax::XFastContextHandler> aResult;
    OOXMLFactory_ns::Pointer_t pFactory;

""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        id = namespace.replace('-', '_')
        print("""    if (!aResult.is())
    {
        pFactory = getFactoryForNamespace(NN_%s);
        aResult.set(createFastChildContextFromFactory(pHandler, pFactory, Element));
    }""" % id)

    print("""
    return aResult;
}
""")


def fastTokenToId(model):
    print("""
std::string fastTokenToId(sal_uInt32 nToken)
{

    std::string sResult;
#ifdef DEBUG_WRITERFILTER

    switch (oox::getNamespace(nToken))
    {""")

    aliases = []
    for alias in sorted(ooxUrlAliases.values()):
        if alias not in aliases:
            aliases.append(alias)
            print("""    case oox::NMSP_%s:
        sResult += "%s:";
        break;""" % (alias, alias))
    print("""    }

    switch (nToken & 0xffff)
    {""")

    tokens = [""]
    for token in [t.getAttribute("localname") for t in getElementsByTagNamesNS(model, "http://relaxng.org/ns/structure/1.0", ["element", "attribute"])]:
        if token not in tokens:
            tokens.append(token)
            print("""    case oox::XML_%s:
        sResult += "%s";
        break;""" % (token, token))

    print("""    }
#else
    (void)nToken;
#endif
    return sResult;
}
""")


def getFastParser():
    print("""uno::Reference <xml::sax::XFastParser> OOXMLStreamImpl::getFastParser()
{
    if (!mxFastParser.is())
    {
        mxFastParser = css::xml::sax::FastParser::create(mxContext);
""")
    for url in sorted(ooxUrlAliases.keys()):
        print("""        mxFastParser->registerNamespace("%s", oox::NMSP_%s);""" % (url, ooxUrlAliases[url]))
    print("""    }

    return mxFastParser;
}

/// @endcond
}}""")


def createImpl(model):
    print("""
#include <com/sun/star/xml/sax/FastParser.hpp>
#include "ooxml/OOXMLFactory.hxx"
#include "ooxml/OOXMLFastHelper.hxx"
#include "ooxml/OOXMLStreamImpl.hxx"
""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        print('#include "OOXMLFactory_%s.hxx"' % namespace)

    print("""namespace writerfilter {
namespace ooxml {

using namespace com::sun::star;

/// @cond GENERATED
""")

    createFastChildContextFromFactory(model)
    getFactoryForNamespace(model)
    createFastChildContextFromStart(model)
    fastTokenToId(model)
    getFastParser()


def parseNamespaces(fro):
    sock = open(fro)
    for i in sock.readlines():
        line = i.strip()
        alias, url = line.split(' ')[1:]  # first column is ID, not interesting for us
        ooxUrlAliases[url] = alias
    sock.close()


namespacesPath = sys.argv[1]
ooxUrlAliases = {}
parseNamespaces(namespacesPath)
modelPath = sys.argv[2]
model = minidom.parse(modelPath)
createImpl(model)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
