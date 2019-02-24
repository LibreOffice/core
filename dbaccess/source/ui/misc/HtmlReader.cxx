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

#include <HtmlReader.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include <comphelper/string.hxx>
#include <strings.hrc>
#include <stringconstants.hxx>
#include <sfx2/sfxhtml.hxx>
#include <osl/diagnose.h>
#include <core_resource.hxx>
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
#include <WCopyTable.hxx>
#include <WExtendPages.hxx>
#include <WNameMatch.hxx>
#include <WColumnSelect.hxx>
#include <QEnumTypes.hxx>
#include <WCPage.hxx>
#include <rtl/tencinfo.h>
#include <UITools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

#define DBAUI_HTML_FONTSIZES    8       // like export, HTML-Options

// OHTMLReader
OHTMLReader::OHTMLReader(SvStream& rIn,const SharedConnection& _rxConnection,
                        const Reference< css::util::XNumberFormatter >& _rxNumberF,
                        const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    : HTMLParser(rIn)
    , ODatabaseExport( _rxConnection, _rxNumberF, _rxContext, rIn )
    , m_nTableCount(0)
    , m_nColumnWidth(87)
{
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( true );
}

OHTMLReader::OHTMLReader(SvStream& rIn,
                         sal_Int32 nRows,
                         const TPositions &_rColumnPositions,
                         const Reference< css::util::XNumberFormatter >& _rxNumberF,
                         const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                         const TColumnVector* pList,
                         const OTypeInfoMap* _pInfoMap,
                         bool _bAutoIncrementEnabled)
    : HTMLParser(rIn)
    , ODatabaseExport( nRows, _rColumnPositions, _rxNumberF, _rxContext, pList, _pInfoMap, _bAutoIncrementEnabled, rIn )
    , m_nTableCount(0)
    , m_nColumnWidth(87)
{
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( true );
}

OHTMLReader::~OHTMLReader()
{
}

SvParserState OHTMLReader::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    SvParserState  eParseState = HTMLParser::CallParser();
    SetColumnTypes(m_pColumnList,m_pInfoMap);
    return m_bFoundTable ? eParseState : SvParserState::Error;
}

#if defined _MSC_VER
#pragma warning(disable: 4702) // unreachable code, bug in MSVC2015
#endif
void OHTMLReader::NextToken( HtmlTokenId nToken )
{
    if(m_bError || !m_nRows) // if there is an error or no more rows to check, return immediately
        return;
    if ( nToken ==  HtmlTokenId::META )
        setTextEncoding();

    if(m_xConnection.is())    // names, which CTOR was called and hence, if a table should be created
    {
        switch(nToken)
        {
            case HtmlTokenId::TABLE_ON:
                ++m_nTableCount;
                {   // can also be TD or TH, if there was no TABLE before
                    const HTMLOptions& rHtmlOptions = GetOptions();
                    for (const auto & rOption : rHtmlOptions)
                    {
                        if( rOption.GetToken() == HtmlOptionId::WIDTH )
                        {   // percentage: of document width respectively outer cell
                            m_nColumnWidth = GetWidthPixel( rOption );
                        }
                    }
                }
                [[fallthrough]];
            case HtmlTokenId::THEAD_ON:
            case HtmlTokenId::TBODY_ON:
                {
                    sal_uInt64 const nTell = rInput.Tell(); // perhaps alters position of the stream
                    if ( !m_xTable.is() )
                    {// use first line as header
                        m_bError = !CreateTable(nToken);
                        if ( m_bAppendFirstLine )
                            rInput.Seek(nTell);
                    }
                }
                break;
            case HtmlTokenId::TABLE_OFF:
                if(!--m_nTableCount)
                {
                    m_xTable = nullptr;
                }
                break;
            case HtmlTokenId::TABLEROW_ON:
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
                    m_bError = true;
                break;
            case HtmlTokenId::TEXTTOKEN:
            case HtmlTokenId::SINGLECHAR:
                if ( m_bInTbl ) //&& !m_bSDNum ) // important, as otherwise we also get the names of the fonts
                    m_sTextToken += aToken;
                break;
            case HtmlTokenId::PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HtmlTokenId::PARABREAK_ON:
                m_sTextToken.clear();
                break;
            case HtmlTokenId::TABLEDATA_ON:
                fetchOptions();
                break;
            case HtmlTokenId::TABLEDATA_OFF:
                {
                    if ( !m_sCurrent.isEmpty() )
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
                    m_sCurrent.clear();
                    m_nColumnPos++;
                    eraseTokens();
                    m_bInTbl = false;
                }
                break;
            case HtmlTokenId::TABLEROW_OFF:
                if ( !m_pUpdateHelper.get() )
                {
                    m_bError = true;
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
            default: break;
        }
    }
    else // branch only valid for type checking
    {
        switch(nToken)
        {
            case HtmlTokenId::THEAD_ON:
            case HtmlTokenId::TBODY_ON:
                // The head of the column is not included
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != HtmlTokenId::TABLEROW_OFF);
                    m_bHead = false;
                }
                break;
            case HtmlTokenId::TABLEDATA_ON:
            case HtmlTokenId::TABLEHEADER_ON:
                fetchOptions();
                break;
            case HtmlTokenId::TEXTTOKEN:
            case HtmlTokenId::SINGLECHAR:
                if ( m_bInTbl ) // && !m_bSDNum ) // important, as otherwise we also get the names of the fonts
                    m_sTextToken += aToken;
                break;
            case HtmlTokenId::PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HtmlTokenId::PARABREAK_ON:
                m_sTextToken.clear();
                break;
            case HtmlTokenId::TABLEDATA_OFF:
                if ( !m_sCurrent.isEmpty() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
                m_nColumnPos++;
                m_bInTbl = false;
                m_sCurrent.clear();
                break;
            case HtmlTokenId::TABLEROW_OFF:
                if ( !m_sCurrent.isEmpty() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
                m_nColumnPos = 0;
                m_nRows--;
                m_sCurrent.clear();
                break;
            default: break;
        }
    }
}

void OHTMLReader::fetchOptions()
{
    m_bInTbl = true;
    const HTMLOptions& options = GetOptions();
    for (const auto & rOption : options)
    {
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::SDNUM:
                m_sNumToken = rOption.GetString();
            break;
            default: break;
        }
    }
}

