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

#include "test/testdllapi.hxx"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/ustring.hxx>

using namespace com::sun::star;

namespace apitest {

class OOO_DLLPUBLIC_TEST XSpreadsheets2
{
public:
    XSpreadsheets2();
    virtual ~XSpreadsheets2();

    // XSpreadsheets2
    void testImportedSheetNameAndIndex();
    void testImportString();
    void testImportValue();
    void testImportFormulaBasicMath();
    void testImportFormulaWithNamedRange();
    void testImportOverExistingNamedRange();
    void testImportNamedRangeDefinedInSource();
    void testImportNamedRangeRedefinedInSource();
    void testImportNewNamedRange();
    void testImportCellStyle();

    virtual uno::Reference< lang::XComponent > getComponent() = 0;
    virtual uno::Reference< uno::XInterface > init() = 0;
    virtual uno::Reference< lang::XComponent > loadFromDesktop(const OUString&) = 0;
    virtual void createFileURL(const OUString&, OUString&) = 0;

protected:
    uno::Reference< sheet::XSpreadsheetDocument> xDocument;

private:
    uno::Reference< sheet::XSpreadsheetDocument> getDoc(const OUString&, uno::Reference< lang::XComponent >&);
    uno::Reference< sheet::XNamedRanges> getNamedRanges(uno::Reference< sheet::XSpreadsheetDocument >);
    void importSheetToCopy();
    bool isExternalReference(const OUString& aDestContent, const OUString& aSrcContent );

    uno::Reference< sheet::XSpreadsheetDocument> xDestDoc;
    uno::Reference< sheet::XSpreadsheet > xDestSheet;
    uno::Reference< sheet::XSpreadsheet > xSrcSheet;
    OUString aSrcSheetName;
    OUString aSrcFileName;
    OUString aDestFileBase;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
