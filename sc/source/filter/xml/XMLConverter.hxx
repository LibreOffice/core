/*************************************************************************
 *
 *  $RCSfile: XMLConverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-02 16:32:20 $
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

#ifndef _SC_XMLCONVERTER_HXX
#define _SC_XMLCONVERTER_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif

#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif

class ScArea;
class ScRangeList;


//___________________________________________________________________

class ScXMLConverter
{
protected:
    static void         AssignString(
                            ::rtl::OUString& rString,
                            const ::rtl::OUString& rNewStr,
                            sal_Bool bAppendStr );

public:
    inline              ScXMLConverter()    {}
    inline              ~ScXMLConverter()   {}

// helper methods
    static sal_Int32    GetTokenCount(
                            const ::rtl::OUString& rString );
    static sal_Int32    GetTokenByOffset(
                            ::rtl::OUString& rToken,
                            const ::rtl::OUString& rString,
                            sal_Int32 nOffset,
                            sal_Unicode cQuote = '\'' );

    static void         AppendString(
                            ::rtl::OUString& rString,
                            const ::rtl::OUString& rNewStr );

    static ScDocument*  GetScDocument(
                            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel );

// IMPORT: CellAddress / CellRange
    static sal_Int32    GetAddressFromString(
                            ScAddress& rAddress,
                            const ::rtl::OUString& rAddressStr,
                            ScDocument* pDocument,
                            sal_Int32 nOffset = 0 );
    static sal_Int32    GetRangeFromString(
                            ScRange& rRange,
                            const ::rtl::OUString& rRangeStr,
                            ScDocument* pDocument,
                            sal_Int32 nOffset = 0 );
    static void         GetRangeListFromString(
                            ScRangeList& rRangeList,
                            const ::rtl::OUString& rRangeListStr,
                            ScDocument* pDocument );

    static sal_Int32    GetAreaFromString(
                            ScArea& rArea,
                            const ::rtl::OUString& rRangeStr,
                            ScDocument* pDocument,
                            sal_Int32 nOffset = 0 );

    static sal_Int32    GetAddressFromString(
                            ::com::sun::star::table::CellAddress& rAddress,
                            const ::rtl::OUString& rAddressStr,
                            ScDocument* pDocument,
                            sal_Int32 nOffset = 0 );
    static sal_Int32    GetRangeFromString(
                            ::com::sun::star::table::CellRangeAddress& rRange,
                            const ::rtl::OUString& rRangeStr,
                            ScDocument* pDocument,
                            sal_Int32 nOffset = 0 );
    static void         GetRangeListFromString(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress >& rRangeSeq,
                            const ::rtl::OUString& rRangeListStr,
                            ScDocument* pDocument );

// EXPORT: CellAddress / CellRange
    static void         GetStringFromAddress(
                            ::rtl::OUString& rString,
                            const ScAddress& rAddress,
                            ScDocument* pDocument,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromRange(
                            ::rtl::OUString& rString,
                            const ScRange& rRange,
                            ScDocument* pDocument,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromRangeList(
                            ::rtl::OUString& rString,
                            const ScRangeList* pRangeList,
                            ScDocument* pDocument );

    static void         GetStringFromArea(
                            ::rtl::OUString& rString,
                            const ScArea& rArea,
                            ScDocument* pDocument,
                            sal_Bool bAppendStr = sal_False );

    static void         GetStringFromAddress(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::table::CellAddress& rAddress,
                            ScDocument* pDocument,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromRange(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            ScDocument* pDocument,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromRangeList(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress >& rRangeSeq,
                            ScDocument* pDocument );

// IMPORT: GeneralFunction / ScSubTotalFunc
    static ::com::sun::star::sheet::GeneralFunction
                        GetFunctionFromString(
                            const ::rtl::OUString& sFunction );
    static ScSubTotalFunc GetSubTotalFuncFromString(
                            const ::rtl::OUString& sFunction );

// EXPORT: GeneralFunction / ScSubTotalFunc
    static void         GetStringFromFunction(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::sheet::GeneralFunction eFunction,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromFunction(
                            ::rtl::OUString& rString,
                            const ScSubTotalFunc eFunction,
                            sal_Bool bAppendStr = sal_False );

// IMPORT: Formulas
    static void         ParseFormula(
                            ::rtl::OUString& sFormula,
                            const sal_Bool bIsFormula = sal_True);
};


#endif

