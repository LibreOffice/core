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

#include <config_features.h>

#include <tools/urlobj.hxx>
#include <svl/converter.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <ucbhelper/content.hxx>
#include <svx/txenctab.hxx>
#include <unotools/sharedunocomponent.hxx>

#if HAVE_FEATURE_DBCONNECTIVITY
#include <svx/dbcharsethelper.hxx>
#endif

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>

#include "scerrors.hxx"
#include "docsh.hxx"
#include "filter.hxx"
#include "progress.hxx"
#include "formulacell.hxx"
#include "editutil.hxx"
#include "cellform.hxx"
#include "dbdocutl.hxx"
#include "dociter.hxx"
#include "globstr.hrc"
#include <svl/zformat.hxx>
#include <svl/intitem.hxx>
#include "patattr.hxx"
#include "scitems.hxx"
#include "docpool.hxx"
#include "segmenttree.hxx"
#include "docparam.hxx"
#include "cellvalue.hxx"

#include <unordered_set>
#include <vector>

using namespace com::sun::star;
using ::std::vector;

#if HAVE_FEATURE_DBCONNECTIVITY

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"

//! move to a header file?
#define SC_DBPROP_ACTIVECONNECTION  "ActiveConnection"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"
#define SC_DBPROP_PROPCHANGE_NOTIFY "PropertyChangeNotificationEnabled"

#define SC_DBPROP_NAME              "Name"
#define SC_DBPROP_TYPE              "Type"
#define SC_DBPROP_PRECISION         "Precision"
#define SC_DBPROP_SCALE             "Scale"

#define SC_DBPROP_EXTENSION         "Extension"
#define SC_DBPROP_CHARSET           "CharSet"

namespace
{
    sal_uLong lcl_getDBaseConnection(uno::Reference<sdbc::XDriverManager2>& _rDrvMgr, uno::Reference<sdbc::XConnection>& _rConnection, OUString& _rTabName, const OUString& rFullFileName, rtl_TextEncoding eCharSet)
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( rFullFileName );
        _rTabName = aURL.getBase( INetURLObject::LAST_SEGMENT, true,
                INetURLObject::DecodeMechanism::Unambiguous );
        OUString aExtension = aURL.getExtension();
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString aPath = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

        _rDrvMgr.set( sdbc::DriverManager::create( xContext ) );

        // get connection

        OUString aConnUrl("sdbc:dbase:");
        aConnUrl += aPath;

        ::std::vector< rtl_TextEncoding > aEncodings;
        svxform::charset_helper::getSupportedTextEncodings( aEncodings );
        ::std::vector< rtl_TextEncoding >::iterator aIter = ::std::find(aEncodings.begin(),aEncodings.end(),(rtl_TextEncoding) eCharSet);
        if ( aIter == aEncodings.end() )
        {
            OSL_FAIL( "DBaseImport: dbtools::OCharsetMap doesn't know text encoding" );
            return SCERR_IMPORT_CONNECT;
        } // if ( aIter == aMap.end() )
        OUString aCharSetStr;
        if ( RTL_TEXTENCODING_DONTKNOW != *aIter )
        {   // it's not the virtual "system charset"
            const char* pIanaName = rtl_getMimeCharsetFromTextEncoding( *aIter );
            OSL_ENSURE( pIanaName, "invalid mime name!" );
            if ( pIanaName )
                aCharSetStr = OUString::createFromAscii( pIanaName );
        }

        uno::Sequence<beans::PropertyValue> aProps(2);
        aProps[0].Name = SC_DBPROP_EXTENSION;
        aProps[0].Value <<= OUString( aExtension );
        aProps[1].Name = SC_DBPROP_CHARSET;
        aProps[1].Value <<= aCharSetStr;

        _rConnection = _rDrvMgr->getConnectionWithInfo( aConnUrl, aProps );
        return 0L;
    }
}

#endif // HAVE_FEATURE_DBCONNECTIVITY

// MoveFile/KillFile/IsDocument: similar to SfxContentHelper

bool ScDocShell::MoveFile( const INetURLObject& rSourceObj, const INetURLObject& rDestObj )
{
    bool bMoveData = true;
    bool bRet = true, bKillSource = false;
    if ( rSourceObj.GetProtocol() != rDestObj.GetProtocol() )
    {
        bMoveData = false;
        bKillSource = true;
    }
    OUString aName = rDestObj.getName();
    INetURLObject aDestPathObj = rDestObj;
    aDestPathObj.removeSegment();
    aDestPathObj.setFinalSlash();

    try
    {
        ::ucbhelper::Content aDestPath( aDestPathObj.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                            uno::Reference< css::ucb::XCommandEnvironment >(),
                            comphelper::getProcessComponentContext() );
        uno::Reference< css::ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        OUString aTransferName = "transfer";
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, uno::makeAny(
                css::ucb::TransferInfo( bMoveData, rSourceObj.GetMainURL(INetURLObject::DecodeMechanism::NONE), aName,
                                                       css::ucb::NameClash::ERROR ) ) );
        }
        else
        {
            OSL_FAIL( "transfer command not available" );
        }
    }
    catch( uno::Exception& )
    {
        // ucb may throw different exceptions on failure now
        bRet = false;
    }

    if ( bKillSource )
        KillFile( rSourceObj );

    return bRet;
}

