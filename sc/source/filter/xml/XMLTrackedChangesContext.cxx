/*************************************************************************
 *
 *  $RCSfile: XMLTrackedChangesContext.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-16 14:16:30 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLTRACKEDCHANGESCONTEXT_HXX
#include "XMLTrackedChangesContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_XMLCONTI_HXX
#include "xmlconti.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_TEXTSUNO_HXX
#include "textuno.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_text_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTrackedChangesContext::ScXMLTrackedChangesContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLTrackedChangesContext::~ScXMLTrackedChangesContext()
{
}

SvXMLImportContext *ScXMLTrackedChangesContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_cell_content_change) == 0)
        {
            pContext = new ScXMLContentChangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (rLocalName.compareToAscii(sXML_insertion) == 0)
        {
            pContext = new ScXMLInsertionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (rLocalName.compareToAscii(sXML_deletion) == 0)
        {
            pContext = new ScXMLDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (rLocalName.compareToAscii(sXML_movement) == 0)
        {
            pContext = new ScXMLMovementContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (rLocalName.compareToAscii(sXML_rejection) == 0)
        {
            pContext = new ScXMLRejectionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLTrackedChangesContext::EndElement()
{
}

ScXMLChangeInfoContext::ScXMLChangeInfoContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aInfo(),
    nParagraphCount(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (aLocalName.compareToAscii(sXML_chg_author) == 0)
            {
                aInfo.sUser = sValue;
            }
            else if (aLocalName.compareToAscii(sXML_chg_date_time) == 0)
            {
                GetScImport().GetMM100UnitConverter().convertDateTime(aInfo.aDateTime, sValue);
            }
        }
    }
}

ScXMLChangeInfoContext::~ScXMLChangeInfoContext()
{
}

SvXMLImportContext *ScXMLChangeInfoContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (rLocalName.compareToAscii(sXML_p) == 0) )
    {
        if(nParagraphCount)
            sBuffer.append(static_cast<sal_Unicode>('\n'));
        nParagraphCount++;
        pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLocalName, xAttrList, sBuffer);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeInfoContext::EndElement()
{
    aInfo.sComment = sBuffer.makeStringAndClear();
    pChangeTrackingImportHelper->SetActionInfo(aInfo);
}

ScXMLBigRangeContext::ScXMLBigRangeContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBigRange& rTempBigRange ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rBigRange(rTempBigRange)
{
    sal_Bool bColumn(sal_False);
    sal_Bool bRow(sal_False);
    sal_Bool bTable(sal_False);
    sal_Int32 nColumn;
    sal_Int32 nRow;
    sal_Int32 nTable;
    sal_Int32 nStartColumn;
    sal_Int32 nEndColumn;
    sal_Int32 nStartRow;
    sal_Int32 nEndRow;
    sal_Int32 nStartTable;
    sal_Int32 nEndTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_column) == 0)
            {
                SvXMLUnitConverter::convertNumber(nColumn, sValue);
                bColumn = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_row) == 0)
            {
                SvXMLUnitConverter::convertNumber(nRow, sValue);
                bRow = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_table) == 0)
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
                bTable = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_start_column) == 0)
                SvXMLUnitConverter::convertNumber(nStartColumn, sValue);
            else if (aLocalName.compareToAscii(sXML_end_column) == 0)
                SvXMLUnitConverter::convertNumber(nEndColumn, sValue);
            else if (aLocalName.compareToAscii(sXML_start_row) == 0)
                SvXMLUnitConverter::convertNumber(nStartRow, sValue);
            else if (aLocalName.compareToAscii(sXML_end_row) == 0)
                SvXMLUnitConverter::convertNumber(nEndRow, sValue);
            else if (aLocalName.compareToAscii(sXML_start_table) == 0)
                SvXMLUnitConverter::convertNumber(nStartTable, sValue);
            else if (aLocalName.compareToAscii(sXML_end_table) == 0)
                SvXMLUnitConverter::convertNumber(nEndTable, sValue);
        }
    }
    if (bColumn)
        nStartColumn = nEndColumn = nColumn;
    if (bRow)
        nStartRow = nEndRow = nRow;
    if (bTable)
        nStartTable = nEndTable = nTable;
    rBigRange.Set(nStartColumn, nStartRow, nStartTable,
        nEndColumn, nEndRow, nEndTable);
}

ScXMLBigRangeContext::~ScXMLBigRangeContext()
{
}

SvXMLImportContext *ScXMLBigRangeContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLBigRangeContext::EndElement()
{
}

ScXMLCellContentDeletionContext::ScXMLCellContentDeletionContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pCell(NULL),
    nMatrixFlag(MM_NONE),
    nID(0),
    bBigRange(sal_False),
    nType(NUMBERFORMAT_ALL),
    bContainsCell(sal_False),
    nMatrixCols(0),
    nMatrixRows(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLCellContentDeletionContext::~ScXMLCellContentDeletionContext()
{
}

SvXMLImportContext *ScXMLCellContentDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_change_track_table_cell) == 0)
        {
            bContainsCell = sal_True;
            pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
                pCell, sFormulaAddress, sFormula, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows );
        }
        else if (rLocalName.compareToAscii(sXML_cell_address) == 0)
        {
            DBG_ASSERT(!nID, "a action with a ID should not contain a BigRange");
            bBigRange = sal_True;
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCellContentDeletionContext::EndElement()
{
    ScMyCellInfo* pCellInfo = NULL;
    if (bContainsCell)
        pCellInfo = new ScMyCellInfo(pCell, sFormulaAddress, sFormula, fValue, nType,
            nMatrixFlag, nMatrixCols, nMatrixRows);
    if (nID)
        pChangeTrackingImportHelper->AddDeleted(nID, pCellInfo);
    else
        pChangeTrackingImportHelper->AddGenerated(pCellInfo, aBigRange);
}

ScXMLDependenceContext::ScXMLDependenceContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDependence(nID);
}

ScXMLDependenceContext::~ScXMLDependenceContext()
{
}

SvXMLImportContext *ScXMLDependenceContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    // here are no elements

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependenceContext::EndElement()
{
}

ScXMLDependingsContext::ScXMLDependingsContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLDependingsContext::~ScXMLDependingsContext()
{
}

SvXMLImportContext *ScXMLDependingsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_dependence) == 0)
            pContext = new ScXMLDependenceContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependingsContext::EndElement()
{
}

ScXMLChangeDeletionContext::ScXMLChangeDeletionContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDeleted(nID);
}

ScXMLChangeDeletionContext::~ScXMLChangeDeletionContext()
{
}

SvXMLImportContext *ScXMLChangeDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    // here are no elements

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeDeletionContext::EndElement()
{
}

ScXMLDeletionsContext::ScXMLDeletionsContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLDeletionsContext::~ScXMLDeletionsContext()
{
}

SvXMLImportContext *ScXMLDeletionsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_change_deletion) == 0)
            pContext = new ScXMLChangeDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_cell_content_deletion) == 0)
            pContext = new ScXMLCellContentDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionsContext::EndElement()
{
}

ScXMLChangeTextPContext::ScXMLChangeTextPContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList,
                                      ScXMLChangeCellContext* pTempChangeCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pTextPContext(NULL),
    sLName(rLName),
    xAttrList(xTempAttrList),
    nPrefix(nPrfx),
    sText()
{
    pChangeCellContext = pTempChangeCellContext;
    // here are no attributes
}

ScXMLChangeTextPContext::~ScXMLChangeTextPContext()
{
    if (pTextPContext)
        delete pTextPContext;
}

SvXMLImportContext *ScXMLChangeTextPContext::CreateChildContext( USHORT nTempPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (rLName.compareToAscii(sXML_s) == 0) && !pTextPContext)
    {
        sal_Int32 nRepeat(0);
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString sValue = xAttrList->getValueByIndex( i );
            rtl::OUString aLocalName;
            USHORT nPrfx = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            if ((nPrfx == XML_NAMESPACE_TEXT) && (aLocalName.compareToAscii(sXML_c) == 0))
                nRepeat = sValue.toInt32();
        }
        if (nRepeat)
            for (sal_Int32 j = 0; j < nRepeat; j++)
                sText.append(static_cast<sal_Unicode>(' '));
        else
            sText.append(static_cast<sal_Unicode>(' '));
    }
    else
    {
        if (!pChangeCellContext->IsEditCell())
            pChangeCellContext->CreateTextPContext(sal_False);
        sal_Bool bWasContext (sal_True);
        if (!pTextPContext)
        {
            bWasContext = sal_False;
            pTextPContext = GetScImport().GetTextImport()->CreateTextChildContext(
                                    GetScImport(), nPrefix, sLName, xAttrList);
        }
        if (pTextPContext)
        {
            if (!bWasContext)
                pTextPContext->Characters(sText.makeStringAndClear());
            pContext = pTextPContext->CreateChildContext(nTempPrefix, rLName, xTempAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLChangeTextPContext::Characters( const ::rtl::OUString& rChars )
{
    if (!pTextPContext)
        sText.append(rChars);
    else
        pTextPContext->Characters(rChars);
}

void ScXMLChangeTextPContext::EndElement()
{
    if (!pTextPContext)
        pChangeCellContext->SetText(sText.makeStringAndClear());
}

ScXMLChangeCellContext::ScXMLChangeCellContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBaseCell*& rTempOldCell, rtl::OUString& rAddress, rtl::OUString& rFormula,
                                            double& fDateTimeValue, sal_uInt16& nType,
                                            sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bEmpty(sal_True),
    bFirstParagraph(sal_True),
    bFormula(sal_False),
    bString(sal_True),
    pEditTextObj(NULL),
    rOldCell(rTempOldCell),
    rType(nType),
    rDateTimeValue(fDateTimeValue)
{
    sal_Bool bIsMatrix(sal_False);
    sal_Bool bIsCoveredMatrix(sal_False);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_value_type) == 0)
            {
                if (sValue.compareToAscii(sXML_float) == 0)
                    bString = sal_False;
                else if (sValue.compareToAscii(sXML_date) == 0)
                    rType = NUMBERFORMAT_DATE;
                else if (sValue.compareToAscii(sXML_time) == 0)
                    rType = NUMBERFORMAT_TIME;
            }
            else if (aLocalName.compareToAscii(sXML_value) == 0)
            {
                SvXMLUnitConverter::convertDouble(fValue, sValue);
                bEmpty = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_date_value) == 0)
            {
                if (GetScImport().GetMM100UnitConverter().setNullDate(GetScImport().GetModel()))
                    GetScImport().GetMM100UnitConverter().convertDateTime(rDateTimeValue, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_time_value) == 0)
            {
                GetScImport().GetMM100UnitConverter().convertTime(rDateTimeValue, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_formula) == 0)
            {
                bEmpty = sal_False;
                rFormula = sValue;
                ScXMLConverter::ParseFormula(rFormula);
                bFormula = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_cell_address) == 0)
            {
                rAddress = sValue;
            }
            else if (aLocalName.compareToAscii(sXML_matrix_covered) == 0)
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIsCoveredMatrix = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_number_matrix_columns_spanned) == 0)
            {
                bIsMatrix = sal_True;
                SvXMLUnitConverter::convertNumber(nMatrixCols, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_number_matrix_rows_spanned) == 0)
            {
                bIsMatrix = sal_True;
                SvXMLUnitConverter::convertNumber(nMatrixRows, sValue);
            }
        }
    }
    if (bIsCoveredMatrix)
        nMatrixFlag = MM_REFERENCE;
    else if (bIsMatrix && nMatrixRows && nMatrixCols)
        nMatrixFlag = MM_FORMULA;
}

ScXMLChangeCellContext::~ScXMLChangeCellContext()
{
}

SvXMLImportContext *ScXMLChangeCellContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (rLocalName.compareToAscii(sXML_p) == 0))
    {
        bEmpty = sal_False;
        if (bFirstParagraph)
        {
            pContext = new ScXMLChangeTextPContext(GetScImport(), nPrefix, rLocalName, xAttrList, this);
            bFirstParagraph = sal_False;
        }
        else
        {
            if (!pEditTextObj)
                CreateTextPContext(sal_True);
            pContext = GetScImport().GetTextImport()->CreateTextChildContext(
                GetScImport(), nPrefix, rLocalName, xAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeCellContext::CreateTextPContext(sal_Bool bIsNewParagraph)
{
    pEditTextObj = new ScEditEngineTextObj();
    pEditTextObj->GetEditEngine()->SetEditTextObjectPool(GetScImport().GetDocument()->GetEditPool());
    uno::Reference <text::XText> xText = pEditTextObj;
    if (xText.is())
    {
        uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
        if (bIsNewParagraph)
        {
            xText->setString(sText);
            xTextCursor->gotoEnd(sal_False);
            uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
            if (xTextRange.is())
                xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_False);
        }
        GetScImport().GetTextImport()->SetCursor(xTextCursor);
    }
}

void ScXMLChangeCellContext::EndElement()
{
    if (!bEmpty)
    {
        if (pEditTextObj)
        {
            if (GetImport().GetTextImport()->GetCursor().is())
            {
                //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
                if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
                {
                    OUString sEmpty;
                    GetImport().GetTextImport()->GetText()->insertString(
                        GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                        sal_True );
                }
            }
            rOldCell = new ScEditCell(pEditTextObj->CreateTextObject(), GetScImport().GetDocument(), GetScImport().GetDocument()->GetEditPool());
            GetScImport().GetTextImport()->ResetCursor();
            delete pEditTextObj;
        }
        else
        {
            if (!bFormula)
            {
                if (sText.getLength() && bString)
                    rOldCell = new ScStringCell(sText);
                else
                    rOldCell = new ScValueCell(fValue);
            }
            else
            {
                // do nothing, this has to do later
                /*ScAddress aCellPos;
                rOldCell = new ScFormulaCell(GetScImport().GetDocument(), aCellPos, sFormula);
                if (bString)
                    static_cast<ScFormulaCell*>(rOldCell)->SetString(sValue);
                else
                    static_cast<ScFormulaCell*>(rOldCell)->SetDouble(fValue);
                static_cast<ScFormulaCell*>(rOldCell)->SetInChangeTrack(sal_True);
                if (bIsCoveredMatrix)
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatrixFlag(MM_REFERENCE);
                else if (bIsMatrix && nMatrixRows && nMatrixCols)
                {
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatrixFlag(MM_FORMULA);
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatColsRows(static_cast<sal_uInt16>(nMatrixCols), static_cast<sal_uInt16>(nMatrixRows));
                }*/
            }
        }
    }
    else
        rOldCell = NULL;
}

