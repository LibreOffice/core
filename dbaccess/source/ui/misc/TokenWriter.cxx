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
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/stream.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <RtfReader.hxx>
#include <HtmlReader.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
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
#include <vcl/svapp.hxx>
#include <UITools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/htmlcfg.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <unotools/syslocale.hxx>
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
ODatabaseImportExport::ODatabaseImportExport( const ::dbtools::SharedConnection& _rxConnection,
        const Reference< XNumberFormatter >& _rxNumberF, const Reference< XComponentContext >& _rM )
    :m_bBookmarkSelection( false )
    ,m_pStream(nullptr)
    ,m_xConnection(_rxConnection)
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
        Reference< XEventListener> xEvt(static_cast<cppu::OWeakObject*>(this),UNO_QUERY);
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
            Reference< XEventListener> xEvt(static_cast<cppu::OWeakObject*>(this),UNO_QUERY);
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

        if ( m_aSelection.getLength() != 0 )
        {
            if ( !m_xResultSet.is() )
            {
                SAL_WARN("dbaccess.ui", "ODatabaseImportExport::impl_initFromDescriptor: selection without result set is nonsense!" );
                m_aSelection.realloc( 0 );
            }
        }

        if ( m_aSelection.getLength() != 0 )
        {
            if ( m_bBookmarkSelection && !m_xRowLocate.is() )
            {
                SAL_WARN("dbaccess.ui", "ODatabaseImportExport::impl_initFromDescriptor: no XRowLocate -> no bookmars!" );
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
        Reference< XEventListener> xEvt(static_cast<cppu::OWeakObject*>(this),UNO_QUERY);

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
                m_xResultSet.set( m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.sdb.RowSet", m_xContext), UNO_QUERY );
                Reference< XPropertySet > xProp( m_xResultSet, UNO_QUERY_THROW );
                xProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( m_xConnection.getTyped() ) );
                xProp->setPropertyValue( PROPERTY_COMMAND_TYPE, makeAny( m_nCommandType ) );
                xProp->setPropertyValue( PROPERTY_COMMAND, makeAny( m_sName ) );
                Reference< XRowSet > xRowSet( xProp, UNO_QUERY );
                xRowSet->execute();
            }
            if ( !m_xRow.is() && m_xResultSet.is() )
            {
                m_xRow.set( m_xResultSet, UNO_QUERY );
                m_xRowLocate.set( m_xResultSet, UNO_QUERY );
                m_xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(m_xRow,UNO_QUERY)->getMetaData();
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
    m_pStream->WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RTF );
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ANSI ).WriteCharPtr( SAL_NEWLINE_STRING );
    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252;

    bool bBold          = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    bool bItalic        = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    bool bUnderline     = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    bool bStrikeout     = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    ::Color aColor(nColor);

    OString aFonts(OUStringToOString(m_aFont.Name, eDestEnc));
    if (aFonts.isEmpty())
    {
        OUString aName = Application::GetSettings().GetStyleSettings().GetAppFont().GetFamilyName();
        aFonts = OUStringToOString(aName, eDestEnc);
    }

    m_pStream->WriteCharPtr( "{\\fonttbl" );
    if (!aFonts.isEmpty())
    {
        sal_Int32 nIdx{0};
        sal_Int32 nTok{-1}; // to compensate pre-increment
        do {
            m_pStream->WriteCharPtr( "\\f" );
            m_pStream->WriteInt32AsString(++nTok);
            m_pStream->WriteCharPtr( "\\fcharset0\\fnil " );
            m_pStream->WriteCharPtr( aFonts.getToken(0, ';', nIdx).getStr() );
            m_pStream->WriteChar( ';' );
        } while (nIdx>=0);
    }
    m_pStream->WriteChar( '}' ) ;
    m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
    // write the rtf color table
    m_pStream->WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_COLORTBL ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RED );
    m_pStream->WriteUInt32AsString(aColor.GetRed());
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_GREEN );
    m_pStream->WriteUInt32AsString(aColor.GetGreen());
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_BLUE );
    m_pStream->WriteUInt32AsString(aColor.GetBlue());

    m_pStream->WriteCharPtr( ";\\red255\\green255\\blue255;\\red192\\green192\\blue192;}" )
                .WriteCharPtr( SAL_NEWLINE_STRING );

    static char const aCell1[] = "\\clbrdrl\\brdrs\\brdrcf0\\clbrdrt\\brdrs\\brdrcf0\\clbrdrb\\brdrs\\brdrcf0\\clbrdrr\\brdrs\\brdrcf0\\clshdng10000\\clcfpat2\\cellx";

    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRGAPH );
    m_pStream->WriteInt32AsString(40);
    m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );

    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< OUString> aNames(xColumns->getElementNames());
        const OUString* pIter = aNames.getConstArray();

        sal_Int32 nCount = aNames.getLength();
        bool bUseResultMetaData = false;
        if ( !nCount )
        {
            nCount = m_xResultSetMetaData->getColumnCount();
            bUseResultMetaData = true;
        }

        for( sal_Int32 i=1; i<=nCount; ++i )
        {
            m_pStream->WriteCharPtr( aCell1 );
            m_pStream->WriteInt32AsString(i*CELL_X);
            m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
        }

        // column description
        m_pStream->WriteChar( '{' ).WriteCharPtr( SAL_NEWLINE_STRING );
        m_pStream->WriteCharPtr( "\\trrh-270\\pard\\intbl" );

        std::unique_ptr<OString[]> pHorzChar(new OString[nCount]);

        for ( sal_Int32 i=1; i <= nCount; ++i )
        {
            sal_Int32 nAlign = 0;
            OUString sColumnName;
            if ( bUseResultMetaData )
                sColumnName = m_xResultSetMetaData->getColumnName(i);
            else
            {
                sColumnName = *pIter;
                Reference<XPropertySet> xColumn;
                xColumns->getByName(sColumnName) >>= xColumn;
                xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;
                ++pIter;
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

            m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
            m_pStream->WriteChar( '{' );
            m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_QC );   // column header always centered

            if ( bBold )        m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B );
            if ( bItalic )      m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I );
            if ( bUnderline )   m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UL );
            if ( bStrikeout )   m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_STRIKE );

            m_pStream->WriteCharPtr( "\\fs20\\f0\\cf0\\cb2" );
            m_pStream->WriteChar( ' ' );
            RTFOutFuncs::Out_String(*m_pStream,sColumnName,eDestEnc);

            m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );
            m_pStream->WriteChar( '}' );
            m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
            m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PARD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_INTBL );
        }

        m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ROW );
        m_pStream->WriteCharPtr( SAL_NEWLINE_STRING ).WriteChar( '}' );
        m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );

        sal_Int32 k=1;
        sal_Int32 kk=0;
        if ( m_aSelection.getLength() )
        {
            const Any* pSelIter = m_aSelection.getConstArray();
            const Any* pEnd   = pSelIter + m_aSelection.getLength();

            bool bContinue = true;
            for( ; pSelIter != pEnd && bContinue; ++pSelIter )
            {
                if ( m_bBookmarkSelection )
                {
                    bContinue = m_xRowLocate->moveToBookmark( *pSelIter );
                }
                else
                {
                    sal_Int32 nPos = -1;
                    OSL_VERIFY( *pSelIter >>= nPos );
                    bContinue = ( m_xResultSet->absolute( nPos ) );
                }

                if ( bContinue )
                    appendRow( pHorzChar.get(), nCount, k, kk );
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

    m_pStream->WriteChar( '}' ).WriteCharPtr( SAL_NEWLINE_STRING );
    m_pStream->WriteUChar( 0 );
    return ((*m_pStream).GetError() == ERRCODE_NONE);
}

void ORTFImportExport::appendRow(OString const * pHorzChar,sal_Int32 _nColumnCount,sal_Int32& k,sal_Int32& kk)
{
    ++kk;
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRGAPH );
    m_pStream->WriteInt32AsString(40);
    m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );

    static char const aCell2[] = "\\clbrdrl\\brdrs\\brdrcf2\\clbrdrt\\brdrs\\brdrcf2\\clbrdrb\\brdrs\\brdrcf2\\clbrdrr\\brdrs\\brdrcf2\\clshdng10000\\clcfpat1\\cellx";

    for ( sal_Int32 i=1; i<=_nColumnCount; ++i )
    {
        m_pStream->WriteCharPtr( aCell2 );
        m_pStream->WriteInt32AsString(i*CELL_X);
        m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
    }

    const bool bBold            = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    const bool bItalic      = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    const bool bUnderline       = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    const bool bStrikeout       = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );
    Reference< XRowSet > xRowSet(m_xRow,UNO_QUERY);

    m_pStream->WriteChar( '{' );
    m_pStream->WriteCharPtr( "\\trrh-270\\pard\\intbl" );
    for ( sal_Int32 i=1; i <= _nColumnCount; ++i )
    {
        m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
        m_pStream->WriteChar( '{' );
        m_pStream->WriteCharPtr( pHorzChar[i-1].getStr() );

        if ( bBold )        m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B );
        if ( bItalic )      m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I );
        if ( bUnderline )   m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UL );
        if ( bStrikeout )   m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_STRIKE );

        m_pStream->WriteCharPtr( "\\fs20\\f1\\cf0\\cb1 " );

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

        m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );
        m_pStream->WriteChar( '}' );
        m_pStream->WriteCharPtr( SAL_NEWLINE_STRING );
        m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PARD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_INTBL );
    }
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ROW ).WriteCharPtr( SAL_NEWLINE_STRING );
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

