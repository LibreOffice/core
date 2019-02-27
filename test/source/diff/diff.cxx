/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define USE_CPPUNIT 1

#include <test/xmldiff.hxx>

#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

#include <set>
#include <cstring>
#include <sstream>
#include <cmath>
#include <cassert>

#if USE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include <rtl/math.hxx>


struct tolerance
{
    ~tolerance()
    {
        xmlFree(elementName);
        xmlFree(attribName);
    }

    tolerance()
        : elementName(nullptr)
        , attribName(nullptr)
        , relative(false)
        , value(0.0)
    {
    }

    tolerance(const tolerance& tol)
    {
        elementName = xmlStrdup(tol.elementName);
        attribName = xmlStrdup(tol.attribName);
        relative = tol.relative;
        value = tol.value;
    }

    xmlChar* elementName;
    xmlChar* attribName;
    bool relative;
    double value;
    bool operator<(const tolerance& rTol) const
    {
        int cmp = xmlStrcmp(elementName, rTol.elementName);
        if(cmp == 0)
        {
            cmp = xmlStrcmp(attribName, rTol.attribName);
        }

        return cmp < 0;
    }
};

class XMLDiff
{
public:
    XMLDiff(const char* pFileName, const char* pContent, int size, const char* pToleranceFileName);
    ~XMLDiff();

    bool compare();
private:
    typedef std::set<tolerance> ToleranceContainer;

    void loadToleranceFile(xmlDocPtr xmlTolerance);
    bool compareAttributes(xmlNodePtr node1, xmlNodePtr node2);
    bool compareElements(xmlNode* node1, xmlNode* node2);

    /// Error message for cppunit that prints out when expected and found are not equal.
    void cppunitAssertEqual(const xmlChar *expected, const xmlChar *found);

    /// Error message for cppunit that prints out when expected and found are not equal - for doubles.
    void cppunitAssertEqualDouble(const xmlNodePtr node, const xmlAttrPtr attr, double expected, double found, double delta);

    ToleranceContainer toleranceContainer;
    xmlDocPtr const xmlFile1;
    xmlDocPtr const xmlFile2;
    std::string fileName;
};


XMLDiff::XMLDiff( const char* pFileName, const char* pContent, int size, const char* pToleranceFile)
    : xmlFile1(xmlParseFile(pFileName))
    , xmlFile2(xmlParseMemory(pContent, size))
    , fileName(pFileName)
{
    if(pToleranceFile)
    {
        xmlDocPtr xmlToleranceFile = xmlParseFile(pToleranceFile);
        loadToleranceFile(xmlToleranceFile);
        xmlFreeDoc(xmlToleranceFile);
    }
}

XMLDiff::~XMLDiff()
{
    xmlFreeDoc(xmlFile1);
    xmlFreeDoc(xmlFile2);
}

namespace {

void readAttributesForTolerance(xmlNodePtr node, tolerance& tol)
{
    xmlChar* elementName = xmlGetProp(node, BAD_CAST("elementName"));
    tol.elementName = elementName;

    xmlChar* attribName = xmlGetProp(node, BAD_CAST("attribName"));
    tol.attribName = attribName;

    xmlChar* value = xmlGetProp(node, BAD_CAST("value"));
    double val = xmlXPathCastStringToNumber(value);
    xmlFree(value);
    tol.value = val;

    xmlChar* relative = xmlGetProp(node, BAD_CAST("relative"));
    bool rel = false;
    if(xmlStrEqual(relative, BAD_CAST("true")))
        rel = true;
    xmlFree(relative);
    tol.relative = rel;
}

}

