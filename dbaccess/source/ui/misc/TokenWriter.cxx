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

#include <TokenWriter.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <RtfReader.hxx>
#include <HtmlReader.hxx>
#include <strings.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <svtools/htmlkywd.hxx>
#include <svtools/rtfkeywd.hxx>
#include <tools/color.hxx>
#include <svtools/htmlout.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <svl/numuno.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <UITools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/htmlcfg.hxx>
#include <o3tl/string_view.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <memory>

using namespace dbaui;
using namespace dbtools;
using namespace svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::util;

#define CELL_X                      1437

ODatabaseImportExport::ODatabaseImportExport(const svx::ODataAccessDescriptor& _aDataDescriptor,
                                             const Reference< XComponentContext >& _rM,
                                             const Reference< css::util::XNumberFormatter >& _rxNumberF)
    :m_bBookmarkSelection( false )
    ,m_pStream(nullptr)
    ,m_xFormatter(_rxNumberF)
    ,m_xContext(_rM)
    ,m_nCommandType(CommandType::TABLE)
    ,m_bNeedToReInitialize(false)
    ,m_bInInitialize(false)
    ,m_bCheckOnly(false)
{
    m_eDestEnc = osl_getThreadTextEncoding();

    osl_atomic_increment( &m_refCount );
    impl_initFromDescriptor( _aDataDescriptor, false );
    osl_atomic_decrement( &m_refCount );
}

// import data
ODatabaseImportExport::ODatabaseImportExport( ::dbtools::SharedConnection _xConnection,
        const Reference< XNumberFormatter >& _rxNumberF, const Reference< XComponentContext >& _rM )
    :m_bBookmarkSelection( false )
    ,m_pStream(nullptr)
    ,m_xConnection(std::move(_xConnection))
    ,m_xFormatter(_rxNumberF)
    ,m_xContext(_rM)
    ,m_nCommandType(css::sdb::CommandType::TABLE)
    ,m_bNeedToReInitialize(false)
    ,m_bInInitialize(false)
    ,m_bCheckOnly(false)
{
    m_eDestEnc = osl_getThreadTextEncoding();
}

ODatabaseImportExport::~ODatabaseImportExport()
{
    acquire();
    dispose();
}

void ODatabaseImportExport::dispose()
{
    // remove me as listener
    Reference< XComponent >  xComponent(m_xConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference< XEventListener> xEvt(this);
        xComponent->removeEventListener(xEvt);
    }
    m_xConnection.clear();

    ::comphelper::disposeComponent(m_xRow);

    m_xObject.clear();
    m_xResultSetMetaData.clear();
    m_xResultSet.clear();
    m_xRow.clear();
    m_xRowLocate.clear();
    m_xFormatter.clear();
    m_xRowSetColumns.clear();
}

void SAL_CALL ODatabaseImportExport::disposing( const EventObject& Source )
{
    Reference<XConnection> xCon(Source.Source,UNO_QUERY);
    if(m_xConnection.is() && m_xConnection == xCon)
    {
        m_xConnection.clear();
        dispose();
        m_bNeedToReInitialize = true;
    }
}

void ODatabaseImportExport::initialize( const ODataAccessDescriptor& _aDataDescriptor )
{
    impl_initFromDescriptor( _aDataDescriptor, true );
}

