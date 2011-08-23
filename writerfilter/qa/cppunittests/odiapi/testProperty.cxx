/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 *
 ************************************************************************/

#include <testshl/simpleheader.hxx>
#include <odiapi/props/Properties.hxx>
#include "FileLoggerImpl.hxx"
#include "ExternalViewLogger.hxx"
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <exception>
#include <stdio.h>

using namespace odiapi::props;
using namespace writerfilter;
using namespace std;
using namespace util;
using namespace osl;
using namespace rtl;

/** Helper function, get a temporary file name
 */
OString getTempFileName(const OUString& fileName)
{
  OUString ousTmpUrl;
  FileBase::getTempDirURL(ousTmpUrl);
  if (!ousTmpUrl.endsWithIgnoreAsciiCaseAsciiL("/", 1))
    ousTmpUrl += OUString::createFromAscii("/");
  ousTmpUrl += fileName;

  OUString sysTmpPath;
  FileBase::getSystemPathFromFileURL(ousTmpUrl, sysTmpPath);

  return OUStringToOString(sysTmpPath, osl_getThreadTextEncoding());
}

class TestProperty : public CppUnit::TestFixture
{
public:
    void testCreateIntProperty()
    {
        Property::Pointer_t intProp = createIntegerProperty(NS_fo::LN_font_weight, 35);
        CPPUNIT_ASSERT_MESSAGE("Wrong property id", intProp->getId() == NS_fo::LN_font_weight);
        CPPUNIT_ASSERT_MESSAGE("Wrong int value", intProp->getIntValue() == 35);
        CPPUNIT_ASSERT_MESSAGE("Wrong string value", intProp->getStringValue() == "35");
    }

    void testCreateStringProperty()
    {
        Property::Pointer_t strProp = createStringProperty(NS_style::LN_font_face, "Times New Roman");
        CPPUNIT_ASSERT_MESSAGE("Wrong property id", strProp->getId() == NS_style::LN_font_face);
        CPPUNIT_ASSERT_MESSAGE("Wrong string value", strProp->getStringValue() == "Times New Roman");
        try
        {
            strProp->getIntValue();
        }
        catch(const logic_error& ex)
        {
            return;
        }
        CPPUNIT_ASSERT_MESSAGE("Operation getIntValue should not be supported by StringProperty", false);
    }

    void testCreateCompositeProperty()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb = createPropertyBag();
        pb->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        Property::Pointer_t cp1 = createCompositeProperty(NS_style::LN_paragraph_properties, pool->insert(pb));