bool ScDocShell::KillFile( const INetURLObject& rURL )
{
    bool bRet = true;
    try
    {
        ::ucbhelper::Content aCnt( rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                        uno::Reference< css::ucb::XCommandEnvironment >(),
                        comphelper::getProcessComponentContext() );
        aCnt.executeCommand( "delete", css::uno::Any( true ) );
    }
    catch( uno::Exception& )
    {
        // ucb may throw different exceptions on failure now
        bRet = false;
    }

    return bRet;
}

bool ScDocShell::IsDocument( const INetURLObject& rURL )
{
    bool bRet = false;
    try
    {
        ::ucbhelper::Content aCnt( rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                        uno::Reference< css::ucb::XCommandEnvironment >(),
                        comphelper::getProcessComponentContext() );
        bRet = aCnt.isDocument();
    }
    catch( uno::Exception& )
    {
        // ucb may throw different exceptions on failure now - warning only
        OSL_FAIL( "Any other exception" );
    }

    return bRet;
}

#if HAVE_FEATURE_DBCONNECTIVITY

static void lcl_setScalesToColumns(ScDocument& rDoc, const vector<long>& rScales)
{
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    if (!pFormatter)
        return;

    SCCOL nColCount = static_cast<SCCOL>(rScales.size());
    for (SCCOL i = 0; i < nColCount; ++i)
    {
        if (rScales[i] < 0)
            continue;

        sal_uInt32 nOldFormat;
        rDoc.GetNumberFormat(static_cast<SCCOL>(i), 0, 0, nOldFormat);
        const SvNumberformat* pOldEntry = pFormatter->GetEntry(nOldFormat);
        if (!pOldEntry)
            continue;

        LanguageType eLang = pOldEntry->GetLanguage();
        bool bThousand, bNegRed;
        sal_uInt16 nPrecision, nLeading;
        pOldEntry->GetFormatSpecialInfo(bThousand, bNegRed, nPrecision, nLeading);

        nPrecision = static_cast<sal_uInt16>(rScales[i]);
        OUString aNewPicture = pFormatter->GenerateFormat(nOldFormat, eLang,
                                                          bThousand, bNegRed, nPrecision, nLeading);

        sal_uInt32 nNewFormat = pFormatter->GetEntryKey(aNewPicture, eLang);
        if (nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_Int32 nErrPos = 0;
            short nNewType = 0;
            bool bOk = pFormatter->PutEntry(
                aNewPicture, nErrPos, nNewType, nNewFormat, eLang);

            if (!bOk)
                continue;
        }

        ScPatternAttr aNewAttrs( rDoc.GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nNewFormat) );
        rDoc.ApplyPatternAreaTab(static_cast<SCCOL>(i), 0, static_cast<SCCOL>(i), MAXROW, 0, aNewAttrs);
    }
}

#endif // HAVE_FEATURE_DBCONNECTIVITY

sal_uLong ScDocShell::DBaseImport( const OUString& rFullFileName, rtl_TextEncoding eCharSet,
                               ScColWidthParam aColWidthParam[MAXCOLCOUNT], ScFlatBoolRowSegments& rRowHeightsRecalc )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rFullFileName;
    (void) eCharSet;
    (void) aColWidthParam;
    (void) rRowHeightsRecalc;

    return ERRCODE_IO_GENERAL;