void XMLDiff::loadToleranceFile(xmlDocPtr xmlToleranceFile)
{
    xmlNodePtr root = xmlDocGetRootElement(xmlToleranceFile);
#if USE_CPPUNIT
    CPPUNIT_ASSERT_MESSAGE("did not find correct tolerance file", xmlStrEqual( root->name, BAD_CAST("tolerances") ));
#else
    if(!xmlStrEqual( root->name, BAD_CAST("tolerances") ))
    {
        assert(false);
        return;
    }
#endif
    xmlNodePtr child = nullptr;
    for (child = root->children; child != nullptr; child = child->next)
    {
        // assume a valid xml file
        if(child->type != XML_ELEMENT_NODE)
            continue;

        assert(xmlStrEqual(child->name, BAD_CAST("tolerance")));

        tolerance tol;
        readAttributesForTolerance(child, tol);
        toleranceContainer.insert(tol);
    }
}

bool XMLDiff::compare()
{
    xmlNode* root1 = xmlDocGetRootElement(xmlFile1);
    xmlNode* root2 = xmlDocGetRootElement(xmlFile2);

#if USE_CPPUNIT
    CPPUNIT_ASSERT(root1);
    CPPUNIT_ASSERT(root2);
    cppunitAssertEqual(root1->name, root2->name);
#else
    if (!root1 || !root2)
        return false;
    if(!xmlStrEqual(root1->name, root2->name))
        return false;
#endif
    return compareElements(root1, root2);
}

namespace {

bool checkForEmptyChildren(xmlNodePtr node)
{
    if(!node)
        return true;

    for(; node != nullptr; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE)
            return false;
    }
    return true;
}

}

bool XMLDiff::compareElements(xmlNode* node1, xmlNode* node2)
{
#if USE_CPPUNIT
    cppunitAssertEqual(node1->name, node2->name);
#else
    if (!xmlStrEqual( node1->name, node2->name ))
        return false;
#endif

    //compare attributes
    bool sameAttribs = compareAttributes(node1, node2);
#if USE_CPPUNIT
    CPPUNIT_ASSERT(sameAttribs);
#else
    if (!sameAttribs)
        return false;
#endif

    // compare children
    xmlNode* child2 = nullptr;
    xmlNode* child1 = nullptr;
    for(child1 = node1->children, child2 = node2->children; child1 != nullptr && child2 != nullptr; child1 = child1->next, child2 = child2->next)
    {
        if (child1->type == XML_ELEMENT_NODE)
        {
            bool bCompare = compareElements(child1, child2);
            if(!bCompare)
            {
                return false;
            }
        }
    }

#if USE_CPPUNIT
    CPPUNIT_ASSERT(checkForEmptyChildren(child1));
    CPPUNIT_ASSERT(checkForEmptyChildren(child2));
#else
    if(!checkForEmptyChildren(child1) || !checkForEmptyChildren(child2))
        return false;
#endif

    return true;
}

void XMLDiff::cppunitAssertEqual(const xmlChar *expected, const xmlChar *found)
{
#if USE_CPPUNIT
    std::stringstream stringStream;
    stringStream << "Reference: " << fileName << "\n- Expected: " << reinterpret_cast<const char*>(expected) << "\n- Found: " << reinterpret_cast<const char*>(found);

    CPPUNIT_ASSERT_MESSAGE(stringStream.str(), xmlStrEqual(expected, found));
#endif
}

void XMLDiff::cppunitAssertEqualDouble(const xmlNodePtr node, const xmlAttrPtr attr, double expected, double found, double delta)
{
#if USE_CPPUNIT
    xmlChar * path = xmlGetNodePath(node);
    std::stringstream stringStream;
    stringStream << "Reference: " << fileName << "\n- Node: " << reinterpret_cast<const char*>(path) << "\n- Attr: " << reinterpret_cast<const char*>(attr->name);
    xmlFree(path);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(stringStream.str(), expected, found, delta);
#endif
}

namespace {

bool compareValuesWithTolerance(double val1, double val2, double tolerance, bool relative)
{
    if(relative)
    {
        return (val1/tolerance) <= val2 && val2 <= (val1*tolerance);
    }
    else
    {
        return (val1 - tolerance) <= val2 && val2 <= (val1 + tolerance);
    }
}

}