const sal_Int16 OHTMLImportExport::nDefaultFontSize[SBA_HTML_FONTSIZES] =
{
    HTMLFONTSZ1_DFLT, HTMLFONTSZ2_DFLT, HTMLFONTSZ3_DFLT, HTMLFONTSZ4_DFLT,
    HTMLFONTSZ5_DFLT, HTMLFONTSZ6_DFLT, HTMLFONTSZ7_DFLT
};

sal_Int16 OHTMLImportExport::nFontSize[SBA_HTML_FONTSIZES] = { 0 };

const sal_Int16 OHTMLImportExport::nCellSpacing = 0;
const char OHTMLImportExport::sIndentSource[nIndentMax+1] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

// Macros for HTML-Export
#define TAG_ON( tag )       HTMLOutFuncs::Out_AsciiTag( (*m_pStream), tag )
#define TAG_OFF( tag )      HTMLOutFuncs::Out_AsciiTag( (*m_pStream), tag, false )
#define OUT_LF()            m_pStream->WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() )
#define TAG_ON_LF( tag )    (TAG_ON( tag ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() ))
#define TAG_OFF_LF( tag )   (TAG_OFF( tag ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() ))

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
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    m_eDestEnc = rHtmlOptions.GetTextEncoding();
    strncpy( sIndent, sIndentSource ,std::min(sizeof(sIndent),sizeof(sIndentSource)));
    sIndent[0] = 0;
}

