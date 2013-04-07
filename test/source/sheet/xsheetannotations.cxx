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
 * Copyright (C) 2011 Laurent Godard lgodard.libre@laposte.net (initial developer)
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

#include <test/sheet/xsheetannotations.hxx>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace com::sun::star::uno;

namespace apitest {

void XSheetAnnotations::testInsertNew()
{
    uno::Reference< sheet::XSheetAnnotations > aSheetAnnotations (init(), UNO_QUERY_THROW);

    // count before inserting
    uno::Reference< container::XIndexAccess > xAnnotationsIndex (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nBefore = xAnnotationsIndex->getCount();

    // insert the annotation
    table::CellAddress xTargetCellAddress (0,3,4);
    aSheetAnnotations->insertNew(xTargetCellAddress, "an inserted annotation");

    // count after inserting
    //uno::Reference< container::XIndexAccess > xAnnotationsIndexAfter (aSheetAnnotations, UNO_QUERY_THROW);
    sal_Int32 nAfter = xAnnotationsIndex->getCount();

    CPPUNIT_ASSERT_MESSAGE("Annotations index not updated", nAfter == nBefore + 1);

    // is the position ok ?
    uno::Reference< sheet::XSheetAnnotation > aLastSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-1), UNO_QUERY_THROW);
    table::CellAddress xResultCellAddress = aLastSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_MESSAGE("Insert Annotation - Wrong SHEET reference position", xResultCellAddress.Sheet == xTargetCellAddress.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Insert Annotation - Wrong COLUMN reference position", xResultCellAddress.Column == xTargetCellAddress.Column);
    CPPUNIT_ASSERT_MESSAGE("Insert Annotation - Wrong ROW reference position", xResultCellAddress.Row == xTargetCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aString = aTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_MESSAGE("Insert Annotation - Wrong string", aString == "an inserted annotation");

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

    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong SHEET reference position", xResultCellAddress.Sheet == xOtherCellAddress.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong COLUMN reference position", xResultCellAddress.Column == xOtherCellAddress.Column);
    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong ROW reference position", xResultCellAddress.Row == xOtherCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aLastTextSheetAnnotation(aLastSheetAnnotation, UNO_QUERY_THROW);
    OUString aLastString = aLastTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong string", aLastString == "an inserted annotation 3");

    // the previous should be xTargetCellAddress
    uno::Reference< sheet::XSheetAnnotation > aPreviousSheetAnnotation (xAnnotationsIndex->getByIndex(nAfter-2), UNO_QUERY_THROW);
    table::CellAddress xPreviousCellAddress = aPreviousSheetAnnotation->getPosition();

    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong SHEET reference position", xPreviousCellAddress.Sheet == xTargetCellAddress.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong COLUMN reference position", xPreviousCellAddress.Column == xTargetCellAddress.Column);
    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong ROW reference position", xPreviousCellAddress.Row == xTargetCellAddress.Row);

    // is the string ok ?
    uno::Reference< text::XTextRange > aPreviousTextSheetAnnotation(aPreviousSheetAnnotation, UNO_QUERY_THROW);
    OUString aPreviousString = aPreviousTextSheetAnnotation->getString();

    CPPUNIT_ASSERT_MESSAGE("Remove Annotation - Wrong string", aPreviousString == "an inserted annotation 1");

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