bool XMLDiff::compareAttributes(xmlNodePtr node1, xmlNodePtr node2)
{
    xmlAttrPtr attr1 = nullptr;
    xmlAttrPtr attr2 = nullptr;
    for(attr1 = node1->properties, attr2 = node2->properties; attr1 != nullptr && attr2 != nullptr; attr1 = attr1->next, attr2 = attr2->next)
    {
#if USE_CPPUNIT
        cppunitAssertEqual(attr1->name, attr2->name);
#else
        if (!xmlStrEqual( attr1->name, attr2->name ))
            return false;
#endif

        xmlChar* val1 = xmlGetProp(node1, attr1->name);
        xmlChar* val2 = xmlGetProp(node2, attr2->name);

        double dVal1 = xmlXPathCastStringToNumber(val1);
        double dVal2 = xmlXPathCastStringToNumber(val2);

        if(!rtl::math::isNan(dVal1) || !rtl::math::isNan(dVal2))
        {
            //compare by value and respect tolerance
            tolerance tol;
            tol.elementName = xmlStrdup(node1->name);
            tol.attribName = xmlStrdup(attr1->name);
            ToleranceContainer::iterator itr = toleranceContainer.find( tol );
            bool useTolerance = false;
            if (itr != toleranceContainer.end())
            {
                useTolerance = true;
            }

            if (useTolerance)
            {
                bool valInTolerance = compareValuesWithTolerance(dVal1, dVal2, itr->value, itr->relative);
#if USE_CPPUNIT
                std::stringstream stringStream("Expected Value: ");
                stringStream << dVal1 << "; Found Value: " << dVal2 << "; Tolerance: " << itr->value;
                stringStream << "; Relative: " << itr->relative;
                CPPUNIT_ASSERT_MESSAGE(stringStream.str(), valInTolerance);
#else
                if (!valInTolerance)
                    return false;
#endif
            }
            else
            {
#if USE_CPPUNIT
                cppunitAssertEqualDouble(node1, attr1, dVal1, dVal2, 1e-08);
#else
                if (dVal1 != dVal2)
                    return false;
#endif
            }
        }
        else
        {

#if USE_CPPUNIT
            cppunitAssertEqual(val1, val2);
#else
            if(!xmlStrEqual( val1, val2 ))
                return false;
#endif
        }

        xmlFree(val1);
        xmlFree(val2);
    }

    // unequal number of attributes
#ifdef CPPUNIT_ASSERT
    std::stringstream failStream("Unequal number of attributes ");

    bool bAttr1 = attr1;
    if (bAttr1)
    {
        failStream << "Attr1: ";
        while (attr1 != nullptr)
        {
            xmlChar* val1 = xmlGetProp(node1, attr1->name);
            failStream << BAD_CAST(attr1->name) << "=" << BAD_CAST(val1) << ", ";
            xmlFree(val1);
            attr1 = attr1->next;
        }
    }
    CPPUNIT_ASSERT_MESSAGE(failStream.str(), !bAttr1);

    bool bAttr2 = attr2;
    if (bAttr2)
    {
        failStream << "Attr2: ";

        while (attr2 != nullptr)
        {
            xmlChar* val2 = xmlGetProp(node2, attr2->name);
            failStream << BAD_CAST(attr2->name) << "=" << BAD_CAST(val2) << ", ";
            xmlFree(val2);
            attr2 = attr2->next;
        }
    }
    CPPUNIT_ASSERT_MESSAGE(failStream.str(), !bAttr2);
#else
    if (attr1 || attr2)
        return false;
#endif

    return true;
}


bool
doXMLDiff(char const*const pFileName, char const*const pContent, int const size,
          char const*const pToleranceFileName)
{
    XMLDiff aDiff(pFileName, pContent, size, pToleranceFileName);
    return aDiff.compare();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