void OHTMLReader::TableDataOn(SvxCellHorJustify& eVal)
{
    const HTMLOptions& rHtmlOptions = GetOptions();
    for (const auto & rOption : rHtmlOptions)
    {
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::ALIGN:
            {
                const OUString& rOptVal = rOption.GetString();
                if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right ))
                    eVal = SvxCellHorJustify::Right;
                else if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_center ))
                    eVal = SvxCellHorJustify::Center;
                else if (rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ))
                    eVal = SvxCellHorJustify::Left;
                else
                    eVal = SvxCellHorJustify::Standard;
            }
            break;
            default: break;
        }
    }
}

void OHTMLReader::TableFontOn(FontDescriptor& _rFont, Color &_rTextColor)
{
    const HTMLOptions& rHtmlOptions = GetOptions();
    for (const auto & rOption : rHtmlOptions)
    {
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::COLOR:
            {
                Color aColor;
                rOption.GetColor( aColor );
                _rTextColor = aColor.GetRGBColor();
            }
            break;
        case HtmlOptionId::FACE :
            {
                const OUString& rFace = rOption.GetString();
                OUStringBuffer aFontName;
                sal_Int32 nPos = 0;
                while( nPos != -1 )
                {
                    // list of fonts, VCL: semicolon as separator, HTML: comma
                    OUString aFName = rFace.getToken( 0, ',', nPos );
                    aFName = comphelper::string::strip(aFName, ' ');
                    if( !aFontName.isEmpty() )
                        aFontName.append(";");
                    aFontName.append(aFName);
                }
                if ( !aFontName.isEmpty() )
                    _rFont.Name = aFontName.makeStringAndClear();
            }
            break;
        case HtmlOptionId::SIZE :
            {
                sal_Int16 nSize = static_cast<sal_Int16>(rOption.GetNumber());
                if ( nSize == 0 )
                    nSize = 1;
                else if ( nSize < DBAUI_HTML_FONTSIZES )
                    nSize = DBAUI_HTML_FONTSIZES;

                _rFont.Height = nSize;
            }
            break;
        default: break;
        }
    }
}

sal_Int16 OHTMLReader::GetWidthPixel( const HTMLOption& rOption )
{
    const OUString& rOptVal = rOption.GetString();
    if ( rOptVal.indexOf('%') != -1 )
    {   // percentage
        OSL_ENSURE( m_nColumnWidth, "WIDTH Option: m_nColumnWidth==0 and Width%" );
        return static_cast<sal_Int16>((rOption.GetNumber() * m_nColumnWidth) / 100);
    }
    else
    {
        if ( rOptVal.indexOf('*') != -1 )
        {   // relative to what?!?
//TODO: collect ColArray of all relevant values and then MakeCol
            return 0;
        }
        else
            return static_cast<sal_Int16>(rOption.GetNumber());  // pixel
    }
}

