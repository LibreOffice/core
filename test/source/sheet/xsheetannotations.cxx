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

#include "cppunit/extensions/HelperMacros.h"
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
    uno::Reference< sheet::XSheetAnnotations > xSheet2Annotations( getAnnotations(1), UNO_QUERY_THROW);

    // insert a note on sheet 2
    table::CellAddress xTargetCellAddress (1,0,0);
    xSheet2Annotations->insertNew(xTargetCellAddress, "an inserted annotation on sheet 2");

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
    table::CellAddress xTargetCellAddress (0,3,4);
    aSheetAnnotations->insertNew(xTargetCellAddress, "an inserted annotation");

    // count after inserting
    //uno::Reference< container::XIndexAccess > xAnnotationsIndexAfter (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nAfter = xAnnotationsIndex->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Annotations index not updated", nBefore + 1, nAfter);

    // is the position ok ?
    uno::Reference< sheet::XSheetAnnotation > aLastSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-1), UNO_QUERY_THROW);
    table::CellAddress xResultCellAddress = aLastSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong SHEET reference position",
        xTargetCellAddress.Sheet, xResultCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong COLUMN reference position",
        xTargetCellAddress.Column, xResultCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong ROW reference position",
        xTargetCellAddress.Row, xResultCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aString = aTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Insert Annotation - Wrong string", OUString("an inserted annotation"),
        aString);

}

void XSheetAnnotations::testRemoveByIndex()
{
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (init(), UNO_QUERY_THROW);

    // insert some annotations
    table::CellAddress xTargetCellAddress (0,4,5);
    aSheetAnnotations->insertNew(xTargetCellAddress, "an inserted annotation 1");
    table::CellAddress xToBeRemovedCellAddress (0,5,6);
    aSheetAnnotations->insertNew(xToBeRemovedCellAddress, "an inserted annotation 2");
    table::CellAddress xOtherCellAddress (0,7,8);
    aSheetAnnotations->insertNew(xOtherCellAddress, "an inserted annotation 3");

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
    table::CellAddress xResultCellAddress = aLastSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong SHEET reference position",
        xOtherCellAddress.Sheet, xResultCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong COLUMN reference position",
        xOtherCellAddress.Column, xResultCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong ROW reference position",
        xOtherCellAddress.Row, xResultCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aLastTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aLastString = aLastTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong string",
        OUString("an inserted annotation 3"), aLastString);

    // the previous should be xTargetCellAddress
    uno::Reference< sheet::XSheetAnnotation > aPreviousSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-2), UNO_QUERY_THROW);
    table::CellAddress xPreviousCellAddress = aPreviousSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong SHEET reference position",
        xTargetCellAddress.Sheet, xPreviousCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong COLUMN reference position",
        xTargetCellAddress.Column, xPreviousCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong ROW reference position",
        xTargetCellAddress.Row, xPreviousCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aPreviousTextSheetAnnotation(aPreviousSheetAnnotation, UNO_QUERY_THROW);
    OUString aPreviousString = aPreviousTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Remove Annotation - Wrong string",
        OUString("an inserted annotation 1"), aPreviousString);
}

void XSheetAnnotations::testGetByIndex()
{

    // testing #fdo80551 - getByIndex not on the first sheet

    // insert annotations in first sheet
    uno::Reference< sheet::XSheetAnnotations > aSheet0Annotations (init(), UNO_QUERY_THROW);
    table::CellAddress xTargetCellAddress0 (0,0,1);
    aSheet0Annotations->insertNew(xTargetCellAddress0, "an inserted annotation 1 on sheet 1");
    table::CellAddress xSecondTargetCellAddress0 (0,0,2);
    aSheet0Annotations->insertNew(xSecondTargetCellAddress0, "an inserted annotation 2 on sheet 1");
    table::CellAddress xThirdCellAddress0 (0,0,3);
    aSheet0Annotations->insertNew(xThirdCellAddress0, "an inserted annotation 3 on sheet 1");

    // insert annotations in third sheet
    uno::Reference< sheet::XSheetAnnotations > aSheet2Annotations (getAnnotations(2), UNO_QUERY_THROW);
    table::CellAddress xTargetCellAddress2 (2,4,5);
    aSheet2Annotations->insertNew(xTargetCellAddress2, "an inserted annotation 1 on sheet 3");
    table::CellAddress xSecondTargetCellAddress2 (2,5,6);
    aSheet2Annotations->insertNew(xSecondTargetCellAddress2, "an inserted annotation 2 on sheet 3");
    table::CellAddress xThirdCellAddress2 (2,7,8);
    aSheet2Annotations->insertNew(xThirdCellAddress2, "an inserted annotation 3 on sheet 3");

    // get second annotation for second sheet
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (getAnnotations(2), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotation > aAnnotation (xAnnotationsIndex->getByIndex(1), UNO_QUERY_THROW);

    table::CellAddress xToBeAnalyzedCellAddress = aAnnotation->getPosition();

    // is the CellAddress ok ?
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong SHEET reference position",
        xSecondTargetCellAddress2.Sheet, xToBeAnalyzedCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong COLUMN reference position",
        xSecondTargetCellAddress2.Column, xToBeAnalyzedCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong ROW reference position",
        xSecondTargetCellAddress2.Row, xToBeAnalyzedCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aTextSheetAnnotation(aAnnotation, UNO_QUERY_THROW);
    OUString aString = aTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "GetByIndex Annotation - Wrong string",
        OUString("an inserted annotation 2 on sheet 3"), aString);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