ScXMLPreviousContext::ScXMLPreviousContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nID(0),
    sFormulaAddress(),
    sFormula(),
    nMatrixFlag(MM_NONE),
    pOldCell(NULL),
    nType(NUMBERFORMAT_ALL),
    nMatrixCols(0),
    nMatrixRows(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLPreviousContext::~ScXMLPreviousContext()
{
}

SvXMLImportContext *ScXMLPreviousContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TABLE) && (rLocalName.compareToAscii(sXML_change_track_table_cell) == 0))
        pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
            pOldCell, sFormulaAddress, sFormula, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLPreviousContext::EndElement()
{
    ScMyCellInfo* pCellInfo = new ScMyCellInfo(pOldCell, sFormulaAddress, sFormula, fValue, nType,
        nMatrixFlag, nMatrixCols, nMatrixRows);
    pChangeTrackingImportHelper->SetPreviousChange(nID, pCellInfo);
}

ScXMLContentChangeContext::ScXMLContentChangeContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_acceptance_state) == 0)
            {
                if (sValue.compareToAscii(sXML_accepted) == 0)
                    nActionState = SC_CAS_ACCEPTED;
                else if (sValue.compareToAscii(sXML_rejected) == 0)
                    nActionState = SC_CAS_REJECTED;
            }
            else if (aLocalName.compareToAscii(sXML_rejecting_change_id) == 0)
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_CONTENT);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLContentChangeContext::~ScXMLContentChangeContext()
{
}

