/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_QA_INC_BORDERTEST_HXX
#define INCLUDED_SW_QA_INC_BORDERTEST_HXX

#include <cppunit/TestAssert.h>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <map>

namespace com::sun::star::table { class XCell; }

typedef std::map<OUString, css::table::BorderLine> BorderLineMap;
typedef std::pair<OUString, css::table::BorderLine> StringBorderPair;

using namespace com::sun::star;

class BorderTest
{
public:
    BorderTest()
    {

    }
    static void testTheBorders(uno::Reference<lang::XComponent> const & mxComponent, bool isBinaryDoc)
    {
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // maps containing TopBorder widths for every cell
    // one map for each tables - there are 8 of them, counting from 0
    BorderLineMap map0;
    map0.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 53, 26, 26)));
    map0.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 79, 26, 26)));
    map0.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 106, 26, 26)));
    map0.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 159, 26, 26)));
    map0.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 212, 26, 26)));
    BorderLineMap map1;
    map1.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 9, 9, 9)));
    map1.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 18, 18, 18)));
    map1.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 26, 26, 26)));
    map1.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 53, 53, 53)));
    map1.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 79, 79, 79)));
    map1.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 106, 106, 106)));
    BorderLineMap map2;
    map2.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 9, 5, 5)));
    map2.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 18, 9, 9)));
    map2.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 26, 14, 14)));
    map2.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 35, 18, 18)));
    map2.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 53, 26, 26)));
    map2.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 79, 41, 41)));
    map2.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 106, 53, 53)));
    map2.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 159, 79, 79)));
    map2.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 212, 106, 106)));
    BorderLineMap map3;
    map3.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 53, 26, 9)));
    map3.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 53, 26, 18)));
    map3.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 53, 26, 26)));
    map3.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 53, 26, 35)));
    map3.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 53, 26, 53)));
    map3.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 53, 26, 79)));
    map3.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 53, 26, 106)));
    map3.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 53, 26, 159)));
    map3.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 53, 26, 212)));
    map3.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 53, 26, 9)));

    // Binary-DOC importer changes 'inset' and 'outset' border styles to other styles
    // during import, so for now - leaving binary-doc results as they were.
    BorderLineMap map4;
    if (isBinaryDoc)
    {
        map4.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 53, 26, 18)));
        map4.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 53, 26, 26)));
        map4.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 53, 26, 35)));
        map4.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 53, 26, 35)));
        map4.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 53, 26, 79)));
        map4.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 53, 26, 106)));
        map4.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 53, 26, 159)));
        map4.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 53, 26, 212)));
    }
    else
    {
        map4.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 0, 26, 4)));
        map4.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 5, 26, 5)));
        map4.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 14, 26, 14)));
        map4.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 23, 26, 23)));
        map4.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 41, 26, 41)));
        map4.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 67, 26, 67)));
        map4.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 93, 26, 93)));
        map4.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 146, 26, 146)));
        map4.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 199, 26, 199)));
    }
    BorderLineMap map5;
    if (isBinaryDoc)
    {
        map5.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 26, 53, 9)));
        map5.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 26, 53, 18)));
        map5.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 26, 53, 26)));
        map5.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 26, 53, 35)));
        map5.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 26, 53, 53)));
        map5.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 26, 53, 79)));
        map5.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 26, 53, 106)));
        map5.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 26, 53, 159)));
        map5.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 26, 53, 212)));
    }
    else
    {
        map5.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 26, 2, 4)));
        map5.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 26, 5, 5)));
        map5.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 26, 14, 14)));
        map5.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 26, 23, 23)));
        map5.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 26, 41, 41)));
        map5.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 26, 67, 67)));
        map5.insert(StringBorderPair(u"G1"_ustr, table::BorderLine(0, 26, 93, 93)));
        map5.insert(StringBorderPair(u"H1"_ustr, table::BorderLine(0, 26, 146, 146)));
        map5.insert(StringBorderPair(u"I1"_ustr, table::BorderLine(0, 26, 199, 199)));
    }
    BorderLineMap map6;
    map6.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 14, 14, 26)));
    map6.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 26, 26, 53)));
    map6.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 41, 41, 79)));
    map6.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 53, 53, 106)));
    map6.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 79, 79, 159)));
    map6.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 106, 106, 212)));
    BorderLineMap map7;
    map7.insert(StringBorderPair(u"A1"_ustr, table::BorderLine(0, 14, 14, 26)));
    map7.insert(StringBorderPair(u"B1"_ustr, table::BorderLine(0, 26, 26, 53)));
    map7.insert(StringBorderPair(u"C1"_ustr, table::BorderLine(0, 41, 41, 79)));
    map7.insert(StringBorderPair(u"D1"_ustr, table::BorderLine(0, 53, 53, 106)));
    map7.insert(StringBorderPair(u"E1"_ustr, table::BorderLine(0, 79, 79, 159)));
    map7.insert(StringBorderPair(u"F1"_ustr, table::BorderLine(0, 106, 106, 212)));

    sal_Int32 currentTable = 0; //to know which map should we check with the current table
    BorderLineMap* tempMap;
    tempMap = &map0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            if (xServiceInfo->supportsService(u"com.sun.star.text.TextTable"_ustr))
            {
                uno::Reference<text::XTextTable> const xTextTable(xServiceInfo, uno::UNO_QUERY_THROW);
                uno::Sequence<OUString> const cells = xTextTable->getCellNames();

                if(currentTable == sal_Int32(1))
                    tempMap = &map1;
                if(currentTable == sal_Int32(2))
                    tempMap = &map2;
                if(currentTable == sal_Int32(3))
                    tempMap = &map3;
                if(currentTable == sal_Int32(4))
                    tempMap = &map4;
                if(currentTable == sal_Int32(5))
                    tempMap = &map5;
                if(currentTable == sal_Int32(6))
                    tempMap = &map6;
                if(currentTable == sal_Int32(7))
                    tempMap = &map7;

                BorderLineMap::iterator it = tempMap->begin();

                for (const auto& rCell : cells)
                {
                    uno::Reference<table::XCell> xCell = xTextTable->getCellByName(rCell);
                    uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
                    uno::Any aAny = xPropSet->getPropertyValue(u"TopBorder"_ustr);
                    table::BorderLine aBorderLine;
                    it = tempMap->find(rCell);
                    if ((aAny >>= aBorderLine) && (it!=tempMap->end()))
                    {
                        sal_Int32 innerLineWidth = aBorderLine.InnerLineWidth;
                        sal_Int32 outerLineWidth = aBorderLine.OuterLineWidth;
                        sal_Int32 lineDistance = aBorderLine.LineDistance;

                        sal_Int32 perfectInner = it->second.InnerLineWidth;
                        sal_Int32 perfectOuter = it->second.OuterLineWidth;
                        sal_Int32 perfectDistance = it->second.LineDistance;
                        CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                        CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                        CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                    }
                }
                ++currentTable;
            }
        }
    } while(xParaEnum->hasMoreElements());
    }

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