#else

    sal_uLong nErr = eERR_OK;

    // Try to get the Text Encoding from the driver
    if( eCharSet == RTL_TEXTENCODING_IBM_850 )
        eCharSet = RTL_TEXTENCODING_DONTKNOW;

    try
    {
        long i;
        long nColCount = 0;
        OUString aTabName;
        uno::Reference<sdbc::XDriverManager2> xDrvMan;
        uno::Reference<sdbc::XConnection> xConnection;
        sal_uLong nRet = lcl_getDBaseConnection(xDrvMan,xConnection,aTabName,rFullFileName,eCharSet);
        if ( !xConnection.is() || !xDrvMan.is() )
            return nRet;
        ::utl::DisposableComponent aConnectionHelper(xConnection);

        ScProgress aProgress( this, ScGlobal::GetRscString( STR_LOAD_DOC ), 0, true );
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(SC_SERVICE_ROWSET),
                            uno::UNO_QUERY);
        ::utl::DisposableComponent aRowSetHelper(xRowSet);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is()) return SCERR_IMPORT_CONNECT;

        sal_Int32 nType = sdb::CommandType::TABLE;

        xRowProp->setPropertyValue( SC_DBPROP_ACTIVECONNECTION, uno::Any(xConnection) );

        xRowProp->setPropertyValue( SC_DBPROP_COMMANDTYPE, uno::Any(nType) );

        xRowProp->setPropertyValue( SC_DBPROP_COMMAND, uno::Any(aTabName) );

        xRowProp->setPropertyValue( SC_DBPROP_PROPCHANGE_NOTIFY, uno::Any(false) );

        xRowSet->execute();

        uno::Reference<sdbc::XResultSetMetaData> xMeta;
        uno::Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp( xRowSet, uno::UNO_QUERY );
        if ( xMetaSupp.is() )
            xMeta = xMetaSupp->getMetaData();
        if ( xMeta.is() )
            nColCount = xMeta->getColumnCount();    // this is the number of real columns

        if ( nColCount > MAXCOL+1 )
        {
            nColCount = MAXCOL+1;
            nErr = SCWARN_IMPORT_COLUMN_OVERFLOW;    // warning
        }

        uno::Reference<sdbc::XRow> xRow( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRow.is(), "can't get Row" );
        if (!xRow.is()) return SCERR_IMPORT_CONNECT;

        // currency flag is not needed for dBase
        uno::Sequence<sal_Int32> aColTypes( nColCount );    // column types
        sal_Int32* pTypeArr = aColTypes.getArray();
        for (i=0; i<nColCount; i++)
            pTypeArr[i] = xMeta->getColumnType( i+1 );

        //  read column names
        //! add type descriptions

        aProgress.SetState( 0 );

        vector<long> aScales(nColCount, -1);
        for (i=0; i<nColCount; i++)
        {
            OUString aHeader = xMeta->getColumnLabel( i+1 );

            switch ( pTypeArr[i] )
            {
                case sdbc::DataType::BIT:
                    aHeader += ",L";
                    break;
                case sdbc::DataType::DATE:
                    aHeader += ",D";
                    break;
                case sdbc::DataType::LONGVARCHAR:
                    aHeader += ",M";
                    break;
                case sdbc::DataType::VARCHAR:
                    aHeader += ",C," + OUString::number( xMeta->getColumnDisplaySize( i+1 ) );
                    break;
                case sdbc::DataType::DECIMAL:
                    {
                        long nPrec = xMeta->getPrecision( i+1 );
                        long nScale = xMeta->getScale( i+1 );
                        aHeader += ",N," +
                                    OUString::number(
                                        SvDbaseConverter::ConvertPrecisionToDbase(
                                            nPrec, nScale ) ) +
                                    "," +
                                    OUString::number( nScale );
                        aScales[i] = nScale;
                    }
                    break;
            }

            aDocument.SetString( static_cast<SCCOL>(i), 0, 0, aHeader );
        }

        lcl_setScalesToColumns(aDocument, aScales);

        SCROW nRow = 1;     // 0 is column titles
        bool bEnd = false;
        while ( !bEnd && xRowSet->next() )
        {
            if ( nRow <= MAXROW )
            {
                bool bSimpleRow = true;
                SCCOL nCol = 0;
                for (i=0; i<nColCount; i++)
                {
                    ScDatabaseDocUtil::StrData aStrData;
                    ScDatabaseDocUtil::PutData( &aDocument, nCol, nRow, 0,
                                                xRow, i+1, pTypeArr[i], false,
                                                &aStrData );

                    if (aStrData.mnStrLength > aColWidthParam[nCol].mnMaxTextLen)
                    {
                        aColWidthParam[nCol].mnMaxTextLen = aStrData.mnStrLength;
                        aColWidthParam[nCol].mnMaxTextRow = nRow;
                    }

                    if (!aStrData.mbSimpleText)
                    {
                        bSimpleRow = false;
                        aColWidthParam[nCol].mbSimpleText = false;
                    }

                    ++nCol;
                }
                if (!bSimpleRow)
                    rRowHeightsRecalc.setTrue(nRow, nRow);
                ++nRow;
            }
            else        // past the end of the spreadsheet
            {
                bEnd = true;                            // don't continue
                nErr = SCWARN_IMPORT_RANGE_OVERFLOW;    // warning message
            }
        }
    }
    catch ( sdbc::SQLException& )
    {
        nErr = SCERR_IMPORT_CONNECT;
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL("Unexpected exception in database");
        nErr = ERRCODE_IO_GENERAL;
    }

    return nErr;
#endif // HAVE_FEATURE_DBCONNECTIVITY
}

#if HAVE_FEATURE_DBCONNECTIVITY

namespace {

void lcl_GetColumnTypes(
    ScDocShell& rDocShell, const ScRange& rDataRange, bool bHasFieldNames,
    OUString* pColNames, sal_Int32* pColTypes, sal_Int32* pColLengths,
    sal_Int32* pColScales, bool& bHasMemo, rtl_TextEncoding eCharSet )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    SvNumberFormatter* pNumFmt = rDoc.GetFormatTable();

    SCTAB nTab = rDataRange.aStart.Tab();
    SCCOL nFirstCol = rDataRange.aStart.Col();
    SCROW nFirstRow = rDataRange.aStart.Row();
    SCCOL nLastCol = rDataRange.aEnd.Col();
    SCROW nLastRow = rDataRange.aEnd.Row();

    typedef std::unordered_set<OUString, OUStringHash> StrSetType;
    StrSetType aFieldNames;