void ODatabaseImportExport::impl_initFromDescriptor( const ODataAccessDescriptor& _aDataDescriptor, bool _bPlusDefaultInit)
{
    if ( !_bPlusDefaultInit )
    {
        m_sDataSourceName = _aDataDescriptor.getDataSource();
        _aDataDescriptor[DataAccessDescriptorProperty::CommandType] >>= m_nCommandType;
        _aDataDescriptor[DataAccessDescriptorProperty::Command]     >>= m_sName;
        // some additional information
        if(_aDataDescriptor.has(DataAccessDescriptorProperty::Connection))
        {
            Reference< XConnection > xPureConn( _aDataDescriptor[DataAccessDescriptorProperty::Connection], UNO_QUERY );
            m_xConnection.reset( xPureConn, SharedConnection::NoTakeOwnership );
            Reference< XEventListener> xEvt(this);
            Reference< XComponent >  xComponent(m_xConnection, UNO_QUERY);
            if (xComponent.is() && xEvt.is())
                xComponent->addEventListener(xEvt);
        }

        if ( _aDataDescriptor.has( DataAccessDescriptorProperty::Selection ) )
            _aDataDescriptor[ DataAccessDescriptorProperty::Selection ] >>= m_aSelection;

        if ( _aDataDescriptor.has( DataAccessDescriptorProperty::BookmarkSelection ) )
            _aDataDescriptor[ DataAccessDescriptorProperty::BookmarkSelection ] >>= m_bBookmarkSelection;

        if ( _aDataDescriptor.has( DataAccessDescriptorProperty::Cursor ) )
        {
            _aDataDescriptor[ DataAccessDescriptorProperty::Cursor ] >>= m_xResultSet;
            m_xRowLocate.set( m_xResultSet, UNO_QUERY );
        }

        if ( m_aSelection.hasElements() )
        {
            if ( !m_xResultSet.is() )
            {
                SAL_WARN("dbaccess.ui", "ODatabaseImportExport::impl_initFromDescriptor: selection without result set is nonsense!" );
                m_aSelection.realloc( 0 );
            }
        }

        if ( m_aSelection.hasElements() )
        {
            if ( m_bBookmarkSelection && !m_xRowLocate.is() )
            {
                SAL_WARN("dbaccess.ui", "ODatabaseImportExport::impl_initFromDescriptor: no XRowLocate -> no bookmarks!" );
                m_aSelection.realloc( 0 );
            }
        }
    }
    else
        initialize();
}

void ODatabaseImportExport::initialize()
{
    m_bInInitialize = true;
    m_bNeedToReInitialize = false;

    if ( !m_xConnection.is() )
    {   // we need a connection
        OSL_ENSURE(!m_sDataSourceName.isEmpty(),"There must be a datsource name!");
        Reference<XNameAccess> xDatabaseContext( DatabaseContext::create(m_xContext), UNO_QUERY_THROW);
        Reference< XEventListener> xEvt(this);

        Reference< XConnection > xConnection;
        SQLExceptionInfo aInfo = ::dbaui::createConnection( m_sDataSourceName, xDatabaseContext, m_xContext, xEvt, xConnection );
        m_xConnection.reset( xConnection );

        if(aInfo.isValid() && aInfo.getType() == SQLExceptionInfo::TYPE::SQLException)
            throw *static_cast<const SQLException*>(aInfo);
    }

    Reference<XNameAccess> xNameAccess;
    switch(m_nCommandType)
    {
        case CommandType::TABLE:
            {
                // only for tables
                Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY);
                if(xSup.is())
                    xNameAccess = xSup->getTables();
            }
            break;
        case CommandType::QUERY:
            {
                Reference<XQueriesSupplier> xSup(m_xConnection,UNO_QUERY);
                if(xSup.is())
                    xNameAccess = xSup->getQueries();
            }
            break;
    }
    if(xNameAccess.is() && xNameAccess->hasByName(m_sName))
    {
        xNameAccess->getByName(m_sName) >>= m_xObject;
    }

    if(m_xObject.is())
    {
        try
        {
            if(m_xObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_FONT))
                m_xObject->getPropertyValue(PROPERTY_FONT) >>= m_aFont;

            // the result set may be already set with the datadescriptor
            if ( !m_xResultSet.is() )
            {
                m_xResultSet.set( m_xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.sdb.RowSet"_ustr, m_xContext), UNO_QUERY );
                Reference< XPropertySet > xProp( m_xResultSet, UNO_QUERY_THROW );
                xProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, Any( m_xConnection.getTyped() ) );
                xProp->setPropertyValue( PROPERTY_COMMAND_TYPE, Any( m_nCommandType ) );
                xProp->setPropertyValue( PROPERTY_COMMAND, Any( m_sName ) );
                Reference< XRowSet > xRowSet( xProp, UNO_QUERY );
                xRowSet->execute();
            }
            if ( !m_xRow.is() && m_xResultSet.is() )
            {
                m_xRow.set( m_xResultSet, UNO_QUERY );
                m_xRowLocate.set( m_xResultSet, UNO_QUERY );
                m_xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(m_xRow,UNO_QUERY_THROW)->getMetaData();
                Reference<XColumnsSupplier> xSup(m_xResultSet,UNO_QUERY_THROW);
                m_xRowSetColumns.set(xSup->getColumns(),UNO_QUERY_THROW);
            }
        }
        catch(Exception& )
        {
            m_xRow = nullptr;
            m_xResultSetMetaData = nullptr;
            ::comphelper::disposeComponent(m_xResultSet);
            throw;
        }
    }
    if ( m_aFont.Name.isEmpty() )
    {
        vcl::Font aApplicationFont = OutputDevice::GetDefaultFont(
            DefaultFontType::SANS_UNICODE,
            Application::GetSettings().GetUILanguageTag().getLanguageType(),
            GetDefaultFontFlags::OnlyOne
        );
        m_aFont = VCLUnoHelper::CreateFontDescriptor( aApplicationFont );
    }

    m_bInInitialize = false;
}

