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
        if len([i for i in attrNode.getElementsByTagName("data") if i.getAttribute("type") in ("base64Binary", "string")]):
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
    return "oox::NMSP_%s" % attrNode.getAttribute("prefix")


def fastLocalName(attrNode):
    if len(attrNode.getAttribute("localname")):
        return "oox::XML_%s" % attrNode.getAttribute("localname")
    else:
        return "oox::XML_TOKEN_COUNT"


def fastToken(attrNode):
    ret = []
    if len(attrNode.getAttribute("prefix")):
        ret.append("%s|" % fastNamespace(attrNode))
    ret.append(fastLocalName(attrNode))
    return "".join(ret)


def collectAttributeToResource(nsNode, defineNode):
    ret_dict = {}
    ret_order = []
    for refNode in getChildrenByName(defineNode, "ref"):
        refName = refNode.getAttribute("name")
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            ret = collectAttributeToResource(nsNode, define)
            ret_dict.update(ret[0])
            ret_order.extend(ret[1])

    attrNodes = defineNode.getElementsByTagName("attribute")
    for attrNode in attrNodes:
        attrToken = fastToken(attrNode)
        resourceName = resourceForAttribute(nsNode, attrNode)
        refDefine = "0"
        if len(resourceName):
            for refNode in attrNode.getElementsByTagName("ref"):
                refName = refNode.getAttribute("name")
                for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    refDefine = idForDefine(nsNode, define)
            ret_dict[attrToken] = "ResourceType::%s, %s" % (resourceName, refDefine)
            ret_order.append(attrToken)

    return [ret_dict, ret_order]


def factoryAttributeToResourceMapInner(nsNode, defineNode):
    ret = []
    attributes = collectAttributeToResource(nsNode, defineNode)
    already_used = set()
    for k in attributes[1]:
        if not (k in already_used):
            ret.append("                { %s, %s }," % (k, attributes[0][k]))
            already_used.add(k)

    return ret


def factoryAttributeToResourceMap(nsNode):
    print("""const AttributeInfo* OOXMLFactory_%s::getAttributeInfoArray(Id nId)
{
    switch (nId)
    {""" % nsToLabel(nsNode))
    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = "\n".join(factoryAttributeToResourceMapInner(nsNode, defineNode))
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print("        {")
            print("            const static AttributeInfo info[] = {")
            print(inner)
            print("                { -1, ResourceType::NoResource, 0 }")
            print("            };")
            print("            return info;")
            print("        }")
            print("        break;")

    print("""    default:
        break;
    }

    return NULL;
}""")
    print()


# factoryGetListValue


