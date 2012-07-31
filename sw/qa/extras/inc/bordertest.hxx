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
 * Copyright (C) 2012 Artur Dorda <artur.dorda+libo@gmail.com> (initial developer)
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

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>

using rtl::OUString;

#ifndef bordertest_hxx
#define bordertest_hxx

typedef std::map<rtl::OUString, com::sun::star::table::BorderLine> BorderLineMap;
typedef std::pair<rtl::OUString, com::sun::star::table::BorderLine> StringBorderPair;

typedef std::map<rtl::OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > AllBordersMap;
typedef std::pair<rtl::OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > StringSequencePair;

using namespace com::sun::star;

class BorderTest
{
public:
    BorderTest()
    {

    }
    void testTheBorders(uno::Reference<lang::XComponent> mxComponent)
    {
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // maps containing TopBorder widths for every cell
    // one map for each tables - there are 8 of them, counting from 0
    BorderLineMap map0;
    map0.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 53, 26, 26)));
    map0.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 79, 26, 26)));
    map0.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 106, 26, 26)));
    map0.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 159, 26, 26)));
    map0.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 212, 26, 26)));
    BorderLineMap map1;
    map1.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 9, 9, 9)));
    map1.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 18, 18, 18)));
    map1.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 26, 26, 26)));
    map1.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 53, 53, 53)));
    map1.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 79, 79, 79)));
    map1.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 106, 106, 106)));
    BorderLineMap map2;
    map2.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 9, 5, 5)));
    map2.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 18, 9, 9)));
    map2.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 26, 14, 14)));
    map2.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 35, 18, 18)));
    map2.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 53, 26, 26)));
    map2.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 79, 41, 41)));
    map2.insert(StringBorderPair(OUString("G1"), table::BorderLine(0, 106, 53, 53)));
    map2.insert(StringBorderPair(OUString("H1"), table::BorderLine(0, 159, 79, 79)));
    map2.insert(StringBorderPair(OUString("I1"), table::BorderLine(0, 212, 106, 106)));
    BorderLineMap map3;
    map3.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 53, 26, 9)));
    map3.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 53, 26, 18)));
    map3.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 53, 26, 26)));
    map3.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 53, 26, 35)));
    map3.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 53, 26, 53)));
    map3.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 53, 26, 79)));
    map3.insert(StringBorderPair(OUString("G1"), table::BorderLine(0, 53, 26, 106)));
    map3.insert(StringBorderPair(OUString("H1"), table::BorderLine(0, 53, 26, 159)));
    map3.insert(StringBorderPair(OUString("I1"), table::BorderLine(0, 53, 26, 212)));
    map3.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 53, 26, 9)));
    BorderLineMap map4;
    map4.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 53, 26, 18)));
    map4.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 53, 26, 26)));
    map4.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 53, 26, 35)));
    map4.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 53, 26, 35)));
    map4.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 53, 26, 79)));
    map4.insert(StringBorderPair(OUString("G1"), table::BorderLine(0, 53, 26, 106)));
    map4.insert(StringBorderPair(OUString("H1"), table::BorderLine(0, 53, 26, 159)));
    map4.insert(StringBorderPair(OUString("I1"), table::BorderLine(0, 53, 26, 212)));
    BorderLineMap map5;
    map5.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 26, 53, 9)));
    map5.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 26, 53, 18)));
    map5.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 26, 53, 26)));
    map5.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 26, 53, 35)));
    map5.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 26, 53, 53)));
    map5.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 26, 53, 79)));
    map5.insert(StringBorderPair(OUString("G1"), table::BorderLine(0, 26, 53, 106)));
    map5.insert(StringBorderPair(OUString("H1"), table::BorderLine(0, 26, 53, 159)));
    map5.insert(StringBorderPair(OUString("I1"), table::BorderLine(0, 26, 53, 212)));
    BorderLineMap map6;
    map6.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 14, 14, 26)));
    map6.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 26, 26, 53)));
    map6.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 41, 41, 79)));
    map6.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 53, 53, 106)));
    map6.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 79, 79, 159)));
    map6.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 106, 106, 212)));
    BorderLineMap map7;
    map7.insert(StringBorderPair(OUString("A1"), table::BorderLine(0, 14, 14, 26)));
    map7.insert(StringBorderPair(OUString("B1"), table::BorderLine(0, 26, 26, 53)));
    map7.insert(StringBorderPair(OUString("C1"), table::BorderLine(0, 41, 41, 79)));
    map7.insert(StringBorderPair(OUString("D1"), table::BorderLine(0, 53, 53, 106)));
    map7.insert(StringBorderPair(OUString("E1"), table::BorderLine(0, 79, 79, 159)));
    map7.insert(StringBorderPair(OUString("F1"), table::BorderLine(0, 106, 106, 212)));

    sal_Int32 currentTable = 0; //to know which map should we check with the current table
    BorderLineMap* tempMap;
    tempMap = &map0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            if (xServiceInfo->supportsService("com.sun.star.text.TextTable"))
            {
                uno::Reference<text::XTextTable> const xTextTable(xServiceInfo, uno::UNO_QUERY_THROW);
                uno::Sequence<rtl::OUString> const cells = xTextTable->getCellNames();
                uno::Sequence< uno::Sequence< sal_Int32 > > borderWidthSequence;
                sal_Int32 nLength = cells.getLength();

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

                BorderLineMap::iterator it;
                it = tempMap->begin();

                for (sal_Int32 i = 0; i < nLength; ++i)
                {
                    uno::Reference<table::XCell> xCell = xTextTable->getCellByName(cells[i]);
                    uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
                    uno::Any aAny = xPropSet->getPropertyValue("TopBorder");
                    table::BorderLine aBorderLine;
                    it = tempMap->find(cells[i]);
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

    void testOdtBorders(uno::Reference<lang::XComponent> mxComponent)
    {
        AllBordersMap map;
        uno::Sequence< table::BorderLine > tempSequence(4);

        tempSequence[0] = table::BorderLine(0, 2, 2, 35);   // left
        tempSequence[1] = table::BorderLine(0, 2, 2, 35);   // right
        tempSequence[2] = table::BorderLine(0, 2, 2, 35);   // top
        tempSequence[3] = table::BorderLine(0, 2, 2, 35);   // bottom
        map.insert(StringSequencePair(OUString("B2"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 2, 0);
        tempSequence[1] = table::BorderLine(0, 0, 2, 0);
        tempSequence[2] = table::BorderLine(0, 0, 2, 0);
        tempSequence[3] = table::BorderLine(0, 0, 2, 0);
        map.insert(StringSequencePair(OUString("D2"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 2, 2, 88);
        tempSequence[1] = table::BorderLine(0, 2, 2, 88);
        tempSequence[2] = table::BorderLine(0, 2, 2, 88);
        tempSequence[3] = table::BorderLine(0, 2, 2, 88);
        map.insert(StringSequencePair(OUString("B4"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 18, 0);
        tempSequence[1] = table::BorderLine(0, 0, 18, 0);
        tempSequence[2] = table::BorderLine(0, 0, 18, 0);
        tempSequence[3] = table::BorderLine(0, 0, 18, 0);
        map.insert(StringSequencePair(OUString("D4"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 35, 35, 35);
        tempSequence[1] = table::BorderLine(0, 35, 35, 35);
        tempSequence[2] = table::BorderLine(0, 35, 35, 35);
        tempSequence[3] = table::BorderLine(0, 35, 35, 35);
        map.insert(StringSequencePair(OUString("B6"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 35, 0);
        tempSequence[1] = table::BorderLine(0, 0, 35, 0);
        tempSequence[2] = table::BorderLine(0, 0, 35, 0);
        tempSequence[3] = table::BorderLine(0, 0, 35, 0);
        map.insert(StringSequencePair(OUString("D6"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 88, 88, 88);
        tempSequence[1] = table::BorderLine(0, 88, 88, 88);
        tempSequence[2] = table::BorderLine(0, 88, 88, 88);
        tempSequence[3] = table::BorderLine(0, 88, 88, 88);
        map.insert(StringSequencePair(OUString("B8"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 88, 0);
        tempSequence[1] = table::BorderLine(0, 0, 88, 0);
        tempSequence[2] = table::BorderLine(0, 0, 88, 0);
        tempSequence[3] = table::BorderLine(0, 0, 88, 0);
        map.insert(StringSequencePair(OUString("D8"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 2, 35, 88);
        tempSequence[1] = table::BorderLine(0, 2, 35, 88);
        tempSequence[2] = table::BorderLine(0, 2, 35, 88);
        tempSequence[3] = table::BorderLine(0, 2, 35, 88);
        map.insert(StringSequencePair(OUString("B10"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 141, 0);
        tempSequence[1] = table::BorderLine(0, 0, 141, 0);
        tempSequence[2] = table::BorderLine(0, 0, 141, 0);
        tempSequence[3] = table::BorderLine(0, 0, 141, 0);
        map.insert(StringSequencePair(OUString("D10"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 2, 88, 88);
        tempSequence[1] = table::BorderLine(0, 2, 88, 88);
        tempSequence[2] = table::BorderLine(0, 2, 88, 88);
        tempSequence[3] = table::BorderLine(0, 2, 88, 88);
        map.insert(StringSequencePair(OUString("B12"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 0, 176, 0);
        tempSequence[1] = table::BorderLine(0, 0, 176, 0);
        tempSequence[2] = table::BorderLine(0, 0, 176, 0);
        tempSequence[3] = table::BorderLine(0, 0, 176, 0);
        map.insert(StringSequencePair(OUString("D12"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 2, 141, 88);
        tempSequence[1] = table::BorderLine(0, 2, 141, 88);
        tempSequence[2] = table::BorderLine(0, 2, 141, 88);
        tempSequence[3] = table::BorderLine(0, 2, 141, 88);
        map.insert(StringSequencePair(OUString("B14"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 35, 88, 88);
        tempSequence[1] = table::BorderLine(0, 35, 88, 88);
        tempSequence[2] = table::BorderLine(0, 35, 88, 88);
        tempSequence[3] = table::BorderLine(0, 35, 88, 88);
        map.insert(StringSequencePair(OUString("B16"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 88, 35, 35);
        tempSequence[1] = table::BorderLine(0, 88, 35, 35);
        tempSequence[2] = table::BorderLine(0, 88, 35, 35);
        tempSequence[3] = table::BorderLine(0, 88, 35, 35);
        map.insert(StringSequencePair(OUString("B18"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 88, 141, 88);
        tempSequence[1] = table::BorderLine(0, 88, 141, 88);
        tempSequence[2] = table::BorderLine(0, 88, 141, 88);
        tempSequence[3] = table::BorderLine(0, 88, 141, 88);
        map.insert(StringSequencePair(OUString("B20"), tempSequence));
        tempSequence[0] = table::BorderLine(0, 141, 88, 88);
        tempSequence[1] = table::BorderLine(0, 141, 88, 88);
        tempSequence[2] = table::BorderLine(0, 141, 88, 88);
        tempSequence[3] = table::BorderLine(0, 141, 88, 88);
        map.insert(StringSequencePair(OUString("B22"), tempSequence));

        uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
        // list of paragraphs
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        do
        {
            uno::Reference<lang::XServiceInfo> xServiceInfo;
            if (xParaEnum->nextElement() >>= xServiceInfo)
            {
                if (xServiceInfo->supportsService("com.sun.star.text.TextTable"))
                {
                    uno::Reference<text::XTextTable> const xTextTable(xServiceInfo, uno::UNO_QUERY_THROW);
                    uno::Sequence<rtl::OUString> const cells = xTextTable->getCellNames();
                    uno::Sequence< uno::Sequence< sal_Int32 > > borderWidthSequence;
                    sal_Int32 nLength = cells.getLength();

                    AllBordersMap::iterator it;
                    it = map.begin();

                    for (sal_Int32 i = 0; i < nLength; ++i)
                    {
                    if(i%10==6 || (i%10==8 && i<60))    // only B and D cells have borders, every 2 rows
                    {
                        uno::Reference<table::XCell> xCell = xTextTable->getCellByName(cells[i]);
                        uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
                        it = map.find(cells[i]);

                        uno::Any aLeftBorder = xPropSet->getPropertyValue("LeftBorder");
                        table::BorderLine aLeftBorderLine;
                        if (aLeftBorder >>= aLeftBorderLine)
                        {
                            sal_Int32 innerLineWidth = aLeftBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aLeftBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aLeftBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[0].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[0].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[0].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aRightBorder = xPropSet->getPropertyValue("RightBorder");
                        table::BorderLine aRightBorderLine;
                        if (aRightBorder >>= aRightBorderLine)
                        {
                            sal_Int32 innerLineWidth = aRightBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aRightBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aRightBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[1].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[1].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[1].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aTopBorder = xPropSet->getPropertyValue("TopBorder");
                        table::BorderLine aTopBorderLine;
                        if (aTopBorder >>= aTopBorderLine)
                        {
                            sal_Int32 innerLineWidth = aTopBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aTopBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aTopBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[2].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[2].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[2].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aBottomBorder = xPropSet->getPropertyValue("BottomBorder");
                        table::BorderLine aBottomBorderLine;
                        if (aBottomBorder >>= aBottomBorderLine)
                        {
                            sal_Int32 innerLineWidth = aBottomBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aBottomBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aBottomBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[3].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[3].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[3].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }
                    }
                    }   //end of the 'for' loop
                }
            }
        } while(xParaEnum->hasMoreElements());
    }
};
#endif
