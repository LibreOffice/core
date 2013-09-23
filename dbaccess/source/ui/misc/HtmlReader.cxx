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

#include "HtmlReader.hxx"
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/tenccvt.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/string.hxx>
#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include <sfx2/sfxhtml.hxx>
#include <osl/diagnose.h>
#include "moduledbu.hxx"
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
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <tools/color.hxx>
#include "WCopyTable.hxx"
#include "WExtendPages.hxx"
#include "WNameMatch.hxx"
#include "WColumnSelect.hxx"
#include "QEnumTypes.hxx"
#include "WCPage.hxx"
#include <tools/inetmime.hxx>
#include <svl/inettype.hxx>
#include <rtl/tencinfo.h>
#include "UITools.hxx"
#include <vcl/svapp.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

#define DBAUI_HTML_FONTSIZES    8       // like export, HTML-Options

DBG_NAME(OHTMLReader)
// OHTMLReader
OHTMLReader::OHTMLReader(SvStream& rIn,const SharedConnection& _rxConnection,
                        const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                        const TColumnVector* pList,
                        const OTypeInfoMap* _pInfoMap)
    :HTMLParser(rIn)
    ,ODatabaseExport( _rxConnection, _rxNumberF, _rxContext, pList, _pInfoMap, rIn )
    ,m_nTableCount(0)
    ,m_nColumnWidth(87)
    ,m_bMetaOptions(sal_False)
    ,m_bSDNum(sal_False)
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::OHTMLReader" );
    DBG_CTOR(OHTMLReader,NULL);
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( sal_True );
}

OHTMLReader::OHTMLReader(SvStream& rIn,
                         sal_Int32 nRows,
                         const TPositions &_rColumnPositions,
                         const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                         const TColumnVector* pList,
                         const OTypeInfoMap* _pInfoMap,
                         sal_Bool _bAutoIncrementEnabled)
    :HTMLParser(rIn)
    ,ODatabaseExport( nRows, _rColumnPositions, _rxNumberF, _rxContext, pList, _pInfoMap, _bAutoIncrementEnabled, rIn )
    ,m_nTableCount(0)
    ,m_nColumnWidth(87)
    ,m_bMetaOptions(sal_False)
    ,m_bSDNum(sal_False)
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::OHTMLReader" );
    DBG_CTOR(OHTMLReader,NULL);
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( sal_True );
}

OHTMLReader::~OHTMLReader()
{
    DBG_DTOR(OHTMLReader,NULL);
}

SvParserState OHTMLReader::CallParser()
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::CallParser" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    SvParserState  eParseState = HTMLParser::CallParser();
    SetColumnTypes(m_pColumnList,m_pInfoMap);
    return m_bFoundTable ? eParseState : SVPAR_ERROR;
}

