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

#include <stdio.h>
#include <tools/urlobj.hxx>
#include <svl/converter.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <ucbhelper/content.hxx>
#include <svx/txenctab.hxx>

#ifndef DISABLE_DBCONNECTIVITY
#include <svx/dbcharsethelper.hxx>
#endif

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/XDriverManager.hpp>
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
#include "cell.hxx"
#include "column.hxx"
#include "editutil.hxx"
#include "cellform.hxx"
#include "dbdocutl.hxx"
#include "dociter.hxx"
#include "globstr.hrc"
#include "svl/zformat.hxx"
#include "svl/intitem.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docpool.hxx"
#include "segmenttree.hxx"
#include "docparam.hxx"

#include <vector>
#include <boost/unordered_set.hpp>

using namespace com::sun::star;
using ::std::vector;

// -----------------------------------------------------------------------

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"
#define SC_SERVICE_DRVMAN           "com.sun.star.sdbc.DriverManager"

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

#ifndef DISABLE_DBCONNECTIVITY

namespace
{
    sal_uLong lcl_getDBaseConnection(uno::Reference<sdbc::XDriverManager>& _rDrvMgr,uno::Reference<sdbc::XConnection>& _rConnection,String& _rTabName,const String& rFullFileName,rtl_TextEncoding eCharSet)
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rFullFileName );
        _rTabName = aURL.getBase( INetURLObject::LAST_SEGMENT, true,
                INetURLObject::DECODE_UNAMBIGUOUS );
        String aExtension = aURL.getExtension();
        aURL.removeSegment();
        aURL.removeFinalSlash();
        String aPath = aURL.GetMainURL(INetURLObject::NO_DECODE);
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        if (!xFactory.is()) return SCERR_EXPORT_CONNECT;

        _rDrvMgr.set( xFactory->createInstance(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_SERVICE_DRVMAN )) ),
                            uno::UNO_QUERY);
        OSL_ENSURE( _rDrvMgr.is(), "can't get DriverManager" );
        if (!_rDrvMgr.is()) return SCERR_EXPORT_CONNECT;

        // get connection

        String aConnUrl = rtl::OUString("sdbc:dbase:");
        aConnUrl += aPath;

        svxform::ODataAccessCharsetHelper aHelper;
        ::std::vector< rtl_TextEncoding > aEncodings;
        aHelper.getSupportedTextEncodings( aEncodings );
        ::std::vector< rtl_TextEncoding >::iterator aIter = ::std::find(aEncodings.begin(),aEncodings.end(),(rtl_TextEncoding) eCharSet);
        if ( aIter == aEncodings.end() )
        {
            OSL_FAIL( "DBaseImport: dbtools::OCharsetMap doesn't know text encoding" );
            return SCERR_IMPORT_CONNECT;
        } // if ( aIter == aMap.end() )
        rtl::OUString aCharSetStr;
        if ( RTL_TEXTENCODING_DONTKNOW != *aIter )
        {   // it's not the virtual "system charset"
            const char* pIanaName = rtl_getMimeCharsetFromTextEncoding( *aIter );
            OSL_ENSURE( pIanaName, "invalid mime name!" );
            if ( pIanaName )
                aCharSetStr = ::rtl::OUString::createFromAscii( pIanaName );
        }

        uno::Sequence<beans::PropertyValue> aProps(2);
        aProps[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_EXTENSION));
        aProps[0].Value <<= rtl::OUString( aExtension );
        aProps[1].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_CHARSET));
        aProps[1].Value <<= aCharSetStr;

        _rConnection = _rDrvMgr->getConnectionWithInfo( aConnUrl, aProps );
        return 0L;
    }
}

#endif // !DISABLE_DBCONNECTIVITY