        CPPUNIT_ASSERT_MESSAGE("Failed to get NS_style::LN_font_face", cp1->findChild(NS_style::LN_font_face)->getStringValue() == "Times");
        CPPUNIT_ASSERT_MESSAGE("Failed to get NS_fo::LN_font_weight", cp1->findChild(NS_fo::LN_font_weight)->getIntValue() == 12);
    }

    void testCompareSimpleProperties()
    {
        Property::Pointer_t pb1 = createStringProperty(NS_style::LN_font_face, "Times New Roman");
        Property::Pointer_t pb2 = createStringProperty(NS_style::LN_font_face, "Times New Roman");
        CPPUNIT_ASSERT_MESSAGE("pb1 == pb2", pb1 == pb2);

        Property::Pointer_t fw = createIntegerProperty(NS_fo::LN_font_weight, 12);
        Property::Pointer_t ff = createStringProperty(NS_style::LN_font_face, "Times");

        CPPUNIT_ASSERT_MESSAGE("fw == fw failed", fw == fw);
        CPPUNIT_ASSERT_MESSAGE("fw > ff failed", ff < fw);
        CPPUNIT_ASSERT_MESSAGE("ff == ff failed", ff == ff);
        CPPUNIT_ASSERT_MESSAGE("!(ff < fw) failed", !(fw < ff));
    }

    void testCompareCompositeProperties()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        Property::Pointer_t cp1 = createCompositeProperty(NS_style::LN_paragraph_properties, pool->insert(pb1));

        PropertyBag_Pointer_t ps2 = createPropertyBag();
        ps2->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        ps2->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        Property::Pointer_t cp2 = createCompositeProperty(NS_style::LN_paragraph_properties, pool->insert(ps2));

        CPPUNIT_ASSERT_MESSAGE("cp1 ==  cp2 failed", cp1 == cp2);
    }

    void testPropertyBagAsStructure()
    {
        PropertyBag_Pointer_t propSeq = createPropertyBag();
        Property::Pointer_t fontWeight12 = createIntegerProperty(NS_fo::LN_font_weight, 12);

        propSeq->insert(fontWeight12);
        CPPUNIT_ASSERT_MESSAGE("Inserting property into property sequence failed", propSeq->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Property not in property sequence", propSeq->find(NS_fo::LN_font_weight)->getIntValue() == 12);
    }

    void testNoDuplicatesInPropertyBagStructures()
    {
        PropertyBag_Pointer_t propSeq = createPropertyBag();
        Property::Pointer_t fontWeight12 = createIntegerProperty(NS_fo::LN_font_weight, 12);
        propSeq->insert(fontWeight12);

        CPPUNIT_ASSERT_MESSAGE("Expect property sequence with 1 element", propSeq->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Expect property sequence with one int value 12", propSeq->find(NS_fo::LN_font_weight)->getIntValue() == 12);

        Property::Pointer_t fontWeight14 = createIntegerProperty(NS_fo::LN_font_weight, 14);
        propSeq->insert(fontWeight14);

        CPPUNIT_ASSERT_MESSAGE("Expect property sequence with 1 element", propSeq->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Expect property sequence with one int value 14", propSeq->find(NS_fo::LN_font_weight)->getIntValue() == 14);
    }

    void testPropertyBagAsArray()
    {
        PropertyBag_Pointer_t pb = createPropertyBag();
        Property::Pointer_t fontWeight12 = createIntegerProperty(NS_fo::LN_font_weight, 12);

        pb->insert(0, fontWeight12);

        CPPUNIT_ASSERT_MESSAGE("Inserting property into property sequence failed", pb->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Property not in property sequence", pb->get(0)->getIntValue() == 12);
        CPPUNIT_ASSERT_MESSAGE("Wrong property id", pb->get(0)->getId() == NS_fo::LN_font_weight);

        Iterator<Property::Pointer_t>::Pointer_t iter = pb->createIterator();
        for (iter->first(); !iter->isDone(); iter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Test property bag as array failed", iter->getCurrent()->getId() == NS_fo::LN_font_weight);
        }
    }

    void testCopyPropertyBag()
    {
        PropertyBag_Pointer_t propBag = createPropertyBag();
        Property::Pointer_t fontWeight12 = createIntegerProperty(NS_fo::LN_font_weight, 12);

        propBag->insert(0, fontWeight12);

        CPPUNIT_ASSERT_MESSAGE("Inserting property into property sequence failed", propBag->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Property not in property sequence", propBag->get(0)->getIntValue() == 12);
        CPPUNIT_ASSERT_MESSAGE("Wrong property id", propBag->get(0)->getId() == NS_fo::LN_font_weight);

        PropertyBag_Pointer_t propBagCopy = propBag->copy();

        CPPUNIT_ASSERT_MESSAGE("Copy property bag failed, distinct instances expected", propBag.get() != propBagCopy.get());

        CPPUNIT_ASSERT_MESSAGE("Copy property bag failed", propBagCopy->size() == 1);
        CPPUNIT_ASSERT_MESSAGE("Copy property bag failed", propBagCopy->get(0)->getIntValue() == 12);
        CPPUNIT_ASSERT_MESSAGE("Copy property bag failed", propBagCopy->get(0)->getId() == NS_fo::LN_font_weight);
    }

    void testClearPropertyBag()
    {
        PropertyBag_Pointer_t pb = createPropertyBag();
        pb->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));

        CPPUNIT_ASSERT_MESSAGE("Insert into property bag failed", pb->size() == 2);

        pb->clear();

        CPPUNIT_ASSERT_MESSAGE("Clearing property bag failed", pb->size() == 0);
    }

    void testSortPropertyBag()
    {
        QName_t sortedOrder [] = { NS_style::LN_font_face, NS_fo::LN_font_weight };

        PropertyBag_Pointer_t pb = createPropertyBag();
        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb->insert(createStringProperty(NS_style::LN_font_face, "Times"));

        pb->sort();

        Iterator<Property::Pointer_t>::Pointer_t iter = pb->createIterator();
        int i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++)
        {
            CPPUNIT_ASSERT_MESSAGE("Sorting property bag failed", sortedOrder[i] == iter->getCurrent()->getId());
        }
    }

    void testDuplicateValuesInArray()
    {
        PropertyBag_Pointer_t propSeq = createPropertyBag();

        Property::Pointer_t fontWeight1 = createIntegerProperty(NS_fo::LN_font_weight, 12);
        propSeq->insert(0, fontWeight1);

        Property::Pointer_t fontWeight2 = createIntegerProperty(NS_fo::LN_font_weight, 12);
        propSeq->insert(1, fontWeight2);

        CPPUNIT_ASSERT_MESSAGE("Inserting property into property sequence failed", propSeq->size() == 2);
        CPPUNIT_ASSERT_MESSAGE("Property not in property sequence",
                               propSeq->get(0)->getId() == propSeq->get(1)->getId() &&
                               propSeq->get(0)->getIntValue() == propSeq->get(1)->getIntValue());
    }

    void testPropertyPool()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph1 = pool->insert(pb1);

        PropertyBag_Pointer_t ps2 = createPropertyBag();
        ps2->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        ps2->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        PropertyPoolHandle_Pointer_t ph2 = pool->insert(ps2);

        CPPUNIT_ASSERT_MESSAGE("ph1 == ph2 failed", ph1 == ph2);

        PropertyBag_Pointer_t ps3 = createPropertyBag();
        ps3->insert(createIntegerProperty(NS_fo::LN_font_weight, 14));
        ps3->insert(createStringProperty(NS_style::LN_font_face, "Times"));

        PropertyPoolHandle_Pointer_t ph3 = pool->insert(ps3);

        CPPUNIT_ASSERT_MESSAGE("ph2 != ph3 failed", ph2 != ph3);

        PropertyBag_Pointer_t ps4 = createPropertyBag();
        ps4->insert(0, createIntegerProperty(NS_fo::LN_font_weight, 12));
        ps4->insert(1, createIntegerProperty(NS_fo::LN_font_weight, 12));
        ps4->insert(2, createIntegerProperty(NS_fo::LN_font_weight, 12));
        ps4->insert(3, createIntegerProperty(NS_fo::LN_font_weight, 12));

        pool->insert(ps4);

        OString tmpFileName = getTempFileName(OUString::createFromAscii("testPropertyPool_int.dot"));
        printf("Pool dump: %s\n", tmpFileName.getStr());
        FileLoggerImpl fl(tmpFileName.getStr());
        pool->dump(&fl);

        OString tmpFileName2 = getTempFileName(OUString::createFromAscii("testPropertyPool_ext.dot"));
        printf("Pool dump: %s\n", tmpFileName2.getStr());
        ExternalViewLoggerImpl evl(tmpFileName2.getStr());
        pool->dump(&evl);
    }

    void testCompareEqualPropertyTypesWithDifferentIdsDoesNotFail()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createIntegerProperty(NS_style::LN_tab_stop, 100));
        pb1->insert(createStringProperty(NS_style::LN_type, "left"));
        Property::Pointer_t tab100 = createCompositeProperty(NS_style::LN_tab_stop, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times New Roman"));
        Property::Pointer_t charProps1 = createCompositeProperty(NS_style::LN_char, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times New Roman"));
        Property::Pointer_t charProps2 = createCompositeProperty(NS_style::LN_char, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("CharProps1 == CharProps2 failed", charProps1 == charProps2);
    }

    void testComplexParagraphProperty()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb1 = createPropertyBag();

        pb1->insert(createIntegerProperty(NS_style::LN_position, 100));
        pb1->insert(createStringProperty(NS_style::LN_type, "left"));
        Property::Pointer_t tab100 = createCompositeProperty(NS_style::LN_tab_stop, pool->insert(pb1));

        pb1->clear();

        pb1->insert(createIntegerProperty(NS_style::LN_position, 200));
        pb1->insert(createStringProperty(NS_style::LN_type, "center"));
        Property::Pointer_t tab200 = createCompositeProperty(NS_style::LN_tab_stop, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("tab100 != tab200 failed", tab100 != tab200);

        pb1->clear();
        pb1->insert(100, tab100);
        pb1->insert(200, tab200);
        Property::Pointer_t tabs = createCompositeProperty(NS_style::LN_tab_stops, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times New Roman"));
        Property::Pointer_t charProps = createCompositeProperty(NS_style::LN_char, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_line_height, 20));
        pb1->insert(tabs);
        pb1->insert(charProps);
        Property::Pointer_t paraProps = createCompositeProperty(NS_style::LN_paragraph_properties, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_style::LN_position, 100));
        pb1->insert(createStringProperty(NS_style::LN_type, "left"));
        Property::Pointer_t tab300 = createCompositeProperty(NS_style::LN_tab_stop, pool->insert(pb1));

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_style::LN_position, 200));
        pb1->insert(createStringProperty(NS_style::LN_type, "center"));
        Property::Pointer_t tab400 = createCompositeProperty(NS_style::LN_tab_stop, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("tab300 != tab400 failed", tab300 != tab400);

        pb1->clear();
        pb1->insert(100, tab300);
        pb1->insert(200, tab400);
        Property::Pointer_t tabulators = createCompositeProperty(NS_style::LN_tab_stops, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("tabs == tabulators failed", tabs == tabulators);

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times New Roman"));
        Property::Pointer_t characterProps = createCompositeProperty(NS_style::LN_char, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("Comparison of character properties failed", charProps == characterProps);

        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_line_height, 20));
        pb1->insert(tabulators);
        pb1->insert(characterProps);
        Property::Pointer_t paragraphProps = createCompositeProperty(NS_style::LN_paragraph_properties, pool->insert(pb1));

        CPPUNIT_ASSERT_MESSAGE("paraProps == failed failed", paraProps == paragraphProps);

        OString tmpFileName = getTempFileName(OUString::createFromAscii("testComplexParaProps_int.dot"));
        printf("Pool dump: %s\n", tmpFileName.getStr());
        FileLoggerImpl fl(tmpFileName.getStr());
        pool->dump(&fl);

        OString tmpFileName2 = getTempFileName(OUString::createFromAscii("testComplexParaProps_ext.dot"));
        printf("Pool dump: %s\n", tmpFileName2.getStr());
        ExternalViewLoggerImpl evl(tmpFileName2.getStr());
        pool->dump(&evl);
    }

    void testInsertEmptyPropertyBag()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb = createPropertyBag();
        PropertyPoolHandle_Pointer_t ph = pool->insert(pb);

        CPPUNIT_ASSERT_MESSAGE("Inserting empty property bag failed", ph->getPropertyBag()->size() == 0);
    }

    void testDumpPropertyPool()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        PropertyPoolHandle_Pointer_t ph1 = pool->insert(pb1);

        Iterator<PropertyBag_Pointer_t>::Pointer_t iter = pool->createIterator();

        int i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /* nothing to do */;

        CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 1);

        /* Insert an equal PropertyBag again, as PropertyBags in the
           pool are unique there must be still just one PropertyBag in the pool
        */
        PropertyBag_Pointer_t pb2 = createPropertyBag();
        pb2->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb2->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        PropertyPoolHandle_Pointer_t ph2 = pool->insert(pb2);

        iter = pool->createIterator();

        i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /* nothing to do */;

        CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 1);

        { // scope

            /* Insert a different PropertyBag into the pool now there must be
               two PropertyBags in the pool */
            PropertyBag_Pointer_t pb3 = createPropertyBag();
            pb3->insert(createIntegerProperty(NS_style::LN_position, 12));
            pb3->insert(createStringProperty(NS_style::LN_type, "left"));
            PropertyPoolHandle_Pointer_t ph3 = pool->insert(pb3);

            iter = pool->createIterator();

            i = 0;
            for (iter->first(); !iter->isDone(); iter->next(), i++) /* nothing to do */;

            CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 2);

        } // end scope

        /* as pb3 is only valid in the above scope the property pool must
           now contain just one property bag
        */
        iter = pool->createIterator();

        i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /*nothing to do*/;

        CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 1);
    }

    void testInsertPropertySubsets()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        PropertyPoolHandle_Pointer_t ph1 = pool->insert(pb1);

        /* Insert an equal PropertyBag again, as PropertyBags in the
           pool are unique there must be still just one PropertyBag in the pool
        */
        pb1->clear();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph2 = pool->insert(pb1);

        CPPUNIT_ASSERT_MESSAGE("ph1 != ph2 failed", ph1 != ph2);

        Iterator<PropertyBag_Pointer_t>::Pointer_t iter = pool->createIterator();

        int i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /* nothing to do */;

        CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 2);
    }

    void testDumpEmptyPropertyPool()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        Iterator<PropertyBag_Pointer_t>::Pointer_t iter = pool->createIterator();

        int i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /*nothing to do*/;

        CPPUNIT_ASSERT_MESSAGE("Dump PropertyBags failed", i == 0);
    }

    void testPropertyPoolGarbageCollection()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb1->insert(createIntegerProperty(NS_fo::LN_line_height, 20));
        PropertyPoolHandle_Pointer_t ph1 = pool->insert(pb1);

        {
            PropertyBag_Pointer_t pb2 = createPropertyBag();
            pb2->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
            pb2->insert(createStringProperty(NS_style::LN_font_face, "Roman"));
            PropertyPoolHandle_Pointer_t ph2 = pool->insert(pb2);

            OString tmpFileName = getTempFileName(OUString::createFromAscii("testPropPoolGarbageColl_1.dot"));
            printf("Pool dump: %s\n", tmpFileName.getStr());
            FileLoggerImpl fl(tmpFileName.getStr());
            pool->dump(&fl);

        }

        OString tmpFileName = getTempFileName(OUString::createFromAscii("testPropPoolGarbageColl_2.dot"));
        printf("Pool dump: %s\n", tmpFileName.getStr());
        FileLoggerImpl fl(tmpFileName.getStr());
        pool->dump(&fl);

        pool->garbageCollection();

        OString tmpFileName2 = getTempFileName(OUString::createFromAscii("testPropPoolGarbageColl_after.dot"));
        printf("Pool dump: %s\n", tmpFileName2.getStr());
        FileLoggerImpl fl2(tmpFileName2.getStr());
        pool->dump(&fl2);

    }

    void testDumpPropertyPoolAfterGarbageCollection()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();

        PropertyBag_Pointer_t pb1 = createPropertyBag();
        pb1->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        pb1->insert(createStringProperty(NS_style::LN_font_face, "Times"));
        pb1->insert(createIntegerProperty(NS_fo::LN_line_height, 20));
        PropertyPoolHandle_Pointer_t ph1 = pool->insert(pb1);

        {
            PropertyBag_Pointer_t pb2 = createPropertyBag();
            pb2->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
            pb2->insert(createStringProperty(NS_style::LN_font_face, "Roman"));
            PropertyPoolHandle_Pointer_t ph2 = pool->insert(pb2);

            Iterator<PropertyBag_Pointer_t>::Pointer_t iter = pool->createIterator();

            int i = 0;
            for (iter->first(); !iter->isDone(); iter->next(), i++) /*nothing to do*/;

            CPPUNIT_ASSERT_MESSAGE("Expectation '2 PropertyBags in PropertyPool' failed", i == 2);
        }

        pool->garbageCollection();

        Iterator<PropertyBag_Pointer_t>::Pointer_t iter = pool->createIterator();

        int i = 0;
        for (iter->first(); !iter->isDone(); iter->next(), i++) /*nothing to do*/;

        CPPUNIT_ASSERT_MESSAGE("Expectation '1 PropertyBag in PropertyPool' failed", i == 1);
    }

    // 'cm', 'mm', 'inch', 'pt', 'px', 'pc'
    void testCreateTwipsProperty()
    {
        Property::Pointer_t tp1 = createTwipsProperty(NS_style::LN_position, "1cm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp1->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp1->getStringValue() == "1.000 cm");

        Property::Pointer_t tp2 = createTwipsProperty(NS_style::LN_position, "1 cm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp2->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp2->getStringValue() == "1.000 cm");

        Property::Pointer_t tp3 = createTwipsProperty(NS_style::LN_position, "1 cm ");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp3->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp3->getStringValue() == "1.000 cm");

        Property::Pointer_t tp4 = createTwipsProperty(NS_style::LN_position, "0 cm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp4->getIntValue() == 0);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp4->getStringValue() == "0.000 cm");

        Property::Pointer_t tp5 = createTwipsProperty(NS_style::LN_position, "10mm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp5->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp5->getStringValue() == "1.000 cm");

        Property::Pointer_t tp6 = createTwipsProperty(NS_style::LN_position, 567);
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp6->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp6->getStringValue() == "1.000 cm");

        Property::Pointer_t tp7 = createTwipsProperty(NS_style::LN_position, "100pt");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp7->getIntValue() == 2000);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp7->getStringValue() == "3.527 cm");

        Property::Pointer_t tp8 = createTwipsProperty(NS_style::LN_position, "1 in");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp8->getIntValue() == 1440);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp8->getStringValue() == "2.540 cm");

        Property::Pointer_t tp9 = createTwipsProperty(NS_style::LN_position, "-1 cm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp9->getIntValue() == -567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp9->getStringValue() == "-1.000 cm");

        Property::Pointer_t tp10 = createTwipsProperty(NS_style::LN_position, "+1 cm");
        CPPUNIT_ASSERT_MESSAGE("getIntValue: wrong twips value returned", tp10->getIntValue() == 567);
        CPPUNIT_ASSERT_MESSAGE("getStringValue: wrong twips value returned", tp10->getStringValue() == "1.000 cm");

        Property::Pointer_t tp11 = createTwipsProperty(NS_style::LN_position, "1 pt ");
        Property::Pointer_t tp12 = createTwipsProperty(NS_style::LN_position, "2pt");
        CPPUNIT_ASSERT_MESSAGE("Comparing twips properties failed", tp11 < tp12);
    }

    void testCreateInvalidTwipsProperty()
    {
        try
        {
            Property::Pointer_t tp = createTwipsProperty(NS_style::LN_position, "0,1 cm");
        }
        catch(std::invalid_argument& )
        {
            return; // OK
        }
        CPPUNIT_ASSERT_MESSAGE("Creating an twips property with invalid number must fail", false);
    }

    void testCreateInvalidTwipsProperty2()
    {
        try
        {
            Property::Pointer_t tp = createTwipsProperty(NS_style::LN_position, "");
        }
        catch(std::invalid_argument& )
        {
            return; // OK
        }
        CPPUNIT_ASSERT_MESSAGE("Creating an twips property with invalid number must fail", false);
    }

    void testCreateInvalidTwipsProperty3()
    {
        try
        {
            Property::Pointer_t tp = createTwipsProperty(NS_style::LN_position, "	cm");
        }
        catch(std::invalid_argument& )
        {
            return; // OK
        }
        CPPUNIT_ASSERT_MESSAGE("Creating an twips property with invalid number must fail", false);
    }

    CPPUNIT_TEST_SUITE(TestProperty);
    CPPUNIT_TEST(testCreateIntProperty);
    CPPUNIT_TEST(testCreateStringProperty);
    CPPUNIT_TEST(testCreateCompositeProperty);
    CPPUNIT_TEST(testPropertyBagAsStructure);
    CPPUNIT_TEST(testNoDuplicatesInPropertyBagStructures);
    CPPUNIT_TEST(testPropertyBagAsArray);
    CPPUNIT_TEST(testDuplicateValuesInArray);
    CPPUNIT_TEST(testCopyPropertyBag);
    CPPUNIT_TEST(testClearPropertyBag);
    CPPUNIT_TEST(testSortPropertyBag);
    CPPUNIT_TEST(testCompareSimpleProperties);
    CPPUNIT_TEST(testCompareCompositeProperties);
    CPPUNIT_TEST(testPropertyPool);
    CPPUNIT_TEST(testComplexParagraphProperty);
    CPPUNIT_TEST(testInsertEmptyPropertyBag);
    CPPUNIT_TEST(testCompareEqualPropertyTypesWithDifferentIdsDoesNotFail);
    CPPUNIT_TEST(testDumpPropertyPool);
    CPPUNIT_TEST(testDumpEmptyPropertyPool);
    CPPUNIT_TEST(testInsertPropertySubsets);
    CPPUNIT_TEST(testPropertyPoolGarbageCollection);
    CPPUNIT_TEST(testDumpPropertyPoolAfterGarbageCollection);
    CPPUNIT_TEST(testCreateTwipsProperty);
    CPPUNIT_TEST(testCreateInvalidTwipsProperty);
    CPPUNIT_TEST(testCreateInvalidTwipsProperty2);
    CPPUNIT_TEST(testCreateInvalidTwipsProperty3);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestProperty, "TestProperty");

NOADDITIONAL;
