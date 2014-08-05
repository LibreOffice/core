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


# factoryMutexDecl


def factoryMutexDecl(nsLabel):
    print("typedef rtl::Static<osl::Mutex, OOXMLFactory_%s> OOXMLFactory_%s_Mutex;" % (nsLabel, nsLabel))
    print()


# factoryConstructor


def factoryConstructor(nsLabel):
    print("""OOXMLFactory_%s::OOXMLFactory_%s()
{
    // multi-thread-safe mutex for all platforms

    osl::MutexGuard aGuard(OOXMLFactory_%s_Mutex::get());
}""" % (nsLabel, nsLabel, nsLabel))
    print()


# factoryDestructor


def factoryDestructor(nsLabel):
    print("""OOXMLFactory_%s::~OOXMLFactory_%s()
{
}""" % (nsLabel, nsLabel))
    print()


# factoryGetInstance


def factoryGetInstance(nsLabel):
    print("""OOXMLFactory_ns::Pointer_t OOXMLFactory_%s::m_pInstance;

OOXMLFactory_ns::Pointer_t OOXMLFactory_%s::getInstance()
{
    if (m_pInstance.get() == NULL)
        m_pInstance.reset(new OOXMLFactory_%s());

    return m_pInstance;
}""" % (nsLabel, nsLabel, nsLabel))
    print()


# factoryAttributeToResourceMap


def nsToLabel(nsNode):
    return nsNode.getAttribute("name").replace('-', '_')


def getChildByName(parentNode, childName):
    elementNodes = [i for i in parentNode.childNodes if i.localName == childName]
    assert len(elementNodes) == 1
    return elementNodes[0]


def resourceForAttribute(nsNode, attrNode):
    resourceName = ""

    for refNode in getChildrenByName(attrNode, "ref"):
        refName = refNode.getAttribute("name")
        for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == refName]:
            resourceName = resourceNode.getAttribute("resource")
            break
        if not len(resourceName):
            for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                resourceName = resourceForAttribute(nsNode, define)
                break
        if len(resourceName):
            break

    if not len(resourceName):
        if len(attrNode.getElementsByTagName("text") + [i for i in attrNode.getElementsByTagName("data") if i.getAttribute("type") in ("base64Binary", "string")]):
            resourceName = "String"
        elif len([i for i in attrNode.getElementsByTagName("data") if i.getAttribute("type") == "boolean"]):
            resourceName = "Boolean"
        elif len([i for i in attrNode.getElementsByTagName("data") if i.getAttribute("type") in ("unsignedInt", "integer", "int")]):
            resourceName = "Integer"
    return resourceName


def idForNamespace(nsNode):
    return "NN_%s" % nsNode.getAttribute("name").replace('-', '_')


def localIdForDefine(defineNode):
    return "DEFINE_%s" % defineNode.getAttribute("name")


def idForDefine(nsNode, defineNode):
    return "%s|%s" % (idForNamespace(nsNode), localIdForDefine(defineNode))


def fastNamespace(attrNode):
    return "NS_%s" % attrNode.getAttribute("prefix")


def fastLocalName(attrNode):
    return "oox::XML_%s" % attrNode.getAttribute("localname")


def fastToken(attrNode):
    ret = []
    if len(attrNode.getAttribute("prefix")):
        ret.append("%s|" % fastNamespace(attrNode))
    ret.append(fastLocalName(attrNode))
    return "".join(ret)


