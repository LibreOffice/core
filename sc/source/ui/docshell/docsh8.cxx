/*************************************************************************
 *
 *  $RCSfile: docsh8.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-27 19:03:06 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/fsys.hxx>       //! Test !!!

#include <tools/urlobj.hxx>
#include <svtools/converter.hxx>
#include <svtools/zforlist.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
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

#include "scerrors.hxx"
#include "docsh.hxx"
#include "filter.hxx"
#include "progress.hxx"
#include "collect.hxx"
#include "cell.hxx"
#include "editutil.hxx"
#include "cellform.hxx"
#include "dbdocutl.hxx"
#include "globstr.hrc"

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"
#define SC_SERVICE_DRVMAN           "com.sun.star.sdbc.DriverManager"

//! move to a header file?
//#define SC_DBPROP_DATASOURCENAME  "DataSourceName"
#define SC_DBPROP_ACTIVECONNECTION  "ActiveConnection"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"

#define SC_DBPROP_NAME              "Name"
#define SC_DBPROP_TYPE              "Type"
#define SC_DBPROP_PRECISION         "Precision"
#define SC_DBPROP_SCALE             "Scale"

#define SC_DBPROP_EXTENSION         "Extension"
#define SC_DBPROP_CHARSET           "CharSet"

#define SC_ROWCOUNT_ERROR       (-1)

// -----------------------------------------------------------------------

long lcl_GetRowCount( const uno::Reference<sdbc::XConnection>& xConnection,
                        const String& rTabName )
{
    try
    {
        uno::Reference<sdbc::XStatement> xStatement = xConnection->createStatement();
        DBG_ASSERT( xStatement.is(), "can't get Statement" );
        if (!xStatement.is()) return SC_ROWCOUNT_ERROR;

        String aQuoteStr;
        uno::Reference<sdbc::XDatabaseMetaData> xDBMeta = xConnection->getMetaData();
        if (xDBMeta.is())
            aQuoteStr = xDBMeta->getIdentifierQuoteString();

        String aSql = String::CreateFromAscii("SELECT COUNT ( * ) FROM ");
        aSql += aQuoteStr;
        aSql += rTabName;
        aSql += aQuoteStr;

        uno::Reference<sdbc::XResultSet> xResSet = xStatement->executeQuery( aSql );
        uno::Reference<sdbc::XRow> xRow( xResSet, uno::UNO_QUERY );
        DBG_ASSERT( xRow.is(), "can't get Row" );
        if (!xRow.is()) return SC_ROWCOUNT_ERROR;

        if ( xResSet->next() )
            return xRow->getInt( 1 );
    }
    catch ( sdbc::SQLException& )
    {
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
    }

    return SC_ROWCOUNT_ERROR;
}

ULONG ScDocShell::DBaseImport( const String& rFullFileName, CharSet eCharSet,
                                BOOL bSimpleColWidth[MAXCOL+1] )
{
    ULONG nErr = eERR_OK;
    long i;

    try
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rFullFileName );
        String aTabName = aURL.getBase( INetURLObject::LAST_SEGMENT, true,
                                        INetURLObject::DECODE_UNAMBIGUOUS );
        String aExtension = aURL.getExtension();
        aURL.removeSegment();
        String aPath = aURL.GetMainURL();

        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        if (!xFactory.is())
            return ERRCODE_IO_GENERAL;

        uno::Reference<sdbc::XDriverManager> xDrvMan( xFactory->createInstance(
                            rtl::OUString::createFromAscii( SC_SERVICE_DRVMAN ) ),
                            uno::UNO_QUERY);
        DBG_ASSERT( xDrvMan.is(), "can't get DriverManager" );
        if (!xDrvMan.is()) return SCERR_IMPORT_CONNECT;

        String aConnUrl = String::CreateFromAscii("sdbc:dbase:");
        aConnUrl += aPath;

        uno::Sequence<beans::PropertyValue> aProps(2);
        aProps[0].Name = rtl::OUString::createFromAscii(SC_DBPROP_EXTENSION);
        aProps[0].Value <<= rtl::OUString( aExtension );
        aProps[1].Name = rtl::OUString::createFromAscii(SC_DBPROP_CHARSET);
        aProps[1].Value <<= (rtl_TextEncoding) eCharSet;

        uno::Reference<sdbc::XConnection> xConnection =
                                xDrvMan->getConnectionWithInfo( aConnUrl, aProps );
        DBG_ASSERT( xConnection.is(), "can't get Connection" );
        if (!xConnection.is()) return SCERR_IMPORT_CONNECT;

        long nRowCount = lcl_GetRowCount( xConnection, aTabName );
        if ( nRowCount < 0 )
        {
            DBG_ERROR("can't get row count");
            nRowCount = 0;
        }

        ScProgress aProgress( this, ScGlobal::GetRscString( STR_LOAD_DOC ), nRowCount );

        uno::Reference<sdbc::XResultSet> xResSet;
        BOOL bUseStatement = FALSE;
        if (bUseStatement)
        {
            uno::Reference<sdbc::XStatement> xStatement = xConnection->createStatement();
            DBG_ASSERT( xStatement.is(), "can't get Statement" );
            if (!xStatement.is()) return SCERR_IMPORT_CONNECT;

            String aQuoteStr;
            uno::Reference<sdbc::XDatabaseMetaData> xDBMeta = xConnection->getMetaData();
            if (xDBMeta.is())
                aQuoteStr = xDBMeta->getIdentifierQuoteString();

            //! don't encode blanks etc.

            String aSql = String::CreateFromAscii("select * from ");
            aSql += aQuoteStr;
            aSql += aTabName;
            aSql += aQuoteStr;

            xResSet = xStatement->executeQuery( aSql );
            DBG_ASSERT( xResSet.is(), "can't get ResultSet" );
            if (!xResSet.is()) return SCERR_IMPORT_CONNECT;
        }
        else
        {
            //! this doesn't work?

            uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(
                                rtl::OUString::createFromAscii( SC_SERVICE_ROWSET ) ),
                                uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
            DBG_ASSERT( xRowProp.is(), "can't get RowSet" );
            if (!xRowProp.is()) return SCERR_IMPORT_CONNECT;

            sal_Int32 nType = sdb::CommandType::TABLE;
            uno::Any aAny;

            aAny <<= xConnection;
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_ACTIVECONNECTION), aAny );

            aAny <<= nType;
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_COMMANDTYPE), aAny );

            aAny <<= rtl::OUString( aTabName );
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_COMMAND), aAny );

            xRowSet->execute();

            xResSet = xRowSet.get();
        }

        long nColCount = 0;
        uno::Reference<sdbc::XResultSetMetaData> xMeta;
        uno::Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp( xResSet, uno::UNO_QUERY );
        if ( xMetaSupp.is() )
            xMeta = xMetaSupp->getMetaData();
        if ( xMeta.is() )
            nColCount = xMeta->getColumnCount();    // this is the number of real columns

        if ( nColCount > MAXCOL+1 )
        {
            nColCount = MAXCOL+1;
            nErr = SCWARN_IMPORT_RANGE_OVERFLOW;    // warning
        }

        if ( nColCount > 0 )
            aDocument.DoColResize( 0, 0, nColCount - 1, nRowCount + 1 );

        uno::Reference<sdbc::XRow> xRow( xResSet, uno::UNO_QUERY );
        DBG_ASSERT( xRow.is(), "can't get Row" );
        if (!xRow.is()) return SCERR_IMPORT_CONNECT;

        // currency flag is not needed for dBase
        uno::Sequence<sal_Int32> aColTypes( nColCount );    // column types
        sal_Int32* pTypeArr = aColTypes.getArray();
        for (i=0; i<nColCount; i++)
            pTypeArr[i] = xMeta->getColumnType( i+1 );

        //  read column names
        //! add type descriptions

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
                    }
                    break;
            }

            aDocument.SetString( i, 0, 0, aHeader );
        }

        USHORT nRow = 1;        // 0 is column titles
        BOOL bEnd = FALSE;
        while ( !bEnd && xResSet->next() )
        {
            if ( nRow <= MAXROW )
            {
                USHORT nCol = 0;
                for (i=0; i<nColCount; i++)
                {
                    ScDatabaseDocUtil::PutData( &aDocument, nCol, nRow, 0,
                                                xRow, i+1, pTypeArr[i], FALSE,
                                                &bSimpleColWidth[nCol] );
                    ++nCol;
                }
                ++nRow;
            }
            else        // past the end of the spreadsheet
            {
                bEnd = TRUE;                            // don't continue
                nErr = SCWARN_IMPORT_RANGE_OVERFLOW;    // warning message
            }

            if ( nRowCount )
                aProgress.SetStateOnPercent( nRow );
        }
    }
    catch ( sdbc::SQLException& )
    {
        nErr = SCERR_IMPORT_CONNECT;
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
        nErr = ERRCODE_IO_GENERAL;
    }

    return nErr;
}

// -----------------------------------------------------------------------

inline IsAsciiDigit( sal_Unicode c )
{
    return 0x31 <= c && c <= 0x39;
}

inline IsAsciiAlpha( sal_Unicode c )
{
    return (0x41 <= c && c <= 0x5a) || (0x61 <= c && c <= 0x7a);
}

void lcl_GetColumnTypes( ScDocShell& rDocShell,
                            const ScRange& rDataRange, BOOL bHasFieldNames,
                            rtl::OUString* pColNames, sal_Int32* pColTypes,
                            sal_Int32* pColLengths, sal_Int32* pColScales,
                            BOOL& bHasMemo )
{
    //  updating of column titles didn't work in 5.2 and isn't always wanted
    //  (saving normally shouldn't modify the document)
    //! read flag from configuration
    BOOL bUpdateTitles = FALSE;

    ScDocument* pDoc = rDocShell.GetDocument();
    SvNumberFormatter* pNumFmt = pDoc->GetFormatTable();

    USHORT nTab = rDataRange.aStart.Tab();
    USHORT nFirstCol = rDataRange.aStart.Col();
    USHORT nFirstRow = rDataRange.aStart.Row();
    USHORT nLastCol = rDataRange.aEnd.Col();
    USHORT nLastRow = rDataRange.aEnd.Row();

    StrCollection aFieldNamesCollection;

    long nField = 0;
    USHORT nFirstDataRow = ( bHasFieldNames ? nFirstRow + 1 : nFirstRow );
    for ( USHORT nCol = nFirstCol; nCol <= nLastCol; nCol++ )
    {
        BOOL bTypeDefined = FALSE;
        BOOL bPrecDefined = FALSE;
        long nFieldLen = 0;
        long nPrecision = 0;
        long nDbType = sdbc::DataType::SQLNULL;
        String aFieldName, aString;

        // Feldname[,Type[,Width[,Prec]]]
        // Typ etc.: L; D; C[,W]; N[,W[,P]]
        if ( bHasFieldNames )
        {
            pDoc->GetString( nCol, nFirstRow, nTab, aString );
            aString.ToUpperAscii();
            xub_StrLen nToken = aString.GetTokenCount( ',' );
            if ( nToken > 1 )
            {
                aFieldName = aString.GetToken( 0, ',' );
                aString.EraseAllChars( ' ' );
                switch ( aString.GetToken( 1, ',' ).GetChar(0) )
                {
                    case 'L' :
                        nDbType = sdbc::DataType::BIT;
                        nFieldLen = 1;
                        bTypeDefined = TRUE;
                        bPrecDefined = TRUE;
                        break;
                    case 'D' :
                        nDbType = sdbc::DataType::DATE;
                        nFieldLen = 8;
                        bTypeDefined = TRUE;
                        bPrecDefined = TRUE;
                        break;
                    case 'M' :
                        nDbType = sdbc::DataType::LONGVARCHAR;
                        nFieldLen = 10;
                        bTypeDefined = TRUE;
                        bPrecDefined = TRUE;
                        bHasMemo = TRUE;
                        break;
                    case 'C' :
                        nDbType = sdbc::DataType::VARCHAR;
                        bTypeDefined = TRUE;
                        bPrecDefined = TRUE;
                        break;
                    case 'N' :
                        nDbType = sdbc::DataType::DECIMAL;
                        bTypeDefined = TRUE;
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
                            bPrecDefined = TRUE;
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
            for ( const sal_Unicode* p = aFieldName.GetBuffer(); c = *p; p++ )
            {
                if ( IsAsciiAlpha( c ) || IsAsciiDigit( c ) || c == '_' )
                    aTmpStr += c;
                else
                    aTmpStr += '_';
            }
            aFieldName = aTmpStr;
            if ( aFieldName.Len() > 10 )
                aFieldName.Erase( 10 );
            StrData* pStrData = new StrData( aFieldName );
            if ( !aFieldNamesCollection.Insert( pStrData ) )
            {   // doppelter Feldname, numerisch erweitern
                USHORT nSub = 1;
                String aFixPart( aFieldName );
                do
                {
                    ++nSub;
                    String aVarPart = String::CreateFromInt32( nSub );
                    if ( aFixPart.Len() + aVarPart.Len() > 10 )
                        aFixPart.Erase( 10 - aVarPart.Len() );
                    aFieldName = aFixPart;
                    aFieldName += aVarPart;
                    pStrData->SetString( aFieldName );
                } while ( !aFieldNamesCollection.Insert( pStrData ) );
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
                ULONG nFormat;
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
        BOOL bSdbLenAdjusted = FALSE;
        BOOL bSdbLenBad = FALSE;
        // Feldlaenge
        if ( nDbType == sdbc::DataType::VARCHAR && !nFieldLen )
        {   // maximale Feldbreite bestimmen
            nFieldLen = pDoc->GetMaxStringLen( nTab, nCol, nFirstDataRow,
                nLastRow );
            if ( nFieldLen == 0 )
                nFieldLen = 1;
        }
        else if ( nDbType == sdbc::DataType::DECIMAL )
        {   // maximale Feldbreite und Nachkommastellen bestimmen
            xub_StrLen nLen;
            USHORT nPrec;
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
                    nLen += nPrecision - nPrec;
                else
                    nLen -= nPrec+1;            // auch den . mit raus
            }
            if ( nLen > nFieldLen )
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
                bSdbLenBad = TRUE;
            nFieldLen = SvDbaseConverter::ConvertPrecisionToOdbc( nFieldLen, nPrecision );
            bSdbLenAdjusted = TRUE;
        }
        if ( nFieldLen > 254 )
        {
            if ( nDbType == sdbc::DataType::VARCHAR )
            {   // zu lang fuer normales Textfeld => Memofeld
                nDbType = sdbc::DataType::LONGVARCHAR;
                nFieldLen = 10;
                bHasMemo = TRUE;
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

ULONG ScDocShell::DBaseExport( const String& rFullFileName, CharSet eCharSet, BOOL& bHasMemo )
{
    //! Test !!!
    DirEntry aDirEntry( rFullFileName );
    if ( aDirEntry.Exists() )
        aDirEntry.Kill();
    //! Test !!!


    ULONG nErr = eERR_OK;
    uno::Any aAny;

    USHORT nFirstCol, nFirstRow, nLastCol, nLastRow;
    USHORT nTab = GetSaveTab();
    aDocument.GetDataStart( nTab, nFirstCol, nFirstRow );
    aDocument.GetCellArea( nTab, nLastCol, nLastRow );
    ScProgress aProgress( this, ScGlobal::GetRscString( STR_SAVE_DOC ),
                                                    nLastRow - nFirstRow );
    SvNumberFormatter* pNumFmt = aDocument.GetFormatTable();

    BOOL bHasFieldNames = TRUE;
    for ( USHORT nDocCol = nFirstCol; nDocCol <= nLastCol && bHasFieldNames; nDocCol++ )
    {   // nur Strings in erster Zeile => sind Feldnamen
        if ( !aDocument.HasStringData( nDocCol, nFirstRow, nTab ) )
            bHasFieldNames = FALSE;
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
                        bHasMemo );

    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetSmartURL( rFullFileName );
    String aTabName = aURL.getBase();
    String aExtension = aURL.getExtension();
    aURL.removeSegment();
    String aPath = aURL.GetMainURL();

    try
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
        if (!xFactory.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbc::XDriverManager> xDrvMan( xFactory->createInstance(
                            rtl::OUString::createFromAscii( SC_SERVICE_DRVMAN ) ),
                            uno::UNO_QUERY);
        DBG_ASSERT( xDrvMan.is(), "can't get DriverManager" );
        if (!xDrvMan.is()) return SCERR_EXPORT_CONNECT;

        // get connection

        String aConnUrl = String::CreateFromAscii("sdbc:dbase:");
        aConnUrl += aPath;

        uno::Sequence<beans::PropertyValue> aProps(2);
        aProps[0].Name = rtl::OUString::createFromAscii(SC_DBPROP_EXTENSION);
        aProps[0].Value <<= rtl::OUString( aExtension );
        aProps[1].Name = rtl::OUString::createFromAscii(SC_DBPROP_CHARSET);
        aProps[1].Value <<= (rtl_TextEncoding) eCharSet;

        uno::Reference<sdbc::XConnection> xConnection =
                                xDrvMan->getConnectionWithInfo( aConnUrl, aProps );
        DBG_ASSERT( xConnection.is(), "can't get Connection" );
        if (!xConnection.is()) return SCERR_EXPORT_CONNECT;

        // get dBase driver

        uno::Reference<sdbc::XDriver> xDriver;
        BOOL bDriverFound = FALSE;

        uno::Reference<container::XEnumerationAccess> xEnAcc( xDrvMan, uno::UNO_QUERY );
        DBG_ASSERT( xEnAcc.is(), "can't get DriverManager EnumerationAccess" );
        if (!xEnAcc.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<container::XEnumeration> xEnum = xEnAcc->createEnumeration();
        DBG_ASSERT( xEnum.is(), "can't get DriverManager Enumeration" );
        if (!xEnum.is()) return SCERR_EXPORT_CONNECT;

        while ( xEnum->hasMoreElements() && !bDriverFound )
        {
            uno::Any aElement = xEnum->nextElement();
            if ( aElement >>= xDriver )
                if ( xDriver.is() && xDriver->acceptsURL( aConnUrl ) )
                    bDriverFound = TRUE;
        }

        DBG_ASSERT( bDriverFound, "can't get dBase driver" );
        if (!bDriverFound) return SCERR_EXPORT_CONNECT;

        // create table

        uno::Reference<sdbcx::XDataDefinitionSupplier> xDDSup( xDriver, uno::UNO_QUERY );
        DBG_ASSERT( xDDSup.is(), "can't get XDataDefinitionSupplier" );
        if (!xDDSup.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XTablesSupplier> xTablesSupp =
                            xDDSup->getDataDefinitionByConnection( xConnection );
        DBG_ASSERT( xTablesSupp.is(), "can't get Data Definition" );
        if (!xTablesSupp.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<container::XNameAccess> xTables = xTablesSupp->getTables();
        DBG_ASSERT( xTables.is(), "can't get Tables" );
        if (!xTables.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XDataDescriptorFactory> xTablesFact( xTables, uno::UNO_QUERY );
        DBG_ASSERT( xTablesFact.is(), "can't get tables factory" );
        if (!xTablesFact.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XAppend> xTablesAppend( xTables, uno::UNO_QUERY );
        DBG_ASSERT( xTablesAppend.is(), "can't get tables XAppend" );
        if (!xTablesAppend.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<beans::XPropertySet> xTableDesc = xTablesFact->createDataDescriptor();
        DBG_ASSERT( xTableDesc.is(), "can't get table descriptor" );
        if (!xTableDesc.is()) return SCERR_EXPORT_CONNECT;

        aAny <<= rtl::OUString( aTabName );
        xTableDesc->setPropertyValue( rtl::OUString::createFromAscii(SC_DBPROP_NAME), aAny );

        // create columns

        uno::Reference<sdbcx::XColumnsSupplier> xColumnsSupp( xTableDesc, uno::UNO_QUERY );
        DBG_ASSERT( xColumnsSupp.is(), "can't get columns supplier" );
        if (!xColumnsSupp.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<container::XNameAccess> xColumns = xColumnsSupp->getColumns();
        DBG_ASSERT( xColumns.is(), "can't get columns" );
        if (!xColumns.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XDataDescriptorFactory> xColumnsFact( xColumns, uno::UNO_QUERY );
        DBG_ASSERT( xColumnsFact.is(), "can't get columns factory" );
        if (!xColumnsFact.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbcx::XAppend> xColumnsAppend( xColumns, uno::UNO_QUERY );
        DBG_ASSERT( xColumnsAppend.is(), "can't get columns XAppend" );
        if (!xColumnsAppend.is()) return SCERR_EXPORT_CONNECT;

        const rtl::OUString* pColNames = aColNames.getConstArray();
        const sal_Int32* pColTypes     = aColTypes.getConstArray();
        const sal_Int32* pColLengths   = aColLengths.getConstArray();
        const sal_Int32* pColScales    = aColScales.getConstArray();
        long nCol;

        for (nCol=0; nCol<nColCount; nCol++)
        {
            uno::Reference<beans::XPropertySet> xColumnDesc = xColumnsFact->createDataDescriptor();
            DBG_ASSERT( xColumnDesc.is(), "can't get column descriptor" );
            if (!xColumnDesc.is()) return SCERR_EXPORT_CONNECT;

            aAny <<= pColNames[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString::createFromAscii(SC_DBPROP_NAME), aAny );

            aAny <<= pColTypes[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString::createFromAscii(SC_DBPROP_TYPE), aAny );

            aAny <<= pColLengths[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString::createFromAscii(SC_DBPROP_PRECISION), aAny );

            aAny <<= pColScales[nCol];
            xColumnDesc->setPropertyValue( rtl::OUString::createFromAscii(SC_DBPROP_SCALE), aAny );

            xColumnsAppend->appendByDescriptor( xColumnDesc );
        }

        xTablesAppend->appendByDescriptor( xTableDesc );

        // re-open connection
//      xConnection = xDrvMan->getConnectionWithInfo( aConnUrl, aProps );
//      DBG_ASSERT( xConnection.is(), "can't get Connection" );
//      if (!xConnection.is()) return SCERR_EXPORT_CONNECT;

        // get row set for writing

        uno::Reference<sdbc::XRowSet> xRowSet( xFactory->createInstance(
                            rtl::OUString::createFromAscii( SC_SERVICE_ROWSET ) ),
                            uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        DBG_ASSERT( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is()) return SCERR_EXPORT_CONNECT;

        aAny <<= xConnection;
        xRowProp->setPropertyValue(
                    rtl::OUString::createFromAscii(SC_DBPROP_ACTIVECONNECTION), aAny );

        aAny <<= (sal_Int32) sdb::CommandType::TABLE;
        xRowProp->setPropertyValue(
                    rtl::OUString::createFromAscii(SC_DBPROP_COMMANDTYPE), aAny );

        aAny <<= rtl::OUString( aTabName );
        xRowProp->setPropertyValue(
                    rtl::OUString::createFromAscii(SC_DBPROP_COMMAND), aAny );

        xRowSet->execute();

        // write data rows

        uno::Reference<sdbc::XResultSetUpdate> xResultUpdate( xRowSet, uno::UNO_QUERY );
        DBG_ASSERT( xResultUpdate.is(), "can't get XResultSetUpdate" );
        if (!xResultUpdate.is()) return SCERR_EXPORT_CONNECT;

        uno::Reference<sdbc::XRowUpdate> xRowUpdate( xRowSet, uno::UNO_QUERY );
        DBG_ASSERT( xRowUpdate.is(), "can't get XRowUpdate" );
        if (!xRowUpdate.is()) return SCERR_EXPORT_CONNECT;

        USHORT nFirstDataRow = ( bHasFieldNames ? nFirstRow + 1 : nFirstRow );
        ScFieldEditEngine aEditEngine( aDocument.GetEditPool() );
        String aString;
        double fVal;

        nLastRow = nFirstDataRow - 1;       //! Test !!!!!!!

        for ( USHORT nDocRow = nFirstDataRow; nDocRow <= nLastRow; nDocRow++ )
        {
            xResultUpdate->moveToInsertRow();

            for (nCol=0; nCol<nColCount; nCol++)
            {
                USHORT nDocCol = nFirstCol + nCol;

                switch (pColTypes[nCol])
                {
                    case sdbc::DataType::LONGVARCHAR:
                        {
                            ScBaseCell* pCell;
                            aDocument.GetCell( nDocCol, nDocRow, nTab, pCell );
                            if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                            {
                                if ( pCell->GetCellType() == CELLTYPE_EDIT )
                                {   // #60761# Paragraphs erhalten
                                    aEditEngine.SetText( *((ScEditCell*)pCell)->GetData() );
                                    aString = aEditEngine.GetText( LINEEND_CRLF );
                                }
                                else
                                {
                                    ULONG nFormat;
                                    Color* pColor;
                                    aDocument.GetNumberFormat( nDocCol, nDocRow, nTab, nFormat );
                                    ScCellFormat::GetString( pCell, nFormat, aString, &pColor, *pNumFmt );
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
                        if ( nErr == eERR_OK && pColLengths[nCol] < aString.Len() )
                            nErr = SCWARN_EXPORT_DATALOST;
                        break;

                        break;

                    case sdbc::DataType::DATE:
                        {
                            aDocument.GetValue( nDocCol, nDocRow, nTab, fVal );
                            // #39274# zwischen 0 Wert und 0 kein Wert unterscheiden
                            BOOL bIsNull = (fVal == 0.0);
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
                        DBG_ERROR( "ScDocShell::DBaseExport: unknown FieldType" );
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
    }
    catch ( sdbc::SQLException& )
    {
        nErr = SCERR_EXPORT_CONNECT;
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
        nErr = ERRCODE_IO_GENERAL;
    }

    return nErr;
}