// -----------------------------------------------------------------------
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
    String aName = rDestObj.getName();
    INetURLObject aDestPathObj = rDestObj;
    aDestPathObj.removeSegment();
    aDestPathObj.setFinalSlash();

    try
    {
        ::ucbhelper::Content aDestPath( aDestPathObj.GetMainURL(INetURLObject::NO_DECODE),
                            uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        uno::Reference< ::com::sun::star::ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        rtl::OUString aTransferName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "transfer" ));
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, uno::makeAny(
                ::com::sun::star::ucb::TransferInfo( bMoveData, rSourceObj.GetMainURL(INetURLObject::NO_DECODE), aName,
                                                       ::com::sun::star::ucb::NameClash::ERROR ) ) );
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
        ::ucbhelper::Content aCnt( rURL.GetMainURL(INetURLObject::NO_DECODE),
                        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "delete" )),
                                comphelper::makeBoolAny( sal_True ) );
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
        ::ucbhelper::Content aCnt( rURL.GetMainURL(INetURLObject::NO_DECODE),
                        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isDocument();
    }
    catch( uno::Exception& )
    {
        // ucb may throw different exceptions on failure now - warning only
        OSL_FAIL( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

#ifndef DISABLE_DBCONNECTIVITY

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
        String aNewPicture;
        pFormatter->GenerateFormat(aNewPicture, nOldFormat, eLang,
                                   bThousand, bNegRed, nPrecision, nLeading);

        sal_uInt32 nNewFormat = pFormatter->GetEntryKey(aNewPicture, eLang);
        if (nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            xub_StrLen nErrPos = 0;
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

#endif // !DISABLE_DBCONNECTIVITY

sal_uLong ScDocShell::DBaseImport( const String& rFullFileName, CharSet eCharSet,
                               ScColWidthParam aColWidthParam[MAXCOLCOUNT], ScFlatBoolRowSegments& rRowHeightsRecalc )
{
#ifdef DISABLE_DBCONNECTIVITY
    (void) rFullFileName;
    (void) eCharSet;
    (void) aColWidthParam;
    (void) rRowHeightsRecalc;

    return ERRCODE_IO_GENERAL;
#else
    ScColumn::DoubleAllocSwitch aAllocSwitch(true);

    sal_uLong nErr = eERR_OK;
    long i;
    long nColCount = 0;

    // Try to get the Text Encoding from the driver
    if( eCharSet == RTL_TEXTENCODING_IBM_850 )
        eCharSet = RTL_TEXTENCODING_DONTKNOW;

    try
    {
        String aTabName;
        uno::Reference<sdbc::XDriverManager> xDrvMan;
        uno::Reference<sdbc::XConnection> xConnection;
        sal_uLong nRet = lcl_getDBaseConnection(xDrvMan,xConnection,aTabName,rFullFileName,eCharSet);
        if ( !xConnection.is() || !xDrvMan.is() )
            return nRet;
        ::utl::DisposableComponent aConnectionHelper(xConnection);

        ScProgress aProgress( this, ScGlobal::GetRscString( STR_LOAD_DOC ), 0 );
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_SERVICE_ROWSET )) ),
                            uno::UNO_QUERY);
        ::utl::DisposableComponent aRowSetHelper(xRowSet);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is()) return SCERR_IMPORT_CONNECT;

        sal_Int32 nType = sdb::CommandType::TABLE;
        uno::Any aAny;

        aAny <<= xConnection;
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_ACTIVECONNECTION)), aAny );

        aAny <<= nType;
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMANDTYPE)), aAny );

        aAny <<= rtl::OUString( aTabName );
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMAND)), aAny );

        aAny <<= false;
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_PROPCHANGE_NOTIFY)), aAny );

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
        ScColumn::bDoubleAlloc = true;      // row count isn't readily available in advance

        vector<long> aScales(nColCount, -1);
        for (i=0; i<nColCount; i++)
        {
            String aHeader = xMeta->getColumnLabel( i+1 );

            switch ( pTypeArr[i] )
            {
                case sdbc::DataType::BIT:
                    aHeader.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",L" ));
                    break;
                case sdbc::DataType::DATE:
                    aHeader.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",D" ));
                    break;
                case sdbc::DataType::LONGVARCHAR:
                    aHeader.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",M" ));
                    break;
                case sdbc::DataType::VARCHAR:
                    aHeader.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",C," ));
                    aHeader += String::CreateFromInt32( xMeta->getColumnDisplaySize( i+1 ) );
                    break;
                case sdbc::DataType::DECIMAL:
                    {
                        long nPrec = xMeta->getPrecision( i+1 );
                        long nScale = xMeta->getScale( i+1 );
                        aHeader.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",N," ));
                        aHeader += String::CreateFromInt32(
                                    SvDbaseConverter::ConvertPrecisionToDbase(
                                        nPrec, nScale ) );
                        aHeader += ',';
                        aHeader += String::CreateFromInt32( nScale );
                        aScales[i] = nScale;
                    }
                    break;
            }

            aDocument.SetString( static_cast<SCCOL>(i), 0, 0, aHeader );
        }

        lcl_setScalesToColumns(aDocument, aScales);

        SCROW nRow = 1;     // 0 is column titles
        sal_Bool bEnd = false;
        while ( !bEnd && xRowSet->next() )
        {
            bool bSimpleRow = true;
            if ( nRow <= MAXROW )
            {
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
                bEnd = sal_True;                            // don't continue
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

    ScColumn::bDoubleAlloc = false;
    if ( nColCount > 0 )
        aDocument.DoColResize( 0, 0, static_cast<SCCOL>(nColCount) - 1, 0 );

    return nErr;
#endif // !DISABLE_DBCONNECTIVITY
}

#ifndef DISABLE_DBCONNECTIVITY

namespace {

inline bool IsAsciiDigit( sal_Unicode c )
{
    return 0x30 <= c && c <= 0x39;
}

inline bool IsAsciiAlpha( sal_Unicode c )
{
    return (0x41 <= c && c <= 0x5a) || (0x61 <= c && c <= 0x7a);
}

void lcl_GetColumnTypes(
    ScDocShell& rDocShell, const ScRange& rDataRange, bool bHasFieldNames,
    rtl::OUString* pColNames, sal_Int32* pColTypes, sal_Int32* pColLengths,
    sal_Int32* pColScales, bool& bHasMemo, CharSet eCharSet )
{
    //  updating of column titles didn't work in 5.2 and isn't always wanted
    //  (saving normally shouldn't modify the document)
    //! read flag from configuration
    bool bUpdateTitles = false;

    ScDocument* pDoc = rDocShell.GetDocument();
    SvNumberFormatter* pNumFmt = pDoc->GetFormatTable();

    SCTAB nTab = rDataRange.aStart.Tab();
    SCCOL nFirstCol = rDataRange.aStart.Col();
    SCROW nFirstRow = rDataRange.aStart.Row();
    SCCOL nLastCol = rDataRange.aEnd.Col();
    SCROW nLastRow = rDataRange.aEnd.Row();

    typedef boost::unordered_set<rtl::OUString, rtl::OUStringHash> StrSetType;
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
        String aFieldName, aString;

        // Feldname[,Type[,Width[,Prec]]]
        // Typ etc.: L; D; C[,W]; N[,W[,P]]
        if ( bHasFieldNames )
        {
            pDoc->GetString( nCol, nFirstRow, nTab, aString );
            aString.ToUpperAscii();
            xub_StrLen nToken = comphelper::string::getTokenCount(aString, ',');
            if ( nToken > 1 )
            {
                aFieldName = aString.GetToken( 0, ',' );
                aString = comphelper::string::remove(aString, ' ');
                switch ( aString.GetToken( 1, ',' ).GetChar(0) )
                {
                    case 'L' :
                        nDbType = sdbc::DataType::BIT;
                        nFieldLen = 1;
                        bTypeDefined = sal_True;
                        bPrecDefined = sal_True;
                        break;
                    case 'D' :
                        nDbType = sdbc::DataType::DATE;
                        nFieldLen = 8;
                        bTypeDefined = sal_True;
                        bPrecDefined = sal_True;
                        break;
                    case 'M' :
                        nDbType = sdbc::DataType::LONGVARCHAR;
                        nFieldLen = 10;
                        bTypeDefined = sal_True;
                        bPrecDefined = sal_True;
                        bHasMemo = sal_True;
                        break;
                    case 'C' :
                        nDbType = sdbc::DataType::VARCHAR;
                        bTypeDefined = sal_True;
                        bPrecDefined = sal_True;
                        break;
                    case 'N' :
                        nDbType = sdbc::DataType::DECIMAL;
                        break;
                }
                if ( bTypeDefined && !nFieldLen && nToken > 2 )
                {
                    nFieldLen = aString.GetToken( 2, ',' ).ToInt32();
                    if ( !bPrecDefined && nToken > 3 )
                    {
                        String aTmp( aString.GetToken( 3, ',' ) );
                        if ( CharClass::isAsciiNumeric(aTmp) )
                        {
                            nPrecision = aTmp.ToInt32();
                            bPrecDefined = sal_True;
                        }
                    }
                }
            }
            else
                aFieldName = aString;

            // Feldnamen pruefen und ggbf. gueltigen Feldnamen erzeugen.
            // Erstes Zeichen muss Buchstabe sein,
            // weitere nur alphanumerisch und Unterstrich erlaubt,
            // "_DBASELOCK" ist reserviert (obsolet weil erstes Zeichen kein Buchstabe),
            // keine doppelten Namen.
            if ( !IsAsciiAlpha( aFieldName.GetChar(0) ) )
                aFieldName.Insert( 'N', 0 );
            String aTmpStr;
            sal_Unicode c;
            for ( const sal_Unicode* p = aFieldName.GetBuffer(); ( c = *p ) != 0; p++ )
            {
                if ( IsAsciiAlpha( c ) || IsAsciiDigit( c ) || c == '_' )
                    aTmpStr += c;
                else
                    aTmpStr += '_';
            }
            aFieldName = aTmpStr;
            if ( aFieldName.Len() > 10 )
                aFieldName.Erase( 10 );

            if (!aFieldNames.insert(aFieldName).second)
            {   // doppelter Feldname, numerisch erweitern
                sal_uInt16 nSub = 1;
                String aFixPart( aFieldName );
                do
                {
                    ++nSub;
                    String aVarPart = String::CreateFromInt32( nSub );
                    if ( aFixPart.Len() + aVarPart.Len() > 10 )
                        aFixPart.Erase( 10 - aVarPart.Len() );
                    aFieldName = aFixPart;
                    aFieldName += aVarPart;
                } while (!aFieldNames.insert(aFieldName).second);
            }
        }
        else
        {
            aFieldName = 'N';
            aFieldName += String::CreateFromInt32(nCol+1);
        }

        if ( !bTypeDefined )
        {   // Feldtyp
            ScBaseCell* pCell;
            pDoc->GetCell( nCol, nFirstDataRow, nTab, pCell );
            if ( !pCell || pCell->HasStringData() )
                nDbType = sdbc::DataType::VARCHAR;
            else
            {
                sal_uInt32 nFormat;
                pDoc->GetNumberFormat( nCol, nFirstDataRow, nTab, nFormat );
                if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA
                  && ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) )
                {
                    nFormat = ScGlobal::GetStandardFormat(
                        ((ScFormulaCell*)pCell)->GetValue(), *pNumFmt, nFormat,
                        ((ScFormulaCell*)pCell)->GetFormatType() );
                }
                switch ( pNumFmt->GetType( nFormat ) )
                {
                    case NUMBERFORMAT_LOGICAL :
                        nDbType = sdbc::DataType::BIT;
                        nFieldLen = 1;
                        break;
                    case NUMBERFORMAT_DATE :
                        nDbType = sdbc::DataType::DATE;
                        nFieldLen = 8;
                        break;
                    case NUMBERFORMAT_TIME :
                    case NUMBERFORMAT_DATETIME :
                        nDbType = sdbc::DataType::VARCHAR;
                        break;
                    default:
                        nDbType = sdbc::DataType::DECIMAL;
                }
            }
        }
        bool bSdbLenAdjusted = false;
        bool bSdbLenBad = false;
        // Feldlaenge
        if ( nDbType == sdbc::DataType::VARCHAR && !nFieldLen )
        {   // maximale Feldbreite bestimmen
            nFieldLen = pDoc->GetMaxStringLen( nTab, nCol, nFirstDataRow,
                nLastRow, eCharSet );
            if ( nFieldLen == 0 )
                nFieldLen = 1;
        }
        else if ( nDbType == sdbc::DataType::DECIMAL )
        {   // maximale Feldbreite und Nachkommastellen bestimmen
            xub_StrLen nLen;
            sal_uInt16 nPrec;
            nLen = pDoc->GetMaxNumberStringLen( nPrec, nTab, nCol,
                nFirstDataRow, nLastRow );
            // dBaseIII Limit Nachkommastellen: 15
            if ( nPrecision > 15 )
                nPrecision = 15;
            if ( nPrec > 15 )
                nPrec = 15;
            if ( bPrecDefined && nPrecision != nPrec )
            {   // Laenge auf vorgegebene Nachkommastellen anpassen
                if ( nPrecision )
                    nLen = sal::static_int_cast<xub_StrLen>( nLen + ( nPrecision - nPrec ) );
                else
                    nLen -= nPrec+1;            // auch den . mit raus
            }
            if ( nLen > nFieldLen && !bTypeDefined )
                nFieldLen = nLen;
            if ( !bPrecDefined )
                nPrecision = nPrec;
            if ( nFieldLen == 0 )
                nFieldLen = 1;
            else if ( nFieldLen > 19 )
                nFieldLen = 19;     // dBaseIII Limit Feldlaenge numerisch: 19
            if ( nPrecision && nFieldLen < nPrecision + 2 )
                nFieldLen = nPrecision + 2;     // 0. muss mit reinpassen
            // 538 MUST: Sdb internal representation adds 2 to the field length!
            // To give the user what he wants we must substract it here.
             //! CAVEAT! There is no way to define a numeric field with a length
             //! of 1 and no decimals!
            if ( nFieldLen == 1 && nPrecision == 0 )
                bSdbLenBad = sal_True;
            nFieldLen = SvDbaseConverter::ConvertPrecisionToOdbc( nFieldLen, nPrecision );
            bSdbLenAdjusted = sal_True;
        }
        if ( nFieldLen > 254 )
        {
            if ( nDbType == sdbc::DataType::VARCHAR )
            {   // zu lang fuer normales Textfeld => Memofeld
                nDbType = sdbc::DataType::LONGVARCHAR;
                nFieldLen = 10;
                bHasMemo = sal_True;
            }
            else
                nFieldLen = 254;                    // dumm gelaufen..
        }

        pColNames[nField] = aFieldName;
        pColTypes[nField] = nDbType;
        pColLengths[nField] = nFieldLen;
        pColScales[nField] = nPrecision;

        // undo change to field length, reflect reality
        if ( bSdbLenAdjusted )
        {
            nFieldLen = SvDbaseConverter::ConvertPrecisionToDbase( nFieldLen, nPrecision );
            if ( bSdbLenBad && nFieldLen == 1 )
                nFieldLen = 2;      // THIS is reality
        }
        if ( bUpdateTitles )
        {   // Angabe anpassen und ausgeben
            String aOutString = aFieldName;
            switch ( nDbType )
            {
                case sdbc::DataType::BIT :
                    aOutString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",L" ));
                    break;
                case sdbc::DataType::DATE :
                    aOutString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",D" ));
                    break;
                case sdbc::DataType::LONGVARCHAR :
                    aOutString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",M" ));
                    break;
                case sdbc::DataType::VARCHAR :
                    aOutString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",C," ));
                    aOutString += String::CreateFromInt32( nFieldLen );
                    break;
                case sdbc::DataType::DECIMAL :
                    aOutString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",N," ));
                    aOutString += String::CreateFromInt32( nFieldLen );
                    aOutString += ',';
                    aOutString += String::CreateFromInt32( nPrecision );
                    break;
            }
            if ( !aOutString.EqualsIgnoreCaseAscii( aString ) )
            {
                pDoc->SetString( nCol, nFirstRow, nTab, aOutString );
                rDocShell.PostPaint( nCol, nFirstRow, nTab, nCol, nFirstRow, nTab, PAINT_GRID );
            }
        }
        ++nField;
    }
}