SvXMLImportContext *ScXMLContentChangeContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (rLocalName.compareToAscii(sXML_change_info) == 0))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_cell_address) == 0)
        {
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
        else if (rLocalName.compareToAscii(sXML_dependences) == 0)
        {
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (rLocalName.compareToAscii(sXML_deletions) == 0)
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_previous) == 0)
        {
            pContext = new ScXMLPreviousContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLContentChangeContext::EndElement()
{
    pChangeTrackingImportHelper->SetBigRange(aBigRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionContext::ScXMLInsertionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nCount(1);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_INSERT_COLS);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_acceptance_state) == 0)
            {
                if (sValue.compareToAscii(sXML_accepted) == 0)
                    nActionState = SC_CAS_ACCEPTED;
                else if (sValue.compareToAscii(sXML_rejected) == 0)
                    nActionState = SC_CAS_REJECTED;
            }
            else if (aLocalName.compareToAscii(sXML_rejecting_change_id) == 0)
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_type) == 0)
            {
                if (sValue.compareToAscii(sXML_row) == 0)
                    nActionType = SC_CAT_INSERT_ROWS;
                else if (sValue.compareToAscii(sXML_table) == 0)
                    nActionType = SC_CAT_INSERT_TABS;
            }
            else if (aLocalName.compareToAscii(sXML_position) == 0)
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_table) == 0)
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_count) == 0)
            {
                SvXMLUnitConverter::convertNumber(nCount, sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, nCount, nTable);
}