bool OHTMLReader::CreateTable(HtmlTokenId nToken)
{
    OUString aTempName(DBA_RES(STR_TBL_TITLE));
    aTempName = aTempName.getToken(0,' ');
    aTempName = ::dbtools::createUniqueName(m_xTables, aTempName);

    bool bCaption = false;
    bool bTableHeader = false;
    OUString aColumnName;
    SvxCellHorJustify eVal;

    OUString aTableName;
    FontDescriptor aFont = VCLUnoHelper::CreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    Color nTextColor;
    do
    {
        switch (nToken)
        {
            case HtmlTokenId::TEXTTOKEN:
            case HtmlTokenId::SINGLECHAR:
                if(bTableHeader)
                    aColumnName += aToken;
                if(bCaption)
                    aTableName += aToken;
                break;
            case HtmlTokenId::PARABREAK_OFF:
                m_sCurrent += aColumnName;
                break;
            case HtmlTokenId::PARABREAK_ON:
                m_sTextToken.clear();
                break;
            case HtmlTokenId::TABLEDATA_ON:
            case HtmlTokenId::TABLEHEADER_ON:
                TableDataOn(eVal);
                bTableHeader = true;
                break;
            case HtmlTokenId::TABLEDATA_OFF:
            case HtmlTokenId::TABLEHEADER_OFF:
                {
                    aColumnName = comphelper::string::strip(aColumnName, ' ' );
                    if (aColumnName.isEmpty() || m_bAppendFirstLine )
                        aColumnName = DBA_RES(STR_COLUMN_NAME);
                    else if ( !m_sCurrent.isEmpty() )
                        aColumnName = m_sCurrent;

                    aColumnName = comphelper::string::strip(aColumnName, ' ');
                    CreateDefaultColumn(aColumnName);
                    aColumnName.clear();
                    m_sCurrent.clear();

                    eVal = SvxCellHorJustify::Standard;
                    bTableHeader = false;
                }
                break;

            case HtmlTokenId::TITLE_ON:
            case HtmlTokenId::CAPTION_ON:
                bCaption = true;
                break;
            case HtmlTokenId::TITLE_OFF:
            case HtmlTokenId::CAPTION_OFF:
                aTableName = comphelper::string::strip(aTableName, ' ');
                if(aTableName.isEmpty())
                    aTableName = ::dbtools::createUniqueName(m_xTables, aTableName);
                else
                    aTableName = aTempName;
                bCaption = false;
                break;
            case HtmlTokenId::FONT_ON:
                TableFontOn(aFont,nTextColor);
                break;
            case HtmlTokenId::BOLD_ON:
                aFont.Weight = css::awt::FontWeight::BOLD;
                break;
            case HtmlTokenId::ITALIC_ON:
                aFont.Slant = css::awt::FontSlant_ITALIC;
                break;
            case HtmlTokenId::UNDERLINE_ON:
                aFont.Underline = css::awt::FontUnderline::SINGLE;
                break;
            case HtmlTokenId::STRIKE_ON:
                aFont.Strikeout = css::awt::FontStrikeout::SINGLE;
                break;
            default: break;
        }
        nToken = GetNextToken();
    }
    while (nToken != HtmlTokenId::TABLEROW_OFF);

    if ( !m_sCurrent.isEmpty() )
        aColumnName = m_sCurrent;
    aColumnName = comphelper::string::strip(aColumnName, ' ');
    if(!aColumnName.isEmpty())
        CreateDefaultColumn(aColumnName);

    if ( m_vDestVector.empty() )
        return false;

    if(aTableName.isEmpty())
        aTableName = aTempName;

    m_bInTbl        = false;
    m_bFoundTable   = true;

    if ( isCheckEnabled() )
        return true;

    return !executeWizard(aTableName,makeAny(nTextColor),aFont) && m_xTable.is();
}

void OHTMLReader::setTextEncoding()
{
    ParseMetaOptions(nullptr, nullptr);
}

TypeSelectionPageFactory OHTMLReader::getTypeSelectionPageFactory()
{
    return &OWizHTMLExtend::Create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