inline void lcl_getLongVarCharEditString( rtl::OUString& rString,
        const ScBaseCell* pCell, ScFieldEditEngine& rEditEngine )
{
    rEditEngine.SetText( *((const ScEditCell*)pCell)->GetData() );
    rString = rEditEngine.GetText( LINEEND_CRLF );
}

inline void lcl_getLongVarCharString( rtl::OUString& rString, ScBaseCell* pCell,
        ScDocument& rDocument, SCCOL nCol, SCROW nRow, SCTAB nTab,
        SvNumberFormatter& rNumFmt )
{
    sal_uInt32 nFormat;
    Color* pColor;
    rDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
    ScCellFormat::GetString( pCell, nFormat, rString, &pColor, rNumFmt );
}

}

#endif // !DISABLE_DBCONNECTIVITY

sal_uLong ScDocShell::DBaseExport( const rtl::OUString& rFullFileName, CharSet eCharSet, bool& bHasMemo )
{
#ifdef DISABLE_DBCONNECTIVITY
    (void) rFullFileName;
    (void) eCharSet;
    (void) bHasMemo;

    return ERRCODE_IO_GENERAL;
#else
    // remove the file so the dBase driver doesn't find an invalid file
    INetURLObject aDeleteObj( rFullFileName, INET_PROT_FILE );
    KillFile( aDeleteObj );

    sal_uLong nErr = eERR_OK;
    uno::Any aAny;

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
                                                    nLastRow - nFirstRow );
    SvNumberFormatter* pNumFmt = aDocument.GetFormatTable();

    sal_Bool bHasFieldNames = sal_True;
    for ( SCCOL nDocCol = nFirstCol; nDocCol <= nLastCol && bHasFieldNames; nDocCol++ )
    {   // nur Strings in erster Zeile => sind Feldnamen
        if ( !aDocument.HasStringData( nDocCol, nFirstRow, nTab ) )
            bHasFieldNames = false;
    }

    long nColCount = nLastCol - nFirstCol + 1;
    uno::Sequence<rtl::OUString> aColNames( nColCount );
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
    rtl::OUString aString;
    String aTabName;

    try
    {
        uno::Reference<sdbc::XDriverManager> xDrvMan;
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

        aAny <<= rtl::OUString( aTabName );
        xTableDesc->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_NAME)), aAny );

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

        const rtl::OUString* pColNames = aColNames.getConstArray();
        const sal_Int32* pColTypes     = aColTypes.getConstArray();
        const sal_Int32* pColLengths   = aColLengths.getConstArray();
        const sal_Int32* pColScales    = aColScales.getConstArray();
        long nCol;

        for (nCol=0; nCol<nColCount; nCol++)
        {
            uno::Reference<beans::XPropertySet> xColumnDesc = xColumnsFact->createDataDescriptor();
            OSL_ENSURE( xColumnDesc.is(), "can't get column descriptor" );
            if (!xColumnDesc.is()) return SCERR_EXPORT_CONNECT;

            aAny <<= pColNames[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_NAME)), aAny );

            aAny <<= pColTypes[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_TYPE)), aAny );

            aAny <<= pColLengths[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_PRECISION)), aAny );

            aAny <<= pColScales[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_SCALE)), aAny );

            xColumnsAppend->appendByDescriptor( xColumnDesc );
        }

        xTablesAppend->appendByDescriptor( xTableDesc );

        // get row set for writing
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_SERVICE_ROWSET )) ),
                            uno::UNO_QUERY);
        ::utl::DisposableComponent aRowSetHelper(xRowSet);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is()) return SCERR_EXPORT_CONNECT;

        aAny <<= xConnection;
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_ACTIVECONNECTION)), aAny );

        aAny <<= (sal_Int32) sdb::CommandType::TABLE;
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMANDTYPE)), aAny );

        aAny <<= rtl::OUString( aTabName );
        xRowProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMAND)), aAny );

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
                            ScBaseCell* pCell;
                            aDocument.GetCell( nDocCol, nDocRow, nTab, pCell );
                            if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                            {
                                if ( pCell->GetCellType() == CELLTYPE_EDIT )
                                {   // Paragraphs erhalten
                                    lcl_getLongVarCharEditString( aString,
                                            pCell, aEditEngine);
                                }
                                else
                                {
                                    lcl_getLongVarCharString( aString, pCell,
                                            aDocument, nDocCol, nDocRow, nTab,
                                            *pNumFmt);
                                }
                                xRowUpdate->updateString( nCol+1, aString );
                            }
                            else
                                xRowUpdate->updateNull( nCol+1 );
                        }
                        break;

                    case sdbc::DataType::VARCHAR:
                        aDocument.GetString( nDocCol, nDocRow, nTab, aString );
                        xRowUpdate->updateString( nCol+1, aString );
                        if ( nErr == eERR_OK && pColLengths[nCol] < aString.getLength() )
                            nErr = SCWARN_EXPORT_DATALOST;
                        break;

                    case sdbc::DataType::DATE:
                        {
                            aDocument.GetValue( nDocCol, nDocRow, nTab, fVal );
                            // zwischen 0 Wert und 0 kein Wert unterscheiden
                            sal_Bool bIsNull = (fVal == 0.0);
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
                                util::Date aUnoDate( aDate.GetDay(), aDate.GetMonth(), aDate.GetYear() );
                                xRowUpdate->updateDate( nCol+1, aUnoDate );
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

            if ( !aProgress.SetStateOnPercent( nDocRow - nFirstRow ) )
            {   // UserBreak
                nErr = SCERR_EXPORT_DATA;
                break;
            }
        }

        comphelper::disposeComponent( xRowSet );
        comphelper::disposeComponent( xConnection );
    }
    catch ( const sdbc::SQLException& aException )
    {
        sal_Int32 nError = aException.ErrorCode;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "ScDocShell::DBaseExport: SQLException ErrorCode: %d, SQLState: %s, Message: %s\n",
                (int)nError, OUStringToOString( aException.SQLState,
                    RTL_TEXTENCODING_UTF8).getStr(), OUStringToOString(
                        aException.Message, RTL_TEXTENCODING_UTF8).getStr());