bool ODatabaseImportExport::Write()
{
    if ( m_bNeedToReInitialize )
    {
        if ( !m_bInInitialize )
            initialize();
    }
    return true;
}

bool ODatabaseImportExport::Read()
{
    if ( m_bNeedToReInitialize )
    {
        if ( !m_bInInitialize )
            initialize();
    }
    return true;
}

bool ORTFImportExport::Write()
{
    ODatabaseImportExport::Write();
    m_pStream->WriteChar( '{' ).WriteOString( OOO_STRING_SVTOOLS_RTF_RTF );
    m_pStream->WriteOString(OOO_STRING_SVTOOLS_RTF_ANSI);
    if (sal_uInt32 nCpg = rtl_getWindowsCodePageFromTextEncoding(m_eDestEnc); nCpg && nCpg != 65001)
    {
        m_pStream->WriteOString(OOO_STRING_SVTOOLS_RTF_ANSICPG).WriteNumberAsString(nCpg);
    }
    m_pStream->WriteOString(SAL_NEWLINE_STRING);

    bool bBold          = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    bool bItalic        = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    bool bUnderline     = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    bool bStrikeout     = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    ::Color aColor;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= aColor;

    OString aFonts(OUStringToOString(m_aFont.Name, RTL_TEXTENCODING_MS_1252));
    if (aFonts.isEmpty())
    {
        OUString aName = Application::GetSettings().GetStyleSettings().GetAppFont().GetFamilyName();
        aFonts = OUStringToOString(aName, RTL_TEXTENCODING_MS_1252);
    }

    m_pStream->WriteOString( "{\\fonttbl" );
    if (!aFonts.isEmpty())
    {
        sal_Int32 nIdx{0};
        sal_Int32 nTok{-1}; // to compensate pre-increment
        do {
            m_pStream->WriteOString( "\\f" );
            m_pStream->WriteNumberAsString(++nTok);
            m_pStream->WriteOString( "\\fcharset0\\fnil " );
            m_pStream->WriteOString( o3tl::getToken(aFonts, 0, ';', nIdx) );
            m_pStream->WriteChar( ';' );
        } while (nIdx>=0);
    }
    m_pStream->WriteChar( '}' ) ;
    m_pStream->WriteOString( SAL_NEWLINE_STRING );
    // write the rtf color table
    m_pStream->WriteChar( '{' ).WriteOString( OOO_STRING_SVTOOLS_RTF_COLORTBL ).WriteOString( OOO_STRING_SVTOOLS_RTF_RED );
    m_pStream->WriteNumberAsString(aColor.GetRed());
    m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_GREEN );
    m_pStream->WriteNumberAsString(aColor.GetGreen());
    m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_BLUE );
    m_pStream->WriteNumberAsString(aColor.GetBlue());

    m_pStream->WriteOString( ";\\red255\\green255\\blue255;\\red192\\green192\\blue192;}" )
                .WriteOString( SAL_NEWLINE_STRING );

    static char const aCell1[] = "\\clbrdrl\\brdrs\\brdrcf0\\clbrdrt\\brdrs\\brdrcf0\\clbrdrb\\brdrs\\brdrcf0\\clbrdrr\\brdrs\\brdrcf0\\clshdng10000\\clcfpat2\\cellx";

    m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteOString( OOO_STRING_SVTOOLS_RTF_TRGAPH );
    m_pStream->WriteOString("40");
    m_pStream->WriteOString( SAL_NEWLINE_STRING );

    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< OUString> aNames(xColumns->getElementNames());

        sal_Int32 nCount = aNames.getLength();
        bool bUseResultMetaData = false;
        if ( !nCount )
        {
            nCount = m_xResultSetMetaData->getColumnCount();
            bUseResultMetaData = true;
        }

        for( sal_Int32 i=1; i<=nCount; ++i )
        {
            m_pStream->WriteOString( aCell1 );
            m_pStream->WriteNumberAsString(i*CELL_X);
            m_pStream->WriteOString( SAL_NEWLINE_STRING );
        }

        // column description
        m_pStream->WriteChar( '{' ).WriteOString( SAL_NEWLINE_STRING );
        m_pStream->WriteOString( "\\trrh-270\\pard\\intbl" );

        std::unique_ptr<OString[]> pHorzChar(new OString[nCount]);

        for ( sal_Int32 i=1; i <= nCount; ++i )
        {
            sal_Int32 nAlign = 0;
            OUString sColumnName;
            if ( bUseResultMetaData )
                sColumnName = m_xResultSetMetaData->getColumnName(i);
            else
            {
                sColumnName = aNames[i];
                Reference<XPropertySet> xColumn;
                xColumns->getByName(sColumnName) >>= xColumn;
                xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;
            }

            const char* pChar;
            switch( nAlign )
            {
                case 1: pChar = OOO_STRING_SVTOOLS_RTF_QC;  break;
                case 2: pChar = OOO_STRING_SVTOOLS_RTF_QR;  break;
                case 0:
                default:pChar = OOO_STRING_SVTOOLS_RTF_QL;  break;
            }

            pHorzChar[i-1] = pChar; // to avoid to always rummage in the ITEMSET later on

            m_pStream->WriteOString( SAL_NEWLINE_STRING );
            m_pStream->WriteChar( '{' );
            m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_QC );   // column header always centered

            if ( bBold )        m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_B );
            if ( bItalic )      m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_I );
            if ( bUnderline )   m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_UL );
            if ( bStrikeout )   m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_STRIKE );

            m_pStream->WriteOString( "\\fs20\\f0\\cf0\\cb2" );
            m_pStream->WriteChar( ' ' );
            RTFOutFuncs::Out_String(*m_pStream, sColumnName, m_eDestEnc);

            m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_CELL );
            m_pStream->WriteChar( '}' );
            m_pStream->WriteOString( SAL_NEWLINE_STRING );
            m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_PARD ).WriteOString( OOO_STRING_SVTOOLS_RTF_INTBL );
        }

        m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_ROW );
        m_pStream->WriteOString( SAL_NEWLINE_STRING ).WriteChar( '}' );
        m_pStream->WriteOString( SAL_NEWLINE_STRING );

        sal_Int32 k=1;
        sal_Int32 kk=0;
        if ( m_aSelection.hasElements() )
        {
            for (auto& any : m_aSelection)
            {
                if ( m_bBookmarkSelection )
                {
                    if (!m_xRowLocate->moveToBookmark(any))
                        break;
                }
                else
                {
                    sal_Int32 nPos = -1;
                    OSL_VERIFY(any >>= nPos);
                    if (!m_xResultSet->absolute(nPos))
                        break;
                }

                appendRow(pHorzChar.get(), nCount, k, kk);
            }
        }
        else
        {
            m_xResultSet->beforeFirst(); // set back before the first row
            while(m_xResultSet->next())
            {
                appendRow(pHorzChar.get(),nCount,k,kk);
            }
        }
    }

    m_pStream->WriteChar( '}' ).WriteOString( SAL_NEWLINE_STRING );
    m_pStream->WriteUChar( 0 );
    return ((*m_pStream).GetError() == ERRCODE_NONE);
}

