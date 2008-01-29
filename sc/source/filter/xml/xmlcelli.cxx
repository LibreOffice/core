/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlcelli.cxx,v $
 *
 *  $Revision: 1.94 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:36:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "xmlcelli.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"
#include "xmlannoi.hxx"
#include "global.hxx"
#include "document.hxx"
#include "cellsuno.hxx"
#include "docuno.hxx"
#include "unonames.hxx"
#include "postit.hxx"

#include "XMLTableShapeImportHelper.hxx"
#include "XMLTextPContext.hxx"
#include "XMLStylesImportHelper.hxx"

#include "arealink.hxx"
#include <svx/linkmgr.hxx>
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "scerrors.hxx"
#include "editutil.hxx"
#include "cell.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <svtools/zforlist.hxx>
#include <svx/outlobj.hxx>
#include <svtools/languageoptions.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/date.hxx>
#include <i18npool/lang.h>
#include <comphelper/extract.hxx>

#define SC_CURRENCYSYMBOL   "CurrencySymbol"

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScMyImportAnnotation::~ScMyImportAnnotation()
{
    delete pRect;
    delete pItemSet;
    delete pOPO;
}

//------------------------------------------------------------------

ScXMLTableRowCellContext::ScXMLTableRowCellContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempIsCovered,
                                      const sal_Int32 nTempRepeatedRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pOUTextValue(NULL),
    pOUTextContent(NULL),
    pOUFormula(NULL),
    pContentValidationName(NULL),
    pMyAnnotation(NULL),
    pDetectiveObjVec(NULL),
    pCellRangeSource(NULL),
    fValue(0.0),
    nMergedRows(1),
    nMergedCols(1),
    nRepeatedRows(nTempRepeatedRows),
    nCellsRepeated(1),
    rXMLImport((ScXMLImport&)rImport),
    nCellType(util::NumberFormat::TEXT),
    bIsMerged(sal_False),
    bIsMatrix(sal_False),
    bHasSubTable(sal_False),
    bIsCovered(bTempIsCovered),
    bIsEmpty(sal_True),
    bHasTextImport(sal_False),
    bIsFirstTextImport(sal_False),
    bSolarMutexLocked(sal_False),
    bFormulaTextResult(sal_False)
{
    rXMLImport.SetRemoveLastChar(sal_False);
    rXMLImport.GetTables().AddColumn(bTempIsCovered);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    rtl::OUString aLocalName;
    rtl::OUString* pStyleName = NULL;
    rtl::OUString* pCurrencySymbol = NULL;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        sal_uInt16 nPrefix = rXMLImport.GetNamespaceMap().GetKeyByAttrName(
                                            xAttrList->getNameByIndex( i ), &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            sal_uInt32 nLength(aLocalName.getLength());

            switch (nLength)
            {
            case 7 :
                {
                    if (IsXMLToken(aLocalName, XML_FORMULA))
                    {
                        if (sValue.getLength())
                        {
                            DBG_ASSERT(!pOUFormula, "here should be only one formula");
                            pOUFormula = new rtl::OUString();
                            sal_uInt16 nFormulaPrefix = GetImport().GetNamespaceMap().
                                    _GetKeyByAttrName( sValue, pOUFormula, sal_False );
                            // #i56720# For any valid namespace, the formula text is the part without
                            // the namespace tag.
                            // Only for an invalid namespace (not defined in the file, XML_NAMESPACE_UNKNOWN)
                            // or no namespace tag (XML_NAMESPACE_NONE) the full text is used.
                            // An invalid namespace can occur from a colon in the formula text if no
                            // namespace tag was added.
                            if ( nFormulaPrefix == XML_NAMESPACE_UNKNOWN || nFormulaPrefix == XML_NAMESPACE_NONE )
                            {
                                delete pOUFormula;
                                pOUFormula = new rtl::OUString(sValue);
                            }
                        }
                    }
                }
                break;
            case 10 :
                {
                    if (IsXMLToken(aLocalName, XML_STYLE_NAME))
                        pStyleName = new rtl::OUString(sValue);
                }
                break;
            case 19 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_ROWS_SPANNED))
                    {
                        bIsMerged = sal_True;
                        nMergedRows = sValue.toInt32();
                    }
                }
                break;
            case 22 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_COLUMNS_SPANNED))
                    {
                        bIsMerged = sal_True;
                        nMergedCols = sValue.toInt32();
                    }
                }
                break;
            case 23 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_COLUMNS_REPEATED))
                        nCellsRepeated = std::max( sValue.toInt32(), (sal_Int32) 1 );
                    else if (IsXMLToken(aLocalName, XML_CONTENT_VALIDATION_NAME))
                    {
                        DBG_ASSERT(!pContentValidationName, "here should be only one Validation Name");
                        pContentValidationName = new rtl::OUString(sValue);
                    }
                }
                break;
            case 26 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_ROWS_SPANNED))
                    {
                        bIsMatrix = sal_True;
                        nMatrixRows = sValue.toInt32();
                    }
                }
                break;
            case 29 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_COLUMNS_SPANNED))
                    {
                        bIsMatrix = sal_True;
                        nMatrixCols = sValue.toInt32();
                    }
                }
                break;
            }
        }
        else if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            sal_uInt32 nLength(aLocalName.getLength());

            switch (nLength)
            {
            case 5 :
                {
                    if (IsXMLToken(aLocalName, XML_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            rXMLImport.GetMM100UnitConverter().convertDouble(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 8 :
                {
                    if (IsXMLToken(aLocalName, XML_CURRENCY))
                        pCurrencySymbol = new rtl::OUString(sValue);
                }
                break;
            case 10 :
                {
                    if (IsXMLToken(aLocalName, XML_VALUE_TYPE))
                    {
                        nCellType = GetCellType(sValue);
                        bIsEmpty = sal_False;
                    }
                    else if (IsXMLToken(aLocalName, XML_DATE_VALUE))
                    {
                        if (sValue.getLength() && rXMLImport.SetNullDateOnUnitConverter())
                        {
                            rXMLImport.GetMM100UnitConverter().convertDateTime(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                    else if (IsXMLToken(aLocalName, XML_TIME_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            rXMLImport.GetMM100UnitConverter().convertTime(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 12 :
                {
                    if (IsXMLToken(aLocalName, XML_STRING_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            DBG_ASSERT(!pOUTextValue, "here should be only one string value");
                            pOUTextValue = new rtl::OUString(sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 13 :
                {
                    if (IsXMLToken(aLocalName, XML_BOOLEAN_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            if ( IsXMLToken(sValue, XML_TRUE) )
                                fValue = 1.0;
                            else if ( IsXMLToken(sValue, XML_FALSE) )
                                fValue = 0.0;
                            else
                                rXMLImport.GetMM100UnitConverter().convertDouble(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            }
        }
    }
    if (pOUFormula)
    {
        if (nCellType == util::NumberFormat::TEXT)
            bFormulaTextResult = sal_True;
        nCellType = util::NumberFormat::UNDEFINED;
    }
    rXMLImport.GetStylesImportHelper()->SetAttributes(pStyleName, pCurrencySymbol, nCellType);
}

sal_Int16 ScXMLTableRowCellContext::GetCellType(const rtl::OUString& sOUValue) const
{
    if (IsXMLToken(sOUValue, XML_FLOAT))
        return util::NumberFormat::NUMBER;
    else
        if (IsXMLToken(sOUValue, XML_STRING))
            return util::NumberFormat::TEXT;
        else
            if (IsXMLToken(sOUValue, XML_TIME))
                return util::NumberFormat::TIME;
            else
                if (IsXMLToken(sOUValue, XML_DATE))
                    return util::NumberFormat::DATETIME;
                else
                    if (IsXMLToken(sOUValue, XML_PERCENTAGE))
                        return util::NumberFormat::PERCENT;
                    else
                        if (IsXMLToken(sOUValue, XML_CURRENCY))
                            return util::NumberFormat::CURRENCY;
                        else
                            if (IsXMLToken(sOUValue, XML_BOOLEAN))
                                return util::NumberFormat::LOGICAL;
                            else
                                return util::NumberFormat::UNDEFINED;
}

ScXMLTableRowCellContext::~ScXMLTableRowCellContext()
{
    if (pOUTextValue)
        delete pOUTextValue;
    if (pOUTextContent)
        delete pOUTextContent;
    if (pOUFormula)
        delete pOUFormula;
    if (pContentValidationName)
        delete pContentValidationName;
    if (pMyAnnotation)
        delete pMyAnnotation;
    if (pDetectiveObjVec)
        delete pDetectiveObjVec;
    if (pCellRangeSource)
        delete pCellRangeSource;
}

void ScXMLTableRowCellContext::LockSolarMutex()
{
    if (!bSolarMutexLocked)
    {
        GetScImport().LockSolarMutex();
        bSolarMutexLocked = sal_True;
    }
}

void ScXMLTableRowCellContext::UnlockSolarMutex()
{
    if (bSolarMutexLocked)
    {
        GetScImport().UnlockSolarMutex();
        bSolarMutexLocked = sal_False;
    }
}

void ScXMLTableRowCellContext::SetCursorOnTextImport(const rtl::OUString& rOUTempText)
{
    com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
    if (CellExists(aCellPos))
    {
        uno::Reference<table::XCellRange> xCellRange(rXMLImport.GetTables().GetCurrentXCellRange());
        if (xCellRange.is())
        {
            xBaseCell.set(xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row));
            if (xBaseCell.is())
            {
                xLockable.set(xBaseCell, uno::UNO_QUERY);
                if (xLockable.is())
                    xLockable->addActionLock();
                uno::Reference<text::XText> xText(xBaseCell, uno::UNO_QUERY);
                if (xText.is())
                {
                    uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor());
                    if (xTextCursor.is())
                    {
                        xTextCursor->setString(rOUTempText);
                        xTextCursor->gotoEnd(sal_False);
                        rXMLImport.GetTextImport()->SetCursor(xTextCursor);
                    }
                }
            }
        }
    }
    else
    {
        DBG_ERRORFILE("this method should only be called for a existing cell");
    }
}

SvXMLImportContext *ScXMLTableRowCellContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rXMLImport.GetTableRowCellElemTokenMap();
    sal_Bool bTextP(sal_False);
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL_P:
        {
            bIsEmpty = sal_False;
            bTextP = sal_True;
            com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
            if (((nCellType == util::NumberFormat::TEXT) || bFormulaTextResult) &&
                !rXMLImport.GetTables().IsPartOfMatrix(aCellPos.Column, aCellPos.Row))
            {
                if (!bHasTextImport)
                {
                    bIsFirstTextImport = sal_True;
                    bHasTextImport = sal_True;
                    pContext = new ScXMLTextPContext(rXMLImport, nPrefix, rLName, xAttrList, this);
                }
                else
                {
                    // com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
                    if (CellExists(aCellPos))
                    {
                        if (bIsFirstTextImport && !rXMLImport.GetRemoveLastChar())
                        {
                            if (pOUTextContent)
                            {
                                SetCursorOnTextImport(*pOUTextContent);
                                delete pOUTextContent;
                                pOUTextContent = NULL;
                            }
                            else
                                SetCursorOnTextImport(rtl::OUString());
                            rXMLImport.SetRemoveLastChar(sal_True);
                            uno::Reference < text::XTextCursor > xTextCursor(rXMLImport.GetTextImport()->GetCursor());
                            if (xTextCursor.is())
                            {
                                uno::Reference < text::XText > xText (xTextCursor->getText());
                                uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
                                if (xText.is() && xTextRange.is())
                                    xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_False);
                            }
                        }
                        pContext = rXMLImport.GetTextImport()->CreateTextChildContext(
                            rXMLImport, nPrefix, rLName, xAttrList);
                        bIsFirstTextImport = sal_False;
                    }
                }
            }
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_TABLE:
        {
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            rtl::OUString aLocalName;
            rtl::OUString sValue;
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                sal_uInt16 nAttrPrefix = rXMLImport.GetNamespaceMap().GetKeyByAttrName(
                                                    xAttrList->getNameByIndex( i ), &aLocalName );
                sValue = xAttrList->getValueByIndex( i );

                if (nAttrPrefix == XML_NAMESPACE_TABLE)
                {
                    if (IsXMLToken(aLocalName, XML_IS_SUB_TABLE))
                        bHasSubTable = IsXMLToken(sValue, XML_TRUE);
                }
            }
            DBG_ASSERT(bHasSubTable, "it should be a subtable");
            pContext = new ScXMLTableContext( rXMLImport , nPrefix,
                                                        rLName, xAttrList,
                                                        sal_True, nMergedCols);
            nMergedCols = 1;
            bIsMerged = sal_False;
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_ANNOTATION:
        {
            bIsEmpty = sal_False;
            pContext = new ScXMLAnnotationContext( rXMLImport, nPrefix, rLName,
                                                    xAttrList, this);
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_DETECTIVE:
        {
            bIsEmpty = sal_False;
            if (!pDetectiveObjVec)
                pDetectiveObjVec = new ScMyImpDetectiveObjVec();
            pContext = new ScXMLDetectiveContext(
                rXMLImport, nPrefix, rLName, pDetectiveObjVec );
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE:
        {
            bIsEmpty = sal_False;
            if (!pCellRangeSource)
                pCellRangeSource = new ScMyImpCellRangeSource();
            pContext = new ScXMLCellRangeSourceContext(
                rXMLImport, nPrefix, rLName, xAttrList, pCellRangeSource );
        }
        break;
    }

    if (!pContext && !bTextP)
    {
        com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        uno::Reference<drawing::XShapes> xShapes (rXMLImport.GetTables().GetCurrentXShapes());
        if (xShapes.is())
        {
            if (aCellPos.Column > MAXCOL)
                aCellPos.Column = MAXCOL;
            if (aCellPos.Row > MAXROW)
                aCellPos.Row = MAXROW;
            XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)rXMLImport.GetShapeImport().get();
            pTableShapeImport->SetOnTable(sal_False);
            pTableShapeImport->SetCell(aCellPos);
            pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                rXMLImport, nPrefix, rLName, xAttrList, xShapes);
            if (pContext)
            {
                bIsEmpty = sal_False;
                rXMLImport.ProgressBarIncrement(sal_False);
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

sal_Bool ScXMLTableRowCellContext::IsMerged (const uno::Reference <table::XCellRange>& xCellRange, const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress) const
{
    table::CellAddress aCell; // don't need to set the sheet, because every sheet can contain the same count of cells.
    aCell.Column = nCol;
    aCell.Row = nRow;
    if (CellExists(aCell))
    {
        uno::Reference<sheet::XSheetCellRange> xMergeSheetCellRange (xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow), uno::UNO_QUERY);
        uno::Reference<sheet::XSpreadsheet> xTable (xMergeSheetCellRange->getSpreadsheet());
        uno::Reference<sheet::XSheetCellCursor> xMergeSheetCursor (xTable->createCursorByRange(xMergeSheetCellRange));
        if (xMergeSheetCursor.is())
        {
            xMergeSheetCursor->collapseToMergedArea();
            uno::Reference<sheet::XCellRangeAddressable> xMergeCellAddress (xMergeSheetCursor, uno::UNO_QUERY);
            if (xMergeCellAddress.is())
            {
                aCellAddress = xMergeCellAddress->getRangeAddress();
                if (aCellAddress.StartColumn == nCol && aCellAddress.EndColumn == nCol &&
                    aCellAddress.StartRow == nRow && aCellAddress.EndRow == nRow)
                    return sal_False;
                else
                    return sal_True;
            }
        }
    }
    return sal_False;
}

void ScXMLTableRowCellContext::DoMerge(const com::sun::star::table::CellAddress& aCellPos,
                 const sal_Int32 nCols, const sal_Int32 nRows)
{
    if (CellExists(aCellPos))
    {
        uno::Reference<table::XCellRange> xCellRange(rXMLImport.GetTables().GetCurrentXCellRange());
        if ( xCellRange.is() )
        {
            table::CellRangeAddress aCellAddress;
            if (IsMerged(xCellRange, aCellPos.Column, aCellPos.Row, aCellAddress))
            {
                //unmerge
                uno::Reference <util::XMergeable> xMergeable (xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                        aCellAddress.EndColumn, aCellAddress.EndRow), uno::UNO_QUERY);
                if (xMergeable.is())
                    xMergeable->merge(sal_False);
            }

            //merge
            uno::Reference <util::XMergeable> xMergeable (xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                    aCellAddress.EndColumn + nCols, aCellAddress.EndRow + nRows), uno::UNO_QUERY);
            if (xMergeable.is())
                xMergeable->merge(sal_True);
        }
    }
}

void ScXMLTableRowCellContext::SetContentValidation(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet)
{
    if (pContentValidationName)
    {
        ScMyImportValidation aValidation;
        if (rXMLImport.GetValidation(*pContentValidationName, aValidation))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(xPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT))), uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                if (aValidation.sErrorMessage.getLength())
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRMESS)), uno::makeAny(aValidation.sErrorMessage));
                if (aValidation.sErrorTitle.getLength())
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRTITLE)), uno::makeAny(aValidation.sErrorTitle));
                if (aValidation.sImputMessage.getLength())
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPMESS)), uno::makeAny(aValidation.sImputMessage));
                if (aValidation.sImputTitle.getLength())
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPTITLE)), uno::makeAny(aValidation.sImputTitle));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWERR)), uno::makeAny(aValidation.bShowErrorMessage));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWINP)), uno::makeAny(aValidation.bShowImputMessage));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)), uno::makeAny(aValidation.aValidationType));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_IGNOREBL)), uno::makeAny(aValidation.bIgnoreBlanks));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWLIST)), uno::makeAny(aValidation.nShowList));
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRALSTY)), uno::makeAny(aValidation.aAlertStyle));
                uno::Reference<sheet::XSheetCondition> xCondition(xPropertySet, uno::UNO_QUERY);
                if (xCondition.is())
                {
                    xCondition->setFormula1(aValidation.sFormula1);
                    xCondition->setFormula2(aValidation.sFormula2);
                    xCondition->setOperator(aValidation.aOperator);
                    // #b4974740# source position must be set as string, because it may
                    // refer to a sheet that hasn't been loaded yet.
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SOURCESTR)), uno::makeAny(aValidation.sBaseCellAddress));
                }
            }
            xPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT)), uno::makeAny(xPropertySet));
        }
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCellRange>& xCellRange,
                                                const table::CellAddress& aCellAddress)
{
    if (CellExists(aCellAddress) && pContentValidationName && pContentValidationName->getLength())
    {
        sal_Int32 nBottom = aCellAddress.Row + nRepeatedRows - 1;
        sal_Int32 nRight = aCellAddress.Column + nCellsRepeated - 1;
        if (nBottom > MAXROW)
            nBottom = MAXROW;
        if (nRight > MAXCOL)
            nRight = MAXCOL;
        uno::Reference <beans::XPropertySet> xProperties (xCellRange->getCellRangeByPosition(aCellAddress.Column, aCellAddress.Row,
                                                            nRight, nBottom), uno::UNO_QUERY);
        if (xProperties.is())
            SetContentValidation(xProperties);
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCell>& xCell)
{
    if (pContentValidationName && pContentValidationName->getLength())
    {
        uno::Reference <beans::XPropertySet> xProperties (xCell, uno::UNO_QUERY);
        if (xProperties.is())
            SetContentValidation(xProperties);
    }
}

void ScXMLTableRowCellContext::SetAnnotation(const table::CellAddress& aCellAddress)
{
    /*uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor(xCell, uno::UNO_QUERY);
    if (xSheetAnnotationAnchor.is())
    {
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation (xSheetAnnotationAnchor->getAnnotation());
        uno::Reference<text::XSimpleText> xSimpleText(xSheetAnnotation, uno::UNO_QUERY);
        if (xSheetAnnotation.is() && xSimpleText.is())
        {
            xSimpleText->setString(aMyAnnotation.sText);
            //xSheetAnnotation->setAuthor(aMyAnnotation.sAuthor);
            //xSheetAnnotation->setDate();
            xSheetAnnotation->setIsVisible(aMyAnnotation.bDisplay);
        }
    }*/
    if( pMyAnnotation )
    {
        double fDate;
        rXMLImport.GetMM100UnitConverter().convertDateTime(fDate, pMyAnnotation->sCreateDate);
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pDoc)
        {
            LockSolarMutex();
            SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
            sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
            String sDate;
            Color* pColor = NULL;
            Color** ppColor = &pColor;
            pNumForm->GetOutputString(fDate, nfIndex, sDate, ppColor);
            ScPostIt aNote(String(pMyAnnotation->sText),pDoc);
            aNote.SetDate(sDate);
            aNote.SetAuthor(String(pMyAnnotation->sAuthor));
            aNote.SetShown(pMyAnnotation->bDisplay);
            if (pMyAnnotation->pRect)
                aNote.SetRectangle(*pMyAnnotation->pRect);
            else
                rXMLImport.AddDefaultNote(aCellAddress);
            if (pMyAnnotation->pItemSet)
                aNote.SetItemSet(*(pMyAnnotation->pItemSet));
            else
                aNote.SetItemSet(aNote.DefaultItemSet());
            if ( pMyAnnotation->pOPO )
            {
                ScNoteEditEngine& aEngine = pDoc->GetNoteEngine();
                aEngine.SetText(pMyAnnotation->pOPO->GetTextObject());
                // No ItemSet and Rectangle indicates notes with simple text.
                // i.e. created with calc 1.x sxc file format
                if (pMyAnnotation->pItemSet && pMyAnnotation->pRect)
                {
                    const EditTextObject& rTextObj = pMyAnnotation->pOPO->GetTextObject();
                    sal_uInt16 nCount = aEngine.GetParagraphCount();
                    for( sal_uInt16 nPara = 0; nPara < nCount; ++nPara )
                    {
                        SfxItemSet aSet( rTextObj.GetParaAttribs( nPara));
                        aEngine.SetParaAttribs(nPara, aSet);
                    }
                }
                ::std::auto_ptr< EditTextObject > pEditText( aEngine.CreateTextObject());
                aNote.SetEditTextObject(pEditText.get());    // if pEditText is NULL, then aNote.mpEditObj will be reset().
            }
            if (pMyAnnotation->pRect)
                aNote.SetRectangle(*pMyAnnotation->pRect);
            else
                aNote.SetRectangle(aNote.MimicOldRectangle(ScAddress(static_cast<SCCOL>(aCellAddress.Column), static_cast<SCROW>(aCellAddress.Row), aCellAddress.Sheet)));
            pDoc->SetNote(static_cast<SCCOL>(aCellAddress.Column), static_cast<SCROW>(aCellAddress.Row), aCellAddress.Sheet, aNote);
        }
        if (pMyAnnotation->bDisplay)
        {
            ScDetectiveFunc aDetFunc(pDoc, aCellAddress.Sheet);
            aDetFunc.ShowComment(static_cast<SCCOL>(aCellAddress.Column), static_cast<SCROW>(aCellAddress.Row), sal_False);
            uno::Reference<container::XIndexAccess> xShapesIndex (rXMLImport.GetTables().GetCurrentXShapes(), uno::UNO_QUERY); // make draw page
            if (xShapesIndex.is())
            {
                sal_Int32 nShapes = xShapesIndex->getCount();
                uno::Reference < drawing::XShape > xShape;
                rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
            }
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetDetectiveObj( const table::CellAddress& rPosition )
{
    if( CellExists(rPosition) && pDetectiveObjVec && pDetectiveObjVec->size() )
    {
        LockSolarMutex();
        ScDetectiveFunc aDetFunc( rXMLImport.GetDocument(), rPosition.Sheet );
        uno::Reference<container::XIndexAccess> xShapesIndex (rXMLImport.GetTables().GetCurrentXShapes(), uno::UNO_QUERY); // make draw page
        ScMyImpDetectiveObjVec::iterator aItr(pDetectiveObjVec->begin());
        ScMyImpDetectiveObjVec::iterator aEndItr(pDetectiveObjVec->end());
        while(aItr != aEndItr)
        {
            ScAddress aScAddress;
            ScUnoConversion::FillScAddress( aScAddress, rPosition );
            aDetFunc.InsertObject( aItr->eObjType, aScAddress, aItr->aSourceRange, aItr->bHasError );
            if (xShapesIndex.is())
            {
                sal_Int32 nShapes = xShapesIndex->getCount();
                uno::Reference < drawing::XShape > xShape;
                rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
            }
            ++aItr;
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetCellRangeSource( const table::CellAddress& rPosition )
{
    if( CellExists(rPosition) && pCellRangeSource  && pCellRangeSource->sSourceStr.getLength() &&
        pCellRangeSource->sFilterName.getLength() && pCellRangeSource->sURL.getLength() )
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pDoc)
        {
            LockSolarMutex();
            ScRange aDestRange( static_cast<SCCOL>(rPosition.Column), static_cast<SCROW>(rPosition.Row), rPosition.Sheet,
                static_cast<SCCOL>(rPosition.Column + pCellRangeSource->nColumns - 1),
                static_cast<SCROW>(rPosition.Row + pCellRangeSource->nRows - 1), rPosition.Sheet );
            String sFilterName( pCellRangeSource->sFilterName );
            String sSourceStr( pCellRangeSource->sSourceStr );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(), pCellRangeSource->sURL,
                sFilterName, pCellRangeSource->sFilterOptions, sSourceStr, aDestRange, pCellRangeSource->nRefresh );
            SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, pCellRangeSource->sURL, &sFilterName, &sSourceStr );
        }
    }
}

bool lcl_IsEmptyOrNote( ScDocument* pDoc, const table::CellAddress& rCurrentPos )
{
    ScAddress aScAddress;
    ScUnoConversion::FillScAddress( aScAddress, rCurrentPos );
    ScBaseCell* pCell = pDoc->GetCell( aScAddress );
    return ( !pCell || pCell->GetCellType() == CELLTYPE_NOTE );
}

void ScXMLTableRowCellContext::EndElement()
{
    if (!bHasSubTable)
    {
        if (bHasTextImport && rXMLImport.GetRemoveLastChar())
        {
            if (GetImport().GetTextImport()->GetCursor().is())
            {
                //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
                if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
                {
                    GetImport().GetTextImport()->GetText()->insertString(
                        GetImport().GetTextImport()->GetCursorAsRange(), rtl::OUString(),
                        sal_True );
                }
                rXMLImport.GetTextImport()->ResetCursor();
            }
        }
        table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        if (aCellPos.Column > 0 && nRepeatedRows > 1)
            aCellPos.Row -= (nRepeatedRows - 1);
        uno::Reference<table::XCellRange> xCellRange(rXMLImport.GetTables().GetCurrentXCellRange());
        if (xCellRange.is())
        {
            if (bIsMerged)
                DoMerge(aCellPos, nMergedCols - 1, nMergedRows - 1);
            if ( !pOUFormula )
            {
                rtl::OUString* pOUText = NULL;
                if(nCellType == util::NumberFormat::TEXT)
                {
                    if (xLockable.is())
                        xLockable->removeActionLock();

                    // #i61702# The formatted text content of xBaseCell / xLockable is invalidated,
                    // so it can't be used after calling removeActionLock (getString always uses the document).

                    if (CellExists(aCellPos) && ((nCellsRepeated > 1) || (nRepeatedRows > 1)))
                    {
                        if (!xBaseCell.is())
                        {
                            try
                            {
                                xBaseCell.set(xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row));
                            }
                            catch (lang::IndexOutOfBoundsException&)
                            {
                                DBG_ERRORFILE("It seems here are to many columns or rows");
                            }
                        }
                        uno::Reference <text::XText> xTempText (xBaseCell, uno::UNO_QUERY);
                        if (xTempText.is())
                        {
                            rtl::OUString sBla(xTempText->getString());
                            pOUText = new rtl::OUString(sBla);
                        }
                    }
                    if (!pOUTextContent && !pOUText && !pOUTextValue)
                        bIsEmpty = sal_True;
                    else if ( (pOUTextContent && !pOUTextContent->getLength()) || !pOUTextContent )
                        if ( (pOUText && !pOUText->getLength()) || !pOUText )
                            if ( (pOUTextValue && !pOUTextValue->getLength()) || !pOUTextValue )
                                bIsEmpty = sal_True;
                }
                sal_Bool bWasEmpty = bIsEmpty;
//              uno::Reference <table::XCell> xCell;
                table::CellAddress aCurrentPos( aCellPos );
                if ((pContentValidationName && pContentValidationName->getLength()) ||
                    pMyAnnotation || pDetectiveObjVec || pCellRangeSource)
                    bIsEmpty = sal_False;
                for (sal_Int32 i = 0; i < nCellsRepeated; ++i)
                {
                    aCurrentPos.Column = aCellPos.Column + i;
                    if (i > 0)
                        rXMLImport.GetTables().AddColumn(sal_False);
                    if (!bIsEmpty)
                    {
                        for (sal_Int32 j = 0; j < nRepeatedRows; ++j)
                        {
                            aCurrentPos.Row = aCellPos.Row + j;
                            if ((aCurrentPos.Column == 0) && (j > 0))
                            {
                                rXMLImport.GetTables().AddRow();
                                rXMLImport.GetTables().AddColumn(sal_False);
                            }
                            if (CellExists(aCurrentPos))
                            {
                                // test - bypass the API
                                // if (xBaseCell.is() && (aCurrentPos == aCellPos))
                                //     xCell.set(xBaseCell);
                                // else
                                // {
                                //     try
                                //     {
                                //      xCell.set(xCellRange->getCellByPosition(aCurrentPos.Column, aCurrentPos.Row));
                                //     }
                                //     catch (lang::IndexOutOfBoundsException&)
                                //     {
                                //         DBG_ERRORFILE("It seems here are to many columns or rows");
                                //     }
                                // }

                                // test - bypass the API
                                // if ((!(bIsCovered) || (xCell->getType() == table::CellContentType_EMPTY)))
                                if ((!(bIsCovered) || lcl_IsEmptyOrNote( rXMLImport.GetDocument(), aCurrentPos )))
                                {
                                    switch (nCellType)
                                    {
                                    case util::NumberFormat::TEXT:
                                        {
                                            sal_Bool bDoIncrement = sal_True;
                                            if (rXMLImport.GetTables().IsPartOfMatrix(aCurrentPos.Column, aCurrentPos.Row))
                                            {
                                                LockSolarMutex();
                                                // test - bypass the API
                                                // ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                                // if (pCellObj)
                                                // {
                                                //     if(pOUTextValue && pOUTextValue->getLength())
                                                //         pCellObj->SetFormulaResultString(*pOUTextValue);
                                                //     else if (pOUTextContent && pOUTextContent->getLength())
                                                //         pCellObj->SetFormulaResultString(*pOUTextContent);
                                                //     else if ( i > 0 && pOUText && pOUText->getLength() )
                                                //     {
                                                //         pCellObj->SetFormulaResultString(*pOUText);
                                                //     }
                                                //     else
                                                //         bDoIncrement = sal_False;
                                                // }
                                                // else
                                                //     bDoIncrement = sal_False;
                                                ScAddress aScAddress;
                                                ScUnoConversion::FillScAddress( aScAddress, aCurrentPos );
                                                ScBaseCell* pCell = rXMLImport.GetDocument()->GetCell( aScAddress );
                                                if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                                                {
                                                    ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
                                                    if (pOUTextValue && pOUTextValue->getLength())
                                                        pFCell->SetString( *pOUTextValue );
                                                    else if (pOUTextContent && pOUTextContent->getLength())
                                                        pFCell->SetString( *pOUTextContent );
                                                    else if ( i > 0 && pOUText && pOUText->getLength() )
                                                        pFCell->SetString( *pOUText );
                                                    else
                                                        bDoIncrement = sal_False;
                                                }
                                                else
                                                    bDoIncrement = sal_False;
                                            }
                                            else
                                            {
                                                // test - bypass the API
                                                // uno::Reference <text::XText> xText (xCell, uno::UNO_QUERY);
                                                // if (xText.is())
                                                // {
                                                //     if(pOUTextValue && pOUTextValue->getLength())
                                                //         xText->setString(*pOUTextValue);
                                                //     else if (pOUTextContent && pOUTextContent->getLength())
                                                //         xText->setString(*pOUTextContent);
                                                //     else if ( i > 0 && pOUText && pOUText->getLength() )
                                                //     {
                                                //         xText->setString(*pOUText);
                                                //     }
                                                //     else
                                                //         bDoIncrement = sal_False;
                                                // }
                                                LockSolarMutex();
                                                ScBaseCell* pNewCell = NULL;
                                                ScDocument* pDoc = rXMLImport.GetDocument();
                                                if (pOUTextValue && pOUTextValue->getLength())
                                                    pNewCell = ScBaseCell::CreateTextCell( *pOUTextValue, pDoc );
                                                else if (pOUTextContent && pOUTextContent->getLength())
                                                    pNewCell = ScBaseCell::CreateTextCell( *pOUTextContent, pDoc );
                                                else if ( i > 0 && pOUText && pOUText->getLength() )
                                                    pNewCell = ScBaseCell::CreateTextCell( *pOUText, pDoc );
                                                if ( pNewCell )
                                                {
                                                    ScAddress aScAddress;
                                                    ScUnoConversion::FillScAddress( aScAddress, aCurrentPos );
                                                    pDoc->PutCell( aScAddress, pNewCell );
                                                }
                                                else
                                                    bDoIncrement = sal_False;
                                            }
                                            // #i56027# This is about setting simple text, not edit cells,
                                            // so ProgressBarIncrement must be called with bEditCell = FALSE.
                                            // Formatted text that is put into the cell by the child context
                                            // is handled below (bIsEmpty is TRUE then).
                                            if (bDoIncrement || bHasTextImport)
                                                rXMLImport.ProgressBarIncrement(sal_False);
                                        }
                                        break;
                                    case util::NumberFormat::NUMBER:
                                    case util::NumberFormat::PERCENT:
                                    case util::NumberFormat::CURRENCY:
                                    case util::NumberFormat::TIME:
                                    case util::NumberFormat::DATETIME:
                                    case util::NumberFormat::LOGICAL:
                                        {
                                            if (rXMLImport.GetTables().IsPartOfMatrix(aCurrentPos.Column, aCurrentPos.Row))
                                            {
                                                LockSolarMutex();
                                                // test - bypass the API
                                                // ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                                // if (pCellObj)
                                                //     pCellObj->SetFormulaResultDouble(fValue);
                                                ScAddress aScAddress;
                                                ScUnoConversion::FillScAddress( aScAddress, aCurrentPos );
                                                ScBaseCell* pCell = rXMLImport.GetDocument()->GetCell( aScAddress );
                                                if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                                                    static_cast<ScFormulaCell*>(pCell)->SetDouble( fValue );
                                            }
                                            else
                                            {
                                                // test - bypass the API
                                                // xCell->setValue(fValue);
                                                LockSolarMutex();

                                                // #i62435# Initialize the value cell's script type
                                                // if the default style's number format is latin-only.
                                                // If the cell uses a different format, the script type
                                                // will be reset when the style is applied.

                                                ScBaseCell* pNewCell = new ScValueCell(fValue);
                                                if ( rXMLImport.IsLatinDefaultStyle() )
                                                    pNewCell->SetScriptType( SCRIPTTYPE_LATIN );
                                                rXMLImport.GetDocument()->PutCell(
                                                    sal::static_int_cast<SCCOL>( aCurrentPos.Column ),
                                                    sal::static_int_cast<SCROW>( aCurrentPos.Row ),
                                                    sal::static_int_cast<SCTAB>( aCurrentPos.Sheet ),
                                                    pNewCell );
                                            }
                                            rXMLImport.ProgressBarIncrement(sal_False);
                                        }
                                        break;
                                    default:
                                        {
                                            DBG_ERROR("no cell type given");
                                        }
                                        break;
                                    }
                                }

                                SetAnnotation(aCurrentPos);
                                SetDetectiveObj( aCurrentPos );
                                SetCellRangeSource( aCurrentPos );
                            }
                            else
                            {
                                if (!bWasEmpty || (pMyAnnotation))
                                {
                                    if (aCurrentPos.Row > MAXROW)
                                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                                    else
                                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                                }
                            }
                        }
                    }
                    else
                    {
                        // #i56027# If the child context put formatted text into the cell,
                        // bIsEmpty is TRUE and ProgressBarIncrement has to be called
                        // with bEditCell = TRUE.
                        if (bHasTextImport)
                            rXMLImport.ProgressBarIncrement(sal_True);
                        if ((i == 0) && (aCellPos.Column == 0))
                            for (sal_Int32 j = 1; j < nRepeatedRows; ++j)
                            {
                                    rXMLImport.GetTables().AddRow();
                                    rXMLImport.GetTables().AddColumn(sal_False);
                            }
                    }
                }
                if (nCellsRepeated > 1 || nRepeatedRows > 1)
                {
                    SetCellProperties(xCellRange, aCellPos); // set now only the validation for the complete range with the given cell as start cell
                    //SetType(xCellRange, aCellPos);
                    SCCOL nStartCol(aCellPos.Column < MAXCOL ? static_cast<SCCOL>(aCellPos.Column) : MAXCOL);
                    SCROW nStartRow(aCellPos.Row < MAXROW ? static_cast<SCROW>(aCellPos.Row) : MAXROW);
                    SCCOL nEndCol(aCellPos.Column + nCellsRepeated - 1 < MAXCOL ? static_cast<SCCOL>(aCellPos.Column + nCellsRepeated - 1) : MAXCOL);
                    SCROW nEndRow(aCellPos.Row + nRepeatedRows - 1 < MAXROW ? static_cast<SCROW>(aCellPos.Row + nRepeatedRows - 1) : MAXROW);
                    ScRange aScRange( nStartCol, nStartRow, aCellPos.Sheet,
                        nEndCol, nEndRow, aCellPos.Sheet );
                    rXMLImport.GetStylesImportHelper()->AddRange(aScRange);
                }
                else if (CellExists(aCellPos))
                {
                    rXMLImport.GetStylesImportHelper()->AddCell(aCellPos);

                    // test - bypass the API
                    // SetCellProperties(xCell); // set now only the validation
                    SetCellProperties(xCellRange, aCellPos);

                    //SetType(xTempCell);
                }
                if (pOUText)
                    delete pOUText;
            }
            else
            {
                if (CellExists(aCellPos))
                {
                    uno::Reference <table::XCell> xCell;
                    try
                    {
                        xCell.set(xCellRange->getCellByPosition(aCellPos.Column , aCellPos.Row));
                    }
                    catch (lang::IndexOutOfBoundsException&)
                    {
                        DBG_ERRORFILE("It seems here are to many columns or rows");
                    }
                    if (xCell.is() && pOUFormula)
                    {
                        SetCellProperties(xCell); // set now only the validation
                        DBG_ASSERT(((nCellsRepeated == 1) && (nRepeatedRows == 1)), "repeated cells with formula not possible now");
                        rXMLImport.GetStylesImportHelper()->AddCell(aCellPos);
                        ScXMLConverter::ParseFormula(*pOUFormula);
                        if (!bIsMatrix)
                        {
                            xCell->setFormula(*pOUFormula);
                            if (bFormulaTextResult && pOUTextValue && pOUTextValue->getLength())
                            {
                                LockSolarMutex();
                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                if (pCellObj)
                                    pCellObj->SetFormulaResultString(*pOUTextValue);
                            }
                            else if (fValue != 0.0)
                            {
                                LockSolarMutex();
                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                if (pCellObj)
                                    pCellObj->SetFormulaResultDouble(fValue);
                            }
                        }
                        else
                        {
                            if (nMatrixCols > 0 && nMatrixRows > 0)
                            {
                                rXMLImport.GetTables().AddMatrixRange(aCellPos.Column, aCellPos.Row,
                                                aCellPos.Column + nMatrixCols - 1, aCellPos.Row + nMatrixRows - 1, *pOUFormula);
                            }
                        }
                        SetAnnotation( aCellPos );
                        SetDetectiveObj( aCellPos );
                        SetCellRangeSource( aCellPos );
                        rXMLImport.ProgressBarIncrement(sal_False);
                    }
                }
                else
                {
                    if (aCellPos.Row > MAXROW)
                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                    else
                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                }

            }
        }
        UnlockSolarMutex();
    }
    bIsMerged = sal_False;
    bHasSubTable = sal_False;
    nMergedCols = 1;
    nMergedRows = 1;
    nCellsRepeated = 1;
}