void OHTMLReader::NextToken( int nToken )
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::NextToken" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    if(m_bError || !m_nRows) // if there is an error or no more rows to check, return immediatelly
        return;
    if ( nToken ==  HTML_META )
        setTextEncoding();

    if(m_xConnection.is())    // names, which CTOR was called and hence, if a table should be created
    {
        switch(nToken)
        {
            case HTML_TABLE_ON:
                ++m_nTableCount;
                {   // can also be TD or TH, if there was no TABLE before
                    const HTMLOptions& rHtmlOptions = GetOptions();
                    for (size_t i = 0, n = rHtmlOptions.size(); i < n; ++i)
                    {
                        const HTMLOption& rOption = rHtmlOptions[i];
                        switch( rOption.GetToken() )
                        {
                            case HTML_O_WIDTH:
                            {   // percentage: of document width respectively outer cell
                                m_nColumnWidth = GetWidthPixel( rOption );
                            }
                            break;
                        }
                    }
                }
            case HTML_THEAD_ON:
            case HTML_TBODY_ON:
                {
                    sal_uInt32 nTell = rInput.Tell(); // perhaps alters position of the stream
                    if ( !m_xTable.is() )
                    {// use first line as header
                        m_bError = !CreateTable(nToken);
                        if ( m_bAppendFirstLine )
                            rInput.Seek(nTell);
                    }
                }
                break;
            case HTML_TABLE_OFF:
                if(!--m_nTableCount)
                {
                    m_xTable = NULL;
                }
                break;
            case HTML_TABLEROW_ON:
                if ( m_pUpdateHelper.get() )
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
                else
                    m_bError = sal_True;
                break;
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if ( m_bInTbl ) //&& !m_bSDNum ) // important, as otherwise we also get the names of the fonts
                    m_sTextToken += aToken;
                break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
            case HTML_TABLEDATA_ON:
                fetchOptions();
                break;
            case HTML_TABLEDATA_OFF:
                {
                    if ( m_sCurrent.Len() )
                        m_sTextToken = m_sCurrent;
                    try
                    {
                        insertValueIntoColumn();
                    }
                    catch(SQLException& e)
                    // handling update failure
                    {
                        showErrorDialog(e);
                    }
                    m_sCurrent.Erase();
                    m_nColumnPos++;
                    eraseTokens();
                    m_bSDNum = m_bInTbl = sal_False;
                }
                break;
            case HTML_TABLEROW_OFF:
                if ( !m_pUpdateHelper.get() )
                {
                    m_bError = sal_True;
                    break;
                }
                try
                {
                    m_nRowCount++;
                    if (m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
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
            case HTML_THEAD_ON:
            case HTML_TBODY_ON:
                // The head of the column is not included
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != HTML_TABLEROW_OFF);
                    m_bHead = sal_False;
                }
                break;
            case HTML_TABLEDATA_ON:
            case HTML_TABLEHEADER_ON:
                fetchOptions();
                break;
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if ( m_bInTbl ) // && !m_bSDNum ) // important, as otherwise we also get the names of the fonts
                    m_sTextToken += aToken;
                break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
            case HTML_TABLEDATA_OFF:
                if ( m_sCurrent.Len() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
                m_nColumnPos++;
                m_bSDNum = m_bInTbl = sal_False;
                m_sCurrent.Erase();
                break;
            case HTML_TABLEROW_OFF:
                if ( m_sCurrent.Len() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
                m_nColumnPos = 0;
                m_nRows--;
                m_sCurrent.Erase();
                break;
        }
    }
}

void OHTMLReader::fetchOptions()
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::fetchOptions" );
    m_bInTbl = sal_True;
    const HTMLOptions& options = GetOptions();
    for (size_t i = 0, n = options.size(); i < n; ++i)
    {
        const HTMLOption& rOption = options[i];
        switch( rOption.GetToken() )
        {
            case HTML_O_SDVAL:
            {
                m_sValToken = rOption.GetString();
                m_bSDNum = sal_True;
            }
            break;
            case HTML_O_SDNUM:
                m_sNumToken = rOption.GetString();
            break;
        }
    }
}

void OHTMLReader::TableDataOn(SvxCellHorJustify& eVal)
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::TableDataOn" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    const HTMLOptions& rHtmlOptions = GetOptions();
    for (size_t i = 0, n = rHtmlOptions.size(); i < n; ++i)
    {
        const HTMLOption& rOption = rHtmlOptions[i];
        switch( rOption.GetToken() )
        {
            case HTML_O_ALIGN:
            {
                const OUString& rOptVal = rOption.GetString();
                if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right ))
                    eVal = SVX_HOR_JUSTIFY_RIGHT;
                else if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_center ))
                    eVal = SVX_HOR_JUSTIFY_CENTER;
                else if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ))
                    eVal = SVX_HOR_JUSTIFY_LEFT;
                else
                    eVal = SVX_HOR_JUSTIFY_STANDARD;
            }
            break;
            case HTML_O_WIDTH:
                m_nWidth = GetWidthPixel( rOption );
            break;
        }
    }
}

void OHTMLReader::TableFontOn(FontDescriptor& _rFont,sal_Int32 &_rTextColor)
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::TableFontOn" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    const HTMLOptions& rHtmlOptions = GetOptions();
    for (size_t i = 0, n = rHtmlOptions.size(); i < n; ++i)
    {
        const HTMLOption& rOption = rHtmlOptions[i];
        switch( rOption.GetToken() )
        {
        case HTML_O_COLOR:
            {
                Color aColor;
                rOption.GetColor( aColor );
                _rTextColor = aColor.GetRGBColor();
            }
            break;
        case HTML_O_FACE :
            {
                const String& rFace = rOption.GetString();
                String aFontName;
                sal_Int32 nPos = 0;
                while( nPos != -1 )
                {
                    // list fo fonts, VCL: semicolon as separator, HTML: comma
                    String aFName = rFace.GetToken( 0, ',', nPos );
                    aFName = comphelper::string::strip(aFName, ' ');
                    if( aFontName.Len() )
                        aFontName += ';';
                    aFontName += aFName;
                }
                if ( aFontName.Len() )
                    _rFont.Name = OUString(aFontName);
            }
            break;
        case HTML_O_SIZE :
            {
                sal_Int16 nSize = (sal_Int16) rOption.GetNumber();
                if ( nSize == 0 )
                    nSize = 1;
                else if ( nSize < DBAUI_HTML_FONTSIZES )
                    nSize = DBAUI_HTML_FONTSIZES;

                _rFont.Height = nSize;
            }
            break;
        }
    }
}

