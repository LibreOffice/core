/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlcelli.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:01:12 $
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
#ifndef SC_XMLCELLI_HXX
#define SC_XMLCELLI_HXX

#ifndef _SC_XMLDETECTIVECONTEXT_HXX
#include "XMLDetectiveContext.hxx"
#endif
#ifndef _SC_XMLCELLRANGESOURCECONTEXT_HXX
#include "XMLCellRangeSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELL_HPP_
#include <com/sun/star/table/XCell.hpp>
#endif
#include <tools/time.hxx>
#include <com/sun/star/util/DateTime.hpp>
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif

#include "grammar.hxx"

class ScXMLImport;
class OutlinerParaObject;

struct ScMyImportAnnotation
{
    rtl::OUString sAuthor;
    rtl::OUString sCreateDate;
    rtl::OUString sText;
    sal_Bool bDisplay;
    Rectangle* pRect;
    SfxItemSet* pItemSet;
    OutlinerParaObject* pOPO;

    ScMyImportAnnotation() : pRect(NULL), pItemSet(NULL), pOPO(NULL) {}
    ~ScMyImportAnnotation();
};

class ScXMLTableRowCellContext : public SvXMLImportContext
{
    com::sun::star::uno::Reference<com::sun::star::table::XCell> xBaseCell;
    com::sun::star::uno::Reference<com::sun::star::document::XActionLockable> xLockable;
    rtl::OUString* pOUTextValue;
    rtl::OUString* pOUTextContent;
    rtl::OUString* pOUFormula;
    rtl::OUString* pContentValidationName;
    ScMyImportAnnotation*   pMyAnnotation;
    ScMyImpDetectiveObjVec* pDetectiveObjVec;
    ScMyImpCellRangeSource* pCellRangeSource;
    double      fValue;
    sal_Int32   nMergedRows, nMergedCols;
    sal_Int32   nMatrixRows, nMatrixCols;
    sal_Int32   nRepeatedRows;
    sal_Int32   nCellsRepeated;
    ScXMLImport& rXMLImport;
    ScGrammar::Grammar  eGrammar;
    sal_Int16   nCellType;
    sal_Bool    bIsMerged;
    sal_Bool    bIsMatrix;
    sal_Bool    bHasSubTable;
    sal_Bool    bIsCovered;
    sal_Bool    bIsEmpty;
    sal_Bool    bHasTextImport;
    sal_Bool    bIsFirstTextImport;
    sal_Bool    bSolarMutexLocked;
    sal_Bool    bFormulaTextResult;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    sal_Int16 GetCellType(const rtl::OUString& sOUValue) const;

    sal_Bool IsMerged (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
                const sal_Int32 nCol, const sal_Int32 nRow,
                com::sun::star::table::CellRangeAddress& aCellAddress) const;
    void DoMerge(const com::sun::star::table::CellAddress& aCellPos,
                 const sal_Int32 nCols, const sal_Int32 nRows);

    void SetContentValidation(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet);
    void SetCellProperties(const com::sun::star::uno::Reference<com::sun::star::table::XCellRange>& xCellRange,
                                                const com::sun::star::table::CellAddress& aCellAddress);
    void SetCellProperties(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell);

    void LockSolarMutex();
    void UnlockSolarMutex();

    sal_Bool CellExists(const com::sun::star::table::CellAddress& aCellPos) const
    {
        return (aCellPos.Column <= MAXCOL && aCellPos.Row <= MAXROW);
    }

public:

    ScXMLTableRowCellContext( ScXMLImport& rImport, USHORT nPrfx,
                       const ::rtl::OUString& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        const sal_Bool bIsCovered, const sal_Int32 nRepeatedRows );

    virtual ~ScXMLTableRowCellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void SetString(const rtl::OUString& rOUTempText) {
        if (pOUTextContent)
            delete pOUTextContent;
        pOUTextContent = new ::rtl::OUString(rOUTempText); }
    void SetCursorOnTextImport(const rtl::OUString& rOUTempText);

    void SetAnnotation(const ::com::sun::star::table::CellAddress& rPosition );
    void SetDetectiveObj( const ::com::sun::star::table::CellAddress& rPosition );
    void SetCellRangeSource( const ::com::sun::star::table::CellAddress& rPosition );

    virtual void EndElement();

    void AddAnnotation(ScMyImportAnnotation* pValue) { pMyAnnotation = pValue; }
};

#endif
