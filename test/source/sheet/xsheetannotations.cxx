/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetannotations.hxx>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <cppunit/TestAssert.h>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetAnnotations::testCount()
{
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (init(), UNO_QUERY_THROW);

    // count on sheet 1 before inserting
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nBefore = xAnnotationsIndex->getCount();

    // get Sheet 2 annotations
    uno::Reference< sheet::XSheetAnnotations > xSheet2Annotations( getAnnotations(1), UNO_SET_THROW);

    // insert a note on sheet 2
    table::CellAddress aTargetCellAddress (1,0,0);
    xSheet2Annotations->insertNew(aTargetCellAddress, u"an inserted annotation on sheet 2"_ustr);

    // count again on sheet 1
    sal_Int32 nAfter = xAnnotationsIndex->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Annotations count should not change on sheet 1", nBefore, nAfter);
}

void XSheetAnnotations::testInsertNew()
{
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (init(), UNO_QUERY_THROW);

    // count before inserting
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nBefore = xAnnotationsIndex->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should already be one note", sal_Int32(1), nBefore );

    // insert the annotation
    table::CellAddress aTargetCellAddress (0,3,4);
    aSheetAnnotations->insertNew(aTargetCellAddress, u"an inserted annotation"_ustr);

    // count after inserting
    //uno::Reference< container::XIndexAccess > xAnnotationsIndexAfter (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nAfter = xAnnotationsIndex->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Annotations index not updated", nBefore + 1, nAfter);

    // is the position ok ?
    uno::Reference< sheet::XSheetAnnotation > aLastSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-1), UNO_QUERY_THROW);
    table::CellAddress aResultCellAddress = aLastSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong SHEET reference position",
        aTargetCellAddress.Sheet, aResultCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong COLUMN reference position",
        aTargetCellAddress.Column, aResultCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong ROW reference position",
        aTargetCellAddress.Row, aResultCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aString = aTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong string", u"an inserted annotation"_ustr,
        aString);

}

void XSheetAnnotations::testRemoveByIndex()
{
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (init(), UNO_QUERY_THROW);

    // insert some annotations
    table::CellAddress aTargetCellAddress (0,4,5);
    aSheetAnnotations->insertNew(aTargetCellAddress, u"an inserted annotation 1"_ustr);
    table::CellAddress aToBeRemovedCellAddress (0,5,6);
    aSheetAnnotations->insertNew(aToBeRemovedCellAddress, u"an inserted annotation 2"_ustr);
    table::CellAddress aOtherCellAddress (0,7,8);
    aSheetAnnotations->insertNew(aOtherCellAddress, u"an inserted annotation 3"_ustr);

    // count before removing
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nBefore = xAnnotationsIndex->getCount();

    // remove the xToBeRemovedCellAddress
    aSheetAnnotations->removeByIndex(nBefore-2);

    // count after removing
    //uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nAfter = xAnnotationsIndex->getCount();

    // the last position should be xOtherCellAddress
    uno::Reference< sheet::XSheetAnnotation > aLastSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-1), UNO_QUERY_THROW);
    table::CellAddress aResultCellAddress = aLastSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong SHEET reference position",
        aOtherCellAddress.Sheet, aResultCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong COLUMN reference position",
        aOtherCellAddress.Column, aResultCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong ROW reference position",
        aOtherCellAddress.Row, aResultCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aLastTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aLastString = aLastTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong string",
        u"an inserted annotation 3"_ustr, aLastString);

    // the previous should be xTargetCellAddress
    uno::Reference< sheet::XSheetAnnotation > aPreviousSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-2), UNO_QUERY_THROW);
    table::CellAddress aPreviousCellAddress = aPreviousSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong SHEET reference position",
        aTargetCellAddress.Sheet, aPreviousCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong COLUMN reference position",
        aTargetCellAddress.Column, aPreviousCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong ROW reference position",
        aTargetCellAddress.Row, aPreviousCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aPreviousTextSheetAnnotation(aPreviousSheetAnnotation, UNO_QUERY_THROW);
    OUString aPreviousString = aPreviousTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong string",
        u"an inserted annotation 1"_ustr, aPreviousString);
}

void XSheetAnnotations::testIndex()
{

    // testing #fdo80551 - getByIndex not on the first sheet

    // insert annotations in first sheet
    uno::Reference< sheet::XSheetAnnotations > aSheet0Annotations (init(), UNO_QUERY_THROW);
    table::CellAddress aTargetCellAddress0 (0,0,1);
    aSheet0Annotations->insertNew(aTargetCellAddress0, u"an inserted annotation 1 on sheet 1"_ustr);
    table::CellAddress aSecondTargetCellAddress0 (0,0,2);
    aSheet0Annotations->insertNew(aSecondTargetCellAddress0, u"an inserted annotation 2 on sheet 1"_ustr);
    table::CellAddress aThirdCellAddress0 (0,0,3);
    aSheet0Annotations->insertNew(aThirdCellAddress0, u"an inserted annotation 3 on sheet 1"_ustr);

    // insert annotations in third sheet
    uno::Reference< sheet::XSheetAnnotations > aSheet2Annotations (getAnnotations(2), UNO_SET_THROW);
    table::CellAddress aTargetCellAddress2 (2,4,5);
    aSheet2Annotations->insertNew(aTargetCellAddress2, u"an inserted annotation 1 on sheet 3"_ustr);
    table::CellAddress aSecondTargetCellAddress2 (2,5,6);
    aSheet2Annotations->insertNew(aSecondTargetCellAddress2, u"an inserted annotation 2 on sheet 3"_ustr);
    table::CellAddress aThirdCellAddress2 (2,7,8);
    aSheet2Annotations->insertNew(aThirdCellAddress2, u"an inserted annotation 3 on sheet 3"_ustr);

    // get second annotation for second sheet
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (getAnnotations(2), UNO_SET_THROW);
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotation > aAnnotation (xAnnotationsIndex->getByIndex(1), UNO_QUERY_THROW);

    table::CellAddress aToBeAnalyzedCellAddress = aAnnotation->getPosition();

    // is the CellAddress ok ?
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong SHEET reference position",
        aSecondTargetCellAddress2.Sheet, aToBeAnalyzedCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong COLUMN reference position",
        aSecondTargetCellAddress2.Column, aToBeAnalyzedCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong ROW reference position",
        aSecondTargetCellAddress2.Row, aToBeAnalyzedCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aTextSheetAnnotation(aAnnotation, UNO_QUERY_THROW);
    OUString aString = aTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong string",
        u"an inserted annotation 2 on sheet 3"_ustr, aString);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