ScXMLInsertionContext::~ScXMLInsertionContext()
{
}

SvXMLImportContext *ScXMLInsertionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (rLocalName.compareToAscii(sXML_change_info) == 0))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_dependences) == 0)
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_deletions) == 0)
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionCutOffContext::ScXMLInsertionCutOffContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_position) == 0)
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
        }
    }
    pChangeTrackingImportHelper->SetInsertionCutOff(nID, nPosition);
}

ScXMLInsertionCutOffContext::~ScXMLInsertionCutOffContext()
{
}

SvXMLImportContext *ScXMLInsertionCutOffContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionCutOffContext::EndElement()
{
}

ScXMLMovementCutOffContext::ScXMLMovementCutOffContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    sal_Int32 nStartPosition(0);
    sal_Int32 nEndPosition(0);
    sal_Bool bPosition(sal_False);
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_position) == 0)
            {
                bPosition = sal_True;
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_start_position) == 0)
            {
                SvXMLUnitConverter::convertNumber(nStartPosition, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_end_position) == 0)
            {
                SvXMLUnitConverter::convertNumber(nEndPosition, sValue);
            }
        }
    }
    if (bPosition)
        nStartPosition = nEndPosition = nPosition;
    pChangeTrackingImportHelper->AddMoveCutOff(nID, nStartPosition, nEndPosition);
}