void ORTFImportExport::appendRow(OString const * pHorzChar,sal_Int32 _nColumnCount,sal_Int32& k,sal_Int32& kk)
{
    ++kk;
    m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteOString( OOO_STRING_SVTOOLS_RTF_TRGAPH );
    m_pStream->WriteOString("40");
    m_pStream->WriteOString( SAL_NEWLINE_STRING );

    static char const aCell2[] = "\\clbrdrl\\brdrs\\brdrcf2\\clbrdrt\\brdrs\\brdrcf2\\clbrdrb\\brdrs\\brdrcf2\\clbrdrr\\brdrs\\brdrcf2\\clshdng10000\\clcfpat1\\cellx";

    for ( sal_Int32 i=1; i<=_nColumnCount; ++i )
    {
        m_pStream->WriteOString( aCell2 );
        m_pStream->WriteNumberAsString(i*CELL_X);
        m_pStream->WriteOString( SAL_NEWLINE_STRING );
    }

    const bool bBold            = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    const bool bItalic      = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    const bool bUnderline       = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    const bool bStrikeout       = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );
    Reference< XRowSet > xRowSet(m_xRow,UNO_QUERY);

    m_pStream->WriteChar( '{' );
    m_pStream->WriteOString( "\\trrh-270\\pard\\intbl" );
    for ( sal_Int32 i=1; i <= _nColumnCount; ++i )
    {
        m_pStream->WriteOString( SAL_NEWLINE_STRING );
        m_pStream->WriteChar( '{' );
        m_pStream->WriteOString( pHorzChar[i-1] );

        if ( bBold )        m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_B );
        if ( bItalic )      m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_I );
        if ( bUnderline )   m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_UL );
        if ( bStrikeout )   m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_STRIKE );

        m_pStream->WriteOString( "\\fs20\\f1\\cf0\\cb1 " );

        try
        {
            Reference<XPropertySet> xColumn(m_xRowSetColumns->getByIndex(i-1),UNO_QUERY_THROW);
            dbtools::FormattedColumnValue aFormatedValue(m_xContext,xRowSet,xColumn);
            OUString sValue = aFormatedValue.getFormattedValue();
            if ( !sValue.isEmpty() )
                RTFOutFuncs::Out_String(*m_pStream,sValue,m_eDestEnc);
        }
        catch (Exception&)
        {
            SAL_WARN("dbaccess.ui","RTF WRITE!");
        }

        m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_CELL );
        m_pStream->WriteChar( '}' );
        m_pStream->WriteOString( SAL_NEWLINE_STRING );
        m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_PARD ).WriteOString( OOO_STRING_SVTOOLS_RTF_INTBL );
    }
    m_pStream->WriteOString( OOO_STRING_SVTOOLS_RTF_ROW ).WriteOString( SAL_NEWLINE_STRING );
    m_pStream->WriteChar( '}' );
    ++k;
}

