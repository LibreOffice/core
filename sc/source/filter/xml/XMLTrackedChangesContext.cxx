/*************************************************************************
 *
 *  $RCSfile: XMLTrackedChangesContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2001-01-24 15:14:56 $
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(NULL)
{
    pChangeTrackingImportHelper = new ScXMLChangeTrackingImportHelper();
    // here are no attributes
}

ScXMLTrackedChangesContext::~ScXMLTrackedChangesContext()
{
    if (pChangeTrackingImportHelper)
        delete pChangeTrackingImportHelper;
}

SvXMLImportContext *ScXMLTrackedChangesContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
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
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

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
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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

ScXMLDependingsContext::ScXMLDependingsContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const NAMESPACE_RTL(OUString)& rLName,
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
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependingsContext::EndElement()
{
}

ScXMLChangeTextPContext::ScXMLChangeTextPContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
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
}

ScXMLChangeTextPContext::~ScXMLChangeTextPContext()
{
    if (pTextPContext)
        delete pTextPContext;
}

SvXMLImportContext *ScXMLChangeTextPContext::CreateChildContext( USHORT nTempPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBaseCell* pTempOldCell ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bEmpty(sal_True),
    bFirstParagraph(sal_True),
    bFormula(sal_False),
    bString(sal_True),
    pEditTextObj(NULL)
{
    pOldCell = pTempOldCell;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_value_type) == 0)
            {
                if (sValue.compareToAscii(sXML_float) == 0)
                    bString = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_value) == 0)
            {
                this->sValue = sValue;
                bEmpty = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_formula) == 0)
            {
                bEmpty = sal_False;
                sFormula = sValue;
                ScXMLConverter::ParseFormula(sFormula);
                bFormula = sal_True;
            }
        }
    }
    double fTempValue;
    if (!SvXMLUnitConverter::convertNumber(fTempValue, sValue))
        bString = sal_True;
}

ScXMLChangeCellContext::~ScXMLChangeCellContext()
{
}

SvXMLImportContext *ScXMLChangeCellContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
            GetScImport().GetTextImport()->ResetCursor();
            pOldCell = new ScEditCell(pEditTextObj->CreateTextObject(), GetScImport().GetDocument(), GetScImport().GetDocument()->GetEditPool());
        }
        else
        {
            if (!bFormula)
            {
                double fTempValue;
                sal_Bool bNumber(SvXMLUnitConverter::convertNumber(fTempValue, sText));
                if (sText.getLength() && !bNumber)
                    pOldCell = new ScStringCell(sText);
                else if (bNumber)
                    pOldCell = new ScValueCell(fTempValue);
                else if (bString)
                    pOldCell = new ScStringCell(sValue);
                else
                    pOldCell = new ScValueCell(fValue);
            }
            else
            {
                pOldCell = new ScFormulaCell(GetScImport().GetDocument(), aCellPos, sFormula);
                if (bString)
                    static_cast<ScFormulaCell*>(pOldCell)->SetString(sValue);
                else
                    static_cast<ScFormulaCell*>(pOldCell)->SetDouble(fValue);
                static_cast<ScFormulaCell*>(pOldCell)->SetInChangeTrack(sal_True);
            }
        }
    }
    else
        pOldCell = NULL;
}

ScXMLPreviousContext::ScXMLPreviousContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nID(0),
    pOldCell(NULL)
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                sal_Int32 nTemp;
                SvXMLUnitConverter::convertNumber(nTemp, sValue);
                nID = static_cast<sal_uInt32>(nTemp);
            }
        }
    }
}

ScXMLPreviousContext::~ScXMLPreviousContext()
{
}

SvXMLImportContext *ScXMLPreviousContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TABLE) && (rLocalName.compareToAscii(sXML_table_cell) == 0))
        pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList, pOldCell);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLPreviousContext::EndElement()
{
    pChangeTrackingImportHelper->SetPreviousChange(nID, pOldCell);
}

ScXMLContentChangeContext::ScXMLContentChangeContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int32 nActionNumber(-1);
    sal_Int32 nRejectingNumber(-1);
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                SvXMLUnitConverter::convertNumber(nActionNumber, sValue);
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
                SvXMLUnitConverter::convertNumber(nRejectingNumber, sValue);
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
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
        if (rLocalName.compareToAscii(sXML_range_address) == 0)
        {
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
        else if (rLocalName.compareToAscii(sXML_dependences) == 0)
        {
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int32 nActionNumber(-1);
    sal_Int32 nRejectingNumber(-1);
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                SvXMLUnitConverter::convertNumber(nActionNumber, sValue);
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
                SvXMLUnitConverter::convertNumber(nRejectingNumber, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_type) == 0)
            {
                if (sValue.compareToAscii(sXML_row) == 0)
                    nActionType = SC_CAT_INSERT_ROWS;
                else if (sValue.compareToAscii(sXML_table) == 0)
                    nActionType = SC_CAT_INSERT_TABS;
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLInsertionContext::~ScXMLInsertionContext()
{
}

SvXMLImportContext *ScXMLInsertionContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLDeletionContext::ScXMLDeletionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int32 nActionNumber(-1);
    sal_Int32 nRejectingNumber(-1);
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                SvXMLUnitConverter::convertNumber(nActionNumber, sValue);
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
                SvXMLUnitConverter::convertNumber(nRejectingNumber, sValue);
            }
            else if (aLocalName.compareToAscii(sXML_type) == 0)
            {
                if (sValue.compareToAscii(sXML_row) == 0)
                {
                    nActionType = SC_CAT_DELETE_ROWS;
                }
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLDeletionContext::~ScXMLDeletionContext()
{
}

SvXMLImportContext *ScXMLDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
                                                   const NAMESPACE_RTL(OUString)& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int32 nActionNumber(-1);
    sal_Int32 nRejectingNumber(-1);
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

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_id) == 0)
            {
                SvXMLUnitConverter::convertNumber(nActionNumber, sValue);
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
                SvXMLUnitConverter::convertNumber(nRejectingNumber, sValue);
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
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