bool OHTMLImportExport::Write()
{
    ODatabaseImportExport::Write();
    if(m_xObject.is())
    {
        m_pStream->WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_doctype ).WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_doctype40 ).WriteChar( '>' ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( SAL_NEWLINE_STRING );
        TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_html );
        WriteHeader();
        OUT_LF();
        WriteBody();
        OUT_LF();
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_html );

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

    IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_head );

    SfxFrameHTMLWriter::Out_DocInfo( (*m_pStream), OUString(),
        xDocProps, sIndent, osl_getThreadTextEncoding() );
    OUT_LF();
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_head );
}

void OHTMLImportExport::WriteBody()
{
    IncIndent(1);
    m_pStream->WriteCharPtr( "<" ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_style ).WriteCharPtr( " " ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_type ).WriteCharPtr( "=\"text/css\">" );

    m_pStream->WriteCharPtr( "<!-- " ); OUT_LF();
    m_pStream->WriteCharPtr( OOO_STRING_SVTOOLS_HTML_body ).WriteCharPtr( " { " ).WriteCharPtr( "font-family: " ).WriteChar( '"' ).WriteCharPtr( OUStringToOString(m_aFont.Name, osl_getThreadTextEncoding()).getStr() ).WriteChar( '\"' );
        // TODO : think about the encoding of the font name
    m_pStream->WriteCharPtr( "; " ).WriteCharPtr( "font-size: " );
    m_pStream->WriteInt32AsString(m_aFont.Height);
    m_pStream->WriteChar( '}' );

    OUT_LF();
    m_pStream->WriteCharPtr( " -->" );
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_style );
    OUT_LF();

    // default Textcolour black
    m_pStream->WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_body ).WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_text ).WriteChar( '=' );
    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    ::Color aColor(nColor);
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    m_pStream->WriteCharPtr( " " OOO_STRING_SVTOOLS_HTML_O_bgcolor "=" );
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    m_pStream->WriteChar( '>' ); OUT_LF();

    WriteTables();

    TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_body );
}

