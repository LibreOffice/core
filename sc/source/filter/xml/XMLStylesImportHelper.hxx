/*************************************************************************
 *
 *  $RCSfile: XMLStylesImportHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-06-21 07:35:48 $
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

#ifndef _SC_XMLSTYLESIMPORTHELPER_HXX
#define _SC_XMLSTYLESIMPORTHELPER_HXX

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif

#ifndef __SGI_STL_SET
#include <set>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

class ScXMLImport;

struct ScMyStyleNumberFormat
{
    rtl::OUString       sStyleName;
    sal_Int32           nNumberFormat;

    ScMyStyleNumberFormat() : nNumberFormat(-1) {}
    ScMyStyleNumberFormat(const rtl::OUString& rStyleName) :
        sStyleName(rStyleName), nNumberFormat(-1) {}
    ScMyStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nFormat) :
        sStyleName(rStyleName), nNumberFormat(nFormat) {}
};

struct LessStyleNumberFormat
{
    sal_Bool operator() (const ScMyStyleNumberFormat& rValue1, const ScMyStyleNumberFormat& rValue2) const
    {
        return rValue1.sStyleName < rValue2.sStyleName;
    }
};

typedef std::set< ScMyStyleNumberFormat, LessStyleNumberFormat >    ScMyStyleNumberFormatSet;

class ScMyStyleNumberFormats
{
    ScMyStyleNumberFormatSet    aSet;

public:
    void AddStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nNumberFormat);
    sal_Int32 GetStyleNumberFormat(const rtl::OUString& rStyleName);
};

struct ScMyCurrencyStyle
{
    rtl::OUString       sCurrency;
    ScRangeListRef      xRanges;

    ScMyCurrencyStyle() : xRanges(new ScRangeList()) {}
    ~ScMyCurrencyStyle() {}
};

struct LessCurrencyStyle
{
    sal_Bool operator() (const ScMyCurrencyStyle& rValue1, const ScMyCurrencyStyle& rValue2) const
    {
        return rValue1.sCurrency < rValue2.sCurrency;
    }
};

typedef std::set<ScMyCurrencyStyle, LessCurrencyStyle>  ScMyCurrencyStylesSet;

class ScMyStyleRanges : public SvRefBase
{
    ScRangeList*            pTextList;
    ScRangeList*            pNumberList;
    ScRangeList*            pTimeList;
    ScRangeList*            pDateTimeList;
    ScRangeList*            pPercentList;
    ScRangeList*            pLogicalList;
    ScMyCurrencyStylesSet*  pCurrencyList;

    void AddRange(const ScRange& rRange, ScRangeList* pList,
        const rtl::OUString& rStyleName, const sal_Int16 nType,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void AddCurrencyRange(const ScRange& rRange, ScRangeListRef xList,
        const rtl::OUString& rStyleName, const rtl::OUString& rCurrency,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void InsertColRow(const ScRange& rRange, const sal_Int16 nDx, const sal_Int16 nDy,
        const sal_Int16 nDz, ScDocument* pDoc);
    void SetStylesToRanges(ScRangeList* pList,
        const rtl::OUString& rStyleName, const sal_Int16 nCellType,
        const rtl::OUString& rCurrency, ScXMLImport& rImport);
    void SetStylesToRanges(ScRangeListRef xList,
        const rtl::OUString& rStyleName, const sal_Int16 nCellType,
        const rtl::OUString& rCurrency, ScXMLImport& rImport);
public:
    ScMyStyleRanges();
    ~ScMyStyleRanges();
    void AddRange(const ScRange& rRange,
        const rtl::OUString& rStyleName, const sal_Int16 nType,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void AddCurrencyRange(const ScRange& rRange,
        const rtl::OUString& rStyleName, const rtl::OUString& rCurrency,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc);
    void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc);
    void SetStylesToRanges(const rtl::OUString& rStyleName, ScXMLImport& rImport);
};
SV_DECL_IMPL_REF( ScMyStyleRanges );

struct ScMyStyle
{
    rtl::OUString       sStyleName;
    ScMyStyleRangesRef  xRanges;

    ScMyStyle() : xRanges(new ScMyStyleRanges()) {}
    ~ScMyStyle() {}
};

struct LessStyle
{
    sal_Bool operator() (const ScMyStyle& rValue1, const ScMyStyle& rValue2) const
    {
        return rValue1.sStyleName < rValue2.sStyleName;
    }
};

typedef std::set<ScMyStyle, LessStyle>  ScMyStylesSet;
typedef std::vector<ScMyStylesSet::iterator> ScMyStyles;

class ScMyStylesImportHelper
{
    ScMyStylesSet       aCellStyles;
    ScMyStyles          aColDefaultStyles;
    ScMyStylesSet::iterator aRowDefaultStyle;
    ScXMLImport&        rImport;
    rtl::OUString       sStyleName;
    rtl::OUString       sPrevStyleName;
    rtl::OUString       sCurrency;
    rtl::OUString       sPrevCurrency;
    rtl::OUString       sEmpty;
    ScRange             aPrevRange;
    sal_uInt32          nMaxRanges;
    sal_Int16           nCellType;
    sal_Int16           nPrevCellType;
    sal_Bool            bPrevRangeAdded;

    void ResetAttributes();
    ScMyStylesSet::iterator GetIterator(const rtl::OUString& sStyleName);
    void AddDefaultRange(const ScRange& rRange);
    void AddSingleRange(const ScRange& rRange);
    void AddRange();
public:
    ScMyStylesImportHelper(ScXMLImport& rImport);
    ~ScMyStylesImportHelper();
    void AddColumnStyle(const rtl::OUString& rStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat);
    void SetRowStyle(const rtl::OUString& rStyleName);
    void SetAttributes(const rtl::OUString& rStyleName,
        const rtl::OUString& sCurrency, const sal_Int16 nCellType);
    void AddRange(const ScRange& rRange);
    void AddRange(const com::sun::star::table::CellRangeAddress& rRange);
    void AddCell(const ScAddress& rAddress);
    void AddCell(const com::sun::star::table::CellAddress& rAddress);
    void InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc); // a row is inserted before nRow
    void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc); // a col is inserted before nCol
    void EndTable();
    void SetStylesToRanges();
};

#endif

