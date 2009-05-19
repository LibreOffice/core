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

#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "libxml/parser.h"
#include "libxml/xmlschemastypes.h"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "components.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvalues.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

template< typename T > sal_Int32 findFirst(
    rtl::OUString const & string, T sub, sal_Int32 fromIndex)
{
    sal_Int32 i = string.indexOf(sub, fromIndex);
    return i == -1 ? string.getLength() : i;
}

xmlChar const * xmlString(char const * str) {
    return reinterpret_cast< xmlChar const * >(str);
}

rtl::OUString fromXmlString(xmlChar const * str) {
    char const * s = reinterpret_cast< char const * >(str);
    return s == 0
        ? rtl::OUString()
        : rtl::OUString(s, rtl_str_getLength(s), RTL_TEXTENCODING_UTF8);
}

rtl::OUString fromXmlString(xmlChar const * str, xmlChar const * end) {
    OSL_ASSERT(str != 0 && end != 0);
    return rtl::OUString(
        reinterpret_cast< char const * >(str), end - str,
        RTL_TEXTENCODING_UTF8);
}

struct XmlString: private boost::noncopyable {
    xmlChar * const str;

    explicit XmlString(xmlChar * theStr): str(theStr) {}

    ~XmlString() { xmlFree(str); }
};

struct XmlDoc: private boost::noncopyable {
    xmlDocPtr doc;

    explicit XmlDoc(xmlDocPtr theDoc): doc(theDoc) {}

    ~XmlDoc() { xmlFreeDoc(doc); }
};

rtl::OUString fullTemplateName(
    rtl::OUString const & component, rtl::OUString const & name)
{
    OSL_ASSERT(component.indexOf('/') == -1);
    rtl::OUStringBuffer buf(component);
    buf.append(sal_Unicode('/'));
    buf.append(name);
    return buf.makeStringAndClear();
}

class NodeRef: public Node {
public:
    NodeRef(
        Node * parent, rtl::OUString const & name,
        rtl::OUString const & templateName):
        Node(parent, name), templateName_(templateName) {}

    virtual rtl::Reference< Node > clone(
        Node * parent, rtl::OUString const & name) const
    { return new NodeRef(parent, name, templateName_); }

    virtual rtl::Reference< Node > getMember(rtl::OUString const &);

    rtl::OUString getTemplateName() const { return templateName_; }

private:
    virtual ~NodeRef() {}

    rtl::OUString templateName_;
};

rtl::Reference< Node > NodeRef::getMember(rtl::OUString const &) {
    throw css::uno::RuntimeException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("unresolved node-ref ")) + name_,
        0);
}

void resolveNodeRef(
    Components::TemplateMap const & templates, NodeMap::iterator it)
{
    if (NodeRef * p = dynamic_cast< NodeRef * >(it->second.get())) {
        NodeMap::const_iterator i(templates.find(p->getTemplateName()));
        if (i == templates.end()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("unknown node-ref ")) +
                 p->getTemplateName()),
                0);
        }
        rtl::Reference< Node > removed(p);
        it->second = i->second->clone(p->getParent(), p->getName());
        removed->unbind(); // must not throw
    }
}

xmlDocPtr parseXmlFile(rtl::OUString const & url) {
    rtl::OUString path1;
    if (osl::FileBase::getSystemPathFromFileURL(url, path1) !=
        osl::FileBase::E_None)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot get system path for ")) +
             url),
            0);
    }
    rtl::OString path2;
    if (!path1.convertToString(
            &path2, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot translate system path ")) +
             path1),
            0);
    }
    xmlDocPtr doc(xmlParseFile(path2.getStr()));
    if (doc == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xmlParseFile failed for ")) +
             url),
            0);
    }
    return doc;
}

xmlNodePtr skipBlank(xmlNodePtr node) {
    while (node != 0 && xmlIsBlankNode(node)) {
        node = node->next;
    }
    return node;
}

bool isOorElement(xmlNodePtr node, char const * name) {
    return node != 0 && xmlStrEqual(node->name, xmlString(name)) &&
        (node->ns == 0 ||
         xmlStrEqual(
             node->ns->href, xmlString("http://openoffice.org/2001/registry")));
}