sal_Int16 OHTMLReader::GetWidthPixel( const HTMLOption& rOption )
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::GetWidthPixel" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    const String& rOptVal = rOption.GetString();
    if ( rOptVal.Search('%') != STRING_NOTFOUND )
    {   // percentage
        OSL_ENSURE( m_nColumnWidth, "WIDTH Option: m_nColumnWidth==0 und Width%" );
        return (sal_Int16)((rOption.GetNumber() * m_nColumnWidth) / 100);
    }
    else
    {
        if ( rOptVal.Search('*') != STRING_NOTFOUND )
        {   // relativ to what?!?
//TODO: collect ColArray of all relevant values and then MakeCol
            return 0;
        }
        else
            return (sal_Int16)rOption.GetNumber();  // pixel
    }
}

sal_Bool OHTMLReader::CreateTable(int nToken)
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::CreateTable" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    String aTempName(ModuleRes(STR_TBL_TITLE));
    aTempName = aTempName.GetToken(0,' ');
    aTempName = String(::dbtools::createUniqueName(m_xTables,OUString(aTempName )));

    int nTmpToken2 = nToken;
    sal_Bool bCaption = sal_False;
    sal_Bool bTableHeader = sal_False;
    String aColumnName;
    SvxCellHorJustify eVal;

    String aTableName;
    FontDescriptor aFont = VCLUnoHelper::CreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    sal_Int32 nTextColor = 0;
    do
    {
        switch(nTmpToken2)
        {
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if(bTableHeader)
                    aColumnName += aToken;
                if(bCaption)
                    aTableName += aToken;
                break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += aColumnName;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
            case HTML_TABLEDATA_ON:
            case HTML_TABLEHEADER_ON:
                TableDataOn(eVal);
                bTableHeader = sal_True;
                break;
            case HTML_TABLEDATA_OFF:
            case HTML_TABLEHEADER_OFF:
                {
                    aColumnName = comphelper::string::strip(aColumnName, ' ' );
                    if (!aColumnName.Len() || m_bAppendFirstLine )
                        aColumnName = String(ModuleRes(STR_COLUMN_NAME));
                    else if ( m_sCurrent.Len() )
                        aColumnName = m_sCurrent;

                    aColumnName = comphelper::string::strip(aColumnName, ' ');
                    CreateDefaultColumn(aColumnName);
                    aColumnName.Erase();
                    m_sCurrent.Erase();

                    eVal = SVX_HOR_JUSTIFY_STANDARD;
                    bTableHeader = sal_False;
                }
                break;

            case HTML_TITLE_ON:
            case HTML_CAPTION_ON:
                bCaption = sal_True;
                break;
            case HTML_TITLE_OFF:
            case HTML_CAPTION_OFF:
                aTableName = comphelper::string::strip(aTableName, ' ');
                if(!aTableName.Len())
                    aTableName = String(::dbtools::createUniqueName(m_xTables,OUString(aTableName)));
                else
                    aTableName = aTempName;
                bCaption = sal_False;
                break;
            case HTML_FONT_ON:
                TableFontOn(aFont,nTextColor);
                break;
            case HTML_BOLD_ON:
                aFont.Weight = ::com::sun::star::awt::FontWeight::BOLD;
                break;
            case HTML_ITALIC_ON:
                aFont.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
                break;
            case HTML_UNDERLINE_ON:
                aFont.Underline = ::com::sun::star::awt::FontUnderline::SINGLE;
                break;
            case HTML_STRIKE_ON:
                aFont.Strikeout = ::com::sun::star::awt::FontStrikeout::SINGLE;
                break;
        }
    }
    while((nTmpToken2 = GetNextToken()) != HTML_TABLEROW_OFF);

    if ( m_sCurrent.Len() )
        aColumnName = m_sCurrent;
    aColumnName = comphelper::string::strip(aColumnName, ' ');
    if(aColumnName.Len())
        CreateDefaultColumn(aColumnName);

    if ( m_vDestVector.empty() )
        return sal_False;

    if(!aTableName.Len())
        aTableName = aTempName;

    m_bInTbl        = sal_False;
    m_bFoundTable   = sal_True;

    if ( isCheckEnabled() )
        return sal_True;

    return !executeWizard(aTableName,makeAny(nTextColor),aFont) && m_xTable.is();
}

void OHTMLReader::setTextEncoding()
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::setTextEncoding" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    m_bMetaOptions = sal_True;
    ParseMetaOptions(NULL, NULL);
}

void OHTMLReader::release()
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::release" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    ReleaseRef();
}

TypeSelectionPageFactory OHTMLReader::getTypeSelectionPageFactory()
{
    SAL_INFO("dbaccess.ui", "OHTMLReader::getTypeSelectionPageFactory" );
    DBG_CHKTHIS(OHTMLReader,NULL);
    return &OWizHTMLExtend::Create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
