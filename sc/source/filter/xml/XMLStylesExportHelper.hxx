/*************************************************************************
 *
 *  $RCSfile: XMLStylesExportHelper.hxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:46:56 $
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

#ifndef _SC_XMLSTYLESEXPORTHELPER_HXX
#define _SC_XMLSTYLESEXPORTHELPER_HXX

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_CONDITIONOPERATOR_HPP_
#include <com/sun/star/sheet/ConditionOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_VALIDATIONALERTSTYLE_HPP_
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_VALIDATIONTYPE_HPP_
#include <com/sun/star/sheet/ValidationType.hpp>
#endif

class ScDocument;
class ScXMLExport;

struct ScMyValidation
{
    rtl::OUString               sName;
    rtl::OUString               sErrorMessage;
    rtl::OUString               sErrorTitle;
    rtl::OUString               sImputMessage;
    rtl::OUString               sImputTitle;
    rtl::OUString               sFormula1;
    rtl::OUString               sFormula2;
    com::sun::star::table::CellAddress          aBaseCell;
    com::sun::star::sheet::ValidationAlertStyle aAlertStyle;
    com::sun::star::sheet::ValidationType       aValidationType;
    com::sun::star::sheet::ConditionOperator    aOperator;
    sal_Int16                   nShowList;
    sal_Bool                    bShowErrorMessage;
    sal_Bool                    bShowImputMessage;
    sal_Bool                    bIgnoreBlanks;

                                ScMyValidation();
                                ~ScMyValidation();

    sal_Bool                    IsEqual(const ScMyValidation& aVal) const;
};

typedef std::vector<ScMyValidation>         ScMyValidationVec;

class ScMyValidationsContainer
{
private:
    ScMyValidationVec           aValidationVec;
    const rtl::OUString         sEmptyString;
    const rtl::OUString         sERRALSTY;
    const rtl::OUString         sIGNOREBL;
    const rtl::OUString         sSHOWLIST;
    const rtl::OUString         sTYPE;
    const rtl::OUString         sSHOWINP;
    const rtl::OUString         sSHOWERR;
    const rtl::OUString         sINPTITLE;
    const rtl::OUString         sINPMESS;
    const rtl::OUString         sERRTITLE;
    const rtl::OUString         sERRMESS;
    const rtl::OUString         sOnError;
    const rtl::OUString         sEventType;
    const rtl::OUString         sStarBasic;
    const rtl::OUString         sLibrary;
    const rtl::OUString         sMacroName;

public:
                                ScMyValidationsContainer();
                                ~ScMyValidationsContainer();
    sal_Bool                    AddValidation(const com::sun::star::uno::Any& aAny,
                                    sal_Int32& nValidationIndex);
    rtl::OUString               GetCondition(ScXMLExport& rExport, const ScMyValidation& aValidation);
    rtl::OUString               GetBaseCellAddress(ScDocument* pDoc, const com::sun::star::table::CellAddress& aCell);
    void                        WriteMessage(ScXMLExport& rExport,
                                    const rtl::OUString& sTitle, const rtl::OUString& sMessage,
                                    const sal_Bool bShowMessage, const sal_Bool bIsHelpMessage);
    void                        WriteValidations(ScXMLExport& rExport);
    const rtl::OUString&        GetValidationName(const sal_Int32 nIndex);
};

//==============================================================================

struct ScMyDefaultStyle
{
    sal_Int32   nIndex;
    sal_Int32   nRepeat;
    sal_Bool    bIsAutoStyle;

    ScMyDefaultStyle() : nIndex(-1), bIsAutoStyle(sal_True),
        nRepeat(1) {}
};

typedef std::vector<ScMyDefaultStyle> ScMyDefaultStyleList;

class ScFormatRangeStyles;

class ScMyDefaultStyles
{
    ScMyDefaultStyleList* pRowDefaults;
    ScMyDefaultStyleList* pColDefaults;

    sal_Int32 GetStyleNameIndex(const ScFormatRangeStyles* pCellStyles,
        const sal_Int32 nTable, const sal_Int32 nPos,
        const sal_Int32 i, const sal_Bool bRow, sal_Bool& bIsAutoStyle);
    void FillDefaultStyles(const sal_Int32 nTable,
        const sal_Int32 nLastRow, const sal_Int32 nLastCol,
        const ScFormatRangeStyles* pCellStyles, ScDocument* pDoc,
        const sal_Bool bRow);
public:
    ScMyDefaultStyles() : pRowDefaults(NULL), pColDefaults(NULL) {}
    ~ScMyDefaultStyles();

    void FillDefaultStyles(const sal_Int32 nTable,
        const sal_Int32 nLastRow, const sal_Int32 nLastCol,
        const ScFormatRangeStyles* pCellStyles, ScDocument* pDoc);

    const ScMyDefaultStyleList* GetRowDefaults() { return pRowDefaults; }
    const ScMyDefaultStyleList* GetColDefaults() { return pColDefaults; }
};

struct ScMyRowFormatRange
{
    sal_Int32   nStartColumn;
    sal_Int32   nRepeatColumns;
    sal_Int32   nRepeatRows;
    sal_Int32   nIndex;
    sal_Int32   nValidationIndex;
    sal_Bool    bIsAutoStyle : 1;

    ScMyRowFormatRange();
    sal_Bool operator<(const ScMyRowFormatRange& rRange) const;
};

typedef std::list<ScMyRowFormatRange> ScMyRowFormatRangesList;

class ScRowFormatRanges
{
    ScMyRowFormatRangesList     aRowFormatRanges;
    const ScMyDefaultStyleList* pRowDefaults;
    const ScMyDefaultStyleList* pColDefaults;
    sal_uInt32                  nSize;

    void AddRange(const sal_Int32 nPrevStartCol, const sal_Int32 nRepeat, const sal_Int32 nPrevIndex,
        const sal_Bool bPrevAutoStyle, const ScMyRowFormatRange& rFormatRange);

public:
    ScRowFormatRanges();
    ScRowFormatRanges(const ScRowFormatRanges* pRanges);
    ~ScRowFormatRanges();

    void SetRowDefaults(const ScMyDefaultStyleList* pDefaults) { pRowDefaults = pDefaults; }
    void SetColDefaults(const ScMyDefaultStyleList* pDefaults) { pColDefaults = pDefaults; }
    void Clear();
    void AddRange(ScMyRowFormatRange& rFormatRange, const sal_Int32 nStartRow);
    sal_Bool GetNext(ScMyRowFormatRange& rFormatRange);
    sal_Int32 GetMaxRows();
    sal_Int32 GetSize();
    void Sort();
};

typedef std::vector<rtl::OUString*>     ScMyOUStringVec;

struct ScMyFormatRange
{
    com::sun::star::table::CellRangeAddress aRangeAddress;
    sal_Int32                               nStyleNameIndex;
    sal_Int32                               nValidationIndex;
    sal_Int32                               nNumberFormat;
    sal_Bool                                bIsAutoStyle : 1;

    ScMyFormatRange();
    sal_Bool operator< (const ScMyFormatRange& rRange) const;
};

typedef std::list<ScMyFormatRange>          ScMyFormatRangeAddresses;
typedef std::vector<ScMyFormatRangeAddresses*>  ScMyFormatRangeListVec;

class ScFormatRangeStyles
{
    ScMyFormatRangeListVec      aTables;
    ScMyOUStringVec             aStyleNames;
    ScMyOUStringVec             aAutoStyleNames;
    const ScMyDefaultStyleList* pRowDefaults;
    const ScMyDefaultStyleList* pColDefaults;

public:
    ScFormatRangeStyles();
    ~ScFormatRangeStyles();

    void SetRowDefaults(const ScMyDefaultStyleList* pDefaults) { pRowDefaults = pDefaults; }
    void SetColDefaults(const ScMyDefaultStyleList* pDefaults) { pColDefaults = pDefaults; }
    void AddNewTable(const sal_Int32 nTable);
    sal_Bool AddStyleName(rtl::OUString* pString, sal_Int32& rIndex, const sal_Bool bIsAutoStyle = sal_True);
    sal_Int32 GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix, sal_Bool& bIsAutoStyle);
    // does not delete ranges
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nColumn, const sal_Int32 nRow,
        sal_Bool& bIsAutoStyle) const;
    // deletes not necessary ranges if wanted
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nColumn, const sal_Int32 nRow,
        sal_Bool& bIsAutoStyle, sal_Int32& nValidationIndex, sal_Int32& nNumberFormat, const sal_Bool bRemoveRange = sal_True );
    void GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int32 nTable, ScRowFormatRanges* pFormatRanges);
    void AddRangeStyleName(const com::sun::star::table::CellRangeAddress aCellRangeAddress, const sal_Int32 nStringIndex,
                    const sal_Bool bIsAutoStyle, const sal_Int32 nValidationIndex, const sal_Int32 nNumberFormat);
    rtl::OUString* GetStyleNameByIndex(const sal_Int32 nIndex, const sal_Bool bIsAutoStyle);
    void Sort();
};

class ScColumnRowStylesBase
{
    ScMyOUStringVec             aStyleNames;

public:
    ScColumnRowStylesBase();
    ~ScColumnRowStylesBase();

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields) = 0;
    sal_Int32 AddStyleName(rtl::OUString* pString);
    sal_Int32 GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix);
    virtual rtl::OUString* GetStyleName(const sal_Int32 nTable, const sal_Int32 nField) = 0;
    rtl::OUString* GetStyleNameByIndex(const sal_Int32 nIndex);
};

struct ScColumnStyle
{
    sal_Int32   nIndex;
    sal_Bool    bIsVisible : 1;

    ScColumnStyle() : nIndex(-1), bIsVisible(sal_True) {}
};


typedef std::vector<ScColumnStyle>  ScMyColumnStyleVec;
typedef std::vector<ScMyColumnStyleVec> ScMyColumnVectorVec;

class ScColumnStyles : public ScColumnRowStylesBase
{
    ScMyColumnVectorVec             aTables;

public:
    ScColumnStyles();
    ~ScColumnStyles();

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields);
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField,
        sal_Bool& bIsVisible);
    void AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex, const sal_Bool bIsVisible);
    virtual rtl::OUString* GetStyleName(const sal_Int32 nTable, const sal_Int32 nField);
};

typedef std::vector<sal_Int32>  ScMysalInt32Vec;
typedef std::vector<ScMysalInt32Vec>    ScMyRowVectorVec;

class ScRowStyles : public ScColumnRowStylesBase
{
    ScMyRowVectorVec                aTables;

public:
    ScRowStyles();
    ~ScRowStyles();

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields);
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField);
    void AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex);
    virtual rtl::OUString* GetStyleName(const sal_Int32 nTable, const sal_Int32 nField);
};

#endif