rtl::OUString getNameAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString name(
        xmlGetNsProp(
            node, xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (name.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing name attribute in ")) +
             fromXmlString(doc->URL)),
            0);
    }
    return fromXmlString(name.str);
}

enum Operation {
    OPERATION_MODIFY, OPERATION_REPLACE, OPERATION_FUSE, OPERATION_REMOVE };

Operation getOperationAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString op(
        xmlGetNsProp(
            node, xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    if (op.str == 0 || xmlStrEqual(op.str, xmlString("modify"))) {
        return OPERATION_MODIFY;
    } else if (xmlStrEqual(op.str, xmlString("replace"))) {
        return OPERATION_REPLACE;
    } else if (xmlStrEqual(op.str, xmlString("fuse"))) {
        return OPERATION_FUSE;
    } else if (xmlStrEqual(op.str, xmlString("remove"))) {
        return OPERATION_REMOVE;
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid op attribute in ")) +
             fromXmlString(doc->URL)),
            0);
    }
}

Type getTypeAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString type(
        xmlGetNsProp(
            node, xmlString("type"),
            xmlString("http://openoffice.org/2001/registry")));
    if (type.str == 0) {
        return TYPE_NONE;
    }
    xmlChar const * p = xmlStrchr(type.str, ':');
    if (p == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
             fromXmlString(doc->URL)),
            0);
    }
    XmlString prefix(xmlStrsub(type.str, 0, p - type.str));
    xmlNsPtr ns(xmlSearchNs(doc, node, prefix.str));
    if (ns == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
             fromXmlString(doc->URL)),
            0);
    }
    if (xmlStrEqual(ns->href, xmlString("http://www.w3.org/2001/XMLSchema"))) {
        if (xmlStrEqual(p + 1, xmlString("boolean"))) {
            return TYPE_BOOLEAN;
        } else if (xmlStrEqual(p + 1, xmlString("short"))) {
            return TYPE_SHORT;
        } else if (xmlStrEqual(p + 1, xmlString("int"))) {
            return TYPE_INT;
        } else if (xmlStrEqual(p + 1, xmlString("long"))) {
            return TYPE_LONG;
        } else if (xmlStrEqual(p + 1, xmlString("double"))) {
            return TYPE_DOUBLE;
        } else if (xmlStrEqual(p + 1, xmlString("string"))) {
            return TYPE_STRING;
        } else if (xmlStrEqual(p + 1, xmlString("hexBinary"))) {
            return TYPE_HEXBINARY;
        }
    } else if (xmlStrEqual(
                   ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        if (xmlStrEqual(p + 1, xmlString("any"))) {
            return TYPE_ANY;
        } else if (xmlStrEqual(p + 1, xmlString("boolean-list"))) {
            return TYPE_BOOLEAN_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("short-list"))) {
            return TYPE_SHORT_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("int-list"))) {
            return TYPE_INT_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("long-list"))) {
            return TYPE_LONG_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("double-list"))) {
            return TYPE_DOUBLE_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("string-list"))) {
            return TYPE_STRING_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("hexBinary-list"))) {
            return TYPE_HEXBINARY_LIST;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
         fromXmlString(doc->URL)),
        0);
}