bool ORTFImportExport::Read()
{
    ODatabaseImportExport::Read();
    SvParserState eState = SvParserState::Error;
    if ( m_pStream )
    {
        tools::SvRef<ORTFReader> xReader(new ORTFReader((*m_pStream),m_xConnection,m_xFormatter,m_xContext));
        if ( isCheckEnabled() )
            xReader->enableCheckOnly();
        eState = xReader->CallParser();
    }

    return eState != SvParserState::Error;
}

const sal_Int16 OHTMLImportExport::nCellSpacing = 0;
const char OHTMLImportExport::sIndentSource[nIndentMax+1] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

OHTMLImportExport::OHTMLImportExport(const svx::ODataAccessDescriptor& _aDataDescriptor,
                                     const Reference< XComponentContext >& _rM,
                                     const Reference< css::util::XNumberFormatter >& _rxNumberF)
        : ODatabaseImportExport(_aDataDescriptor,_rM,_rxNumberF)
    ,m_nIndent(0)
#if OSL_DEBUG_LEVEL > 0
    ,m_bCheckFont(false)
#endif
{
    // set HTML configuration
    m_eDestEnc = RTL_TEXTENCODING_UTF8;
    strncpy( sIndent, sIndentSource ,std::min(sizeof(sIndent),sizeof(sIndentSource)));
    sIndent[0] = 0;
}

