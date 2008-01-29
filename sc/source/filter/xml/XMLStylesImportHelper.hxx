/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLStylesImportHelper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:35:45 $
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

#include <set>
#include <vector>

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
    ScRangeList*            pUndefinedList;
    ScMyCurrencyStylesSet*  pCurrencyList;

    void AddRange(const ScRange& rRange, ScRangeList* pList,
        const rtl::OUString* pStyleName, const sal_Int16 nType,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void AddCurrencyRange(const ScRange& rRange, ScRangeListRef xList,
        const rtl::OUString* pStyleName, const rtl::OUString* pCurrency,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void InsertColRow(const ScRange& rRange, const SCsCOL nDx, const SCsROW nDy,
        const SCsTAB nDz, ScDocument* pDoc);
    void SetStylesToRanges(ScRangeList* pList,
        const rtl::OUString* pStyleName, const sal_Int16 nCellType,
        const rtl::OUString* pCurrency, ScXMLImport& rImport);
    void SetStylesToRanges(ScRangeListRef xList,
        const rtl::OUString* pStyleName, const sal_Int16 nCellType,
        const rtl::OUString* pCurrency, ScXMLImport& rImport);
public:
    ScMyStyleRanges();
    ~ScMyStyleRanges();
    void AddRange(const ScRange& rRange,
        const rtl::OUString* pStyleName, const sal_Int16 nType,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void AddCurrencyRange(const ScRange& rRange,
        const rtl::OUString* pStyleName, const rtl::OUString* pCurrency,
        ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
    void InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc);
    void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc);
    void SetStylesToRanges(const rtl::OUString* pStyleName, ScXMLImport& rImport);
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
    rtl::OUString*      pStyleName;
    rtl::OUString*      pPrevStyleName;
    rtl::OUString*      pCurrency;
    rtl::OUString*      pPrevCurrency;
    ScRange             aPrevRange;
    sal_uInt32          nMaxRanges;
    sal_Int16           nCellType;
    sal_Int16           nPrevCellType;
    sal_Bool            bPrevRangeAdded;

    void ResetAttributes();
    ScMyStylesSet::iterator GetIterator(const rtl::OUString* pStyleName);
    void AddDefaultRange(const ScRange& rRange);
    void AddSingleRange(const ScRange& rRange);
    void AddRange();
    sal_Bool IsEqual(const rtl::OUString* pFirst, const rtl::OUString* pSecond)
    {
        return ((pFirst && pSecond && pFirst->equals(*pSecond)) ||
                (!pFirst && !pSecond) ||
                (!pFirst && pSecond && !pSecond->getLength()) ||
                (!pSecond &&  pFirst && !pFirst->getLength()));
    }
public:
    ScMyStylesImportHelper(ScXMLImport& rImport);
    ~ScMyStylesImportHelper();
    void AddColumnStyle(const rtl::OUString& rStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat);
    void SetRowStyle(const rtl::OUString& rStyleName);
    void SetAttributes(rtl::OUString* pStyleName,
        rtl::OUString* pCurrency, const sal_Int16 nCellType);
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