void OHTMLImportExport::WriteTables()
{
    OString aStrOut  = OOO_STRING_SVTOOLS_HTML_table;
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_frame;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_TF_void;

    Sequence< OUString> aNames;
    Reference<XNameAccess> xColumns;
    bool bUseResultMetaData = false;
    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        xColumns = xColSup->getColumns();
        aNames = xColumns->getElementNames();
        if ( !aNames.getLength() )
        {
            sal_Int32 nCount = m_xResultSetMetaData->getColumnCount();
            aNames.realloc(nCount);
            for (sal_Int32 i= 0; i < nCount; ++i)
                aNames[i] = m_xResultSetMetaData->getColumnName(i+1);
            bUseResultMetaData = true;
        }
    }

    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_O_align;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_AL_left;
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_O_cellspacing;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + OString::number(nCellSpacing);
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_O_cols;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + OString::number(aNames.getLength());
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + OOO_STRING_SVTOOLS_HTML_O_border;
    aStrOut = aStrOut + "=1";

    IncIndent(1);
    TAG_ON( aStrOut.getStr() );

    FontOn();

    TAG_ON( OOO_STRING_SVTOOLS_HTML_caption );
    TAG_ON( OOO_STRING_SVTOOLS_HTML_bold );

    m_pStream->WriteCharPtr( OUStringToOString(m_sName, osl_getThreadTextEncoding()).getStr() );
        // TODO : think about the encoding of the name
    TAG_OFF( OOO_STRING_SVTOOLS_HTML_bold );
    TAG_OFF( OOO_STRING_SVTOOLS_HTML_caption );

    FontOff();
    OUT_LF();
    // </FONT>

    IncIndent(1);
    TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_thead );

    IncIndent(1);
    TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tablerow );

    if(m_xObject.is())
    {
        std::unique_ptr<sal_Int32[]> pFormat(new sal_Int32[aNames.getLength()]);

        std::unique_ptr<const char *[]> pHorJustify(new const char*[aNames.getLength()]);
        std::unique_ptr<sal_Int32[]> pColWidth(new sal_Int32[aNames.getLength()]);

        sal_Int32 nHeight = 0;
        m_xObject->getPropertyValue(PROPERTY_ROW_HEIGHT) >>= nHeight;

        // 1. writing the column description
        const OUString* pIter = aNames.getConstArray();
        const OUString* pEnd = pIter + aNames.getLength();

        for( sal_Int32 i=0;pIter != pEnd; ++pIter,++i )
        {
            sal_Int32 nAlign = 0;
            pFormat[i] = 0;
            pColWidth[i] = 100;
            if ( !bUseResultMetaData )
            {
                Reference<XPropertySet> xColumn;
                xColumns->getByName(*pIter) >>= xColumn;
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

            WriteCell(pFormat[i],pColWidth[i],nHeight,pHorJustify[i],*pIter,OOO_STRING_SVTOOLS_HTML_tableheader);
        }

        IncIndent(-1);
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_thead );

        IncIndent(1);
        TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tbody );

        // 2. and now the data
        Reference< XRowSet > xRowSet(m_xRow,UNO_QUERY);
        sal_Int32 kk=0;
        m_xResultSet->beforeFirst(); // set back before the first row
        while(m_xResultSet->next())
        {
            IncIndent(1);
            TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tablerow );

            ++kk;
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
            TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
        }
    }
    else
    {
        IncIndent(-1);
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_thead );

        IncIndent(1);
        TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tbody );
    }

    IncIndent(-1); OUT_LF(); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tbody );
    IncIndent(-1); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_table );
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
    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + OOO_STRING_SVTOOLS_HTML_O_width;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + OString::number(nWidthPixel);
    // line height
    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + OOO_STRING_SVTOOLS_HTML_O_height;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + OString::number(nHeightPixel);

    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + OOO_STRING_SVTOOLS_HTML_O_align;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + pChar;

    SvNumberFormatsSupplierObj* pSupplierImpl = m_xFormatter.is() ? SvNumberFormatsSupplierObj::getImplementation(m_xFormatter->getNumberFormatsSupplier()) : nullptr;
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

    TAG_ON( aStrTD.getStr() );

    FontOn();

    bool bBold          = ( css::awt::FontWeight::BOLD     == m_aFont.Weight );
    bool bItalic        = ( css::awt::FontSlant_ITALIC     == m_aFont.Slant );
    bool bUnderline     = ( css::awt::FontUnderline::NONE  != m_aFont.Underline );
    bool bStrikeout     = ( css::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    if ( bBold )        TAG_ON( OOO_STRING_SVTOOLS_HTML_bold );
    if ( bItalic )      TAG_ON( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bUnderline )   TAG_ON( OOO_STRING_SVTOOLS_HTML_underline );
    if ( bStrikeout )   TAG_ON( OOO_STRING_SVTOOLS_HTML_strike );

    if ( rValue.isEmpty() )
        TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );        // no completely empty cell
    else
        HTMLOutFuncs::Out_String( (*m_pStream), rValue ,m_eDestEnc);

    if ( bStrikeout )   TAG_OFF( OOO_STRING_SVTOOLS_HTML_strike );
    if ( bUnderline )   TAG_OFF( OOO_STRING_SVTOOLS_HTML_underline );
    if ( bItalic )      TAG_OFF( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bBold )        TAG_OFF( OOO_STRING_SVTOOLS_HTML_bold );

    FontOff();

    TAG_OFF_LF( pHtmlTag );
}

