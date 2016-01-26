/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "RtfReader.hxx"
#include <tools/debug.hxx>
#include <tools/stream.hxx>
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
#include <toolkit/helper/vclunohelper.hxx>
#include "dbu_misc.hrc"
#include <vcl/msgbox.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/string.hxx>
#include <tools/color.hxx>
#include "WExtendPages.hxx"
#include "moduledbu.hxx"
#include "QEnumTypes.hxx"
#include "UITools.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

// ORTFReader
ORTFReader::ORTFReader( SvStream& rIn,
                        const SharedConnection& _rxConnection,
                        const Reference< css::util::XNumberFormatter >& _rxNumberF,
                        const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                        const TColumnVector* pList,
                        const OTypeInfoMap* _pInfoMap)
    :SvRTFParser(rIn)
    ,ODatabaseExport( _rxConnection, _rxNumberF, _rxContext, pList, _pInfoMap, rIn )
{
    m_bAppendFirstLine = false;
}

ORTFReader::ORTFReader(SvStream& rIn,
                       sal_Int32 nRows,
                       const TPositions &_rColumnPositions,
                       const Reference< css::util::XNumberFormatter >& _rxNumberF,
                       const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                       const TColumnVector* pList,
                       const OTypeInfoMap* _pInfoMap,
                       bool _bAutoIncrementEnabled)
   :SvRTFParser(rIn)
   ,ODatabaseExport( nRows, _rColumnPositions, _rxNumberF, _rxContext, pList, _pInfoMap, _bAutoIncrementEnabled, rIn )
{
    m_bAppendFirstLine = false;
}

ORTFReader::~ORTFReader()
{
}

SvParserState ORTFReader::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    SvParserState  eParseState = SvRTFParser::CallParser();
    SetColumnTypes(m_pColumnList,m_pInfoMap);
    return m_bFoundTable ? eParseState : SVPAR_ERROR;
}

void ORTFReader::NextToken( int nToken )
{
    if(m_bError || !m_nRows) // if there is an error or no more rows to check, return immediately
        return;

    if(m_xConnection.is())    // names, which CTOR was called and hence, if a table should be created
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
                        while(aToken[0] != ';' && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                        m_vecColor.push_back(aColor.GetRGBColor());
                        nTmpToken2 = GetNextToken();
                    }
                    while(nTmpToken2 == RTF_RED && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                    SkipToken();
                }
                break;

            case RTF_DEFLANG:
            case RTF_LANG: // inquire language
                m_nDefToken = (rtl_TextEncoding)nTokenValue;
                break;
            case RTF_TROWD:
                {
                    bool bInsertRow = true;
                    if ( !m_xTable.is() ) // use first line as header
                    {
                        sal_Size nTell = rInput.Tell(); // perhaps alters position of the stream

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
                            m_pUpdateHelper->moveToInsertRow(); // otherwise append new line
                        }
                        catch(SQLException& e)
                        // handling update failure
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

                m_bInTbl = true; // Now we are in a table description
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl) // important, as otherwise we also get the names of the fonts
                    m_sTextToken += aToken;
                break;
            case RTF_CELL:
                {
                    try
                    {
                        insertValueIntoColumn();
                    }
                    catch(SQLException& e)
                    // handling update failure
                    {
                        showErrorDialog(e);
                    }
                    m_nColumnPos++;
                    eraseTokens();
                }
                break;
            case RTF_ROW:
                // it can happen that the last cell is not concluded with \cell
                try
                {
                    insertValueIntoColumn();
                    m_nRowCount++;
                    if(m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
                        m_pUpdateHelper->updateInt(1,m_nRowCount);
                    m_pUpdateHelper->insertRow();
                }
                catch(SQLException& e)
                // handling update failure
                {
                    showErrorDialog(e);
                }
                m_nColumnPos = 0;
                break;
        }
    }
    else // branch only valid for type checking
    {
        switch(nToken)
        {
            case RTF_TROWD:
                // The head of the column is not included
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != RTF_ROW && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);
                    m_bHead = false;
                }
                break;
            case RTF_INTBL:
                m_bInTbl = true;
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

bool ORTFReader::CreateTable(int nToken)
{
    OUString aTableName(ModuleRes(STR_TBL_TITLE));
    aTableName = aTableName.getToken(0,' ');
    aTableName = ::dbtools::createUniqueName(m_xTables, aTableName);

    OUString aColumnName;

    FontDescriptor aFont = VCLUnoHelper::CreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    do
    {
        switch (nToken)
        {
            case RTF_UNKNOWNCONTROL:
            case RTF_UNKNOWNDATA:
                m_bInTbl = false;
                aColumnName.clear();
                break;
            case RTF_INTBL:
                if(m_bInTbl)
                    aColumnName.clear();

                m_bInTbl = true;
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl)
                    aColumnName += aToken;
                break;
            case RTF_CELL:
                {
                    aColumnName = comphelper::string::strip(aColumnName, ' ');
                    if (aColumnName.isEmpty() || m_bAppendFirstLine )
                        aColumnName = ModuleRes(STR_COLUMN_NAME);

                    CreateDefaultColumn(aColumnName);
                    aColumnName.clear();
                }
                break;
            case RTF_CF:
                break;
            case RTF_B:
                aFont.Weight = css::awt::FontWeight::BOLD;
                break;
            case RTF_I:
                aFont.Slant = css::awt::FontSlant_ITALIC;
                break;
            case RTF_UL:
                aFont.Underline = css::awt::FontUnderline::SINGLE;
                break;
            case RTF_STRIKE:
                aFont.Strikeout = css::awt::FontStrikeout::SINGLE;
                break;
        }
        nToken = GetNextToken();
    }
    while(nToken != RTF_TROWD && eState != SVPAR_ERROR && eState != SVPAR_ACCEPTED);

    bool bOk = !m_vDestVector.empty();
    if(bOk)
    {
        if ( !aColumnName.isEmpty() )
        {
            if ( m_bAppendFirstLine )
                aColumnName = ModuleRes(STR_COLUMN_NAME);
            CreateDefaultColumn(aColumnName);
        }

        m_bInTbl        = false;
        m_bFoundTable   = true;

        if ( isCheckEnabled() )
            return true;
        Any aTextColor;
        if(!m_vecColor.empty())
            aTextColor <<= m_vecColor[0];

        bOk = !executeWizard(aTableName,aTextColor,aFont) && m_xTable.is();
    }
    return bOk;
}

void ORTFReader::release()
{
    ReleaseRef();
}

TypeSelectionPageFactory ORTFReader::getTypeSelectionPageFactory()
{
    return &OWizRTFExtend::Create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