bool OHTMLImportExport::Write()
{
    ODatabaseImportExport::Write();
    if(m_xObject.is())
    {
        m_pStream->WriteChar( '<' ).WriteOString( OOO_STRING_SVTOOLS_HTML_doctype ).WriteChar( ' ' ).WriteOString( OOO_STRING_SVTOOLS_HTML_doctype5 ).WriteChar( '>' ).WriteOString( SAL_NEWLINE_STRING ).WriteOString( SAL_NEWLINE_STRING );
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_html).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        WriteHeader();
        m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        WriteBody();
        m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_html, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

        return ((*m_pStream).GetError() == ERRCODE_NONE);
    }
    return false;
}

bool OHTMLImportExport::Read()
{
    ODatabaseImportExport::Read();
    SvParserState eState = SvParserState::Error;
    if ( m_pStream )
    {
        tools::SvRef<OHTMLReader> xReader(new OHTMLReader((*m_pStream),m_xConnection,m_xFormatter,m_xContext));
        if ( isCheckEnabled() )
            xReader->enableCheckOnly();
        xReader->SetTableName(m_sDefaultTableName);
        eState = xReader->CallParser();
    }

    return eState != SvParserState::Error;
}

void OHTMLImportExport::WriteHeader()
{
    uno::Reference<document::XDocumentProperties> xDocProps(
        document::DocumentProperties::create( m_xContext ) );
    if (xDocProps.is()) {
        xDocProps->setTitle(m_sName);
    }

    IncIndent(1);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_head).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

    SfxFrameHTMLWriter::Out_DocInfo( (*m_pStream), OUString(),
        xDocProps, sIndent );
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    IncIndent(-1);
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_head, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
}

void OHTMLImportExport::WriteBody()
{
    IncIndent(1);
    m_pStream->WriteOString( "<" ).WriteOString( OOO_STRING_SVTOOLS_HTML_style ).WriteOString( " " ).WriteOString( OOO_STRING_SVTOOLS_HTML_O_type ).WriteOString( "=\"text/css\">" );

    m_pStream->WriteOString( "<!-- " );
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    m_pStream->WriteOString( OOO_STRING_SVTOOLS_HTML_body ).WriteOString( " { " ).WriteOString( "font-family: " ).WriteChar( '"' ).WriteOString( OUStringToOString(m_aFont.Name, osl_getThreadTextEncoding()) ).WriteChar( '\"' );
        // TODO : think about the encoding of the font name
    m_pStream->WriteOString( "; " ).WriteOString( "font-size: " );
    m_pStream->WriteNumberAsString(m_aFont.Height);
    m_pStream->WriteChar( '}' );

    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    m_pStream->WriteOString( " -->" );
    IncIndent(-1);
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_style, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

    // default Textcolour black
    m_pStream->WriteChar( '<' ).WriteOString( OOO_STRING_SVTOOLS_HTML_body ).WriteChar( ' ' ).WriteOString( OOO_STRING_SVTOOLS_HTML_O_text ).WriteChar( '=' );
    ::Color aColor;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= aColor;
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    m_pStream->WriteOString( " " OOO_STRING_SVTOOLS_HTML_O_bgcolor "=" );
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    m_pStream->WriteChar( '>' );
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

    WriteTables();

    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_body, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
}