void OHTMLImportExport::FontOn()
{
#if OSL_DEBUG_LEVEL > 0
    m_bCheckFont = true;
#endif

    // <FONT FACE="xxx">
    OString aStrOut  = "<";
    aStrOut  = aStrOut + OOO_STRING_SVTOOLS_HTML_font;
    aStrOut  = aStrOut + " ";
    aStrOut  = aStrOut + OOO_STRING_SVTOOLS_HTML_O_face;
    aStrOut  = aStrOut + "=";
    aStrOut  = aStrOut + "\"";
    aStrOut  = aStrOut + OUStringToOString(m_aFont.Name,osl_getThreadTextEncoding());
        // TODO : think about the encoding of the font name
    aStrOut  = aStrOut + "\"";
    aStrOut  = aStrOut + " ";
    aStrOut  = aStrOut + OOO_STRING_SVTOOLS_HTML_O_color;
    aStrOut  = aStrOut + "=";
    m_pStream->WriteCharPtr( aStrOut.getStr() );

    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    ::Color aColor(nColor);

    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );
    m_pStream->WriteCharPtr( ">" );
}

inline void OHTMLImportExport::FontOff()
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(m_bCheckFont,"No FontOn() called");
#endif
    TAG_OFF( OOO_STRING_SVTOOLS_HTML_font );
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