def factoryAttributeToResourceMapInner(nsNode, defineNode):
    ret = []
    defineName = defineNode.getAttribute("name")
    for refNode in getChildrenByName(defineNode, "ref"):
        refName = refNode.getAttribute("name")
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            ret.extend(factoryAttributeToResourceMapInner(nsNode, define))

    attrNodes = defineNode.getElementsByTagName("attribute")
    for attrNode in attrNodes:
        attrToken = fastToken(attrNode)
        if attrNode == attrNodes[0]:
            ret.append("      // %s" % defineName)
        resourceName = resourceForAttribute(nsNode, attrNode)
        refDefine = "0"
        if len(resourceName):
            for refNode in attrNode.getElementsByTagName("ref"):
                refName = refNode.getAttribute("name")
                for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    refDefine = idForDefine(nsNode, define)
            ret.append("        (*pMap)[%s] = AttributeInfo(RT_%s, %s);" % (attrToken, resourceName, refDefine))
        else:
            ret.append("      // empty resource: %s" % fastToken(attrNode))

    return ret


def factoryAttributeToResourceMap(nsNode):
    print("""AttributeToResourceMapPointer OOXMLFactory_%s::createAttributeToResourceMap(Id nId)
{
    AttributeToResourceMapPointer pMap(new AttributeToResourceMap());

    switch (nId)
    {""" % nsToLabel(nsNode))
    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = "\n".join(factoryAttributeToResourceMapInner(nsNode, defineNode))
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print(inner)
            print("        break;")

    print("""    default:
        break;
    }

    return pMap;
}""")
    print()


# factoryGetListValue


def idToLabel(idName):
    if ":" in idName:
        ns, ln = idName.split(':')
        return "NS_%s::LN_%s" % (ns, ln)
    else:
        return idName

def appendValueData(values, name, value):
    first = name[0:1]

    if not (first in values):
        values[first] = []

    values[first].append([name, value])

def printValueData(values):
    if "" in values:
        output_else = ""
        for i in values[""]:
            print("        %sif (rValue == \"%s\") { rOutValue = %s; return true; }" % (output_else, i[0], i[1]))
            output_else = "else "
        print("        else switch (rValue[0])")
    else:
        print("        if (rValue.isEmpty())")
        print("            return false;")
        print("        switch (rValue[0])")

    print("        {")
    for k in sorted(values.keys()):
        if k != "":
            print("        case '%s':" % k)
            output_else = ""
            for i in values[k]:
                print("            %sif (rValue == \"%s\") { rOutValue = %s; }" % (output_else, i[0], i[1]))
                output_else = "else "
            print("            else { return false; }")
            print("            return true;")
            print("            break;")
    print("        }")

def factoryGetListValue(nsNode):
    print("""bool OOXMLFactory_%s::getListValue(Id nId, const OUString& rValue, sal_uInt32& rOutValue)
{
    (void) rValue;
    (void) rOutValue;

    switch (nId)
    {""" % nsToLabel(nsNode))

    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("resource") == "List"]:
        print("    case %s:" % idForDefine(nsNode, resourceNode))
        values = {}
        for valueNode in getChildrenByName(resourceNode, "value"):
            valueData = ""
            if len(valueNode.childNodes):
                valueData = valueNode.childNodes[0].data
            appendValueData(values, valueData, idToLabel(valueNode.getAttribute("tokenid")))
        printValueData(values)
        print("        return false;")
        print("        break;")

    print("""    default:
        break;
    }

    return false;
}""")


# factoryCreateElementMap


def contextResource(files, nsNode, refNode):
    refName = refNode.getAttribute("name")
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == refName]:
        return resourceNode.getAttribute("resource")

    for includeNode in getChildrenByName(getChildByName(nsNode, "grammar"), "include"):
        namespaceNode = files[includeNode.getAttribute("href")]
        for resourceNode in [i for i in getChildrenByName(namespaceNode, "resource") if i.getAttribute("name") == refName]:
            return resourceNode.getAttribute("resource")

    if refName == "BUILT_IN_ANY_TYPE":
        return "Any"
    else:
        for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
            for resourceNode in [i for i in getChildrenByName(namespaceNode, "resource") if i.getAttribute("name") == refName]:
                return resourceNode.getAttribute("resource")
        return ""


