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


def createHeader(model, ns):
    nsToken = ns.replace('-', '_')
    print("""
#ifndef INCLUDED_OOXML_FACTORY_%s_HXX
#define INCLUDED_OOXML_FACTORY_%s_HXX
#include "ooxml/OOXMLFactory.hxx"
#include "OOXMLFactory_generated.hxx"
#include "ooxml/OOXMLnamespaceids.hxx"
#include "ooxml/resourceids.hxx"

namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
""" % (nsToken.upper(), nsToken.upper()))

    print("""class OOXMLFactory_%s : public OOXMLFactory_ns
{
public:
    typedef boost::intrusive_ptr <OOXMLFactory_ns> Pointer_t;

    static Pointer_t getInstance();

    virtual AttributeToResourceMapPointer createAttributeToResourceMap(Id nId);
    virtual ListValueMapPointer createListValueMap(Id nId);
    virtual CreateElementMapPointer createCreateElementMap(Id nId);
    virtual TokenToIdMapPointer createTokenToIdMap(Id nId);
#ifdef DEBUG_DOMAINMAPPER
    virtual string getDefineName(Id nId) const;
#endif
""" % nsToken)

    actions = []
    for nsNode in [i for i in model.getElementsByTagName("namespace") if i.getAttribute("name") == ns]:
        for resource in nsNode.getElementsByTagName("resource"):
            for action in [i.getAttribute("name") for i in resource.childNodes if i.nodeType == minidom.Node.ELEMENT_NODE and i.tagName == "action"]:
                if action != "characters" and action not in actions:
                    actions.append(action)
    for action in actions:
        print("    void %sAction(OOXMLFastContextHandler* pHandler);" % action)

    print("""virtual void charactersAction(OOXMLFastContextHandler* pHandler, const OUString & sText);
    virtual void attributeAction(OOXMLFastContextHandler* pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue);

#ifdef DEBUG_DOMAINMAPPER
    virtual string getName() const;
#endif


    virtual ~OOXMLFactory_%s();

protected:
    static Pointer_t m_pInstance;

    OOXMLFactory_%s();
};
""" % (nsToken, nsToken))

    print("""/// @endcond
}}
#endif //INCLUDED_OOXML_FACTORY_%s_HXX""" % nsToken.upper())

modelPath = sys.argv[1]
filePath = sys.argv[2]
model = minidom.parse(modelPath)
ns = filePath.split('OOXMLFactory_')[1].split('.hxx')[0]
createHeader(model, ns)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