#endif
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
            ScBaseCell* pCell = NULL;
            bool bTest = true;
            while (bTest && ((pCell = aIter.GetNext( nDocCol, nDocRow)) != NULL))
            {
                SCCOL nCol = nDocCol - nFirstCol;
                switch (pColTypes[nCol])
                {
                    case sdbc::DataType::LONGVARCHAR:
                        {
                            if ( pCell->GetCellType() != CELLTYPE_NOTE )
                            {
                                if ( pCell->GetCellType() == CELLTYPE_EDIT )
                                    lcl_getLongVarCharEditString( aString,
                                            pCell, aEditEngine);
                                else
                                    lcl_getLongVarCharString( aString,
                                            pCell, aDocument, nDocCol,
                                            nDocRow, nTab, *pNumFmt);
                            }
                        }
                        break;

                    case sdbc::DataType::VARCHAR:
                        aDocument.GetString( nDocCol, nDocRow, nTab, aString);
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
                        rtl::OUString aOUString( aString);
                        rtl::OString aOString;
                        if (!aOUString.convertToString( &aOString, eCharSet,
                                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                                    RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
                        {
                            bTest = false;
                            bEncErr = true;
                        }
                        nLen = aOString.getLength();
#if OSL_DEBUG_LEVEL > 1
                        if (!bTest)
                            fprintf( stderr, "ScDocShell::DBaseExport encoding error, string with default replacements: ``%s''\n",
                                    OUStringToOString( aOUString, eCharSet).getStr());
#endif
                    }
                    else
                        nLen = aString.getLength() * sizeof(sal_Unicode);
                    if (!bEncErr &&
                            pColTypes[nCol] != sdbc::DataType::LONGVARCHAR &&
                            pColLengths[nCol] < nLen)
                    {
                        bTest = false;
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "ScDocShell::DBaseExport: field width: %d, encoded length: %d\n",
                                (int)pColLengths[nCol], (int)nLen);
#endif
                    }
                }
                else
                    bTest = true;
            }
            String sPosition( ScAddress( nDocCol, nDocRow, nTab).GetColRowString());
            String sEncoding( SvxTextEncodingTable().GetTextString( eCharSet));
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
#endif // !DISABLE_DBCONNECTIVITY
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
