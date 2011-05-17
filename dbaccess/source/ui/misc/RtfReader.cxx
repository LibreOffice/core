/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "RtfReader.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include "dbustrings.hrc"
#include <svtools/rtftoken.h>
#include "dbu_misc.hrc"
#include <vcl/msgbox.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include <tools/color.hxx>
#include "WExtendPages.hxx"
#include "moduledbu.hxx"
#include "QEnumTypes.hxx"
#include "UITools.hxx"
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

DBG_NAME(ORTFReader)
// ==========================================================================
// ORTFReader
// ==========================================================================
ORTFReader::ORTFReader( SvStream& rIn,
                        const SharedConnection& _rxConnection,
                        const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                        const TColumnVector* pList,
                        const OTypeInfoMap* _pInfoMap)
    :SvRTFParser(rIn)
    ,ODatabaseExport( _rxConnection, _rxNumberF, _rM, pList, _pInfoMap, rIn )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::ORTFReader" );
    DBG_CTOR(ORTFReader,NULL);
    m_bAppendFirstLine = false;
}
// ---------------------------------------------------------------------------
ORTFReader::ORTFReader(SvStream& rIn,
                       sal_Int32 nRows,
                       const TPositions &_rColumnPositions,
                       const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                       const TColumnVector* pList,
                       const OTypeInfoMap* _pInfoMap,
                       sal_Bool _bAutoIncrementEnabled)
   :SvRTFParser(rIn)
   ,ODatabaseExport( nRows, _rColumnPositions, _rxNumberF, _rM, pList, _pInfoMap, _bAutoIncrementEnabled, rIn )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::ORTFReader" );
    DBG_CTOR(ORTFReader,NULL);
    m_bAppendFirstLine = false;
}
// ---------------------------------------------------------------------------
ORTFReader::~ORTFReader()
{
    DBG_DTOR(ORTFReader,NULL);
}
// ---------------------------------------------------------------------------
SvParserState ORTFReader::CallParser()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::CallParser" );
    DBG_CHKTHIS(ORTFReader,NULL);
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    SvParserState  eParseState = SvRTFParser::CallParser();
    SetColumnTypes(m_pColumnList,m_pInfoMap);
    return m_bFoundTable ? eParseState : SVPAR_ERROR;
}
// ---------------------------------------------------------------------------
void ORTFReader::NextToken( int nToken )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::NextToken" );
    DBG_CHKTHIS(ORTFReader,NULL);
    if(m_bError || !m_nRows) // falls Fehler oder keine Rows mehr zur "Uberpr"ufung dann gleich zur"uck
        return;

    if(m_xConnection.is())    // gibt an welcher CTOR gerufen wurde und damit, ob eine Tabelle erstellt werden soll
    {
        switch(nToken)
        {
            case RTF_COLORTBL:
                {

                    int nTmpToken2 = GetNextToken();
                    do
                    {
                        Color aColor;
                        do
                        {
                            switch(nTmpToken2)
                            {
                                case RTF_RED:   aColor.SetRed((sal_uInt8)nTokenValue); break;
                                case RTF_BLUE:  aColor.SetBlue((sal_uInt8)nTokenValue); break;
                                case RTF_GREEN: aColor.SetGreen((sal_uInt8)nTokenValue); break;
                                default: break;
                            }
                            nTmpToken2 = GetNextToken();
                        }
                        while(aToken.GetChar(0) != ';' && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                        m_vecColor.push_back(aColor.GetRGBColor());
                        nTmpToken2 = GetNextToken();
                    }
                    while(nTmpToken2 == RTF_RED && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                    SkipToken();
                }
                break;

            case RTF_DEFLANG:
            case RTF_LANG: // Sprache abfragen
                m_nDefToken = (rtl_TextEncoding)nTokenValue;
                break;
            case RTF_TROWD:
                {
                    bool bInsertRow = true;
                    if ( !m_xTable.is() ) // erste Zeile als Header verwenden
                    {
                        sal_uInt32 nTell = rInput.Tell(); // ver�ndert vielleicht die Position des Streams

                        m_bError = !CreateTable(nToken);
                        bInsertRow = m_bAppendFirstLine;
                        if ( m_bAppendFirstLine )
                        {
                            rInput.Seek(nTell);
                            rInput.ResetError();
                        }
                    }
                    if ( bInsertRow && !m_bError)
                    {
                        try
                        {
                            m_pUpdateHelper->moveToInsertRow(); // sonst neue Zeile anh"angen
                        }
                        catch(SQLException& e)
                        // UpdateFehlerbehandlung
                        {
                            showErrorDialog(e);
                        }
                    }
                }
                break;
            case RTF_INTBL:
                if(m_bInTbl)
                {
                    eraseTokens();
                }

                m_bInTbl = sal_True; // jetzt befinden wir uns in einer Tabellenbeschreibung
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl) // wichtig, da wir sonst auch die Namen der Fonts bekommen
                    m_sTextToken += aToken;
                break;
            case RTF_CELL:
                {
                    try
                    {
                        insertValueIntoColumn();
                    }
                    catch(SQLException& e)
                    // UpdateFehlerbehandlung
                    {
                        showErrorDialog(e);
                    }
                    m_nColumnPos++;
                    eraseTokens();
                }
                break;
            case RTF_ROW:
                // es kann vorkommen, da� die letzte Celle nicht mit \cell abgeschlossen ist
                try
                {
                    insertValueIntoColumn();
                    m_nRowCount++;
                    if(m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
                        m_pUpdateHelper->updateInt(1,m_nRowCount);
                    m_pUpdateHelper->insertRow();
                }
                catch(SQLException& e)
                //////////////////////////////////////////////////////////////////////
                // UpdateFehlerbehandlung
                {
                    showErrorDialog(e);
                }
                m_nColumnPos = 0;
                break;
        }
    }
    else // Zweig nur f"ur Typpr"ufung g"ultig
    {
        switch(nToken)
        {
            case RTF_TROWD:
                // Der Spalten Kopf z"ahlt nicht mit
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != RTF_ROW && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                    m_bHead = sal_False;
                }
                break;
            case RTF_INTBL:
                m_bInTbl = sal_True;
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl)
                    m_sTextToken += aToken;
                break;
            case RTF_CELL:
                adjustFormat();
                m_nColumnPos++;
                break;
            case RTF_ROW:
                adjustFormat();
                m_nColumnPos = 0;
                m_nRows--;
                break;
        }
    }
}
// ---------------------------------------------------------------------------
sal_Bool ORTFReader::CreateTable(int nToken)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::CreateTable" );
    DBG_CHKTHIS(ORTFReader,NULL);
    String aTableName(ModuleRes(STR_TBL_TITLE));
    aTableName = aTableName.GetToken(0,' ');
    aTableName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTableName)));

    int nTmpToken2 = nToken;
    String aColumnName;

    FontDescriptor aFont = ::dbaui::CreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    do
    {
        switch(nTmpToken2)
        {
            case RTF_UNKNOWNCONTROL:
            case RTF_UNKNOWNDATA:
                m_bInTbl = sal_False;
                aColumnName.Erase();
                break;
            case RTF_INTBL:
                if(m_bInTbl)
                    aColumnName.Erase();

                m_bInTbl = sal_True;
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl)
                    aColumnName += aToken;
                break;
            case RTF_CELL:
                {
                    aColumnName.EraseLeadingChars();
                    aColumnName.EraseTrailingChars();
                    if (!aColumnName.Len() || m_bAppendFirstLine )
                        aColumnName = String(ModuleRes(STR_COLUMN_NAME));

                    CreateDefaultColumn(aColumnName);
                    aColumnName.Erase();
                }
                break;
            case RTF_CF:
                break;
            case RTF_B:
                aFont.Weight = ::com::sun::star::awt::FontWeight::BOLD;
                break;
            case RTF_I:
                aFont.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
                break;
            case RTF_UL:
                aFont.Underline = ::com::sun::star::awt::FontUnderline::SINGLE;
                break;
            case RTF_STRIKE:
                aFont.Strikeout = ::com::sun::star::awt::FontStrikeout::SINGLE;
                break;
        }
    }
    while((nTmpToken2 = GetNextToken()) != RTF_TROWD && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);

    sal_Bool bOk = !m_vDestVector.empty();
    if(bOk)
    {
        if ( aColumnName.Len() )
        {
            if ( m_bAppendFirstLine )
                aColumnName = String(ModuleRes(STR_COLUMN_NAME));
            CreateDefaultColumn(aColumnName);
        }

        m_bInTbl        = sal_False;
        m_bFoundTable   = sal_True;

        if ( isCheckEnabled() )
            return sal_True;
        Any aTextColor;
        if(!m_vecColor.empty())
            aTextColor <<= m_vecColor[0];

        bOk = !executeWizard(aTableName,aTextColor,aFont) && m_xTable.is();
    }
    return bOk;
}
// -----------------------------------------------------------------------------
void ORTFReader::release()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::release" );
    DBG_CHKTHIS(ORTFReader,NULL);
    ReleaseRef();
}

// -----------------------------------------------------------------------------
TypeSelectionPageFactory ORTFReader::getTypeSelectionPageFactory()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "ORTFReader::getTypeSelectionPageFactory" );
    DBG_CHKTHIS(ORTFReader,NULL);
    return &OWizRTFExtend::Create;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