bool getBooleanAttribute(
    xmlDocPtr doc, xmlNodePtr node, char const * ns, char const * name,
    bool deflt)
{
    XmlString attr(xmlGetNsProp(node, xmlString(name), xmlString(ns)));
    if (attr.str == 0) {
        return deflt;
    }
    if (xmlStrEqual(attr.str, xmlString("true"))) {
        return true;
    }
    if (xmlStrEqual(attr.str, xmlString("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("bad boolean attribute value in ")) +
         fromXmlString(doc->URL)),
        0);
}

css::uno::Any parseValue(xmlDocPtr doc, xmlNodePtr node, Type type) {
    XmlString text(xmlNodeListGetString(doc, node->xmlChildrenNode, 1));
    switch (type) {
    case TYPE_BOOLEAN:
        {
            XmlString col(xmlSchemaCollapseString(text.str));
            xmlChar * p = col.str == 0 ? text.str : col.str;
            if (xmlStrEqual(p, xmlString("true")) ||
                xmlStrEqual(p, xmlString("1")))
            {
                return css::uno::makeAny(true);
            } else if (xmlStrEqual(p, xmlString("false")) ||
                       xmlStrEqual(p, xmlString("0")))
            {
                return css::uno::makeAny(false);
            }
        }
        break;
    case TYPE_SHORT:
        {
            XmlString col(xmlSchemaCollapseString(text.str));
            sal_Int64 n = rtl_str_toInt64(
                reinterpret_cast< char * >(col.str == 0 ? text.str : col.str),
                10); //TODO: check valid lexical representation
            if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
                return css::uno::makeAny(static_cast< sal_Int16 >(n));
            }
        }
        break;
    case TYPE_INT:
        {
            XmlString col(xmlSchemaCollapseString(text.str));
            return css::uno::makeAny(
                rtl_str_toInt32(
                    reinterpret_cast< char * >(
                        col.str == 0 ? text.str : col.str),
                    10)); //TODO: check valid lexical representation
        }
    case TYPE_LONG:
        {
            XmlString col(xmlSchemaCollapseString(text.str));
            return css::uno::makeAny(
                rtl_str_toInt64(
                    reinterpret_cast< char * >(
                        col.str == 0 ? text.str : col.str),
                    10)); //TODO: check valid lexical representation
        }
    case TYPE_DOUBLE:
        {
            XmlString col(xmlSchemaCollapseString(text.str));
            return css::uno::makeAny(
                rtl_str_toDouble(
                    reinterpret_cast< char * >(
                        col.str == 0 ? text.str : col.str)));
                //TODO: check valid lexical representation
        }
    case TYPE_STRING:
        return css::uno::makeAny(fromXmlString(text.str));
    case TYPE_HEXBINARY:
        //TODO
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid value of type any in ")) +
             fromXmlString(doc->URL)),
            0);
    case TYPE_BOOLEAN_LIST:
        //TODO
    case TYPE_SHORT_LIST:
        //TODO
    case TYPE_INT_LIST:
        //TODO
    case TYPE_LONG_LIST:
        //TODO
    case TYPE_DOUBLE_LIST:
        //TODO
    case TYPE_STRING_LIST:
        {
            XmlString sep(
                xmlGetNsProp(
                    node, xmlString("separator"),
                    xmlString("http://openoffice.org/2001/registry")));
            comphelper::SequenceAsVector< rtl::OUString > seq;
            if (sep.str == 0) {
                XmlString col(xmlSchemaCollapseString(text.str));
                xmlChar const * p = col.str == 0 ? text.str : col.str;
                if (*p != '\0') {
                    for (;;) {
                        xmlChar const * q = xmlStrchr(p, ' ');
                        if (q == 0) {
                            seq.push_back(fromXmlString(p));
                            break;
                        }
                        seq.push_back(fromXmlString(p, q));
                        p = q + 1;
                    }
                }
            } else if (*text.str != '\0') {
                //XXX There is no way to encode a string-list that contains the
                // empty string as its only element.
                //TODO: use xmlStrstr
                rtl::OUString text2(fromXmlString(text.str));
                rtl::OUString sep2(fromXmlString(sep.str));
                for (sal_Int32 i = 0;;) {
                    sal_Int32 j = findFirst(text2, sep2, i);
                    seq.push_back(text2.copy(i, j - i));
                    if (j == text2.getLength()) {
                        break;
                    } else {
                        i = j + sep2.getLength();
                    }
                }
            }
            return css::uno::makeAny(seq.getAsConstList());
        }
    case TYPE_HEXBINARY_LIST:
        //TODO
    default:
        OSL_ASSERT(false);
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value in ")) +
         fromXmlString(doc->URL)),
        0);
}