def idToLabel(idName):
    ns, ln = idName.split(':')
    return "NS_%s::LN_%s" % (ns, ln)


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
}
""")


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
    ret = {}
    for refNode in defineNode.getElementsByTagName("ref"):
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        refName = refNode.getAttribute("name")

        block = {}
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            block = factoryCreateElementMapInner(files, nsNode, define)

        if len(block) == 0:
            block1 = {}
            for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
                for define in [i for i in getChildrenByName(getChildByName(namespaceNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    block1.update(factoryCreateElementMapInner(files, namespaceNode, define, nsNode))
        else:
            block1 = block

        if len(block1):
            ret.update(block1)

    for elementNode in defineNode.getElementsByTagName("element"):
        resource = ""
        for refNode in getChildrenByName(elementNode, "ref"):
            refName = refNode.getAttribute("name")
            resource = contextResource(files, resourceNamespaceNode, refNode)
            if len(resource):
                break
        if len(resource):
            ret[fastToken(elementNode)] = "        case %s: rOutResource = ResourceType::%s; rOutElement = %s; break;" % (fastToken(elementNode), resource, idForRef(nsNode, getChildByName(elementNode, "ref")))

    return ret


def factoryCreateElementMapFromStart(files, nsNode):
    for startNode in getChildrenByName(nsNode, "start"):
        startName = startNode.getAttribute("name")
        block = None
        for defineNode in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == startName]:
            block = factoryCreateElementMapInner(files, nsNode, defineNode)
        print("        /* start: %s*/" % startName)
        if block:
            for k in block.keys():
                print(block[k])


def factoryCreateElementMap(files, nsNode):
    print("""bool OOXMLFactory_%s::getElementId(Id nDefine, Id nId, ResourceType& rOutResource, Id& rOutElement)
{
    (void) rOutResource;
    (void) rOutElement;

    switch (nDefine)
    {""" % nsToLabel(nsNode))

    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = factoryCreateElementMapInner(files, nsNode, defineNode)
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print("        switch (nId)")
            print("        {")
            for k in sorted(inner.keys()):
                print(inner[k])
            print("        default: return false;")
            print("        }")
            print("        return true;")
            print("        break;")
    print("    default:")
    print("        switch (nId)")
    print("        {")
    factoryCreateElementMapFromStart(files, nsNode)
    print("""        default: return false;
        }
        return true;
        break;
    }

    return false;
}
""")


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
    extra_space = ""
    if actionNode.hasAttribute("tokenid"):
        ret.append("    if (sal::static_int_cast<Id>(pHandler->getId()) == %s)" % idToLabel(actionNode.getAttribute("tokenid")))
        ret.append("    {")
        extra_space = "    "
    for condNode in getChildrenByName(actionNode, "cond"):
        ret.append("    {")
        ret.append("        OOXMLPropertySetEntryToInteger aHandler(%s);" % idToLabel(condNode.getAttribute("tokenid")))
        ret.append("        if (OOXMLFastContextHandlerStream* pStream = dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler))")
        ret.append("            pStream->getPropertySetAttrs()->resolve(aHandler);")
        ret.append("")
        ret.append("        if (sal::static_int_cast<Id>(aHandler.getValue()) == %s)" % idToLabel(condNode.getAttribute("value")))
        ret.append("        {")
        extra_space = "        "

    if actionNode.getAttribute("action") in ("handleXNotes", "handleHdrFtr", "handleComment", "handlePicture", "handleBreak", "handleOLE", "handleFontRel", "handleHyperlinkURL"):
        ret.append("    %sif (OOXMLFastContextHandlerProperties* pProperties = dynamic_cast<OOXMLFastContextHandlerProperties*>(pHandler))" % extra_space)
        ret.append("    %s    pProperties->%s();" % (extra_space, actionNode.getAttribute("action")))
    elif actionNode.getAttribute("action") == "propagateCharacterPropertiesAsSet":
        ret.append("    %spHandler->propagateCharacterPropertiesAsSet(%s);" % (extra_space, idToLabel(actionNode.getAttribute("sendtokenid"))))
    elif actionNode.getAttribute("action") in ("startCell", "endCell"):
        ret.append("    %sif (OOXMLFastContextHandlerTextTableCell* pTextTableCell = dynamic_cast<OOXMLFastContextHandlerTextTableCell*>(pHandler))" % extra_space)
        ret.append("    %s    pTextTableCell->%s();" % (extra_space, actionNode.getAttribute("action")))
    elif actionNode.getAttribute("action") in ("startRow", "endRow"):
        ret.append("    %sif (OOXMLFastContextHandlerTextTableRow* pTextTableRow = dynamic_cast<OOXMLFastContextHandlerTextTableRow*>(pHandler))" % extra_space)
        ret.append("    %s    pTextTableRow->%s();" % (extra_space, actionNode.getAttribute("action")))
    elif actionNode.getAttribute("action") == "handleGridBefore" or actionNode.getAttribute("action") == "handleGridAfter":
        ret.append("    %sif (OOXMLFastContextHandlerTextTableRow* pTextTableRow = dynamic_cast<OOXMLFastContextHandlerTextTableRow*>(pHandler))" % extra_space)
        ret.append("    %s    pTextTableRow->%s();" % (extra_space, actionNode.getAttribute("action")))
    elif actionNode.getAttribute("action") in ("sendProperty", "handleHyperlink"):
        ret.append("    %sif (OOXMLFastContextHandlerStream* pStream = dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler))" % extra_space)
        ret.append("    %s    pStream->%s();" % (extra_space, actionNode.getAttribute("action")))
    elif actionNode.getAttribute("action") == "fieldstart":
        ret.append("    %spHandler->startField();" % (extra_space))
    elif actionNode.getAttribute("action") == "fieldsep":
        ret.append("    %spHandler->fieldSeparator();" % (extra_space))
    elif actionNode.getAttribute("action") == "fieldend":
        ret.append("    %spHandler->endField();" % (extra_space))
    elif actionNode.getAttribute("action") == "fieldlock":
        ret.append("    %s{" % (extra_space))
        ret.append("        %sOOXMLPropertySetEntryToBool aHandler(NS_ooxml::LN_CT_FldChar_fldLock);" % (extra_space))
        ret.append("        %sif (OOXMLFastContextHandlerStream* pStream = dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler))" % (extra_space))
        ret.append("            %spStream->getPropertySetAttrs()->resolve(aHandler);" % (extra_space))
        ret.append("        %sif (aHandler.getValue())" % (extra_space))
        ret.append("            %spHandler->lockField();" % (extra_space))
        ret.append("    %s}" % (extra_space))
    elif actionNode.getAttribute("action") == "printproperty":
        ret.append("    %sif (OOXMLFastContextHandlerStream* pStream = dynamic_cast<OOXMLFastContextHandlerStream*>(pHandler))" % extra_space)
        ret.append("    %s    pStream->sendProperty(%s);" % (extra_space, idToLabel(actionNode.getAttribute("sendtokenid"))))
    elif actionNode.getAttribute("action") == "sendPropertiesWithId":
        ret.append("    %spHandler->sendPropertiesWithId(%s);" % (extra_space, idToLabel(actionNode.getAttribute("sendtokenid"))))
    elif actionNode.getAttribute("action") == "text":
        ret.append("    %spHandler->text(sText);" % (extra_space))
    elif actionNode.getAttribute("action") == "positionOffset":
        ret.append("    %spHandler->positionOffset(sText);" % (extra_space))
    elif actionNode.getAttribute("action") == "positivePercentage":
        ret.append("    %spHandler->positivePercentage(sText);" % (extra_space))
    elif actionNode.getAttribute("action") == "alignH":
        ret.append("    %spHandler->alignH(sText);" % (extra_space))
    elif actionNode.getAttribute("action") == "alignV":
        ret.append("    %spHandler->alignV(sText);" % (extra_space))
    elif actionNode.getAttribute("action") == "tokenproperty":
        ret.append("    %sOOXMLFastHelper<OOXMLIntegerValue>::newProperty(pHandler, %s, pHandler->getToken());" % (extra_space, idToLabel("ooxml:token")))
    else:
        ret.append("    %spHandler->%s();" % (extra_space, actionNode.getAttribute("action")))

    for condNode in getChildrenByName(actionNode, "cond"):
        ret.append("        }")
        ret.append("    }")
    if actionNode.hasAttribute("tokenid"):
        ret.append("    }")

    return ret


def factoryAction(nsNode, action):
    switchblock1 = []
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if len([j for j in getChildrenByName(i, "action") if j.getAttribute("name") == action])]:
        switchblock1.append("case %s:" % idForDefine(nsNode, resourceNode))
        for actionNode in [i for i in getChildrenByName(resourceNode, "action") if i.getAttribute("name") == action]:
            switchblock1.extend(factoryChooseAction(actionNode))
        switchblock1.append("    break;")
        switchblock1.append("")

    switchblock2 = []
    if action == "characters":
        for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("resource") == "Value"]:
            if not len(getChildrenByName(resourceNode, "attribute")):
                resourceName = resourceNode.getAttribute("name")
                switchblock2.append("case %s:" % idForDefine(nsNode, resourceNode))
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
        if switchblock1[-1] == "":
            switchblock1 = switchblock1[:-1]
        sys.stdout.write("    ")
        print("\n    ".join(switchblock1))
        print()
        print("    default:")
        print("        break;")
        print("    }")
    if len(switchblock2):
        print("    switch (nDefine)")
        print("    {")
        print("\n    ".join(switchblock2))
        print()
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
    factoryAction(nsNode, "characters")
    print()


# factoryGetResourceId


def collectTokenToId(nsNode, defineNode):
    ret = {}
    for refNode in defineNode.getElementsByTagName("ref"):
        refName = refNode.getAttribute("name")
        parent = refNode.parentNode
        if parent.localName in ("element", "attribute"):
            continue
        refblock1 = {}
        for define in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == refName]:
            refblock1.update(collectTokenToId(nsNode, define))
        if not len(refblock1):
            for namespaceNode in getChildrenByName(nsNode.parentNode, "namespace"):
                for define in [i for i in getChildrenByName(getChildByName(namespaceNode, "grammar"), "define") if i.getAttribute("name") == refName]:
                    ret.update(collectTokenToId(namespaceNode, define))
        else:
            ret.update(refblock1)

    defineName = defineNode.getAttribute("name")
    for resourceNode in [i for i in getChildrenByName(nsNode, "resource") if i.getAttribute("name") == defineName]:
        for node in [i for i in resourceNode.childNodes if i.localName in ("element", "attribute")]:
            if node.hasAttribute("tokenid"):
                ret[fastToken(node)] = idToLabel(node.getAttribute("tokenid"))

    return ret


def factoryTokenToIdMapInner(nsNode, defineNode):
    ids = collectTokenToId(nsNode, defineNode)
    ret = []
    for i in sorted(ids.keys()):
        ret.append("        case %s: return %s;" % (i, ids[i]))

    return ret


def factoryGetResourceId(nsNode):
    print("""Id OOXMLFactory_%s::getResourceId(Id nDefine, sal_Int32 nToken)
{
    (void) nDefine;
    (void) nToken;

    switch (nDefine)
    {""" % nsToLabel(nsNode))
    for defineNode in getChildrenByName(getChildByName(nsNode, "grammar"), "define"):
        inner = "\n".join(factoryTokenToIdMapInner(nsNode, defineNode))
        if len(inner):
            print("    case %s:" % idForDefine(nsNode, defineNode))
            print("        switch (nToken)")
            print("        {")
            print(inner)
            print("        }")
            print("        break;")
    print("    default:")
    print("        switch (nToken)")
    print("        {")
    for startNode in getChildrenByName(nsNode, "start"):
        startName = startNode.getAttribute("name")
        for defineNode in [i for i in getChildrenByName(getChildByName(nsNode, "grammar"), "define") if i.getAttribute("name") == startName]:
            inner = factoryTokenToIdMapInner(nsNode, defineNode)
            if len(inner):
                print("\n".join(inner))
    print("""        }
        break;
    }
    return 0;
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
        ret.append("        if (OOXMLFastContextHandler%s* pHandler = dynamic_cast<OOXMLFastContextHandler%s*>(_pHandler))" % (resource, resource))
        ret.append("        {")
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
        print("""void OOXMLFactory_%s::attributeAction(OOXMLFastContextHandler* _pHandler, Token_t nToken, const OOXMLValue::Pointer_t& pValue)
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
        print("void OOXMLFactory_%s::attributeAction(OOXMLFastContextHandler*, Token_t, const OOXMLValue::Pointer_t&)" % nsLabel)
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
#pragma warning(disable:4060) // switch statement contains no 'case' or 'default' labels
#pragma warning(disable:4065) // switch statement contains 'default' but no 'case' labels
#pragma warning(disable:4702) // unreachable code
#endif

namespace writerfilter {
namespace ooxml {

using namespace com::sun::star;

/// @cond GENERATED""" % nsName)
    print()

    files = {}
    for nsNode in getChildrenByName(modelNode, "namespace"):
        files[nsNode.getAttribute("name")] = nsNode

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
        factoryGetResourceId(nsNode)
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