ScXMLMovementCutOffContext::~ScXMLMovementCutOffContext()
{
}

SvXMLImportContext *ScXMLMovementCutOffContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementCutOffContext::EndElement()
{
}

ScXMLCutOffsContext::ScXMLCutOffsContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLCutOffsContext::~ScXMLCutOffsContext()
{
}

SvXMLImportContext *ScXMLCutOffsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_insertion_cut_off) == 0)
            pContext = new ScXMLInsertionCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_movement_cut_off) == 0)
            pContext = new ScXMLMovementCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCutOffsContext::EndElement()
{
}

ScXMLDeletionContext::ScXMLDeletionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nMultiSpanned(0);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_DELETE_COLS);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_acceptance_state) == 0)
            {
                if (sValue.compareToAscii(sXML_accepted) == 0)
                    nActionState = SC_CAS_ACCEPTED;
                else if (sValue.compareToAscii(sXML_rejected) == 0)
                    nActionState = SC_CAS_REJECTED;
            }
            else if (aLocalName.compareToAscii(sXML_rejecting_change_id) == 0)
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_type) == 0)
            {
                if (sValue.compareToAscii(sXML_row) == 0)
                {
                    nActionType = SC_CAT_DELETE_ROWS;
                }
            }
            else if (aLocalName.compareToAscii(sXML_position) == 0)
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_table) == 0)
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_multi_deletion_spanned) == 0)
            {
                SvXMLUnitConverter::convertNumber(nMultiSpanned, sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, 1, nTable);
    pChangeTrackingImportHelper->SetMultiSpanned(static_cast<sal_Int16>(nMultiSpanned));
}

ScXMLDeletionContext::~ScXMLDeletionContext()
{
}

SvXMLImportContext *ScXMLDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (rLocalName.compareToAscii(sXML_change_info) == 0))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_dependences) == 0)
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_deletions) == 0)
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_cut_offs) == 0)
            pContext = new ScXMLCutOffsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLMovementContext::ScXMLMovementContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_acceptance_state) == 0)
            {
                if (sValue.compareToAscii(sXML_accepted) == 0)
                    nActionState = SC_CAS_ACCEPTED;
                else if (sValue.compareToAscii(sXML_rejected) == 0)
                    nActionState = SC_CAS_REJECTED;
            }
            else if (aLocalName.compareToAscii(sXML_rejecting_change_id) == 0)
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLMovementContext::~ScXMLMovementContext()
{
}

SvXMLImportContext *ScXMLMovementContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (rLocalName.compareToAscii(sXML_change_info) == 0))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_dependences) == 0)
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_deletions) == 0)
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_source_range_address) == 0)
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aSourceRange);
        else if (rLocalName.compareToAscii(sXML_target_range_address) == 0)
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aTargetRange);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementContext::EndElement()
{
    pChangeTrackingImportHelper->SetMoveRanges(aSourceRange, aTargetRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLRejectionContext::ScXMLRejectionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (aLocalName.compareToAscii(sXML_acceptance_state) == 0)
            {
                if (sValue.compareToAscii(sXML_accepted) == 0)
                    nActionState = SC_CAS_ACCEPTED;
                else if (sValue.compareToAscii(sXML_rejected) == 0)
                    nActionState = SC_CAS_REJECTED;
            }
            else if (aLocalName.compareToAscii(sXML_rejecting_change_id) == 0)
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLRejectionContext::~ScXMLRejectionContext()
{
}

SvXMLImportContext *ScXMLRejectionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (rLocalName.compareToAscii(sXML_change_info) == 0))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLocalName.compareToAscii(sXML_dependences) == 0)
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (rLocalName.compareToAscii(sXML_deletions) == 0)
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLRejectionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}