void OHTMLImportExport::WriteTables()
{
    OString aStrOut  = OOO_STRING_SVTOOLS_HTML_table
            " "
            OOO_STRING_SVTOOLS_HTML_frame
            "="
            OOO_STRING_SVTOOLS_HTML_TF_void ""_ostr;

    Sequence< OUString> aNames;
    Reference<XNameAccess> xColumns;
    bool bUseResultMetaData = false;
    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        xColumns = xColSup->getColumns();
        aNames = xColumns->getElementNames();
        if ( !aNames.hasElements() )
        {
            sal_Int32 nCount = m_xResultSetMetaData->getColumnCount();
            aNames.realloc(nCount);
            auto aNamesRange = asNonConstRange(aNames);
            for (sal_Int32 i= 0; i < nCount; ++i)
                aNamesRange[i] = m_xResultSetMetaData->getColumnName(i+1);
            bUseResultMetaData = true;
        }
    }

    aStrOut += " "
            OOO_STRING_SVTOOLS_HTML_O_align
            "="
            OOO_STRING_SVTOOLS_HTML_AL_left
            " "
            OOO_STRING_SVTOOLS_HTML_O_cellspacing
            "=" +
            OString::number(nCellSpacing) +
            " "
            OOO_STRING_SVTOOLS_HTML_O_cols
            "=" +
            OString::number(aNames.getLength()) +
            " "
            OOO_STRING_SVTOOLS_HTML_O_border
            "=1";

    IncIndent(1);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, aStrOut);

    FontOn();

    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_caption);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_bold);

    m_pStream->WriteOString( OUStringToOString(m_sName, osl_getThreadTextEncoding()) );
        // TODO : think about the encoding of the name
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_bold, false);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_caption, false);

    FontOff();
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    // </FONT>

    IncIndent(1);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_thead).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

    IncIndent(1);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tablerow).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

    if(m_xObject.is())
    {
        std::unique_ptr<sal_Int32[]> pFormat(new sal_Int32[aNames.getLength()]);

        std::unique_ptr<const char *[]> pHorJustify(new const char*[aNames.getLength()]);
        std::unique_ptr<sal_Int32[]> pColWidth(new sal_Int32[aNames.getLength()]);

        sal_Int32 nHeight = 0;
        m_xObject->getPropertyValue(PROPERTY_ROW_HEIGHT) >>= nHeight;

        // 1. writing the column description
        for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        {
            sal_Int32 nAlign = 0;
            pFormat[i] = 0;
            pColWidth[i] = 100;
            if ( !bUseResultMetaData )
            {
                Reference<XPropertySet> xColumn;
                xColumns->getByName(aNames[i]) >>= xColumn;
                xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;
                pFormat[i] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_FORMATKEY));
                pColWidth[i] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_WIDTH));
            }

            switch( nAlign )
            {
                case 1:     pHorJustify[i] = OOO_STRING_SVTOOLS_HTML_AL_center; break;
                case 2:     pHorJustify[i] = OOO_STRING_SVTOOLS_HTML_AL_right;  break;
                default:    pHorJustify[i] = OOO_STRING_SVTOOLS_HTML_AL_left;       break;
            }

            if(i == aNames.getLength()-1)
                IncIndent(-1);

            WriteCell(pFormat[i],pColWidth[i],nHeight,pHorJustify[i],aNames[i],OOO_STRING_SVTOOLS_HTML_tableheader);
        }

        IncIndent(-1);
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tablerow, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_thead, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

        IncIndent(1);
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tbody).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

        // 2. and now the data
        Reference< XRowSet > xRowSet(m_xRow,UNO_QUERY);
        m_xResultSet->beforeFirst(); // set back before the first row
        while(m_xResultSet->next())
        {
            IncIndent(1);
            HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tablerow).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

            for(sal_Int32 i=1;i<=aNames.getLength();++i)
            {
                if(i == aNames.getLength())
                    IncIndent(-1);

                OUString aValue;
                try
                {
                    Reference<XPropertySet> xColumn(m_xRowSetColumns->getByIndex(i-1),UNO_QUERY_THROW);
                    dbtools::FormattedColumnValue aFormatedValue(m_xContext,xRowSet,xColumn);
                    OUString sValue = aFormatedValue.getFormattedValue();
                    if (!sValue.isEmpty())
                    {
                        aValue = sValue;
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
                WriteCell(pFormat[i-1],pColWidth[i-1],nHeight,pHorJustify[i-1],aValue,OOO_STRING_SVTOOLS_HTML_tabledata);
            }
            HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tablerow, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        }
    }
    else
    {
        IncIndent(-1);
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tablerow, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_thead, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());

        IncIndent(1);
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tbody).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    }

    IncIndent(-1);
    m_pStream->WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_tbody, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
    IncIndent(-1);
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_table, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
}