rtl::OUString parseTemplateReference(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::OUString const * defaultTemplateName)
{
    XmlString nodeType(
        xmlGetNsProp(
            node, xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    if (nodeType.str == 0) {
        if (defaultTemplateName != 0) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: missing node-type attribute in ")) +
             fromXmlString(doc->URL)),
            0);
    }
    XmlString component(
        xmlGetNsProp(
            node, xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    return fullTemplateName(
        (component.str == 0 ? componentName : fromXmlString(component.str)),
        fromXmlString(nodeType.str));
}

void parseXcsGroupContent(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::Reference< GroupNode > const & group);

rtl::Reference< Node > parseXcsGroup(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    Node * parent, rtl::OUString const & name,
    rtl::OUString const & templateName)
{
    rtl::Reference< GroupNode > group(
        new GroupNode(
            parent, name,
            getBooleanAttribute(
                doc, node, "http://openoffice.org/2001/registry", "extensible",
                false),
            templateName));
    parseXcsGroupContent(componentName, doc, node, group);
    return group.get();
}

Node * parseXcsSet(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    Node * parent, rtl::OUString const & name,
    rtl::OUString const & templateName)
{
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    std::vector< rtl::OUString > additional;
    while (isOorElement(p, "item")) {
        additional.push_back(parseTemplateReference(componentName, doc, p, 0));
        p = skipBlank(p->next);
    }
    if (p != 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad set content in ")) +
             fromXmlString(doc->URL)),
            0);
    }
    return new SetNode(
        parent, name, parseTemplateReference(componentName, doc, node, 0),
        additional, templateName);
}

void parseXcsGroupContent(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::Reference< GroupNode > const & group)
{
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    for (; p != 0; p = skipBlank(p->next)) {
        rtl::Reference< Node > member;
        if (isOorElement(p, "prop")) {
            Type type(getTypeAttribute(doc, p));
            if (type == TYPE_NONE) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: missing type attribute in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            xmlNodePtr q(skipBlank(p->xmlChildrenNode));
            if (isOorElement(q, "info")) {
                q = skipBlank(q->next);
            }
            if (isOorElement(q, "constraints")) { //TODO
                q = skipBlank(q->next);
            }
            LocalizedValues values;
            if (isOorElement(q, "value")) {
                values.insert(
                    LocalizedValues::value_type(
                        rtl::OUString(), parseValue(doc, q, type)));
                q = skipBlank(q->next);
            }
            if (q != 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: bad prop content in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            member =
                getBooleanAttribute(
                    doc, p, "http://openoffice.org/2001/registry", "localized",
                    false)
                ? static_cast< Node * >(
                    new LocalizedPropertyNode(
                        group.get(), getNameAttribute(doc, p), type,
                        getBooleanAttribute(
                            doc, p, "http://openoffice.org/2001/registry",
                            "nillable", true), values))
                : static_cast< Node * >(
                    new PropertyNode(
                        group.get(), getNameAttribute(doc, p), type,
                        getBooleanAttribute(
                            doc, p, "http://openoffice.org/2001/registry",
                            "nillable", true),
                        values[rtl::OUString()], false));
        } else if (isOorElement(p, "node-ref")) {
            xmlNodePtr q(skipBlank(p->xmlChildrenNode));
            if (isOorElement(q, "info")) {
                q = skipBlank(q->next);
            }
            if (q != 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: bad set content in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            member = new NodeRef(
                group.get(), getNameAttribute(doc, p),
                parseTemplateReference(componentName, doc, p, 0));
        } else if (isOorElement(p, "group")) {
            member = parseXcsGroup(
                componentName, doc, p, group.get(), getNameAttribute(doc, p),
                rtl::OUString());
        } else if (isOorElement(p, "set")) {
            member = parseXcsSet(
                componentName, doc, p, group.get(), getNameAttribute(doc, p),
                rtl::OUString());
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: bad component-schema/component content in ")) +
                 fromXmlString(doc->URL)),
                0);
        }
        if (!group->getMembers().insert(
                NodeMap::value_type(member->getName(), member)).
            second)
        {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: multiple members with same name in ")) +
                 fromXmlString(doc->URL)),
                0);
        }
    }
}

xmlNodePtr parseXcsTemplates(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    Components::TemplateMap * templates)
{
    if (!isOorElement(node, "templates")) {
        return node;
    }
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    for (; p != 0; p = skipBlank(p->next)) {
        rtl::OUString tmplName;
        rtl::Reference< Node > tmpl;
        if (isOorElement(p, "group")) {
            tmplName = fullTemplateName(
                componentName, getNameAttribute(doc, p));
            tmpl = parseXcsGroup(
                componentName, doc, p, 0, rtl::OUString(), tmplName);
        } else if (isOorElement(p, "set")) {
            tmplName = fullTemplateName(
                componentName, getNameAttribute(doc, p));
            tmpl = parseXcsSet(
                componentName, doc, p, 0, rtl::OUString(), tmplName);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: bad component-schema/templates content in ")) +
                 fromXmlString(doc->URL)),
                0);
        }
        if (!templates->insert(NodeMap::value_type(tmplName, tmpl)).second) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: multiple templates with same name in ")) +
                 fromXmlString(doc->URL)),
                0);
        }
    }
    return node->next;
}

