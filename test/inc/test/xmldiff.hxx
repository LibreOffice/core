/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <string>
#include <set>
#include <test/testdllapi.hxx>

#define USE_CPPUNIT 1

struct tolerance
{
    ~tolerance()
    {
        xmlFree(elementName);
        xmlFree(attribName);
    }

    tolerance()
    {
        elementName = NULL;
        attribName = NULL;
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
    bool operator==(const tolerance& rTol) const { return xmlStrEqual(elementName, rTol.elementName) && xmlStrEqual(attribName, rTol.attribName); }
    bool operator<(const tolerance& rTol) const
    {
        int cmp = xmlStrcmp(elementName, rTol.elementName);
        if(cmp == 0)
        {
            cmp = xmlStrcmp(attribName, rTol.attribName);
        }

        if(cmp>=0)
            return false;
        else
            return true;
    }
};

class OOO_DLLPUBLIC_TEST XMLDiff
{
public:
    XMLDiff(const char* pFileName, const char* pContent, int size, const char* pToleranceFileName);
    ~XMLDiff();

    bool compare();
private:
    typedef std::set<tolerance> ToleranceContainer;

    void loadToleranceFile(xmlDocPtr xmlTolerance);
    bool compareAttributes(xmlNodePtr node1, xmlNodePtr node2);
    bool compareElements(xmlNodePtr node1, xmlNodePtr node2);

    ToleranceContainer toleranceContainer;
    xmlDocPtr xmlFile1;
    xmlDocPtr xmlFile2;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
