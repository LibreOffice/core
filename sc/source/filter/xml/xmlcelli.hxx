/*************************************************************************
 *
 *  $RCSfile: xmlcelli.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-28 08:19:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

class ScXMLImport;

struct ScMyAnnotation
{
    rtl::OUString sAuthor;
    rtl::OUString sCreateDate;
    rtl::OUString sText;
    sal_Bool bDisplay : 1;
};

class ScXMLTableRowCellContext : public SvXMLImportContext
{
    rtl::OUString sOUText;
    rtl::OUString sOUDateValue;
    rtl::OUString sOUTimeValue;
    rtl::OUString sOUBooleanValue;
    rtl::OUString sOUTextValue;
    rtl::OUString sOUFormula;
    rtl::OUString sCurrencySymbol;
    rtl::OUString sStyleName;
    rtl::OUString sContentValidationName;
    double      fValue;
    sal_Int32   nMergedRows, nMergedCols;
    sal_Int32   nMatrixRows, nMatrixCols;
    sal_Int32   nRepeatedRows;
    sal_Int32   nCellsRepeated;
    sal_Int16   nCellType;
    sal_Bool    bIsMerged : 1;
    sal_Bool    bIsMatrix : 1;
    sal_Bool    bIsFormula : 1;
    sal_Bool    bHasSubTable : 1;
    sal_Bool    bIsCovered : 1;
    sal_Bool    bHasAnnotation : 1;
    sal_Bool    bIsEmpty : 1;
    sal_Bool    bHasTextImport : 1;
    sal_Bool    bIsFirstTextImport : 1;
    ScMyAnnotation          aMyAnnotation;
    ScMyImpDetectiveObjVec  aDetectiveObjVec;
    ScMyImpCellRangeSource  aCellRangeSource;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard);

    void SetType(const com::sun::star::uno::Reference<com::sun::star::table::XCellRange>& xCellRange,
                                                const com::sun::star::table::CellAddress& aCellAddress);
    void SetType(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell);
    sal_Int32 SetCurrencySymbol(sal_Int32 nKey);

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

public:

    ScXMLTableRowCellContext( ScXMLImport& rImport, USHORT nPrfx,
                       const NAMESPACE_RTL(OUString)& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        const sal_Bool bIsCovered, const sal_Int32 nRepeatedRows );

    virtual ~ScXMLTableRowCellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void SetString(const rtl::OUString& sOUTempText) { sOUText = sOUTempText; }

    void SetAnnotation(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell);
    void SetDetectiveObj( const ::com::sun::star::table::CellAddress& rPosition );
    void SetCellRangeSource( const ::com::sun::star::table::CellAddress& rPosition );

    virtual void EndElement();

    void AddAnnotation(const ScMyAnnotation& aValue) { aMyAnnotation = aValue; bHasAnnotation = sal_True; }
};

#endif