xmlNodePtr parseXcsComponent(
    xmlDocPtr doc, rtl::OUString const & component, xmlNodePtr node,
    NodeMap * components)
{
    if (!isOorElement(node, "component")) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad component-schema content in ")) +
             fromXmlString(doc->URL)),
            0);
    }
    rtl::Reference< GroupNode > comp(
        new GroupNode(0, component, false, rtl::OUString()));
    parseXcsGroupContent(component, doc, node, comp);
    if (!components->insert(NodeMap::value_type(component, comp.get())).second)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: multiply defined component-schema in ")) +
             fromXmlString(doc->URL)),
            0);
    }
    return node->next;
}

void parseXcsFile(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    XmlDoc doc(parseXmlFile(url));
    xmlNodePtr root(xmlDocGetRootElement(doc.doc));
    if (root == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcs: no root element in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    if (!xmlStrEqual(root->name, xmlString("component-schema")) ||
        root->ns == 0 ||
        !xmlStrEqual(
            root->ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: non component-schema root element in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    XmlString package(
        xmlGetNsProp(
            root, xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (package.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: no root package attribute in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    rtl::OUStringBuffer buf(fromXmlString(package.str));
    buf.append(sal_Unicode('.'));
    buf.append(getNameAttribute(doc.doc, root));
    rtl::OUString comp(buf.makeStringAndClear());
    //TODO: root xml:lang attribute
    xmlNodePtr p(skipBlank(root->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    while (isOorElement(p, "import")) { //TODO: process?
        p = skipBlank(p->next);
    }
    while (isOorElement(p, "uses")) { //TODO: process?
        p = skipBlank(p->next);
    }
    p = skipBlank(parseXcsTemplates(comp, doc.doc, p, templates));
    p = skipBlank(parseXcsComponent(doc.doc, comp, p, components));
    if (p != 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad component-schema content in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
}

void parseXcuNode(
    rtl::OUString const & componentName,
    Components::TemplateMap const & templates, xmlDocPtr doc,
    xmlNodePtr xmlNode, rtl::Reference< Node > const & node)
{
    if (GroupNode * group = dynamic_cast< GroupNode * >(node.get())) {
        for (xmlNodePtr p(skipBlank(xmlNode->xmlChildrenNode)); p != 0;
             p = skipBlank(p->next))
        {
            if (isOorElement(p, "prop")) {
                rtl::OUString name(getNameAttribute(doc, p));
                PropertyNode * property = 0;
                LocalizedPropertyNode * localized = 0;
                NodeMap::iterator i(group->getMembers().find(name));
                if (i != group->getMembers().end()) {
                    property = dynamic_cast< PropertyNode * >(i->second.get());
                    localized = dynamic_cast< LocalizedPropertyNode * >(
                        i->second.get());
                    if (property == 0 && localized == 0) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: inappropriate prop in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                }
                Type type(getTypeAttribute(doc, p));
                if (type == TYPE_ANY) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: invalid type attribute in ")) +
                         fromXmlString(doc->URL)),
                        0);
                }
                //TODO: oor:finalized attributes
                xmlNodePtr q(skipBlank(p->xmlChildrenNode));
                LocalizedValues values;
                while (isOorElement(q, "value")) {
                    if (type == TYPE_NONE) {
                        if (property != 0) {
                            type = property->getType();
                        } else if (localized != 0) {
                            type = localized->getType();
                        } else {
                            throw css::uno::RuntimeException(
                                (rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "xcu: missing type attribute in ")) +
                                 fromXmlString(doc->URL)),
                                0);
                        }
                        if (type == TYPE_ANY) {
                            throw css::uno::RuntimeException(
                                (rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "xcu: missing type attribute in ")) +
                                 fromXmlString(doc->URL)),
                                0);
                        }
                    }
                    bool nil = getBooleanAttribute(
                        doc, q, "http://www.w3.org/2001/XMLSchema-instance",
                        "nil", false);
                    if (nil &&
                        ((property != 0 && !property->isNillable()) ||
                         (localized != 0 && !localized->isNillable())))
                    {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: xsi:nil attribute for non-nillable"
                                    " prop in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                    XmlString lang(xmlNodeGetLang(q));
                    if (lang.str != 0 && localized == 0) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: xml:lang attribute for non-localized"
                                    " prop in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                    // For nil values, any actually provided value is simply
                    // ignored for now:
                    if (!values.insert(
                            LocalizedValues::value_type(
                                lang.str == 0
                                    ? rtl::OUString() : fromXmlString(lang.str),
                                nil ? com::sun::star::uno::Any()
                                    : parseValue(doc, q, type))).
                        second)
                    {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: multiple values (for same xml:lang)"
                                    " in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                    q = skipBlank(q->next);
                }
                if (q != 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: bad prop content in ")) +
                         fromXmlString(doc->URL)),
                        0);
                }
                if (i == group->getMembers().end()) {
                    if (!group->isExtensible()) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: unknown prop name in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                    switch (getOperationAttribute(doc, p)) {
                    case OPERATION_MODIFY:
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: invalid modify of extension property"
                                    " in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    case OPERATION_REPLACE:
                    case OPERATION_FUSE:
                        group->getMembers().insert(
                            NodeMap::value_type(
                                name,
                                new PropertyNode(
                                    group, name, type, true,
                                    values[rtl::OUString()], true)));
                        break;
                    case OPERATION_REMOVE:
                        // ignore unknown (presumably extension) properties
                        break;
                    }
                } else {
                    switch (getOperationAttribute(doc, p)) {
                    case OPERATION_MODIFY:
                    case OPERATION_FUSE:
                        if (property != 0) {
                            property->setValue(values[rtl::OUString()]);
                        } else {
                            for (LocalizedValues::iterator j(values.begin());
                                 j != values.end(); ++j)
                            {
                                localized->getValues()[j->first] = j->second;
                            }
                        }
                        break;
                    case OPERATION_REPLACE:
                        if (property != 0) {
                            property->setValue(values[rtl::OUString()]);
                        } else {
                            localized->setValues(values);
                        }
                        break;
                    case OPERATION_REMOVE:
                        {
                            if (!property->isExtension()) {
                                throw css::uno::RuntimeException(
                                    (rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "xcu: invalid remove of"
                                            " non-extension property in ")) +
                                     fromXmlString(doc->URL)),
                                    0);
                            }
                            rtl::Reference< Node > removed(i->second);
                            group->getMembers().erase(i);
                            removed->unbind(); // must not throw
                        }
                        break;
                    }
                }
            } else if (isOorElement(p, "node")) {
                Operation op = getOperationAttribute(doc, p);
                if (op != OPERATION_MODIFY && op != OPERATION_FUSE) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: invalid operation on group node in")) +
                         fromXmlString(doc->URL)),
                        0);
                }
                //TODO: oor:component, oor:finalized, oor:mandatory, oor:node-type attributes
                NodeMap::iterator i(
                    group->getMembers().find(getNameAttribute(doc, p)));
                if (i == group->getMembers().end()) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: unknown node name in ")) +
                         fromXmlString(doc->URL)),
                        0);
                }
                resolveNodeRef(templates, i);
                parseXcuNode(componentName, templates, doc, p, i->second);
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: bad component-data or node content in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
        }
    } else if (SetNode * set = dynamic_cast< SetNode * >(node.get())) {
        for (xmlNodePtr p(skipBlank(xmlNode->xmlChildrenNode)); p != 0;
             p = skipBlank(p->next))
        {
            if (!isOorElement(p, "node")) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: non-node element within set node element"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            rtl::OUString name(getNameAttribute(doc, p));
            rtl::OUString templateName(
                parseTemplateReference(
                    componentName, doc, p, &set->getDefaultTemplateName()));
            if (!set->isValidTemplate(templateName)) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: set member node references invalid template"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            Components::TemplateMap::const_iterator i(
                templates.find(templateName));
            if (i == templates.end()) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: set member node references undefined template"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    0);
            }
            switch (getOperationAttribute(doc, p)) {
            case OPERATION_MODIFY:
                {
                    NodeMap::iterator j(set->getMembers().find(name));
                    if (j == set->getMembers().end()) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: invalid modify of unknown set member"
                                    " node in ")) +
                             fromXmlString(doc->URL)),
                            0);
                    }
                    parseXcuNode(componentName, templates, doc, p, j->second);
                }
                break;
            case OPERATION_REPLACE:
                {
                    rtl::Reference< Node > member(
                        i->second->clone(group, name));
                    parseXcuNode(componentName, templates, doc, p, member);
                    NodeMap::iterator j(set->getMembers().find(name));
                    if (j == set->getMembers().end()) {
                        set->getMembers().insert(
                            NodeMap::value_type(name, member));
                    } else {
                        rtl::Reference< Node > removed(j->second);
                        j->second = member;
                        removed->unbind(); // must not throw
                    };
                }
                break;
            case OPERATION_FUSE:
                {
                    NodeMap::iterator j(set->getMembers().find(name));
                    if (j == set->getMembers().end()) {
                        rtl::Reference< Node > member(
                            i->second->clone(group, name));
                        parseXcuNode(componentName, templates, doc, p, member);
                        set->getMembers().insert(
                            NodeMap::value_type(
                                member->getName(), member));
                    } else {
                        parseXcuNode(
                            componentName, templates, doc, p, j->second);
                    }
                }
                break;
            case OPERATION_REMOVE:
                {
                    NodeMap::iterator j(set->getMembers().find(name));
                    // Ignore unknown members:
                    if (j != set->getMembers().end()) {
                        rtl::Reference< Node > removed(j->second);
                        set->getMembers().erase(j);
                        removed->unbind(); // must not throw
                    }
                }
                break;
            }
        }
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcu: inappropriate node in ")) +
             fromXmlString(doc->URL)),
            0);
    }
}