void OHTMLImportExport::WriteCell( sal_Int32 nFormat, sal_Int32 nWidthPixel, sal_Int32 nHeightPixel, const char* pChar,
                                   const OUString& rValue, const char* pHtmlTag)
{
    OString aStrTD = pHtmlTag;

    nWidthPixel  = nWidthPixel  ? nWidthPixel   : 86;
    nHeightPixel = nHeightPixel ? nHeightPixel  : 17;

    // despite the <TABLE COLS=n> and <COL WIDTH=x> designation necessary,
    // as Netscape is not paying attention to them.
    // column width
    aStrTD += " "
            OOO_STRING_SVTOOLS_HTML_O_width
            "=" +
            OString::number(nWidthPixel) +
    // line height
            " "
            OOO_STRING_SVTOOLS_HTML_O_height
            "=" +
            OString::number(nHeightPixel) +
            " "
            OOO_STRING_SVTOOLS_HTML_O_align
            "=" +
            pChar;

    SvNumberFormatsSupplierObj* pSupplierImpl = m_xFormatter.is() ? comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>(m_xFormatter->getNumberFormatsSupplier()) : nullptr;
    SvNumberFormatter* pFormatter = pSupplierImpl ? pSupplierImpl->GetNumberFormatter() : nullptr;
    if(pFormatter)
    {
        double fVal = 0.0;

        try
        {
            fVal = m_xFormatter->convertStringToNumber(nFormat,rValue);
            HTMLOutFuncs::CreateTableDataOptionsValNum(false, fVal,nFormat, *pFormatter);
        }
        catch(const Exception&)
        {
            HTMLOutFuncs::CreateTableDataOptionsValNum(false, fVal,nFormat, *pFormatter);
        }
    }

    HTMLOutFuncs::Out_AsciiTag(*m_pStream, aStrTD);

    FontOn();

    bool bBold          = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    bool bItalic        = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    bool bUnderline     = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    bool bStrikeout     = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    if ( bBold )        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_bold);
    if ( bItalic )      HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_italic);
    if ( bUnderline )   HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_underline);
    if ( bStrikeout )   HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_strike);

    if ( rValue.isEmpty() )
        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_linebreak);        // no completely empty cell
    else
        HTMLOutFuncs::Out_String( (*m_pStream), rValue );

    if ( bStrikeout )   HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_strike, false);
    if ( bUnderline )   HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_underline, false);
    if ( bItalic )      HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_italic, false);
    if ( bBold )        HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_bold, false);

    FontOff();

    HTMLOutFuncs::Out_AsciiTag(*m_pStream, pHtmlTag, false).WriteOString(SAL_NEWLINE_STRING).WriteOString(GetIndentStr());
}

void OHTMLImportExport::FontOn()
{
#if OSL_DEBUG_LEVEL > 0
    m_bCheckFont = true;
#endif

    // <FONT FACE="xxx">
    OString aStrOut  = "<"
            OOO_STRING_SVTOOLS_HTML_font
            " "
            OOO_STRING_SVTOOLS_HTML_O_face
            "="
            "\"" +
            OUStringToOString(m_aFont.Name,osl_getThreadTextEncoding()) +
        // TODO : think about the encoding of the font name
            "\""
            " "
            OOO_STRING_SVTOOLS_HTML_O_color
            "=";
    m_pStream->WriteOString( aStrOut );

    ::Color aColor;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= aColor;

    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );
    m_pStream->WriteOString( ">" );
}

inline void OHTMLImportExport::FontOff()
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(m_bCheckFont,"No FontOn() called");
#endif
    HTMLOutFuncs::Out_AsciiTag(*m_pStream, OOO_STRING_SVTOOLS_HTML_font, false);
#if OSL_DEBUG_LEVEL > 0
    m_bCheckFont = false;
#endif
}

void OHTMLImportExport::IncIndent( sal_Int16 nVal )
{
    sIndent[m_nIndent] = '\t';
    m_nIndent = m_nIndent + nVal;
    if ( m_nIndent < 0 )
        m_nIndent = 0;
    else if ( m_nIndent > nIndentMax )
        m_nIndent = nIndentMax;
    sIndent[m_nIndent] = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