def idForRef(nsNode, refNode):
    refName = refNode.getAttribute("name")
    result1 = ""
    for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
        result1 = idForDefine(nsNode, define)
    if refName == "BUILT_IN_ANY_TYPE":
        return "0"
    elif result1 == "":
        for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
            for define in [i for i in getChildrenByName(getChildByName(namespaceNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                return idForDefine(namespaceNode, define)
    else:
        return result1


def factoryCreateElementMapInner(files, nsNode, defineNode, resourceNamespaceNode=None):
    if not resourceNamespaceNode:
        resourceNamespaceNode = nsNode
    ret = []
    for refNode in defineNode.getElementsByTagName("ref"):
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        refName = refNode.getAttribute("name")

        block = []
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            block = factoryCreateElementMapInner(files, nsNode, define)

        if len(block) == 0:
            block1 = []
            for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
                for define in [i for i in getChildrenByName(getChildByName(namespaceNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    block1.extend(factoryCreateElementMapInner(files, namespaceNode, define, nsNode))
        else:
            block1 = block

        if len(block1):
            ret.append("         /* ref: %s*/" % refName)
            ret.extend(block1)

    for elementNode in defineNode.getElementsByTagName("element"):
        resource = ""
        for refNode in getChildrenByName(elementNode, "ref"):
            refName = refNode.getAttribute("name")
            resource = contextResource(files, resourceNamespaceNode, refNode)
            if len(resource):
                break
        if len(resource):
            ret.append("        (*pMap)[%s] = CreateElement(RT_%s, %s);" % (fastToken(elementNode), resource, idForRef(nsNode, getChildByName(elementNode, "ref"))))

    return ret


def factoryCreateElementMapFromStart(files, nsNode):
    for startNode in getChildrenByName(nsNode, "start"):
        startName = startNode.getAttribute("name")
        block = None
        for defineNode in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == startName]:
            block = factoryCreateElementMapInner(files, nsNode, defineNode)
        print("        /* start: %s*/" % startName)
        if block:
            print("\n".join(block))
    print("""        break;
    }

    return pMap;
}""")
    print()


def factoryCreateElementMap(files, nsNode):
    print("""CreateElementMapPointer OOXMLFactory_%s::createCreateElementMap(Id nId)
{
    CreateElementMapPointer pMap(new CreateElementMap());

    switch (nId)
    {""" % nsToLabel(nsNode))

    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = "\n".join(factoryCreateElementMapInner(files, nsNode, defineNode))
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print(inner)
            print("        break;")
    print("    default:")
    factoryCreateElementMapFromStart(files, nsNode)


# factoryActions


def charactersActionForValues(nsNode, refNode):
    ret = []

    refName = refNode.getAttribute("name")
    for defineNode in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if defineNode.getAttribute("name") == refName]:
        ret.append("    {")
        ret.append("    // %s" % defineNode.getAttribute("name"))
        for dataNode in getChildrenByName(defineNode, "data"):
            if dataNode.getAttribute("type") == "int":
                ret.append("    OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(sText));")
                ret.append("    pValueHandler->setValue(pValue);")
        ret.append("    }")

    return ret


def factoryChooseAction(actionNode):
    ret = []
    if actionNode.hasAttribute("tokenid"):
        ret.append("    if (sal::static_int_cast<Id>(pHandler->getId()) == %s)" % idToLabel(actionNode.getAttribute("tokenid")))
        ret.append("    {")
    for condNode in getChildrenByName(actionNode, "cond"):
        ret.append("    {")
        ret.append("        OOXMLPropertySetEntryToInteger aHandler(%s);" % idToLabel(condNode.getAttribute("tokenid")))
        ret.append("        dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler)->getPropertySetAttrs()->resolve(aHandler);")
        ret.append("")
        ret.append("        if (sal::static_int_cast<Id>(aHandler.getValue()) == %s)" % idToLabel(condNode.getAttribute("value")))
        ret.append("        {")

    if actionNode.getAttribute("action") in ("handleXNotes", "handleHdrFtr", "handleComment", "handlePicture", "handleBreak", "handleOLE", "handleFontRel"):
        ret.append("    dynamic_cast<OOXMLFastContextHandlerProperties*>(pHandler)->%s();" % actionNode.getAttribute("action"))
    elif actionNode.getAttribute("action") == "propagateCharacterPropertiesAsSet":
        ret.append("    pHandler->propagateCharacterPropertiesAsSet(%s);" % idToLabel(actionNode.getAttribute("sendtokenid")))
    elif actionNode.getAttribute("action") in ("startCell", "endCell"):
        ret.append("    dynamic_cast<OOXMLFastContextHandlerTextTableCell*>(pHandler)->%s();" % actionNode.getAttribute("action"))
    elif actionNode.getAttribute("action") in ("startRow", "endRow"):
        ret.append("    dynamic_cast<OOXMLFastContextHandlerTextTableRow*>(pHandler)->%s();" % actionNode.getAttribute("action"))
    elif actionNode.getAttribute("action") == "handleGridBefore":
        ret.append("    dynamic_cast<OOXMLFastContextHandlerTextTableRow*>(pHandler)->%s();" % actionNode.getAttribute("action"))
    elif actionNode.getAttribute("action") in ("sendProperty", "handleHyperlink"):
        ret.append("    dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler)->%s();" % actionNode.getAttribute("action"))
    elif actionNode.getAttribute("action") == "fieldstart":
        ret.append("    pHandler->startField();")
    elif actionNode.getAttribute("action") == "fieldsep":
        ret.append("    pHandler->fieldSeparator();")
    elif actionNode.getAttribute("action") == "fieldend":
        ret.append("    pHandler->endField();")
    elif actionNode.getAttribute("action") == "printproperty":
        ret.append("    dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler)->sendProperty(%s);" % idToLabel(actionNode.getAttribute("sendtokenid")))
    elif actionNode.getAttribute("action") == "sendPropertiesWithId":
        ret.append("    pHandler->sendPropertiesWithId(%s);" % idToLabel(actionNode.getAttribute("sendtokenid")))
    elif actionNode.getAttribute("action") == "text":
        ret.append("    pHandler->text(sText);")
    elif actionNode.getAttribute("action") == "positionOffset":
        ret.append("    pHandler->positionOffset(sText);")
    elif actionNode.getAttribute("action") == "positivePercentage":
        ret.append("    pHandler->positivePercentage(sText);")
    elif actionNode.getAttribute("action") == "alignH":
        ret.append("    pHandler->alignH(sText);")
    elif actionNode.getAttribute("action") == "alignV":
        ret.append("    pHandler->alignV(sText);")
    elif actionNode.getAttribute("action") == "tokenproperty":
        ret.append("    OOXMLFastHelper<OOXMLIntegerValue>::newProperty(pHandler, %s, pHandler->getToken());" % idToLabel("ooxml:token"))
    else:
        ret.append("    pHandler->%s();" % actionNode.getAttribute("action"))

    for condNode in getChildrenByName(actionNode, "cond"):
        ret.append("        }")
        ret.append("    }")
    if actionNode.hasAttribute("tokenid"):
        ret.append("    }")

    return ret


def factoryAction(nsNode, action):
    switchblock1 = []
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if len([j for j in getChildrenByName(i, "action") if j.getAttribute("name") == action])]:
        switchblock1.append("    case %s:" % idForDefine(nsNode, resourceNode))
        for actionNode in [i for i in getChildrenByName(resourceNode, "action") if i.getAttribute("name") == action]:
            switchblock1.extend(factoryChooseAction(actionNode))
        switchblock1.append("    break;")
        switchblock1.append("")

    switchblock2 = []
    if action == "characters":
        for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("resource") == "Value"]:
            if not len(getChildrenByName(resourceNode, "attribute")):
                resourceName = resourceNode.getAttribute("name")
                switchblock2.append("    case %s:" % idForDefine(nsNode, resourceNode))
                ret = []
                for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == resourceName]:
                    for refNode in getChildrenByName(define, "ref"):
                        ret.extend(charactersActionForValues(nsNode, refNode))
                switchblock2.extend(ret)
                switchblock2.append("    break;")

    sys.stdout.write("void OOXMLFactory_%s::%sAction(OOXMLFastContextHandler*" % (nsToLabel(nsNode), action))
    if len(switchblock1) or len(switchblock2):
        sys.stdout.write(" pHandler")
    if action == "characters":
        sys.stdout.write(", const OUString&")
        if "sText" in "".join(switchblock1) or "sText" in "".join(switchblock2):
            sys.stdout.write(" sText")
    print(")")
    print("{")
    if len(switchblock1) or len(switchblock2):
        print("    sal_uInt32 nDefine = pHandler->getDefine();")
    if len(switchblock1):
        print("    switch (nDefine)")
        print("    {")
        print("")
        if switchblock1[-1] == "":
            switchblock1 = switchblock1[:-1]
        print("\n        ".join(switchblock1))
        print("    default:")
        print("        break;")
        print("    }")
    if len(switchblock2):
        print("    switch (nDefine)")
        print("    {")
        print("\n        ".join(switchblock2))
        print("    default:")
        print("        break;")
        print("    }")
    print("}")


def factoryActions(nsNode):
    actions = []
    for resourceNode in getChildrenByName(nsNode, "resource"):
        for actionNode in getChildrenByName(resourceNode, "action"):
            actionName = actionNode.getAttribute("name")
            if actionName != "characters" and actionName not in actions:
                actions.append(actionName)
    for action in sorted(actions):
        factoryAction(nsNode, action)
        print()
        print()
    factoryAction(nsNode, "characters")
    print()


# factoryGetDefineName


def factoryGetDefineName(nsNode):
    print("""#ifdef DEBUG_DOMAINMAPPER
string OOXMLFactory_%s::getDefineName(Id nId) const
{
    static IdToStringMapPointer pMap;

    if (pMap.get() == NULL)
    {
        pMap = IdToStringMapPointer(new IdToStringMap());

""" % nsToLabel(nsNode))
    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        print("""        (*pMap)[%s] = "%s";""" % (idForDefine(nsNode, defineNode), defineNode.getAttribute("name")))
    print("""    }

    return (*pMap)[nId];
}
#endif

""")


# factoryTokenToIdMap


def factoryTokenToIdMapInner(nsNode, defineNode):
    ret = []
    for refNode in defineNode.getElementsByTagName("ref"):
        refName = refNode.getAttribute("name")
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        refblock1 = []
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            refblock1.extend(factoryTokenToIdMapInner(nsNode, define))
        if not len(refblock1):
            for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
                for define in [i for i in getChildrenByName(getChildByName(namespaceNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    ret.extend(factoryTokenToIdMapInner(namespaceNode, define))
        else:
            ret.extend(refblock1)

    body = []
    defineName = defineNode.getAttribute("name")
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == defineName]:
        for node in [i for i in resourceNode.childNodes if i.localName in ("element", "attribute")]:
            if node.hasAttribute("tokenid"):
                body.append("(*pMap)[%s] = %s;" % (fastToken(node), idToLabel(node.getAttribute("tokenid"))))
    if len(body):
        ret.append("// %s" % defineName)
        ret.extend(body)

    return ret


def factoryTokenToIdMap(nsNode):
    print("""TokenToIdMapPointer OOXMLFactory_%s::createTokenToIdMap(Id nId)
{
    TokenToIdMapPointer pMap(new TokenToIdMap());

    switch (nId)
    {""" % nsToLabel(nsNode))
    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = "\n".join(factoryTokenToIdMapInner(nsNode, defineNode))
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print(inner)
            print("        break;")
    print("    default:")
    for startNode in getChildrenByName(nsNode, "start"):
        startName = startNode.getAttribute("name")
        print("    // %s" % startName)
        for defineNode in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == startName]:
            inner = factoryTokenToIdMapInner(nsNode, defineNode)
            if len(inner):
                print("\n".join(inner))
    print("""        break;
    }

    return pMap;
}
""")


# factoryAttributeAction


def factoryAttributeActionDefineInner(nsNode, defineNode):
    ret = []

    defineName = defineNode.getAttribute("name")
    block = []
    output_else = ""
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == defineName]:
        for attributeNode in getChildrenByName(resourceNode, "attribute"):
            if attributeNode.hasAttribute("action"):
                block.append("            %sif (nToken == static_cast<Token_t>(%s))" % (output_else, fastToken(attributeNode)))
                block.append("                pHandler->%s(pValue);" % attributeNode.getAttribute("action"))
                output_else = "else "
    if len(block):
        resource = ""
        for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == defineName]:
            resource = resourceNode.getAttribute("resource")
            break
        ret.append("        {")
        ret.append("            OOXMLFastContextHandler%s* pHandler = dynamic_cast<OOXMLFastContextHandler%s*>(_pHandler);" % (resource, resource))
        ret.extend(block)
        ret.append("        }")

    return ret