void parseXcuFile(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    XmlDoc doc(parseXmlFile(url));
    xmlNodePtr root(xmlDocGetRootElement(doc.doc));
    if (root == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcu: no root element in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    if (!xmlStrEqual(root->name, xmlString("component-data")) ||
        root->ns == 0 ||
        !xmlStrEqual(
            root->ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: non component-data root element in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    XmlString package(
        xmlGetNsProp(
            root, xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (package.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: no root package attribute in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    rtl::OUStringBuffer buf(fromXmlString(package.str));
    buf.append(sal_Unicode('.'));
    buf.append(getNameAttribute(doc.doc, root));
    rtl::OUString comp(buf.makeStringAndClear());
    //TODO: root oor:finalized, oor:op attributes
    NodeMap::iterator i(components->find(comp));
    if (i == components->end()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: unknown component-data name in ")) +
             fromXmlString(doc.doc->URL)),
            0);
    }
    resolveNodeRef(*templates, i);
    parseXcuNode(comp, *templates, doc.doc, root, i->second);
}

void parseFiles(
    rtl::OUString const & extension,
    void (* parseFile)(
        rtl::OUString const &, Components::TemplateMap *, NodeMap *),
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components, bool recursive)
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
            0);
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
                0);
        }
        osl::FileStatus stat(
            FileStatusMask_Type | FileStatusMask_FileName |
            FileStatusMask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot stat in directory ")) +
                 url),
                0);
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            parseFiles(
                extension, parseFile, stat.getFileURL(), templates, components,
                true);
        } else {
            rtl::OUString file(stat.getFileName());
            if (file.getLength() >= extension.getLength() &&
                file.match(extension, file.getLength() - extension.getLength()))
            {
                (*parseFile)(stat.getFileURL(), templates, components);
            }
        }
    }
}

void parseXcsXcuLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcs")), &parseXcsFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/schema")),
        templates, components, false);
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/data")), templates,
        components, false);
}

void parseModuleLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile, url,
        templates, components, false);
}

void parseResLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile, url,
        templates, components, false);
}

void parseSystemLayer() {
    //TODO
}

}

Components & Components::singleton() {
    static Components * c = new Components(); // leaks
    return *c;
}

namespace {

rtl::OUString parseSegment(
    rtl::OUString const & segment, bool * setElement,
    rtl::OUString * templateName)
{
    OSL_ASSERT(setElement != 0);
    sal_Int32 i = segment.indexOf('[');
    if (i == -1) {
        *setElement = false;
        return segment;
    }
    *setElement = true;
    if (templateName != 0) {
        // For backwards compatibility, treat an empty template name the same
        // as "*".
        *templateName = segment.copy(0, i);
        if (templateName->equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"))) {
            *templateName = rtl::OUString();
        }
    }
    if (segment[segment.getLength() - 1] != ']' ||
        (segment[i + 1] != '\'' && segment[i + 1] != '"') ||
        segment.getLength() - 2 <= i + 1 ||
        segment[segment.getLength() - 2] != segment[i + 1])
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("bad path segment ")) + segment,
            0);
    }
    rtl::OUStringBuffer buf;
    i += 2;
    while (i < segment.getLength() - 2) {
        sal_Unicode c = segment[i];
        if (c == '&') {
            if (segment.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("amp;"), i + 1))
            {
                buf.append(sal_Unicode('&'));
                i += RTL_CONSTASCII_LENGTH("&amp;");
            } else if (segment.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("quot;"), i + 1))
            {
                buf.append(sal_Unicode('"'));
                i += RTL_CONSTASCII_LENGTH("&quot;");
            } else if (segment.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("apos;"), i + 1))
            {
                buf.append(sal_Unicode('\''));
                i += RTL_CONSTASCII_LENGTH("&apos;");
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("bad path segment ")) +
                     segment),
                    0);
            }
        } else {
            buf.append(c);
            ++i;
        }
    }
    return buf.makeStringAndClear();
}

}