    long nField = 0;
    SCROW nFirstDataRow = ( bHasFieldNames ? nFirstRow + 1 : nFirstRow );
    for ( SCCOL nCol = nFirstCol; nCol <= nLastCol; nCol++ )
    {
        bool bTypeDefined = false;
        bool bPrecDefined = false;
        sal_Int32 nFieldLen = 0;
        sal_Int32 nPrecision = 0;
        sal_Int32 nDbType = sdbc::DataType::SQLNULL;
        OUString aFieldName;
        OUString aString;

        // Fieldname[,Type[,Width[,Prec]]]
        // Type etc.: L; D; C[,W]; N[,W[,P]]
        if ( bHasFieldNames )
        {
            aString = rDoc.GetString(nCol, nFirstRow, nTab);
            aString = aString.toAsciiUpperCase();
            sal_Int32 nToken = comphelper::string::getTokenCount(aString, ',');
            if ( nToken > 1 )
            {
                aFieldName = aString.getToken( 0, ',' );
                aString = aString.replaceAll(" ", "");
                switch ( aString.getToken( 1, ',' )[0] )
                {
                    case 'L' :
                        nDbType = sdbc::DataType::BIT;
                        nFieldLen = 1;
                        bTypeDefined = true;
                        bPrecDefined = true;
                        break;
                    case 'D' :
                        nDbType = sdbc::DataType::DATE;
                        nFieldLen = 8;
                        bTypeDefined = true;
                        bPrecDefined = true;
                        break;
                    case 'M' :
                        nDbType = sdbc::DataType::LONGVARCHAR;
                        nFieldLen = 10;
                        bTypeDefined = true;
                        bPrecDefined = true;
                        bHasMemo = true;
                        break;
                    case 'C' :
                        nDbType = sdbc::DataType::VARCHAR;
                        bTypeDefined = true;
                        bPrecDefined = true;
                        break;
                    case 'N' :
                        nDbType = sdbc::DataType::DECIMAL;
                        bTypeDefined = true;
                        break;
                }
                if ( bTypeDefined && !nFieldLen && nToken > 2 )
                {
                    nFieldLen = aString.getToken( 2, ',' ).toInt32();
                    if ( !bPrecDefined && nToken > 3 )
                    {
                        OUString aTmp( aString.getToken( 3, ',' ) );
                        if ( CharClass::isAsciiNumeric(aTmp) )
                        {
                            nPrecision = aTmp.toInt32();
                            if (nPrecision && nFieldLen < nPrecision+1)
                                nFieldLen = nPrecision + 1;     // include decimal separator
                            bPrecDefined = true;
                        }
                    }
                }
            }
            else
                aFieldName = aString;

            // Check field name and generate valid field name if necessary.
            // First character has to be alphabetical, subsequent characters
            // have to be alphanumerical or underscore.
            // "_DBASELOCK" is reserved (obsolete because first character is
            // not alphabetical).
            // No duplicated names.
            if ( !rtl::isAsciiAlpha(aFieldName[0]) )
                aFieldName = "N" + aFieldName;
            OUString aTmpStr;
            sal_Unicode c;
            for ( const sal_Unicode* p = aFieldName.getStr(); ( c = *p ) != 0; p++ )
            {
                if ( rtl::isAsciiAlpha(c) || rtl::isAsciiDigit(c) || c == '_' )
                    aTmpStr += OUStringLiteral1(c);
                else
                    aTmpStr += "_";
            }
            aFieldName = aTmpStr;
            if ( aFieldName.getLength() > 10 )
                aFieldName = aFieldName.copy(0,  10);

            if (!aFieldNames.insert(aFieldName).second)
            {   // Duplicated field name, append numeric suffix.
                sal_uInt16 nSub = 1;
                OUString aFixPart( aFieldName );
                do
                {
                    ++nSub;
                    OUString aVarPart = OUString::number( nSub );
                    if ( aFixPart.getLength() + aVarPart.getLength() > 10 )
                        aFixPart = aFixPart.copy( 0, 10 - aVarPart.getLength() );
                    aFieldName = aFixPart;
                    aFieldName += aVarPart;
                } while (!aFieldNames.insert(aFieldName).second);
            }
        }
        else
        {
            aFieldName = "N" + OUString::number(nCol+1);
        }

        if ( !bTypeDefined )
        {   // Field type.
            ScRefCellValue aCell(rDoc, ScAddress(nCol, nFirstDataRow, nTab));
            if (aCell.isEmpty() || aCell.hasString())
                nDbType = sdbc::DataType::VARCHAR;
            else
            {
                sal_uInt32 nFormat;
                rDoc.GetNumberFormat( nCol, nFirstDataRow, nTab, nFormat );
                switch ( pNumFmt->GetType( nFormat ) )
                {
                    case css::util::NumberFormat::LOGICAL :
                        nDbType = sdbc::DataType::BIT;
                        nFieldLen = 1;
                        break;
                    case css::util::NumberFormat::DATE :
                        nDbType = sdbc::DataType::DATE;
                        nFieldLen = 8;
                        break;
                    case css::util::NumberFormat::TIME :
                    case css::util::NumberFormat::DATETIME :
                        nDbType = sdbc::DataType::VARCHAR;
                        break;
                    default:
                        nDbType = sdbc::DataType::DECIMAL;
                }
            }
        }
        // Field length.
        if ( nDbType == sdbc::DataType::VARCHAR && !nFieldLen )
        {   // Determine maximum field width.
            nFieldLen = rDoc.GetMaxStringLen( nTab, nCol, nFirstDataRow,
                nLastRow, eCharSet );
            if ( nFieldLen == 0 )
                nFieldLen = 1;
        }
        else if ( nDbType == sdbc::DataType::DECIMAL )
        {   // Determine maximum field width and precision.
            sal_Int32 nLen;
            sal_uInt16 nPrec;
            nLen = rDoc.GetMaxNumberStringLen( nPrec, nTab, nCol,
                nFirstDataRow, nLastRow );
            // dBaseIII precision limit: 15
            if ( nPrecision > 15 )
                nPrecision = 15;
            if ( nPrec > 15 )
                nPrec = 15;
            if ( bPrecDefined && nPrecision != nPrec )
            {
                if (nPrecision < nPrec)
                {
                    // This is a hairy case. User defined nPrecision but a
                    // number format has more precision. Modifying a dBase
                    // field may as well render the resulting file useless for
                    // an application that relies on its defined structure,
                    // especially if we are resaving an already existing file.
                    // So who's right, the user who (or the loaded file that)
                    // defined the field, or the user who applied the format?
                    // Commit f59e350d1733125055f1144f8b3b1b0a46f6d1ca gave the
                    // format a higher priority, which is debatable.
                    SAL_WARN( "sc", "lcl_GetColumnTypes: conflicting dBase field precision for "
                            << aFieldName << " (" << nPrecision << "<" << nPrec << ")");

                    // Adjust length to larger predefined integer part. There
                    // may be a reason that the field was prepared for larger
                    // numbers.
                    if (nFieldLen - nPrecision > nLen - nPrec)
                        nLen = nFieldLen - (nPrecision ? nPrecision+1 : 0) + 1 + nPrec;
                    // And override precision.
                    nPrecision = nPrec;
                }
                else
                {
#if 1
                    // Adjust length to predefined precision.
                    nLen = nLen + ( nPrecision - nPrec );
#else
    /* If the above override for (nPrecision < nPrec) was not in place then
     * nPrecision could be 0 and this would be the code path to correctly
     * calculate nLen. But as is, nPrecision is never 0 here, see CID#982304 */

                    // Adjust length to predefined precision.
                    if ( nPrecision )
                        nLen = nLen + ( nPrecision - nPrec );
                    else
                        nLen -= nPrec+1;    // also remove the decimal separator
#endif
                }
            }
            if (nFieldLen < nLen)
            {
                if (!bTypeDefined)
                    nFieldLen = nLen;
                else
                {
                    // Again a hairy case and conflict. Furthermore, the
                    // larger overall length may be a result of only a higher
                    // precision obtained from formats.
                    SAL_WARN( "sc", "lcl_GetColumnTypes: conflicting dBase field length for "
                            << aFieldName << " (" << nFieldLen << "<" << nLen << ")");
                    nFieldLen = nLen;
                }
            }
            if ( !bPrecDefined )
                nPrecision = nPrec;
            if ( nFieldLen == 0 )
                nFieldLen = 1;
            else if ( nFieldLen > 19 )
                nFieldLen = 19;     // dBaseIII numeric field length limit: 19
            if ( nPrecision && nFieldLen < nPrecision + 2 )
                nFieldLen = nPrecision + 2;     // 0. must fit into
            // 538 MUST: Sdb internal representation adds 2 to the field length!
            // To give the user what he wants we must subtract it here.
             //! CAVEAT! There is no way to define a numeric field with a length
             //! of 1 and no decimals!
            nFieldLen = SvDbaseConverter::ConvertPrecisionToOdbc( nFieldLen, nPrecision );
        }
        if ( nFieldLen > 254 )
        {
            if ( nDbType == sdbc::DataType::VARCHAR )
            {   // Too long for a normal text field => memo field.
                nDbType = sdbc::DataType::LONGVARCHAR;
                nFieldLen = 10;
                bHasMemo = true;
            }
            else
                nFieldLen = 254;                    // bad luck..
        }

        pColNames[nField] = aFieldName;
        pColTypes[nField] = nDbType;
        pColLengths[nField] = nFieldLen;
        pColScales[nField] = nPrecision;

        ++nField;
    }
}