def factoryAttributeActionInner(nsNode):
    ret = []

    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = factoryAttributeActionDefineInner(nsNode, defineNode)
        if len(inner):
            ret.append("    case %s:" % idForDefine(nsNode, defineNode))
            ret.extend(inner)
            ret.append("        break;")

    return ret


def factoryAttributeAction(nsNode):
    nsLabel = nsToLabel(nsNode)
    inner = factoryAttributeActionInner(nsNode)
    if len(inner):
        print("""void OOXMLFactory_%s::attributeAction(OOXMLFastContextHandler* _pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue)
{
    switch (_pHandler->getDefine())
    {""" % nsLabel)
        print("\n".join(inner))
        print("    default:")
        print("        break;")
        print("    }")
        print("}")
        print()
    else:
        print("void OOXMLFactory_%s::attributeAction(OOXMLFastContextHandler*, Token_t, OOXMLValue::Pointer_t)" % nsLabel)
        print("{")
        print("}")
        print()


# createImpl


def getChildrenByName(parentNode, childName):
    return [i for i in parentNode.childNodes if i.localName == childName]


def createImpl(modelNode, nsName):
    print("""
#include "ooxml/resourceids.hxx"
#include "OOXMLFactory_%s.hxx"
#include "ooxml/OOXMLFastHelper.hxx"
#include "oox/token/tokens.hxx"

#ifdef _MSC_VER
#pragma warning(disable:4065) // switch statement contains 'default' but no 'case' labels
#endif

namespace writerfilter {
namespace ooxml {

/// @cond GENERATED""" % nsName)
    print()

    files = {}
    for nsNode in getChildrenByName(modelNode, "namespace"):
        if nsNode.hasAttribute("file"):
            files[nsNode.getAttribute("file")] = nsNode

    for nsNode in [i for i in getChildrenByName(modelNode, "namespace") if i.getAttribute("name") == nsName]:
        nsLabel = nsToLabel(nsNode)

        factoryMutexDecl(nsLabel)
        factoryConstructor(nsLabel)
        factoryDestructor(nsLabel)
        factoryGetInstance(nsLabel)
        factoryAttributeToResourceMap(nsNode)
        factoryGetListValue(nsNode)
        factoryCreateElementMap(files, nsNode)
        factoryActions(nsNode)
        factoryGetDefineName(nsNode)
        factoryTokenToIdMap(nsNode)
        factoryAttributeAction(nsNode)

    print("""/// @endcond
}}""")


def main():
    modelPath = sys.argv[1]
    filePath = sys.argv[2]
    modelNode = getChildByName(minidom.parse(modelPath), "model")
    nsName = filePath.split('OOXMLFactory_')[1].split('.cxx')[0]
    createImpl(modelNode, nsName)

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