rtl::Reference< Node > Components::resolvePath(
    rtl::Reference< Node > const & base, rtl::OUString const & path)
{
    if (path.getLength() == 0) {
        if (!base.is()) {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                0);
        }
        return base;
    }
    rtl::Reference< Node > p;
    sal_Int32 n = -1;
    if (path[0] == '/') {
        n = findFirst(path, '/', 1);
        bool setElement;
        rtl::OUString seg(parseSegment(path.copy(1, n - 1), &setElement, 0));
        if (setElement) {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                0);
        }
        NodeMap::iterator i(components_.find(path.copy(1, n - 1)));
        p = i == components_.end() ? 0 : i->second;
    } else {
        if (base == 0) {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                0);
        }
        p = base;
    }
    while (p != 0 && n != path.getLength()) {
        sal_Int32 n1 = findFirst(path, '/', n + 1);
        rtl::OUString segment(path.copy(n + 1, n1 - (n + 1)));
        // For backwards compatibility, ignore a final slash:
        if (segment.getLength() == 0 && n1 == path.getLength()) {
            break;
        }
        bool setElement;
        rtl::OUString templateName;
        rtl::OUString seg(parseSegment(segment, &setElement, &templateName));
        if (setElement) {
            SetNode * set = dynamic_cast< SetNode * >(p.get());
            if (set == 0 ||
                (templateName.getLength() != 0 &&
                 !set->isValidTemplate(templateName)))
            {
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                     path),
                    0);
            }
            p = set->getMember(seg);
            if (templateName.getLength() != 0 && p != 0) {
                //TODO: check match
            }
        } else {
            // For backwards compatibility, allow set members to be accessed
            // with simple path segments, like group members:
            p = p->getMember(seg);
        }
        n = n1;
    }
    return p;
}

rtl::Reference< Node > Components::getTemplate(rtl::OUString const & fullName)
    const
{
    NodeMap::const_iterator i(templates_.find(fullName));
    return i == templates_.end() ? 0 : i->second;
}

namespace {

rtl::OUString expand(rtl::OUString const & str) {
    rtl::OUString s(str);
    rtl::Bootstrap::expandMacros(s); //TODO: detect failure
    return s;
}

}

Components::Components() {
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseModuleLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$OOO_BASE_DIR/share/registry/modules"))),
        &templates_, &components_);
    parseResLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseModuleLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$BRAND_BASE_DIR/share/registry/modules"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/unorc:UNO_SHARED_PACKAGES_CACHE}/"
                    "registry/com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))),
        &templates_, &components_);
    parseSystemLayer();
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/unorc:UNO_USER_PACKAGES_CACHE}/"
                    "registry/com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}/"
                    "user/registry"))),
        &templates_, &components_);
}

Components::~Components() {}

}