inline void lcl_getLongVarCharEditString( OUString& rString,
        const ScRefCellValue& rCell, ScFieldEditEngine& rEditEngine )
{
    if (!rCell.mpEditText)
        return;

    rEditEngine.SetText(*rCell.mpEditText);
    rString = rEditEngine.GetText( LINEEND_CRLF );
}

inline void lcl_getLongVarCharString(
    OUString& rString, ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, SvNumberFormatter& rNumFmt )
{
    Color* pColor;
    ScAddress aPos(nCol, nRow, nTab);
    sal_uInt32 nFormat = rDoc.GetNumberFormat(aPos);
    rString = ScCellFormat::GetString(rDoc, aPos, nFormat, &pColor, rNumFmt);
}

}

#endif // HAVE_FEATURE_DBCONNECTIVITY

sal_uLong ScDocShell::DBaseExport( const OUString& rFullFileName, rtl_TextEncoding eCharSet, bool& bHasMemo )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rFullFileName;
    (void) eCharSet;
    (void) bHasMemo;

    return ERRCODE_IO_GENERAL;
#else
    // remove the file so the dBase driver doesn't find an invalid file
    INetURLObject aDeleteObj( rFullFileName, INetProtocol::File );
    KillFile( aDeleteObj );

    sal_uLong nErr = eERR_OK;

    SCCOL nFirstCol, nLastCol;
    SCROW  nFirstRow, nLastRow;
    SCTAB nTab = GetSaveTab();
    aDocument.GetDataStart( nTab, nFirstCol, nFirstRow );
    aDocument.GetCellArea( nTab, nLastCol, nLastRow );
    if ( nFirstCol > nLastCol )
        nFirstCol = nLastCol;
    if ( nFirstRow > nLastRow )
        nFirstRow = nLastRow;
    ScProgress aProgress( this, ScGlobal::GetRscString( STR_SAVE_DOC ),
                                                    nLastRow - nFirstRow, true );
    SvNumberFormatter* pNumFmt = aDocument.GetFormatTable();

    bool bHasFieldNames = true;
    for ( SCCOL nDocCol = nFirstCol; nDocCol <= nLastCol && bHasFieldNames; nDocCol++ )
    {   // nur Strings in erster Zeile => sind Feldnamen
        if ( !aDocument.HasStringData( nDocCol, nFirstRow, nTab ) )
            bHasFieldNames = false;
    }

    long nColCount = nLastCol - nFirstCol + 1;
    uno::Sequence<OUString> aColNames( nColCount );
    uno::Sequence<sal_Int32> aColTypes( nColCount );
    uno::Sequence<sal_Int32> aColLengths( nColCount );
    uno::Sequence<sal_Int32> aColScales( nColCount );

    ScRange aDataRange( nFirstCol, nFirstRow, nTab, nLastCol, nLastRow, nTab );
    lcl_GetColumnTypes( *this, aDataRange, bHasFieldNames,
                        aColNames.getArray(), aColTypes.getArray(),
                        aColLengths.getArray(), aColScales.getArray(),
                        bHasMemo, eCharSet );
    // also needed for exception catch
    SCROW nDocRow = 0;
    ScFieldEditEngine aEditEngine(&aDocument, aDocument.GetEditPool());
    OUString aString;
    OUString aTabName;

    try
    {
        uno::Reference<sdbc::XDriverManager2> xDrvMan;
        uno::Reference<sdbc::XConnection> xConnection;
        sal_uLong nRet = lcl_getDBaseConnection(xDrvMan,xConnection,aTabName,rFullFileName,eCharSet);
        if ( !xConnection.is() || !xDrvMan.is() )
            return nRet;
        ::utl::DisposableComponent aConnectionHelper(xConnection);

        // get dBase driver
        uno::Reference< sdbc::XDriverAccess> xAccess(xDrvMan,uno::UNO_QUERY);
        uno::Reference< sdbcx::XDataDefinitionSupplier > xDDSup( xAccess->getDriverByURL( xConnection->getMetaData()->getURL() ), uno::UNO_QUERY );
        if ( !xDDSup.is() )
            return SCERR_EXPORT_CONNECT;

        // create table
        uno::Reference<sdbcx::XTablesSupplier> xTablesSupp =xDDSup->getDataDefinitionByConnection( xConnection );
        OSL_ENSURE( xTablesSupp.is(), "can't get Data Definition" );
        if (!xTablesSupp.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<container::XNameAccess> xTables = xTablesSupp->getTables();
        OSL_ENSURE( xTables.is(), "can't get Tables" );
        if (!xTables.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XDataDescriptorFactory> xTablesFact( xTables, uno::UNO_QUERY );
        OSL_ENSURE( xTablesFact.is(), "can't get tables factory" );
        if (!xTablesFact.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XAppend> xTablesAppend( xTables, uno::UNO_QUERY );
        OSL_ENSURE( xTablesAppend.is(), "can't get tables XAppend" );
        if (!xTablesAppend.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<beans::XPropertySet> xTableDesc = xTablesFact->createDataDescriptor();
        OSL_ENSURE( xTableDesc.is(), "can't get table descriptor" );
        if (!xTableDesc.is()) return SCERR_EXPORT_CONNECT;

        xTableDesc->setPropertyValue( SC_DBPROP_NAME, uno::Any(aTabName) );

        // create columns

        uno::Reference<sdbcx::XColumnsSupplier> xColumnsSupp( xTableDesc, uno::UNO_QUERY );
        OSL_ENSURE( xColumnsSupp.is(), "can't get columns supplier" );
        if (!xColumnsSupp.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<container::XNameAccess> xColumns = xColumnsSupp->getColumns();
        OSL_ENSURE( xColumns.is(), "can't get columns" );
        if (!xColumns.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XDataDescriptorFactory> xColumnsFact( xColumns, uno::UNO_QUERY );
        OSL_ENSURE( xColumnsFact.is(), "can't get columns factory" );
        if (!xColumnsFact.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XAppend> xColumnsAppend( xColumns, uno::UNO_QUERY );
        OSL_ENSURE( xColumnsAppend.is(), "can't get columns XAppend" );
        if (!xColumnsAppend.is()) return SCERR_EXPORT_CONNECT;

        const OUString* pColNames = aColNames.getConstArray();
        const sal_Int32* pColTypes     = aColTypes.getConstArray();
        const sal_Int32* pColLengths   = aColLengths.getConstArray();
        const sal_Int32* pColScales    = aColScales.getConstArray();
        long nCol;

        for (nCol=0; nCol<nColCount; nCol++)
        {
            uno::Reference<beans::XPropertySet> xColumnDesc = xColumnsFact->createDataDescriptor();
            OSL_ENSURE( xColumnDesc.is(), "can't get column descriptor" );
            if (!xColumnDesc.is()) return SCERR_EXPORT_CONNECT;

            xColumnDesc->setPropertyValue( SC_DBPROP_NAME, uno::Any(pColNames[nCol]) );

            xColumnDesc->setPropertyValue( SC_DBPROP_TYPE, uno::Any(pColTypes[nCol]) );

            xColumnDesc->setPropertyValue( SC_DBPROP_PRECISION, uno::Any(pColLengths[nCol]) );

            xColumnDesc->setPropertyValue( SC_DBPROP_SCALE, uno::Any(pColScales[nCol]) );

            xColumnsAppend->appendByDescriptor( xColumnDesc );
        }

        xTablesAppend->appendByDescriptor( xTableDesc );

        // get row set for writing
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(SC_SERVICE_ROWSET),
                            uno::UNO_QUERY);
        ::utl::DisposableComponent aRowSetHelper(xRowSet);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is()) return SCERR_EXPORT_CONNECT;

        xRowProp->setPropertyValue( SC_DBPROP_ACTIVECONNECTION, uno::Any(xConnection) );

        xRowProp->setPropertyValue( SC_DBPROP_COMMANDTYPE, uno::Any((sal_Int32) sdb::CommandType::TABLE) );

        xRowProp->setPropertyValue( SC_DBPROP_COMMAND, uno::Any(aTabName) );

        xRowSet->execute();

        // write data rows

        uno::Reference<sdbc::XResultSetUpdate> xResultUpdate( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xResultUpdate.is(), "can't get XResultSetUpdate" );
        if (!xResultUpdate.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbc::XRowUpdate> xRowUpdate( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRowUpdate.is(), "can't get XRowUpdate" );
        if (!xRowUpdate.is()) return SCERR_EXPORT_CONNECT;

        SCROW nFirstDataRow = ( bHasFieldNames ? nFirstRow + 1 : nFirstRow );
        double fVal;

        for ( nDocRow = nFirstDataRow; nDocRow <= nLastRow; nDocRow++ )
        {
            xResultUpdate->moveToInsertRow();

            for (nCol=0; nCol<nColCount; nCol++)
            {
                SCCOL nDocCol = sal::static_int_cast<SCCOL>( nFirstCol + nCol );

                switch (pColTypes[nCol])
                {
                    case sdbc::DataType::LONGVARCHAR:
                    {
                        ScRefCellValue aCell(aDocument, ScAddress(nDocCol, nDocRow, nTab));
                        if (!aCell.isEmpty())
                        {
                            if (aCell.meType == CELLTYPE_EDIT)
                            {   // Paragraphs erhalten
                                lcl_getLongVarCharEditString(aString, aCell, aEditEngine);
                            }
                            else
                            {
                                lcl_getLongVarCharString(
                                    aString, aDocument, nDocCol, nDocRow, nTab, *pNumFmt);
                            }
                            xRowUpdate->updateString( nCol+1, aString );
                        }
                        else
                            xRowUpdate->updateNull( nCol+1 );
                    }
                    break;

                    case sdbc::DataType::VARCHAR:
                        aString = aDocument.GetString(nDocCol, nDocRow, nTab);
                        xRowUpdate->updateString( nCol+1, aString );
                        if ( nErr == eERR_OK && pColLengths[nCol] < aString.getLength() )
                            nErr = SCWARN_EXPORT_DATALOST;
                        break;

                    case sdbc::DataType::DATE:
                        {
                            aDocument.GetValue( nDocCol, nDocRow, nTab, fVal );
                            // zwischen 0 Wert und 0 kein Wert unterscheiden
                            bool bIsNull = (fVal == 0.0);
                            if ( bIsNull )
                                bIsNull = !aDocument.HasValueData( nDocCol, nDocRow, nTab );
                            if ( bIsNull )
                            {
                                xRowUpdate->updateNull( nCol+1 );
                                if ( nErr == eERR_OK &&
                                        aDocument.HasStringData( nDocCol, nDocRow, nTab ) )
                                    nErr = SCWARN_EXPORT_DATALOST;
                            }
                            else
                            {
                                Date aDate = *(pNumFmt->GetNullDate());     // tools date
                                aDate += (long)fVal;                        //! approxfloor?
                                xRowUpdate->updateDate( nCol+1, aDate.GetUNODate() );
                            }
                        }
                        break;

                    case sdbc::DataType::DECIMAL:
                    case sdbc::DataType::BIT:
                        aDocument.GetValue( nDocCol, nDocRow, nTab, fVal );
                        if ( fVal == 0.0 && nErr == eERR_OK &&
                                            aDocument.HasStringData( nDocCol, nDocRow, nTab ) )
                            nErr = SCWARN_EXPORT_DATALOST;
                        if ( pColTypes[nCol] == sdbc::DataType::BIT )
                            xRowUpdate->updateBoolean( nCol+1, ( fVal != 0.0 ) );
                        else
                            xRowUpdate->updateDouble( nCol+1, fVal );
                        break;

                    default:
                        OSL_FAIL( "ScDocShell::DBaseExport: unknown FieldType" );
                        if ( nErr == eERR_OK )
                            nErr = SCWARN_EXPORT_DATALOST;
                        aDocument.GetValue( nDocCol, nDocRow, nTab, fVal );
                        xRowUpdate->updateDouble( nCol+1, fVal );
                }
            }

            xResultUpdate->insertRow();

            //! error handling and recovery of old
            //! ScDocShell::SbaSdbExport is still missing!

            aProgress.SetStateOnPercent( nDocRow - nFirstRow );
        }

        comphelper::disposeComponent( xRowSet );
        comphelper::disposeComponent( xConnection );
    }
    catch ( const sdbc::SQLException& aException )
    {
        sal_Int32 nError = aException.ErrorCode;
        SAL_WARN("sc", "ScDocShell::DBaseExport: SQLException ErrorCode: " << nError << ", SQLState: " << aException.SQLState <<
            ", Message: " << aException.Message << "\n");

        if (nError == 22018 || nError == 22001)
        {
            // SQL error 22018: Character not in target encoding.
            // SQL error 22001: String length exceeds field width (after encoding).
            bool bEncErr = (nError == 22018);
            bool bIsOctetTextEncoding = rtl_isOctetTextEncoding( eCharSet);
            OSL_ENSURE( !bEncErr || bIsOctetTextEncoding, "ScDocShell::DBaseExport: encoding error and not an octect textencoding");
            SCCOL nDocCol = nFirstCol;
            const sal_Int32* pColTypes = aColTypes.getConstArray();
            const sal_Int32* pColLengths = aColLengths.getConstArray();
            ScHorizontalCellIterator aIter( &aDocument, nTab, nFirstCol,
                    nDocRow, nLastCol, nDocRow);
            ScRefCellValue* pCell = nullptr;
            bool bTest = true;
            while (bTest && ((pCell = aIter.GetNext( nDocCol, nDocRow)) != nullptr))
            {
                SCCOL nCol = nDocCol - nFirstCol;
                switch (pColTypes[nCol])
                {
                    case sdbc::DataType::LONGVARCHAR:
                        {
                            if (pCell->meType == CELLTYPE_EDIT)
                                lcl_getLongVarCharEditString(aString, *pCell, aEditEngine);
                            else
                                lcl_getLongVarCharString(
                                    aString, aDocument, nDocCol, nDocRow, nTab, *pNumFmt);
                        }
                        break;

                    case sdbc::DataType::VARCHAR:
                        aString = aDocument.GetString(nDocCol, nDocRow, nTab);
                        break;

                    // NOTE: length of DECIMAL fields doesn't need to be
                    // checked here, the database driver adjusts the field
                    // width accordingly.

                    default:
                        bTest = false;
                }
                if (bTest)
                {
                    sal_Int32 nLen;
                    if (bIsOctetTextEncoding)
                    {
                        OString aOString;
                        if (!aString.convertToString( &aOString, eCharSet,
                                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                                    RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
                        {
                            bTest = false;
                            bEncErr = true;
                        }
                        nLen = aOString.getLength();
                        if (!bTest)
                            SAL_WARN("sc", "ScDocShell::DBaseExport encoding error, string with default replacements: ``" << aString << "''\n");
                    }
                    else
                        nLen = aString.getLength() * sizeof(sal_Unicode);
                    if (!bEncErr &&
                            pColTypes[nCol] != sdbc::DataType::LONGVARCHAR &&
                            pColLengths[nCol] < nLen)
                    {
                        bTest = false;
                        SAL_INFO("sc", "ScDocShell::DBaseExport: field width: " << pColLengths[nCol] << ", encoded length: " << nLen << "\n");
                    }
                }
                else
                    bTest = true;
            }
            OUString sPosition( ScAddress( nDocCol, nDocRow, nTab).GetColRowString());
            OUString sEncoding( SvxTextEncodingTable().GetTextString( eCharSet));
            nErr = *new TwoStringErrorInfo( (bEncErr ? SCERR_EXPORT_ENCODING :
                        SCERR_EXPORT_FIELDWIDTH), sPosition, sEncoding,
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR);
        }
        else if ( !aException.Message.isEmpty() )
            nErr = *new StringErrorInfo( (SCERR_EXPORT_SQLEXCEPTION), aException.Message, ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR);
        else
            nErr = SCERR_EXPORT_DATA;
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL("Unexpected exception in database");
        nErr = ERRCODE_IO_GENERAL;
    }

    return nErr;
#endif // HAVE_FEATURE_DBCONNECTIVITY
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
