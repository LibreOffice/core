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

#include "dbase/DTable.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <svl/converter.hxx>
#include "dbase/DConnection.hxx"
#include "dbase/DColumns.hxx"
#include <osl/thread.h>
#include <tools/config.hxx>
#include "dbase/DIndex.hxx"
#include "dbase/DIndexes.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <svl/zforlist.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <ucbhelper/content.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/property.hxx>
#include <comphelper/string.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <connectivity/PColumn.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/dbconversion.hxx>
#include "resource/dbase_res.hrc"
#include <rtl/strbuf.hxx>

#include <algorithm>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::ucbhelper;
using namespace ::utl;
using namespace ::cppu;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

// stored as the Field Descriptor terminator
#define FIELD_DESCRIPTOR_TERMINATOR 0x0D
#define DBF_EOL                     0x1A

namespace
{
sal_Size lcl_getFileSize(SvStream& _rStream)
{
    sal_Size nFileSize = 0;
    _rStream.Seek(STREAM_SEEK_TO_END);
    _rStream.SeekRel(-1);
    char cEOL;
    _rStream.ReadChar( cEOL );
    nFileSize = _rStream.Tell();
    if ( cEOL == DBF_EOL )
        nFileSize -= 1;
    return nFileSize;
}
/**
    calculates the Julian date
*/
void lcl_CalcJulDate(sal_Int32& _nJulianDate,sal_Int32& _nJulianTime, const com::sun::star::util::DateTime& rDateTime)
{
    com::sun::star::util::DateTime aDateTime = rDateTime;
    // weird: months fix
    if (aDateTime.Month > 12)
    {
        aDateTime.Month--;
        sal_uInt16 delta = rDateTime.Month / 12;
        aDateTime.Year += delta;
        aDateTime.Month -= delta * 12;
        aDateTime.Month++;
    }

    _nJulianTime = ((aDateTime.Hours*3600000)+(aDateTime.Minutes*60000)+(aDateTime.Seconds*1000)+(aDateTime.NanoSeconds/1000000));
    /* conversion factors */
    sal_uInt16 iy0;
    sal_uInt16 im0;
    if ( aDateTime.Month <= 2 )
    {
        iy0 = aDateTime.Year - 1;
        im0 = aDateTime.Month + 12;
    }
    else
    {
        iy0 = aDateTime.Year;
        im0 = aDateTime.Month;
    }
    sal_Int32 ia = iy0 / 100;
    sal_Int32 ib = 2 - ia + (ia >> 2);
    /* calculate julian date    */
    if ( aDateTime.Year <= 0 )
    {
        _nJulianDate = (sal_Int32) ((365.25 * iy0) - 0.75)
            + (sal_Int32) (30.6001 * (im0 + 1) )
            + aDateTime.Day + 1720994;
    } // if ( rDateTime.Year <= 0 )
    else
    {
        _nJulianDate = static_cast<sal_Int32>( ((365.25 * iy0)
            + (sal_Int32) (30.6001 * (im0 + 1))
            + aDateTime.Day + 1720994));
    }
    double JD = _nJulianDate + 0.5;
    _nJulianDate = (sal_Int32)( JD + 0.5);
    const double gyr = aDateTime.Year + (0.01 * aDateTime.Month) + (0.0001 * aDateTime.Day);
    if ( gyr >= 1582.1015 ) /* on or after 15 October 1582  */
        _nJulianDate += ib;
}

/**
    calculates date time from the Julian Date
*/
void lcl_CalDate(sal_Int32 _nJulianDate,sal_Int32 _nJulianTime,com::sun::star::util::DateTime& _rDateTime)
{
    if ( _nJulianDate )
    {
        sal_Int32 ialp;
        sal_Int32 ka = _nJulianDate;
        if ( _nJulianDate >= 2299161 )
        {
            ialp = (sal_Int32)( ((double) _nJulianDate - 1867216.25 ) / ( 36524.25 ));
            ka = _nJulianDate + 1 + ialp - ( ialp >> 2 );
        }
        sal_Int32 kb = ka + 1524;
        sal_Int32 kc =  (sal_Int32) ( ((double) kb - 122.1 ) / 365.25 );
        sal_Int32 kd = (sal_Int32) ((double) kc * 365.25);
        sal_Int32 ke = (sal_Int32) ((double) ( kb - kd ) / 30.6001 );
        _rDateTime.Day = static_cast<sal_uInt16>(kb - kd - ((sal_Int32) ( (double) ke * 30.6001 )));
        if ( ke > 13 )
            _rDateTime.Month = static_cast<sal_uInt16>(ke - 13);
        else
            _rDateTime.Month = static_cast<sal_uInt16>(ke - 1);
        if ( (_rDateTime.Month == 2) && (_rDateTime.Day > 28) )
            _rDateTime.Day = 29;
        if ( (_rDateTime.Month == 2) && (_rDateTime.Day == 29) && (ke == 3) )
            _rDateTime.Year = static_cast<sal_uInt16>(kc - 4716);
        else if ( _rDateTime.Month > 2 )
            _rDateTime.Year = static_cast<sal_uInt16>(kc - 4716);
        else
            _rDateTime.Year = static_cast<sal_uInt16>(kc - 4715);
    }

    if ( _nJulianTime )
    {
        double d_s = _nJulianTime / 1000.0;
        double d_m = d_s / 60.0;
        double d_h  = d_m / 60.0;
        _rDateTime.Hours = (sal_uInt16) (d_h);
        _rDateTime.Minutes = (sal_uInt16) d_m;
        _rDateTime.Seconds = static_cast<sal_uInt16>(( d_m - (double) _rDateTime.Minutes ) * 60.0);
    }
}

}


void ODbaseTable::readHeader()
{
    OSL_ENSURE(m_pFileStream,"No Stream available!");
    if(!m_pFileStream)
        return;
    m_pFileStream->RefreshBuffer(); // Make sure, that the header information actually is read again
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

    sal_uInt8 nType=0;
    (*m_pFileStream).ReadUChar( nType );
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();

    m_pFileStream->Read(m_aHeader.db_aedat, 3*sizeof(sal_uInt8));
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();
    (*m_pFileStream).ReadUInt32( m_aHeader.db_anz );
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();
    (*m_pFileStream).ReadUInt16( m_aHeader.db_kopf );
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();
    (*m_pFileStream).ReadUInt16( m_aHeader.db_slng );
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();
    if (m_aHeader.db_slng == 0)
        throwInvalidDbaseFormat();
    m_pFileStream->Read(m_aHeader.db_frei, 20*sizeof(sal_uInt8));
    if(ERRCODE_NONE != m_pFileStream->GetErrorCode())
        throwInvalidDbaseFormat();

    if ( ( ( m_aHeader.db_kopf - 1 ) / 32 - 1 ) <= 0 ) // number of fields
    {
        // no dbase file
        throwInvalidDbaseFormat();
    }
    else
    {
        // Consistency check of the header:
        m_aHeader.db_typ = (DBFType)nType;
        switch (m_aHeader.db_typ)
        {
            case dBaseIII:
            case dBaseIV:
            case dBaseV:
            case VisualFoxPro:
            case VisualFoxProAuto:
            case dBaseFS:
            case dBaseFSMemo:
            case dBaseIVMemoSQL:
            case dBaseIIIMemo:
            case FoxProMemo:
                m_pFileStream->SetEndian(SvStreamEndian::LITTLE);
                if ( m_aHeader.db_frei[17] != 0x00
                    && !m_aHeader.db_frei[18] && !m_aHeader.db_frei[19] && getConnection()->isTextEncodingDefaulted() )
                {
                    switch(m_aHeader.db_frei[17])
                    {
                        case 0x01: m_eEncoding = RTL_TEXTENCODING_IBM_437; break;       // DOS USA  code page 437
                        case 0x02: m_eEncoding = RTL_TEXTENCODING_IBM_850; break;       // DOS Multilingual code page 850
                        case 0x03: m_eEncoding = RTL_TEXTENCODING_MS_1252; break;       // Windows ANSI code page 1252
                        case 0x04: m_eEncoding = RTL_TEXTENCODING_APPLE_ROMAN; break;   // Standard Macintosh
                        case 0x64: m_eEncoding = RTL_TEXTENCODING_IBM_852; break;       // EE MS-DOS    code page 852
                        case 0x65: m_eEncoding = RTL_TEXTENCODING_IBM_865; break;       // Nordic MS-DOS    code page 865
                        case 0x66: m_eEncoding = RTL_TEXTENCODING_IBM_866; break;       // Russian MS-DOS   code page 866
                        case 0x67: m_eEncoding = RTL_TEXTENCODING_IBM_861; break;       // Icelandic MS-DOS
                        //case 0x68: m_eEncoding = ; break;     // Kamenicky (Czech) MS-DOS
                        //case 0x69: m_eEncoding = ; break;     // Mazovia (Polish) MS-DOS
                        case 0x6A: m_eEncoding = RTL_TEXTENCODING_IBM_737; break;       // Greek MS-DOS (437G)
                        case 0x6B: m_eEncoding = RTL_TEXTENCODING_IBM_857; break;       // Turkish MS-DOS
                        case 0x6C: m_eEncoding = RTL_TEXTENCODING_IBM_863; break;       // MS-DOS, Canada
                        case 0x78: m_eEncoding = RTL_TEXTENCODING_MS_950; break;        // Windows, Traditional Chinese
                        case 0x79: m_eEncoding = RTL_TEXTENCODING_MS_949; break;        // Windows, Korean (Hangul)
                        case 0x7A: m_eEncoding = RTL_TEXTENCODING_MS_936; break;        // Windows, Simplified Chinese
                        case 0x7B: m_eEncoding = RTL_TEXTENCODING_MS_932; break;        // Windows, Japanese (Shift-jis)
                        case 0x7C: m_eEncoding = RTL_TEXTENCODING_MS_874; break;        // Windows, Thai
                        case 0x7D: m_eEncoding = RTL_TEXTENCODING_MS_1255; break;       // Windows, Hebrew
                        case 0x7E: m_eEncoding = RTL_TEXTENCODING_MS_1256; break;       // Windows, Arabic
                        case 0x96: m_eEncoding = RTL_TEXTENCODING_APPLE_CYRILLIC; break;    // Russian Macintosh
                        case 0x97: m_eEncoding = RTL_TEXTENCODING_APPLE_CENTEURO; break;    // Eastern European Macintosh
                        case 0x98: m_eEncoding = RTL_TEXTENCODING_APPLE_GREEK; break;   // Greek Macintosh
                        case 0xC8: m_eEncoding = RTL_TEXTENCODING_MS_1250; break;       // Windows EE   code page 1250
                        case 0xC9: m_eEncoding = RTL_TEXTENCODING_MS_1251; break;       // Russian Windows
                        case 0xCA: m_eEncoding = RTL_TEXTENCODING_MS_1254; break;       // Turkish Windows
                        case 0xCB: m_eEncoding = RTL_TEXTENCODING_MS_1253; break;       // Greek Windows
                        case 0xCC: m_eEncoding = RTL_TEXTENCODING_MS_1257; break;       // Windows, Baltic
                        default:
                            // Default Encoding
                            m_eEncoding = RTL_TEXTENCODING_IBM_850;
                            break;
                    }
                }
                break;
            case dBaseIVMemo:
                m_pFileStream->SetEndian(SvStreamEndian::LITTLE);
                break;
            default:
            {
                throwInvalidDbaseFormat();
            }
        }
    }
}

void ODbaseTable::fillColumns()
{
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
    m_pFileStream->Seek(32L);

    if(!m_aColumns.is())
        m_aColumns = new OSQLColumns();
    else
        m_aColumns->get().clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();

    // Number of fields:
    const sal_Int32 nFieldCount = (m_aHeader.db_kopf - 1) / 32 - 1;
    OSL_ENSURE(nFieldCount,"No columns in table!");

    m_aColumns->get().reserve(nFieldCount);
    m_aTypes.reserve(nFieldCount);
    m_aPrecisions.reserve(nFieldCount);
    m_aScales.reserve(nFieldCount);

    OUString aTypeName;
    const bool bCase = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();
    const bool bFoxPro = m_aHeader.db_typ == VisualFoxPro || m_aHeader.db_typ == VisualFoxProAuto || m_aHeader.db_typ == FoxProMemo;

    sal_Int32 i = 0;
    for (; i < nFieldCount; i++)
    {
        DBFColumn aDBFColumn;
        sal_Size nRead = m_pFileStream->Read(&aDBFColumn, sizeof(aDBFColumn));
        if (nRead != sizeof(aDBFColumn))
        {
            SAL_WARN("connectivity.drivers", "ODbaseTable::fillColumns: short read!");
            break;
        }
        if ( FIELD_DESCRIPTOR_TERMINATOR == aDBFColumn.db_fnm[0] ) // 0x0D stored as the Field Descriptor terminator.
            break;

        aDBFColumn.db_fnm[sizeof(aDBFColumn.db_fnm)-1] = 0; //ensure null termination for broken input
        const OUString aColumnName(reinterpret_cast<char *>(aDBFColumn.db_fnm), strlen(reinterpret_cast<char *>(aDBFColumn.db_fnm)), m_eEncoding);

        bool bIsRowVersion = bFoxPro && ( aDBFColumn.db_frei2[0] & 0x01 ) == 0x01;

        m_aRealFieldLengths.push_back(aDBFColumn.db_flng);
        sal_Int32 nPrecision = aDBFColumn.db_flng;
        sal_Int32 eType;
        bool bIsCurrency = false;

        char cType[2];
        cType[0] = aDBFColumn.db_typ;
        cType[1] = 0;
        aTypeName = OUString(cType, 1, RTL_TEXTENCODING_ASCII_US);
        SAL_INFO( "connectivity.drivers","column type: " << aDBFColumn.db_typ);

        switch (aDBFColumn.db_typ)
        {
        case 'C':
            eType = DataType::VARCHAR;
            aTypeName = "VARCHAR";
            break;
        case 'F':
        case 'N':
            aTypeName = "DECIMAL";
            if ( aDBFColumn.db_typ == 'N' )
                aTypeName = "NUMERIC";
            eType = DataType::DECIMAL;

            // for numeric fields two characters more are written, than the precision of the column description predescribes,
            // to keep room for the possible sign and the comma. This has to be considered...
            nPrecision = SvDbaseConverter::ConvertPrecisionToOdbc(nPrecision,aDBFColumn.db_dez);
            // This is not true for older versions ....
            break;
        case 'L':
            eType = DataType::BIT;
            aTypeName = "BOOLEAN";
            break;
        case 'Y':
            bIsCurrency = true;
            eType = DataType::DOUBLE;
            aTypeName = "DOUBLE";
            break;
        case 'D':
            eType = DataType::DATE;
            aTypeName = "DATE";
            break;
        case 'T':
            eType = DataType::TIMESTAMP;
            aTypeName = "TIMESTAMP";
            break;
        case 'I':
            eType = DataType::INTEGER;
            aTypeName = "INTEGER";
            break;
        case 'M':
            if ( bFoxPro && ( aDBFColumn.db_frei2[0] & 0x04 ) == 0x04 )
            {
                eType = DataType::LONGVARBINARY;
                aTypeName = "LONGVARBINARY";
            }
            else
            {
                aTypeName = "LONGVARCHAR";
                eType = DataType::LONGVARCHAR;
            }
            nPrecision = 2147483647;
            break;
        case 'P':
            aTypeName = "LONGVARBINARY";
            eType = DataType::LONGVARBINARY;
            nPrecision = 2147483647;
            break;
        case '0':
        case 'B':
            if ( m_aHeader.db_typ == VisualFoxPro || m_aHeader.db_typ == VisualFoxProAuto )
            {
                aTypeName = "DOUBLE";
                eType = DataType::DOUBLE;
            }
            else
            {
                aTypeName = "LONGVARBINARY";
                eType = DataType::LONGVARBINARY;
                nPrecision = 2147483647;
            }
            break;
        default:
            eType = DataType::OTHER;
        }

        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(aDBFColumn.db_dez);

        Reference< XPropertySet> xCol = new sdbcx::OColumn(aColumnName,
                                                    aTypeName,
                                                    OUString(),
                                                    OUString(),
                                                    ColumnValue::NULLABLE,
                                                    nPrecision,
                                                    aDBFColumn.db_dez,
                                                    eType,
                                                    false,
                                                    bIsRowVersion,
                                                    bIsCurrency,
                                                    bCase,
                                                    m_CatalogName, getSchema(), getName());
        m_aColumns->get().push_back(xCol);
    } // for (; i < nFieldCount; i++)
    OSL_ENSURE(i,"No columns in table!");
}

ODbaseTable::ODbaseTable(sdbcx::OCollection* _pTables, ODbaseConnection* _pConnection)
    : ODbaseTable_BASE(_pTables,_pConnection)
    , m_pMemoStream(nullptr)
    , m_bWriteableMemo(false)
{
    // initialize the header
    memset(&m_aHeader, 0, sizeof(m_aHeader));
    m_aHeader.db_typ = dBaseIII;
    m_eEncoding = getConnection()->getTextEncoding();
}

ODbaseTable::ODbaseTable(sdbcx::OCollection* _pTables, ODbaseConnection* _pConnection,
                         const OUString& _Name,
                         const OUString& _Type,
                         const OUString& _Description ,
                         const OUString& _SchemaName,
                         const OUString& _CatalogName )
    : ODbaseTable_BASE(_pTables,_pConnection,_Name,
                       _Type,
                       _Description,
                       _SchemaName,
                       _CatalogName)
    , m_pMemoStream(nullptr)
    , m_bWriteableMemo(false)
{
    memset(&m_aHeader, 0, sizeof(m_aHeader));
    m_eEncoding = getConnection()->getTextEncoding();
}


void ODbaseTable::construct()
{
    // initialize the header
    m_aHeader.db_typ    = dBaseIII;
    m_aHeader.db_anz    = 0;
    m_aHeader.db_kopf   = 0;
    m_aHeader.db_slng   = 0;
    m_aMemoHeader.db_size = 0;

    OUString sFileName(getEntry(m_pConnection, m_Name));

    INetURLObject aURL;
    aURL.SetURL(sFileName);

    OSL_ENSURE( m_pConnection->matchesExtension( aURL.getExtension() ),
        "ODbaseTable::ODbaseTable: invalid extension!");
        // getEntry is expected to ensure the correct file name

    m_pFileStream = createStream_simpleError( sFileName, STREAM_READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYWRITE);
    m_bWriteable = ( m_pFileStream != nullptr );

    if ( !m_pFileStream )
    {
        m_bWriteable = false;
        m_pFileStream = createStream_simpleError( sFileName, StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYNONE);
    }

    if(m_pFileStream)
    {
        readHeader();
        if (HasMemoFields())
        {
        // Create Memo-Filename (.DBT):
        // nyi: Ugly for Unix and Mac!

            if ( m_aHeader.db_typ == FoxProMemo || VisualFoxPro == m_aHeader.db_typ || VisualFoxProAuto == m_aHeader.db_typ ) // foxpro uses another extension
                aURL.SetExtension("fpt");
            else
                aURL.SetExtension("dbt");

            // If the memo file isn't found, the data will be displayed anyhow.
            // However, updates can't be done
            // but the operation is executed
            m_pMemoStream = createStream_simpleError( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYWRITE);
            if ( !m_pMemoStream )
            {
                m_bWriteableMemo = false;
                m_pMemoStream = createStream_simpleError( aURL.GetMainURL(INetURLObject::NO_DECODE), StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYNONE);
            }
            if (m_pMemoStream)
                ReadMemoHeader();
        }
        fillColumns();

        sal_Size nFileSize = lcl_getFileSize(*m_pFileStream);
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
        // seems to be empty or someone wrote bullshit into the dbase file
        // try and recover if m_aHeader.db_slng is sane
        if (m_aHeader.db_anz == 0 && m_aHeader.db_slng)
        {
            sal_Size nRecords = (nFileSize-m_aHeader.db_kopf)/m_aHeader.db_slng;
            if (nRecords > 0)
                m_aHeader.db_anz = nRecords;
        }

        // Buffersize dependent on the file size
        m_pFileStream->SetBufferSize(nFileSize > 1000000 ? 32768 :
                                  nFileSize > 100000 ? 16384 :
                                  nFileSize > 10000 ? 4096 : 1024);

        if (m_pMemoStream)
        {
            // set the buffer exactly to the length of a record
            m_pMemoStream->Seek(STREAM_SEEK_TO_END);
            nFileSize = m_pMemoStream->Tell();
            m_pMemoStream->Seek(STREAM_SEEK_TO_BEGIN);

            // Buffersize dependent on the file size
            m_pMemoStream->SetBufferSize(nFileSize > 1000000 ? 32768 :
                                          nFileSize > 100000 ? 16384 :
                                          nFileSize > 10000 ? 4096 :
                                          m_aMemoHeader.db_size);
        }

        AllocBuffer();
    }
}

bool ODbaseTable::ReadMemoHeader()
{
    m_pMemoStream->SetEndian(SvStreamEndian::LITTLE);
    m_pMemoStream->RefreshBuffer();         // make sure that the header information is actually read again
    m_pMemoStream->Seek(0L);

    (*m_pMemoStream).ReadUInt32( m_aMemoHeader.db_next );
    switch (m_aHeader.db_typ)
    {
        case dBaseIIIMemo:  // dBase III: fixed block size
        case dBaseIVMemo:
            // sometimes dBase3 is attached to dBase4 memo
            m_pMemoStream->Seek(20L);
            (*m_pMemoStream).ReadUInt16( m_aMemoHeader.db_size );
            if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size != 512)  // 1 is also for dBase 3
                m_aMemoHeader.db_typ  = MemodBaseIV;
            else if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size == 512)
            {
                // There are files using size specification, though they are dBase-files
                char sHeader[4];
                m_pMemoStream->Seek(m_aMemoHeader.db_size);
                m_pMemoStream->Read(sHeader,4);

                if ((m_pMemoStream->GetErrorCode() != ERRCODE_NONE) || ((sal_uInt8)sHeader[0]) != 0xFF || ((sal_uInt8)sHeader[1]) != 0xFF || ((sal_uInt8)sHeader[2]) != 0x08)
                    m_aMemoHeader.db_typ  = MemodBaseIII;
                else
                    m_aMemoHeader.db_typ  = MemodBaseIV;
            }
            else
            {
                m_aMemoHeader.db_typ  = MemodBaseIII;
                m_aMemoHeader.db_size = 512;
            }
            break;
        case VisualFoxPro:
        case VisualFoxProAuto:
        case FoxProMemo:
            m_aMemoHeader.db_typ    = MemoFoxPro;
            m_pMemoStream->Seek(6L);
            m_pMemoStream->SetEndian(SvStreamEndian::BIG);
            (*m_pMemoStream).ReadUInt16( m_aMemoHeader.db_size );
            break;
        default:
            SAL_WARN( "connectivity.drivers", "ODbaseTable::ReadMemoHeader: unsupported memo type!" );
            break;
    }
    return true;
}

OUString ODbaseTable::getEntry(OConnection* _pConnection,const OUString& _sName )
{
    OUString sURL;
    try
    {
        Reference< XResultSet > xDir = _pConnection->getDir()->getStaticResultSet();
        Reference< XRow> xRow(xDir,UNO_QUERY);
        OUString sName;
        OUString sExt;
        INetURLObject aURL;
        static const char s_sSeparator[] = "/";
        xDir->beforeFirst();
        while(xDir->next())
        {
            sName = xRow->getString(1);
            aURL.SetSmartProtocol(INetProtocol::File);
            OUString sUrl = _pConnection->getURL() +  s_sSeparator + sName;
            aURL.SetSmartURL( sUrl );

            // cut the extension
            sExt = aURL.getExtension();

            // name and extension have to coincide
            if ( _pConnection->matchesExtension( sExt ) )
            {
                sName = sName.replaceAt(sName.getLength() - (sExt.getLength() + 1), sExt.getLength() + 1, OUString());
                if ( sName == _sName )
                {
                    Reference< XContentAccess > xContentAccess( xDir, UNO_QUERY );
                    sURL = xContentAccess->queryContentIdentifierString();
                    break;
                }
            }
        }
        xDir->beforeFirst(); // move back to before first record
    }
    catch(const Exception&)
    {
        OSL_ASSERT(false);
    }
    return sURL;
}

void ODbaseTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    TStringVector aVector;
    aVector.reserve(m_aColumns->get().size());

    for(OSQLColumns::Vector::const_iterator aIter = m_aColumns->get().begin();aIter != m_aColumns->get().end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new ODbaseColumns(this,m_aMutex,aVector);
}

void ODbaseTable::refreshIndexes()
{
    TStringVector aVector;
    if(m_pFileStream && (!m_pIndexes || m_pIndexes->getCount() == 0))
    {
        INetURLObject aURL;
        aURL.SetURL(getEntry(m_pConnection,m_Name));

        aURL.setExtension("inf");
        Config aInfFile(aURL.getFSysPath(INetURLObject::FSYS_DETECT));
        aInfFile.SetGroup(dBASE_III_GROUP);
        sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
        OString aKeyName;

        for (sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++)
        {
            // Refences the key an index-file?
            aKeyName = aInfFile.GetKeyName( nKey );
            //...if yes, add the index list of the table
            if (aKeyName.copy(0,3).equals("NDX"))
            {
                OString aIndexName = aInfFile.ReadKey(aKeyName);
                aURL.setName(OStringToOUString(aIndexName, m_eEncoding));
                try
                {
                    Content aCnt(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
                    if (aCnt.isDocument())
                    {
                        aVector.push_back(aURL.getBase());
                    }
                }
                catch(const Exception&) // an exception is thrown when no file exists
                {
                }
            }
        }
    }
    if(m_pIndexes)
        m_pIndexes->reFill(aVector);
    else
        m_pIndexes  = new ODbaseIndexes(this,m_aMutex,aVector);
}


void SAL_CALL ODbaseTable::disposing()
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns = nullptr;
}

Sequence< Type > SAL_CALL ODbaseTable::getTypes(  ) throw(RuntimeException, std::exception)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == cppu::UnoType<XKeysSupplier>::get()||
            *pBegin == cppu::UnoType<XDataDescriptorFactory>::get()))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    aOwnTypes.push_back(cppu::UnoType<com::sun::star::lang::XUnoTunnel>::get());
    return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
}


Any SAL_CALL ODbaseTable::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XKeysSupplier>::get()||
        rType == cppu::UnoType<XDataDescriptorFactory>::get())
        return Any();

    Any aRet = OTable_TYPEDEF::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
}


Sequence< sal_Int8 > ODbaseTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel

sal_Int64 ODbaseTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : ODbaseTable_BASE::getSomething(rId);
}

bool ODbaseTable::fetchRow(OValueRefRow& _rRow, const OSQLColumns & _rCols, bool _bUseTableDefs, bool bRetrieveData)
{
    if (!m_pBuffer)
        return false;

    // Read the data
    bool bIsCurRecordDeleted = (char)m_pBuffer[0] == '*';

    // only read the bookmark

    // Mark record as deleted
    _rRow->setDeleted(bIsCurRecordDeleted);
    *(_rRow->get())[0] = m_nFilePos;

    if (!bRetrieveData)
        return true;

    sal_Size nByteOffset = 1;
    // Fields:
    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    OSQLColumns::Vector::const_iterator aEnd  = _rCols.get().end();
    const sal_Size nCount = _rRow->get().size();
    for (sal_Size i = 1; aIter != aEnd && nByteOffset <= m_nBufferSize && i < nCount;++aIter, i++)
    {
        // Lengths depending on data type:
        sal_Int32 nLen = 0;
        sal_Int32 nType = 0;
        if(_bUseTableDefs)
        {
            nLen    = m_aPrecisions[i-1];
            nType   = m_aTypes[i-1];
        }
        else
        {
            (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)) >>= nLen;
            (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))      >>= nType;
        }

        switch(nType)
        {
            case DataType::INTEGER:
            case DataType::DOUBLE:
            case DataType::TIMESTAMP:
            case DataType::DATE:
            case DataType::BIT:
            case DataType::LONGVARCHAR:
            case DataType::LONGVARBINARY:
                nLen = m_aRealFieldLengths[i-1];
                break;
            case DataType::DECIMAL:
                if(_bUseTableDefs)
                    nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,m_aScales[i-1]);
                else
                    nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,getINT32((*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                break;  // the sign and the comma

            case DataType::BINARY:
            case DataType::OTHER:
                nByteOffset += nLen;
                continue;
        }

        // Is the variable bound?
        if ( !(_rRow->get())[i]->isBound() )
        {
            // No - next field.
            nByteOffset += nLen;
            OSL_ENSURE( nByteOffset <= m_nBufferSize ,"ByteOffset > m_nBufferSize!");
            continue;
        } // if ( !(_rRow->get())[i]->isBound() )
        if ( ( nByteOffset + nLen) > m_nBufferSize )
            break; // length doesn't match buffer size.

        char *pData = reinterpret_cast<char *>(m_pBuffer + nByteOffset);

        if (nType == DataType::CHAR || nType == DataType::VARCHAR)
        {
            sal_Int32 nLastPos = -1;
            for (sal_Int32 k = 0; k < nLen; ++k)
            {
                if (pData[k] != ' ')
                    // Record last non-empty position.
                    nLastPos = k;
            }
            if (nLastPos < 0)
            {
                // Empty string.  Skip it.
                (_rRow->get())[i]->setNull();
            }
            else
            {
                // Commit the string.  Use intern() to ref-count it.
                *(_rRow->get())[i] = OUString::intern(pData, static_cast<sal_Int32>(nLastPos+1), m_eEncoding);
            }
        } // if (nType == DataType::CHAR || nType == DataType::VARCHAR)
        else if ( DataType::TIMESTAMP == nType )
        {
            sal_Int32 nDate = 0,nTime = 0;
            memcpy(&nDate, pData, 4);
            memcpy(&nTime, pData+ 4, 4);
            if ( !nDate && !nTime )
            {
                (_rRow->get())[i]->setNull();
            }
            else
            {
                ::com::sun::star::util::DateTime aDateTime;
                lcl_CalDate(nDate,nTime,aDateTime);
                *(_rRow->get())[i] = aDateTime;
            }
        }
        else if ( DataType::INTEGER == nType )
        {
            sal_Int32 nValue = 0;
            if (static_cast<size_t>(nLen) > sizeof(nValue))
                return false;
            memcpy(&nValue, pData, nLen);
            *(_rRow->get())[i] = nValue;
        }
        else if ( DataType::DOUBLE == nType )
        {
            double d = 0.0;
            if (getBOOL((*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))) // Currency is treated separately
            {
                sal_Int64 nValue = 0;
                if (static_cast<size_t>(nLen) > sizeof(nValue))
                    return false;
                memcpy(&nValue, pData, nLen);

                if ( m_aScales[i-1] )
                    d = (double)(nValue / pow(10.0,(int)m_aScales[i-1]));
                else
                    d = (double)(nValue);
            }
            else
            {
                if (static_cast<size_t>(nLen) > sizeof(d))
                    return false;
                memcpy(&d, pData, nLen);
            }

            *(_rRow->get())[i] = d;
        }
        else
        {
            sal_Int32 nPos1 = -1, nPos2 = -1;
            // If the string contains Nul-characters, then convert them to blanks!
            for (sal_Int32 k = 0; k < nLen; k++)
            {
                if (pData[k] == '\0')
                    pData[k] = ' ';

                if (pData[k] != ' ')
                {
                    if (nPos1 < 0)
                        // first non-empty char position.
                        nPos1 = k;

                    // last non-empty char position.
                    nPos2 = k;
                }
            }

            if (nPos1 < 0)
            {
                // Empty string.  Skip it.
                nByteOffset += nLen;
                (_rRow->get())[i]->setNull();   // no values -> done
                continue;
            }

            OUString aStr = OUString::intern(pData+nPos1, nPos2-nPos1+1, m_eEncoding);

            switch (nType)
            {
                case DataType::DATE:
                {
                    if (aStr.getLength() != nLen)
                    {
                        (_rRow->get())[i]->setNull();
                        break;
                    }
                    const sal_uInt16  nYear   = (sal_uInt16)aStr.copy( 0, 4 ).toInt32();
                    const sal_uInt16  nMonth  = (sal_uInt16)aStr.copy( 4, 2 ).toInt32();
                    const sal_uInt16  nDay    = (sal_uInt16)aStr.copy( 6, 2 ).toInt32();

                    const ::com::sun::star::util::Date aDate(nDay,nMonth,nYear);
                    *(_rRow->get())[i] = aDate;
                }
                break;
                case DataType::DECIMAL:
                    *(_rRow->get())[i] = ORowSetValue(aStr);
                break;
                case DataType::BIT:
                {
                    bool b;
                    switch (*pData)
                    {
                        case 'T':
                        case 'Y':
                        case 'J':   b = true; break;
                        default:    b = false; break;
                    }
                    *(_rRow->get())[i] = b;
                }
                break;
                case DataType::LONGVARBINARY:
                case DataType::BINARY:
                case DataType::LONGVARCHAR:
                {
                    const long nBlockNo = aStr.toInt32();   // read blocknumber
                    if (nBlockNo > 0 && m_pMemoStream) // Read data from memo-file, only if
                    {
                        if ( !ReadMemo(nBlockNo, (_rRow->get())[i]->get()) )
                            break;
                    }
                    else
                        (_rRow->get())[i]->setNull();
                }   break;
                default:
                    SAL_WARN( "connectivity.drivers","Falscher Type");
            }
            (_rRow->get())[i]->setTypeKind(nType);
        }

        nByteOffset += nLen;
        OSL_ENSURE( nByteOffset <= m_nBufferSize ,"ByteOffset > m_nBufferSize!");
    }
    return true;
}


void ODbaseTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // if not everything has been written yet
    if (m_pMemoStream && m_pMemoStream->IsWritable())
        m_pMemoStream->Flush();

    delete m_pMemoStream;
    m_pMemoStream = nullptr;

    ODbaseTable_BASE::FileClose();
}

bool ODbaseTable::CreateImpl()
{
    OSL_ENSURE(!m_pFileStream, "SequenceError");

    if ( m_pConnection->isCheckEnabled() && ::dbtools::convertName2SQLName(m_Name,OUString()) != m_Name )
    {
        const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                STR_SQL_NAME_ERROR,
                "$name$", m_Name
             ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }

    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    OUString aName = getEntry(m_pConnection, m_Name);
    if(aName.isEmpty())
    {
        OUString aIdent = m_pConnection->getContent()->getIdentifier()->getContentIdentifier();
        if ( aIdent.lastIndexOf('/') != (aIdent.getLength()-1) )
            aIdent += "/";
        aIdent += m_Name;
        aName = aIdent.getStr();
    }
    aURL.SetURL(aName);

    if ( !m_pConnection->matchesExtension( aURL.getExtension() ) )
        aURL.setExtension(m_pConnection->getExtension());

    try
    {
        Content aContent(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
        if (aContent.isDocument())
        {
            // Only if the file exists with length > 0 raise an error
            SvStream* pFileStream = createStream_simpleError( aURL.GetMainURL(INetURLObject::NO_DECODE), StreamMode::READ);

            if (pFileStream && pFileStream->Seek(STREAM_SEEK_TO_END))
            {
                return false;
            }
            delete pFileStream;
        }
    }
    catch(const Exception&) // an exception is thrown when no file exists
    {
    }

    bool bMemoFile = false;

    bool bOk = CreateFile(aURL, bMemoFile);

    FileClose();

    if (!bOk)
    {
        try
        {
            Content aContent(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
            aContent.executeCommand( "delete", css::uno::Any( true ) );
        }
        catch(const Exception&) // an exception is thrown when no file exists
        {
        }
        return false;
    }

    if (bMemoFile)
    {
        OUString aExt = aURL.getExtension();
        aURL.setExtension("dbt");                      // extension for memo file

        bool bMemoAlreadyExists = false;
        try
        {
            Content aMemo1Content(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
            bMemoAlreadyExists = aMemo1Content.isDocument();
        }
        catch(const Exception&) // an exception is thrown when no file exists
        {
        }
        if (bMemoAlreadyExists)
        {
            aURL.setExtension(aExt);      // kill dbf file
            try
            {
                Content aMemoContent(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
                aMemoContent.executeCommand( "delete", css::uno::Any( true ) );
            }
            catch(const Exception&)
            {

                const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                        STR_COULD_NOT_DELETE_FILE,
                        "$name$", aName
                     ) );
                ::dbtools::throwGenericSQLException( sError, *this );
            }
        }
        if (!CreateMemoFile(aURL))
        {
            aURL.setExtension(aExt);      // kill dbf file
            try
            {
                Content aMemoContent(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
                aMemoContent.executeCommand( "delete", css::uno::Any( true ) );
            }
            catch(const ContentCreationException&)
            {
                const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                        STR_COULD_NOT_DELETE_FILE,
                        "$name$", aName
                     ) );
                ::dbtools::throwGenericSQLException( sError, *this );
            }
            return false;
        }
        m_aHeader.db_typ = dBaseIIIMemo;
    }
    else
        m_aHeader.db_typ = dBaseIII;

    return true;
}

void ODbaseTable::throwInvalidColumnType(const sal_uInt16 _nErrorId, const OUString& _sColumnName)
{
    try
    {
        // we have to drop the file because it is corrupted now
        DropImpl();
    }
    catch(const Exception&)
    {
    }

    const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
            _nErrorId,
            "$columnname$", _sColumnName
         ) );
    ::dbtools::throwGenericSQLException( sError, *this );
}

// creates in principle dBase IV file format
bool ODbaseTable::CreateFile(const INetURLObject& aFile, bool& bCreateMemo)
{
    bCreateMemo = false;
    Date aDate( Date::SYSTEM );                     // current date

    m_pFileStream = createStream_simpleError( aFile.GetMainURL(INetURLObject::NO_DECODE),STREAM_READWRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC );

    if (!m_pFileStream)
        return false;

    sal_uInt8 nDbaseType = dBaseIII;
    Reference<XIndexAccess> xColumns(getColumns(),UNO_QUERY);
    Reference<XPropertySet> xCol;
    const OUString sPropType = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE);

    try
    {
        const sal_Int32 nCount = xColumns->getCount();
        for(sal_Int32 i=0;i<nCount;++i)
        {
            xColumns->getByIndex(i) >>= xCol;
            OSL_ENSURE(xCol.is(),"This should be a column!");

            switch (getINT32(xCol->getPropertyValue(sPropType)))
            {
                case DataType::DOUBLE:
                case DataType::INTEGER:
                case DataType::TIMESTAMP:
                case DataType::LONGVARBINARY:
                    nDbaseType = VisualFoxPro;
                    i = nCount; // no more columns need to be checked
                    break;
            } // switch (getINT32(xCol->getPropertyValue(sPropType)))
        }
    }
    catch ( const Exception& e )
    {
        (void)e;

        try
        {
            // we have to drop the file because it is corrupted now
            DropImpl();
        }
        catch(const Exception&) { }
        throw;
    }

    char aBuffer[21];               // write buffer
    memset(aBuffer,0,sizeof(aBuffer));

    m_pFileStream->Seek(0L);
    (*m_pFileStream).WriteUChar( nDbaseType );                            // dBase format
    (*m_pFileStream).WriteUChar( aDate.GetYear() % 100 );                 // current date


    (*m_pFileStream).WriteUChar( aDate.GetMonth() );
    (*m_pFileStream).WriteUChar( aDate.GetDay() );
    (*m_pFileStream).WriteUInt32( 0 );                                    // number of data records
    (*m_pFileStream).WriteUInt16( (m_pColumns->getCount()+1) * 32 + 1 );  // header information,
                                                                          // pColumns contains always an additional column
    (*m_pFileStream).WriteUInt16( 0 );                                     // record length will be determined later
    m_pFileStream->Write(aBuffer, 20);

    sal_uInt16 nRecLength = 1;                                              // Length 1 for deleted flag
    sal_Int32  nMaxFieldLength = m_pConnection->getMetaData()->getMaxColumnNameLength();
    OUString aName;
    const OUString sPropName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const OUString sPropPrec = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION);
    const OUString sPropScale = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE);

    try
    {
        const sal_Int32 nCount = xColumns->getCount();
        for(sal_Int32 i=0;i<nCount;++i)
        {
            xColumns->getByIndex(i) >>= xCol;
            OSL_ENSURE(xCol.is(),"This should be a column!");

            char cTyp( 'C' );

            xCol->getPropertyValue(sPropName) >>= aName;

            OString aCol;
            if ( DBTypeConversion::convertUnicodeString( aName, aCol, m_eEncoding ) > nMaxFieldLength)
            {
                throwInvalidColumnType( STR_INVALID_COLUMN_NAME_LENGTH, aName );
            }

            (*m_pFileStream).WriteCharPtr( aCol.getStr() );
            m_pFileStream->Write(aBuffer, 11 - aCol.getLength());

            sal_Int32 nPrecision = 0;
            xCol->getPropertyValue(sPropPrec) >>= nPrecision;
            sal_Int32 nScale = 0;
            xCol->getPropertyValue(sPropScale) >>= nScale;

            bool bBinary = false;

            switch (getINT32(xCol->getPropertyValue(sPropType)))
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                    cTyp = 'C';
                    break;
                case DataType::DOUBLE:
                    if (getBOOL(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))) // Currency will be treated separately
                        cTyp = 'Y';
                    else
                        cTyp = 'B';
                    break;
                case DataType::INTEGER:
                    cTyp = 'I';
                    break;
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::BIGINT:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::REAL:
                    cTyp = 'N';                             // only dBase 3 format
                    break;
                case DataType::TIMESTAMP:
                    cTyp = 'T';
                    break;
                case DataType::DATE:
                    cTyp = 'D';
                    break;
                case DataType::BIT:
                    cTyp = 'L';
                    break;
                case DataType::LONGVARBINARY:
                    bBinary = true;
                    SAL_FALLTHROUGH;
                case DataType::LONGVARCHAR:
                    cTyp = 'M';
                    break;
                default:
                    {
                        throwInvalidColumnType(STR_INVALID_COLUMN_TYPE, aName);
                    }
            }

            (*m_pFileStream).WriteChar( cTyp );
            if ( nDbaseType == VisualFoxPro )
                (*m_pFileStream).WriteUInt32( nRecLength-1 );
            else
                m_pFileStream->Write(aBuffer, 4);

            switch(cTyp)
            {
                case 'C':
                    OSL_ENSURE(nPrecision < 255, "ODbaseTable::Create: Column zu lang!");
                    if (nPrecision > 254)
                    {
                        throwInvalidColumnType(STR_INVALID_COLUMN_PRECISION, aName);
                    }
                    (*m_pFileStream).WriteUChar( std::min((unsigned)nPrecision, 255U) );      // field length
                    nRecLength = nRecLength + (sal_uInt16)::std::min((sal_uInt16)nPrecision, (sal_uInt16)255UL);
                    (*m_pFileStream).WriteUChar( 0 );                                                                // decimals
                    break;
                case 'F':
                case 'N':
                    OSL_ENSURE(nPrecision >=  nScale,
                            "ODbaseTable::Create: Feldlaenge muss groesser Nachkommastellen sein!");
                    if (nPrecision <  nScale)
                    {
                        throwInvalidColumnType(STR_INVALID_PRECISION_SCALE, aName);
                    }
                    if (getBOOL(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))) // Currency will be treated separately
                    {
                        (*m_pFileStream).WriteUChar( 10 );          // standard length
                        (*m_pFileStream).WriteUChar( 4 );
                        nRecLength += 10;
                    }
                    else
                    {
                        sal_Int32 nPrec = SvDbaseConverter::ConvertPrecisionToDbase(nPrecision,nScale);

                        (*m_pFileStream).WriteUChar( nPrec );
                        (*m_pFileStream).WriteUChar( nScale );
                        nRecLength += (sal_uInt16)nPrec;
                    }
                    break;
                case 'L':
                    (*m_pFileStream).WriteUChar( 1 );
                    (*m_pFileStream).WriteUChar( 0 );
                    ++nRecLength;
                    break;
                case 'I':
                    (*m_pFileStream).WriteUChar( 4 );
                    (*m_pFileStream).WriteUChar( 0 );
                    nRecLength += 4;
                    break;
                case 'Y':
                case 'B':
                case 'T':
                case 'D':
                    (*m_pFileStream).WriteUChar( 8 );
                    (*m_pFileStream).WriteUChar( 0 );
                    nRecLength += 8;
                    break;
                case 'M':
                    bCreateMemo = true;
                    (*m_pFileStream).WriteUChar( 10 );
                    (*m_pFileStream).WriteUChar( 0 );
                    nRecLength += 10;
                    if ( bBinary )
                        aBuffer[0] = 0x06;
                    break;
                default:
                    throwInvalidColumnType(STR_INVALID_COLUMN_TYPE, aName);
            }
            m_pFileStream->Write(aBuffer, 14);
            aBuffer[0] = 0x00;
        }

        (*m_pFileStream).WriteUChar( FIELD_DESCRIPTOR_TERMINATOR );              // end of header
        (*m_pFileStream).WriteChar( (char)DBF_EOL );
        m_pFileStream->Seek(10L);
        (*m_pFileStream).WriteUInt16( nRecLength );                                     // set record length afterwards

        if (bCreateMemo)
        {
            m_pFileStream->Seek(0L);
            if (nDbaseType == VisualFoxPro)
                (*m_pFileStream).WriteUChar( FoxProMemo );
            else
                (*m_pFileStream).WriteUChar( dBaseIIIMemo );
        } // if (bCreateMemo)
    }
    catch ( const Exception& e )
    {
        (void)e;

        try
        {
            // we have to drop the file because it is corrupted now
            DropImpl();
        }
        catch(const Exception&) { }
        throw;
    }
    return true;
}


// creates in principle dBase III file format
bool ODbaseTable::CreateMemoFile(const INetURLObject& aFile)
{
    // filehandling macro for table creation
    m_pMemoStream = createStream_simpleError( aFile.GetMainURL(INetURLObject::NO_DECODE),STREAM_READWRITE | StreamMode::SHARE_DENYWRITE);

    if (!m_pMemoStream)
        return false;

    m_pMemoStream->SetStreamSize(512);

    m_pMemoStream->Seek(0L);
    (*m_pMemoStream).WriteUInt32( 1 );                  // pointer to the first free block

    m_pMemoStream->Flush();
    delete m_pMemoStream;
    m_pMemoStream = nullptr;
    return true;
}

bool ODbaseTable::Drop_Static(const OUString& _sUrl, bool _bHasMemoFields, OCollection* _pIndexes )
{
    INetURLObject aURL;
    aURL.SetURL(_sUrl);

    bool bDropped = ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::NO_DECODE));

    if(bDropped)
    {
        if (_bHasMemoFields)
        {  // delete the memo fields
            aURL.setExtension("dbt");
            bDropped = ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::NO_DECODE));
        }

        if(bDropped)
        {
            if(_pIndexes)
            {
                try
                {
                    sal_Int32 i = _pIndexes->getCount();
                    while (i)
                    {
                        _pIndexes->dropByIndex(--i);
                    }
                }
                catch(const SQLException&)
                {
                }
            }
            aURL.setExtension("inf");

            // as the inf file does not necessarily exist, we aren't allowed to use UCBContentHelper::Kill
            try
            {
                ::ucbhelper::Content aDeleteContent( aURL.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                aDeleteContent.executeCommand( "delete", makeAny( true ) );
            }
            catch(const Exception&)
            {
                // silently ignore this ....
            }
        }
    }
    return bDropped;
}

bool ODbaseTable::DropImpl()
{
    FileClose();

    if(!m_pIndexes)
        refreshIndexes(); // look for indexes which must be deleted as well

    bool bDropped = Drop_Static(getEntry(m_pConnection,m_Name),HasMemoFields(),m_pIndexes);
    if(!bDropped)
    {// we couldn't drop the table so we have to reopen it
        construct();
        if(m_pColumns)
            m_pColumns->refresh();
    }
    return bDropped;
}


bool ODbaseTable::InsertRow(OValueRefVector& rRow, bool bFlush, const Reference<XIndexAccess>& _xCols)
{
    // fill buffer with blanks
    if (!AllocBuffer())
        return false;

    memset(m_pBuffer, 0, m_aHeader.db_slng);
    m_pBuffer[0] = ' ';

    // Copy new row completely:
    // ... and add at the end as new Record:
    sal_Size nTempPos = m_nFilePos;

    m_nFilePos = (sal_Size)m_aHeader.db_anz + 1;
    bool bInsertRow = UpdateBuffer( rRow, nullptr, _xCols, true );
    if ( bInsertRow )
    {
        sal_Size nFileSize = 0, nMemoFileSize = 0;

        nFileSize = lcl_getFileSize(*m_pFileStream);

        if (HasMemoFields() && m_pMemoStream)
        {
            m_pMemoStream->Seek(STREAM_SEEK_TO_END);
            nMemoFileSize = m_pMemoStream->Tell();
        }

        if (!WriteBuffer())
        {
            m_pFileStream->SetStreamSize(nFileSize);                // restore old size

            if (HasMemoFields() && m_pMemoStream)
                m_pMemoStream->SetStreamSize(nMemoFileSize);    // restore old size
            m_nFilePos = nTempPos;              // restore file position
        }
        else
        {
            (*m_pFileStream).WriteChar( (char)DBF_EOL ); // write EOL
            // raise number of datasets in the header:
            m_pFileStream->Seek( 4L );
            (*m_pFileStream).WriteUInt32( m_aHeader.db_anz + 1 );

            // if AppendOnly no flush!
            if (bFlush)
                m_pFileStream->Flush();

            // raise number if successfully
            m_aHeader.db_anz++;
            *rRow.get()[0] = m_nFilePos;                                // set bookmark
            m_nFilePos = nTempPos;
        }
    }
    else
        m_nFilePos = nTempPos;

    return bInsertRow;
}


bool ODbaseTable::UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow, const Reference<XIndexAccess>& _xCols)
{
    // fill buffer with blanks
    if (!AllocBuffer())
        return false;

    // position on desired record:
    sal_Size nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);
    m_pFileStream->Read(m_pBuffer, m_aHeader.db_slng);

    sal_Size nMemoFileSize( 0 );
    if (HasMemoFields() && m_pMemoStream)
    {
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        nMemoFileSize = m_pMemoStream->Tell();
    }
    if (!UpdateBuffer(rRow, pOrgRow, _xCols, false) || !WriteBuffer())
    {
        if (HasMemoFields() && m_pMemoStream)
            m_pMemoStream->SetStreamSize(nMemoFileSize);    // restore old size
    }
    else
    {
        m_pFileStream->Flush();
    }
    return true;
}


bool ODbaseTable::DeleteRow(const OSQLColumns& _rCols)
{
    // Set the Delete-Flag (be it set or not):
    // Position on desired record:
    sal_Size nFilePos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nFilePos);

    OValueRefRow aRow = new OValueRefVector(_rCols.get().size());

    if (!fetchRow(aRow,_rCols,true,true))
        return false;

    Reference<XPropertySet> xCol;
    OUString aColName;
    ::comphelper::UStringMixEqual aCase(isCaseSensitive());
    for (sal_Int32 i = 0; i < m_pColumns->getCount(); i++)
    {
        Reference<XPropertySet> xIndex = isUniqueByColumnName(i);
        if (xIndex.is())
        {
            xCol.set(m_pColumns->getByIndex(i), css::uno::UNO_QUERY);
            OSL_ENSURE(xCol.is(),"ODbaseTable::DeleteRow column is null!");
            if(xCol.is())
            {
                xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;

                Reference<XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
                OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
                ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
                OSL_ENSURE(pIndex,"ODbaseTable::DeleteRow: No Index returned!");

                OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
                sal_Int32 nPos = 1;
                for(;aIter != _rCols.get().end();++aIter,++nPos)
                {
                    if(aCase(getString((*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME))),aColName))
                        break;
                }
                if (aIter == _rCols.get().end())
                    continue;

                pIndex->Delete(m_nFilePos,*(aRow->get())[nPos]);
            }
        }
    }

    m_pFileStream->Seek(nFilePos);
    (*m_pFileStream).WriteUChar( '*' ); // mark the row in the table as deleted
    m_pFileStream->Flush();
    return true;
}

Reference<XPropertySet> ODbaseTable::isUniqueByColumnName(sal_Int32 _nColumnPos)
{
    if(!m_pIndexes)
        refreshIndexes();
    if(m_pIndexes->hasElements())
    {
        Reference<XPropertySet> xCol;
        m_pColumns->getByIndex(_nColumnPos) >>= xCol;
        OSL_ENSURE(xCol.is(),"ODbaseTable::isUniqueByColumnName column is null!");
        OUString sColName;
        xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sColName;

        Reference<XPropertySet> xIndex;
        for(sal_Int32 i=0;i<m_pIndexes->getCount();++i)
        {
            xIndex.set(m_pIndexes->getByIndex(i), css::uno::UNO_QUERY);
            if(xIndex.is() && getBOOL(xIndex->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            {
                Reference<XNameAccess> xCols(Reference<XColumnsSupplier>(xIndex,UNO_QUERY)->getColumns());
                if(xCols->hasByName(sColName))
                    return xIndex;

            }
        }
    }
    return Reference<XPropertySet>();
}

static double toDouble(const OString& rString)
{
    return ::rtl::math::stringToDouble( rString, '.', ',' );
}


bool ODbaseTable::UpdateBuffer(OValueRefVector& rRow, OValueRefRow pOrgRow, const Reference<XIndexAccess>& _xCols, const bool bForceAllFields)
{
    OSL_ENSURE(m_pBuffer,"Buffer is NULL!");
    if ( !m_pBuffer )
        return false;
    sal_Int32 nByteOffset  = 1;

    // Update fields:
    Reference<XPropertySet> xCol;
    Reference<XPropertySet> xIndex;
    sal_uInt16 i;
    OUString aColName;
    const sal_Int32 nColumnCount = m_pColumns->getCount();
    ::std::vector< Reference<XPropertySet> > aIndexedCols(nColumnCount);

    ::comphelper::UStringMixEqual aCase(isCaseSensitive());

    Reference<XIndexAccess> xColumns = m_pColumns;
    // first search a key that exist already in the table
    for (i = 0; i < nColumnCount; ++i)
    {
        sal_Int32 nPos = i;
        if(_xCols != xColumns)
        {
            m_pColumns->getByIndex(i) >>= xCol;
            OSL_ENSURE(xCol.is(),"ODbaseTable::UpdateBuffer column is null!");
            xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;

            for(nPos = 0;nPos<_xCols->getCount();++nPos)
            {
                Reference<XPropertySet> xFindCol(
                    _xCols->getByIndex(nPos), css::uno::UNO_QUERY);
                OSL_ENSURE(xFindCol.is(),"ODbaseTable::UpdateBuffer column is null!");
                if(aCase(getString(xFindCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),aColName))
                    break;
            }
            if (nPos >= _xCols->getCount())
                continue;
        }

        ++nPos;
        xIndex = isUniqueByColumnName(i);
        aIndexedCols[i] = xIndex;
        if (xIndex.is())
        {
            // first check if the value is different to the old one and when if it conform to the index
            if(pOrgRow.is() && (rRow.get()[nPos]->getValue().isNull() || rRow.get()[nPos] == (pOrgRow->get())[nPos]))
                continue;
            else
            {
                Reference<XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
                OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
                ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
                OSL_ENSURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");

                if (pIndex->Find(0,*rRow.get()[nPos]))
                {
                    // There is no unique value
                    if ( aColName.isEmpty() )
                    {
                        m_pColumns->getByIndex(i) >>= xCol;
                        OSL_ENSURE(xCol.is(),"ODbaseTable::UpdateBuffer column is null!");
                        xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
                        xCol.clear();
                    } // if ( !aColName.getLength() )
                    const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                            STR_DUPLICATE_VALUE_IN_COLUMN
                            ,"$columnname$", aColName
                         ) );
                    ::dbtools::throwGenericSQLException( sError, *this );
                }
            }
        }
    }

    // when we are here there is no double key in the table

    for (i = 0; i < nColumnCount && nByteOffset <= m_nBufferSize ; ++i)
    {
        // Lengths for each data type:
        OSL_ENSURE(i < m_aPrecisions.size(),"Illegal index!");
        sal_Int32 nLen = 0;
        sal_Int32 nType = 0;
        sal_Int32 nScale = 0;
        if ( i < m_aPrecisions.size() )
        {
            nLen    = m_aPrecisions[i];
            nType   = m_aTypes[i];
            nScale  = m_aScales[i];
        }
        else
        {
            m_pColumns->getByIndex(i) >>= xCol;
            if ( xCol.is() )
            {
                xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)) >>= nLen;
                xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))      >>= nType;
                xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))     >>= nScale;
            }
        }

        bool bSetZero = false;
        switch (nType)
        {
            case DataType::INTEGER:
            case DataType::DOUBLE:
            case DataType::TIMESTAMP:
                bSetZero = true;
                SAL_FALLTHROUGH;
            case DataType::LONGVARBINARY:
            case DataType::DATE:
            case DataType::BIT:
            case DataType::LONGVARCHAR:
                nLen = m_aRealFieldLengths[i];
                break;
            case DataType::DECIMAL:
                nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,nScale);
                break;  // The sign and the comma
            default:
                break;

        } // switch (nType)

        sal_Int32 nPos = i;
        if(_xCols != xColumns)
        {
            m_pColumns->getByIndex(i) >>= xCol;
            OSL_ENSURE(xCol.is(),"ODbaseTable::UpdateBuffer column is null!");
            xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
            for(nPos = 0;nPos<_xCols->getCount();++nPos)
            {
                Reference<XPropertySet> xFindCol(
                    _xCols->getByIndex(nPos), css::uno::UNO_QUERY);
                if(aCase(getString(xFindCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),aColName))
                    break;
            }
            if (nPos >= _xCols->getCount())
            {
                nByteOffset += nLen;
                continue;
            }
        }


        ++nPos; // the row values start at 1
        const ORowSetValue &thisColVal = rRow.get()[nPos]->get();
        const bool thisColIsBound = thisColVal.isBound();
        const bool thisColIsNull = !thisColIsBound || thisColVal.isNull();
        // don't overwrite non-bound columns
        if ( ! (bForceAllFields || thisColIsBound) )
        {
            // No - don't overwrite this field, it has not changed.
            nByteOffset += nLen;
            continue;
        }
        if (aIndexedCols[i].is())
        {
            Reference<XUnoTunnel> xTunnel(aIndexedCols[i],UNO_QUERY);
            OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
            ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
            OSL_ENSURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");
            // Update !!
            if (pOrgRow.is() && !thisColIsNull)
                pIndex->Update(m_nFilePos, *(pOrgRow->get())[nPos], thisColVal);
            else
                pIndex->Insert(m_nFilePos, thisColVal);
        }

        char* pData = reinterpret_cast<char *>(m_pBuffer + nByteOffset);
        if (thisColIsNull)
        {
            if ( bSetZero )
                memset(pData,0,nLen);   // Clear to NULL char ('\0')
            else
                memset(pData,' ',nLen); // Clear to space/blank ('\0x20')
            nByteOffset += nLen;
            OSL_ENSURE( nByteOffset <= m_nBufferSize ,"ByteOffset > m_nBufferSize!");
            continue;
        }

        try
        {
            switch (nType)
            {
                case DataType::TIMESTAMP:
                    {
                        sal_Int32 nJulianDate = 0, nJulianTime = 0;
                        lcl_CalcJulDate(nJulianDate,nJulianTime, thisColVal);
                        // Exactly 8 bytes to copy:
                        memcpy(pData,&nJulianDate,4);
                        memcpy(pData+4,&nJulianTime,4);
                    }
                    break;
                case DataType::DATE:
                {
                    ::com::sun::star::util::Date aDate;
                    if(thisColVal.getTypeKind() == DataType::DOUBLE)
                        aDate = ::dbtools::DBTypeConversion::toDate(thisColVal.getDouble());
                    else
                        aDate = thisColVal;
                    char s[9];
                    snprintf(s,
                        sizeof(s),
                        "%04d%02d%02d",
                        (int)aDate.Year,
                        (int)aDate.Month,
                        (int)aDate.Day);

                    // Exactly 8 bytes to copy:
                    strncpy(pData,s,sizeof s - 1);
                } break;
                case DataType::INTEGER:
                    {
                        sal_Int32 nValue = thisColVal;
                        if (static_cast<size_t>(nLen) > sizeof(nValue))
                            return false;
                        memcpy(pData,&nValue,nLen);
                    }
                    break;
                case DataType::DOUBLE:
                    {
                        const double d = thisColVal;
                        m_pColumns->getByIndex(i) >>= xCol;

                        if (getBOOL(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))) // Currency is treated separately
                        {
                            sal_Int64 nValue = 0;
                            if ( m_aScales[i] )
                                nValue = (sal_Int64)(d * pow(10.0,(int)m_aScales[i]));
                            else
                                nValue = (sal_Int64)(d);
                            if (static_cast<size_t>(nLen) > sizeof(nValue))
                                return false;
                            memcpy(pData,&nValue,nLen);
                        } // if (getBOOL(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))) // Currency is treated separately
                        else
                        {
                            if (static_cast<size_t>(nLen) > sizeof(d))
                                return false;
                            memcpy(pData,&d,nLen);
                        }
                    }
                    break;
                case DataType::DECIMAL:
                {
                    memset(pData,' ',nLen); // Clear to NULL

                    const double n = thisColVal;

                    // one, because const_cast GetFormatPrecision on SvNumberFormat is not constant,
                    // even though it really could and should be
                    const OString aDefaultValue( ::rtl::math::doubleToString( n, rtl_math_StringFormat_F, nScale, '.', nullptr, 0));
                    const sal_Int32 nValueLen = aDefaultValue.getLength();
                    if ( nValueLen <= nLen )
                    {
                        // Write value right-justified, padded with blanks to the left.
                        memcpy(pData+nLen-nValueLen,aDefaultValue.getStr(),nValueLen);
                        // write the resulting double back
                        *rRow.get()[nPos] = toDouble(aDefaultValue);
                    }
                    else
                    {
                        m_pColumns->getByIndex(i) >>= xCol;
                        OSL_ENSURE(xCol.is(),"ODbaseTable::UpdateBuffer column is null!");
                        xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
                        ::std::list< ::std::pair<const sal_Char* , OUString > > aStringToSubstitutes;
                        aStringToSubstitutes.push_back(::std::pair<const sal_Char* , OUString >("$columnname$", aColName));
                        aStringToSubstitutes.push_back(::std::pair<const sal_Char* , OUString >("$precision$", OUString::number(nLen)));
                        aStringToSubstitutes.push_back(::std::pair<const sal_Char* , OUString >("$scale$", OUString::number(nScale)));
                        aStringToSubstitutes.push_back(::std::pair<const sal_Char* , OUString >("$value$", OStringToOUString(aDefaultValue,RTL_TEXTENCODING_UTF8)));

                        const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                                STR_INVALID_COLUMN_DECIMAL_VALUE
                                ,aStringToSubstitutes
                             ) );
                        ::dbtools::throwGenericSQLException( sError, *this );
                    }
                } break;
                case DataType::BIT:
                    *pData = thisColVal.getBool() ? 'T' : 'F';
                    break;
                case DataType::LONGVARBINARY:
                case DataType::LONGVARCHAR:
                {
                    char cNext = pData[nLen]; // Mark's scratch and replaced by 0
                    pData[nLen] = '\0';       // This is because the buffer is always a sign of greater ...

                    sal_Size nBlockNo = strtol(pData,nullptr,10); // Block number read

                    // Next initial character restore again:
                    pData[nLen] = cNext;
                    if (!m_pMemoStream || !WriteMemo(thisColVal, nBlockNo))
                        break;

                    OString aBlock(OString::number(nBlockNo));
                    //align aBlock at the right of a nLen sequence, fill to the left with '0'
                    OStringBuffer aStr;
                    comphelper::string::padToLength(aStr, nLen - aBlock.getLength(), '0');
                    aStr.append(aBlock);

                    // Copy characters:
                    memcpy(pData, aStr.getStr(), nLen);
                }   break;
                default:
                {
                    memset(pData,' ',nLen); // Clear to NULL

                    OUString sStringToWrite( thisColVal.getString() );

                    // convert the string, using the connection's encoding
                    OString sEncoded;

                    DBTypeConversion::convertUnicodeStringToLength( sStringToWrite, sEncoded, nLen, m_eEncoding );
                    memcpy( pData, sEncoded.getStr(), sEncoded.getLength() );

                }
                break;
            }
        }
        catch( const SQLException&  )
        {
            throw;
        }
        catch ( const Exception& )
        {
            m_pColumns->getByIndex(i) >>= xCol;
            OSL_ENSURE( xCol.is(), "ODbaseTable::UpdateBuffer column is null!" );
            if ( xCol.is() )
                xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;

            const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_INVALID_COLUMN_VALUE,
                    "$columnname$", aColName
                 ) );
            ::dbtools::throwGenericSQLException( sError, *this );
        }
        // And more ...
        nByteOffset += nLen;
        OSL_ENSURE( nByteOffset <= m_nBufferSize ,"ByteOffset > m_nBufferSize!");
    }
    return true;
}


bool ODbaseTable::WriteMemo(const ORowSetValue& aVariable, sal_Size& rBlockNr)
{
    // if the BlockNo 0 is given, the block will be appended at the end
    sal_Size nSize = 0;
    OString aStr;
    ::com::sun::star::uno::Sequence<sal_Int8> aValue;
    sal_uInt8 nHeader[4];
    const bool bBinary = aVariable.getTypeKind() == DataType::LONGVARBINARY && m_aMemoHeader.db_typ == MemoFoxPro;
    if ( bBinary )
    {
        aValue = aVariable.getSequence();
        nSize = aValue.getLength();
    }
    else
    {
        nSize = DBTypeConversion::convertUnicodeString( aVariable.getString(), aStr, m_eEncoding );
    }

    // append or overwrite
    bool bAppend = rBlockNr == 0;

    if (!bAppend)
    {
        switch (m_aMemoHeader.db_typ)
        {
            case MemodBaseIII: // dBase III-Memofield, ends with 2 * Ctrl-Z
                bAppend = nSize > (512 - 2);
                break;
            case MemoFoxPro:
            case MemodBaseIV: // dBase IV-Memofield with length
            {
                char sHeader[4];
                m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
                m_pMemoStream->SeekRel(4L);
                m_pMemoStream->Read(sHeader,4);

                sal_Size nOldSize;
                if (m_aMemoHeader.db_typ == MemoFoxPro)
                    nOldSize = ((((unsigned char)sHeader[0]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[3];
                else
                    nOldSize = ((((unsigned char)sHeader[3]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[0]  - 8;

                // fits the new length in the used blocks
                sal_Size nUsedBlocks = ((nSize + 8) / m_aMemoHeader.db_size) + (((nSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0),
                      nOldUsedBlocks = ((nOldSize + 8) / m_aMemoHeader.db_size) + (((nOldSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0);
                bAppend = nUsedBlocks > nOldUsedBlocks;
            }
        }
    }

    if (bAppend)
    {
        sal_Size nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        // fill last block
        rBlockNr = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        m_pMemoStream->SetStreamSize(rBlockNr * m_aMemoHeader.db_size);
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
    }
    else
    {
        m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
    }

    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofield, ends with Ctrl-Z
        {
            const char cEOF = (char) DBF_EOL;
            nSize++;
            m_pMemoStream->Write( aStr.getStr(), aStr.getLength() );
            (*m_pMemoStream).WriteChar( cEOF ).WriteChar( cEOF );
        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld with length
        {
            if ( MemodBaseIV == m_aMemoHeader.db_typ )
                (*m_pMemoStream).WriteUChar( 0xFF )
                                .WriteUChar( 0xFF )
                                .WriteUChar( 0x08 );
            else
                (*m_pMemoStream).WriteUChar( 0x00 )
                                .WriteUChar( 0x00 )
                                .WriteUChar( 0x00 );

            sal_uInt32 nWriteSize = nSize;
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                if ( bBinary )
                    (*m_pMemoStream).WriteUChar( 0x00 ); // Picture
                else
                    (*m_pMemoStream).WriteUChar( 0x01 ); // Memo
                for (int i = 4; i > 0; nWriteSize >>= 8)
                    nHeader[--i] = (sal_uInt8) (nWriteSize % 256);
            }
            else
            {
                (*m_pMemoStream).WriteUChar( 0x00 );
                nWriteSize += 8;
                for (int i = 0; i < 4; nWriteSize >>= 8)
                    nHeader[i++] = (sal_uInt8) (nWriteSize % 256);
            }

            m_pMemoStream->Write(nHeader,4);
            if ( bBinary )
                m_pMemoStream->Write( aValue.getConstArray(), aValue.getLength() );
            else
                m_pMemoStream->Write( aStr.getStr(), aStr.getLength() );
            m_pMemoStream->Flush();
        }
    }


    // Write the new block number
    if (bAppend)
    {
        sal_Size nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        m_aMemoHeader.db_next = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        // Write the new block number
        m_pMemoStream->Seek(0L);
        (*m_pMemoStream).WriteUInt32( m_aMemoHeader.db_next );
        m_pMemoStream->Flush();
    }
    return true;
}


// XAlterTable
void SAL_CALL ODbaseTable::alterColumnByName( const OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);


    Reference<XDataDescriptorFactory> xOldColumn;
    m_pColumns->getByName(colName) >>= xOldColumn;

    try
    {
        alterColumn(m_pColumns->findColumn(colName)-1,descriptor,xOldColumn);
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        throw NoSuchElementException(colName, *this);
    }
}

void SAL_CALL ODbaseTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    if(index < 0 || index >= m_pColumns->getCount())
        throw IndexOutOfBoundsException(OUString::number(index),*this);

    Reference<XDataDescriptorFactory> xOldColumn;
    m_pColumns->getByIndex(index) >>= xOldColumn;
    alterColumn(index,descriptor,xOldColumn);
}

void ODbaseTable::alterColumn(sal_Int32 index,
                              const Reference< XPropertySet >& descriptor ,
                              const Reference< XDataDescriptorFactory >& xOldColumn )
{
    if(index < 0 || index >= m_pColumns->getCount())
        throw IndexOutOfBoundsException(OUString::number(index),*this);

    ODbaseTable* pNewTable = nullptr;
    try
    {
        OSL_ENSURE(descriptor.is(),"ODbaseTable::alterColumn: descriptor can not be null!");
        // creates a copy of the original column and copy all properties from descriptor in xCopyColumn
        Reference<XPropertySet> xCopyColumn;
        if(xOldColumn.is())
            xCopyColumn = xOldColumn->createDataDescriptor();
        else
            xCopyColumn = new OColumn(getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers());

        ::comphelper::copyProperties(descriptor,xCopyColumn);

        // creates a temp file

        OUString sTempName = createTempFile();

        pNewTable = new ODbaseTable(m_pTables,static_cast<ODbaseConnection*>(m_pConnection));
        Reference<XPropertySet> xHoldTable = pNewTable;
        pNewTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sTempName));
        Reference<XAppend> xAppend(pNewTable->getColumns(),UNO_QUERY);
        OSL_ENSURE(xAppend.is(),"ODbaseTable::alterColumn: No XAppend interface!");

        // copy the structure
        sal_Int32 i=0;
        for(;i < index;++i)
        {
            Reference<XPropertySet> xProp;
            m_pColumns->getByIndex(i) >>= xProp;
            Reference<XDataDescriptorFactory> xColumn(xProp,UNO_QUERY);
            Reference<XPropertySet> xCpy;
            if(xColumn.is())
                xCpy = xColumn->createDataDescriptor();
            else
                xCpy = new OColumn(getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers());
            ::comphelper::copyProperties(xProp,xCpy);
            xAppend->appendByDescriptor(xCpy);
        }
        ++i; // now insert our new column
        xAppend->appendByDescriptor(xCopyColumn);

        for(;i < m_pColumns->getCount();++i)
        {
            Reference<XPropertySet> xProp;
            m_pColumns->getByIndex(i) >>= xProp;
            Reference<XDataDescriptorFactory> xColumn(xProp,UNO_QUERY);
            Reference<XPropertySet> xCpy;
            if(xColumn.is())
                xCpy = xColumn->createDataDescriptor();
            else
                xCpy = new OColumn(getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers());
            ::comphelper::copyProperties(xProp,xCpy);
            xAppend->appendByDescriptor(xCpy);
        }

        // construct the new table
        if(!pNewTable->CreateImpl())
        {
            const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_COLUMN_NOT_ALTERABLE,
                    "$columnname$", ::comphelper::getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))
                 ) );
            ::dbtools::throwGenericSQLException( sError, *this );
        }

        pNewTable->construct();

        // copy the data
        copyData(pNewTable,0);

        // now drop the old one
        if( DropImpl() ) // we don't want to delete the memo columns too
        {
            try
            {
                // rename the new one to the old one
                pNewTable->renameImpl(m_Name);
            }
            catch(const css::container::ElementExistException&)
            {
                const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                        STR_COULD_NOT_DELETE_FILE,
                        "$filename$", m_Name
                     ) );
                ::dbtools::throwGenericSQLException( sError, *this );
            }
            // release the temp file
            pNewTable = nullptr;
            ::comphelper::disposeComponent(xHoldTable);
        }
        else
        {
            pNewTable = nullptr;
        }
        FileClose();
        construct();
        if(m_pColumns)
            m_pColumns->refresh();

    }
    catch(const SQLException&)
    {
        throw;
    }
    catch(const Exception&)
    {
        SAL_WARN( "connectivity.drivers","ODbaseTable::alterColumn: Exception occurred!");
        throw;
    }
}

Reference< XDatabaseMetaData> ODbaseTable::getMetaData() const
{
    return getConnection()->getMetaData();
}

void SAL_CALL ODbaseTable::rename( const OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    if(m_pTables && m_pTables->hasByName(newName))
        throw ElementExistException(newName,*this);


    renameImpl(newName);

    ODbaseTable_BASE::rename(newName);

    construct();
    if(m_pColumns)
        m_pColumns->refresh();
}
namespace
{
    void renameFile(OConnection* _pConenction,const OUString& oldName,
                    const OUString& newName,const OUString& _sExtension)
    {
        OUString aName = ODbaseTable::getEntry(_pConenction,oldName);
        if(aName.isEmpty())
        {
            OUString aIdent = _pConenction->getContent()->getIdentifier()->getContentIdentifier();
            if ( aIdent.lastIndexOf('/') != (aIdent.getLength()-1) )
                aIdent += "/";
            aIdent += oldName;
            aName = aIdent;
        }
        INetURLObject aURL;
        aURL.SetURL(aName);

        aURL.setExtension( _sExtension );
        OUString sNewName(newName + "." + _sExtension);

        try
        {
            Content aContent(aURL.GetMainURL(INetURLObject::NO_DECODE),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());

            Sequence< PropertyValue > aProps( 1 );
            aProps[0].Name      = "Title";
            aProps[0].Handle    = -1; // n/a
            aProps[0].Value     = makeAny( sNewName );
            Sequence< Any > aValues;
            aContent.executeCommand( "setPropertyValues",makeAny(aProps) ) >>= aValues;
            if(aValues.getLength() && aValues[0].hasValue())
                throw Exception();
        }
        catch(const Exception&)
        {
            throw ElementExistException(newName);
        }
    }
}

void SAL_CALL ODbaseTable::renameImpl( const OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    FileClose();


    renameFile(m_pConnection,m_Name,newName,m_pConnection->getExtension());
    if ( HasMemoFields() )
    {  // delete the memo fields
        OUString sExt("dbt");
        renameFile(m_pConnection,m_Name,newName,sExt);
    }
}

void ODbaseTable::addColumn(const Reference< XPropertySet >& _xNewColumn)
{
    OUString sTempName = createTempFile();

    ODbaseTable* pNewTable = new ODbaseTable(m_pTables,static_cast<ODbaseConnection*>(m_pConnection));
    Reference< XPropertySet > xHold = pNewTable;
    pNewTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sTempName));
    {
        Reference<XAppend> xAppend(pNewTable->getColumns(),UNO_QUERY);
        bool bCase = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();
        // copy the structure
        for(sal_Int32 i=0;i < m_pColumns->getCount();++i)
        {
            Reference<XPropertySet> xProp;
            m_pColumns->getByIndex(i) >>= xProp;
            Reference<XDataDescriptorFactory> xColumn(xProp,UNO_QUERY);
            Reference<XPropertySet> xCpy;
            if(xColumn.is())
                xCpy = xColumn->createDataDescriptor();
            else
            {
                xCpy = new OColumn(bCase);
                ::comphelper::copyProperties(xProp,xCpy);
            }

            xAppend->appendByDescriptor(xCpy);
        }
        Reference<XPropertySet> xCpy = new OColumn(bCase);
        ::comphelper::copyProperties(_xNewColumn,xCpy);
        xAppend->appendByDescriptor(xCpy);
    }

    // construct the new table
    if(!pNewTable->CreateImpl())
    {
        const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_ADDABLE,
                "$columnname$", ::comphelper::getString(_xNewColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))
             ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }

    pNewTable->construct();
    // copy the data
    copyData(pNewTable,pNewTable->m_pColumns->getCount());
    // drop the old table
    if(DropImpl())
    {
        pNewTable->renameImpl(m_Name);
        // release the temp file
    }
    xHold.clear();

    FileClose();
    construct();
    if(m_pColumns)
        m_pColumns->refresh();
}

void ODbaseTable::dropColumn(sal_Int32 _nPos)
{
    OUString sTempName = createTempFile();

    ODbaseTable* pNewTable = new ODbaseTable(m_pTables,static_cast<ODbaseConnection*>(m_pConnection));
    Reference< XPropertySet > xHold = pNewTable;
    pNewTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sTempName));
    {
        Reference<XAppend> xAppend(pNewTable->getColumns(),UNO_QUERY);
        bool bCase = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();
        // copy the structure
        for(sal_Int32 i=0;i < m_pColumns->getCount();++i)
        {
            if(_nPos != i)
            {
                Reference<XPropertySet> xProp;
                m_pColumns->getByIndex(i) >>= xProp;
                Reference<XDataDescriptorFactory> xColumn(xProp,UNO_QUERY);
                Reference<XPropertySet> xCpy;
                if(xColumn.is())
                    xCpy = xColumn->createDataDescriptor();
                else
                {
                    xCpy = new OColumn(bCase);
                    ::comphelper::copyProperties(xProp,xCpy);
                }
                xAppend->appendByDescriptor(xCpy);
            }
        }
    }

    // construct the new table
    if(!pNewTable->CreateImpl())
    {
        xHold.clear();
        const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_DROP,
                "$position$", OUString::number(_nPos)
             ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }
    pNewTable->construct();
    // copy the data
    copyData(pNewTable,_nPos);
    // drop the old table
    if(DropImpl())
        pNewTable->renameImpl(m_Name);
        // release the temp file

    xHold.clear();

    FileClose();
    construct();
}

OUString ODbaseTable::createTempFile()
{
    OUString aIdent = m_pConnection->getContent()->getIdentifier()->getContentIdentifier();
    if ( aIdent.lastIndexOf('/') != (aIdent.getLength()-1) )
        aIdent += "/";

    OUString sTempName(aIdent);
    OUString sExt("." + m_pConnection->getExtension());
    OUString sName(m_Name);
    TempFile aTempFile(sName, true, &sExt, &sTempName);
    if(!aTempFile.IsValid())
        getConnection()->throwGenericSQLException(STR_COULD_NOT_ALTER_TABLE, *this);

    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    aURL.SetURL(aTempFile.GetURL());

    OUString sNewName(aURL.getName().copy(0, aURL.getName().getLength() - sExt.getLength()));

    return sNewName;
}

void ODbaseTable::copyData(ODbaseTable* _pNewTable,sal_Int32 _nPos)
{
    sal_Int32 nPos = _nPos + 1; // +1 because we always have the bookmark column as well
    OValueRefRow aRow = new OValueRefVector(m_pColumns->getCount());
    OValueRefRow aInsertRow;
    if(_nPos)
    {
        aInsertRow = new OValueRefVector(_pNewTable->m_pColumns->getCount());
        ::std::for_each(aInsertRow->get().begin(),aInsertRow->get().end(),TSetRefBound(true));
    }
    else
        aInsertRow = aRow;

    // we only have to bind the values which we need to copy into the new table
    ::std::for_each(aRow->get().begin(),aRow->get().end(),TSetRefBound(true));
    if(_nPos && (_nPos < (sal_Int32)aRow->get().size()))
        (aRow->get())[nPos]->setBound(false);


    sal_Int32 nCurPos;
    OValueRefVector::Vector::iterator aIter;
    for(sal_uInt32 nRowPos = 0; nRowPos < m_aHeader.db_anz;++nRowPos)
    {
        bool bOk = seekRow( IResultSetHelper::BOOKMARK, nRowPos+1, nCurPos );
        if ( bOk )
        {
            bOk = fetchRow( aRow, *m_aColumns, true, true);
            if ( bOk && !aRow->isDeleted() ) // copy only not deleted rows
            {
                // special handling when pos == 0 then we don't have to distinguish between the two rows
                if(_nPos)
                {
                    aIter = aRow->get().begin()+1;
                    sal_Int32 nCount = 1;
                    for(OValueRefVector::Vector::iterator aInsertIter = aInsertRow->get().begin()+1; aIter != aRow->get().end() && aInsertIter != aInsertRow->get().end();++aIter,++nCount)
                    {
                        if(nPos != nCount)
                        {
                            (*aInsertIter)->setValue( (*aIter)->getValue() );
                            ++aInsertIter;
                        }
                    }
                }
                bOk = _pNewTable->InsertRow(*aInsertRow,true,_pNewTable->m_pColumns);
                SAL_WARN_IF(!bOk, "connectivity.drivers", "Row could not be inserted!"); (void)bOk;
            }
            else
            {
                SAL_WARN_IF(!bOk, "connectivity.drivers", "Row could not be fetched!"); (void)bOk;
            }
        }
        else
        {
            OSL_ASSERT(false);
        }
    } // for(sal_uInt32 nRowPos = 0; nRowPos < m_aHeader.db_anz;++nRowPos)
}

void ODbaseTable::throwInvalidDbaseFormat()
{
    FileClose();
    // no dbase file

    const OUString sError( getConnection()->getResources().getResourceStringWithSubstitution(
                STR_INVALID_DBASE_FILE,
                "$filename$", getEntry(m_pConnection,m_Name)
             ) );
    ::dbtools::throwGenericSQLException( sError, *this );
}

void ODbaseTable::refreshHeader()
{
    if ( m_aHeader.db_anz == 0 )
        readHeader();
}

bool ODbaseTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    // prepare positioning:
    OSL_ENSURE(m_pFileStream,"ODbaseTable::seekRow: FileStream is NULL!");

    sal_uInt32  nNumberOfRecords = (sal_uInt32)m_aHeader.db_anz;
    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::NEXT:
            ++m_nFilePos;
            break;
        case IResultSetHelper::PRIOR:
            if (m_nFilePos > 0)
                --m_nFilePos;
            break;
        case IResultSetHelper::FIRST:
            m_nFilePos = 1;
            break;
        case IResultSetHelper::LAST:
            m_nFilePos = nNumberOfRecords;
            break;
        case IResultSetHelper::RELATIVE1:
            m_nFilePos = (((sal_Int32)m_nFilePos) + nOffset < 0) ? 0L
                            : (sal_uInt32)(((sal_Int32)m_nFilePos) + nOffset);
            break;
        case IResultSetHelper::ABSOLUTE1:
        case IResultSetHelper::BOOKMARK:
            m_nFilePos = (sal_uInt32)nOffset;
            break;
    }

    if (m_nFilePos > (sal_Int32)nNumberOfRecords)
        m_nFilePos = (sal_Int32)nNumberOfRecords + 1;

    if (m_nFilePos == 0 || m_nFilePos == (sal_Int32)nNumberOfRecords + 1)
        goto Error;
    else
    {
        sal_Size nEntryLen = m_aHeader.db_slng;

        OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");
        sal_Size nPos = m_aHeader.db_kopf + (sal_Size)(m_nFilePos-1) * nEntryLen;

        m_pFileStream->Seek(nPos);
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;

        sal_Size nRead = m_pFileStream->Read(m_pBuffer, nEntryLen);
        if (nRead != nEntryLen)
        {
            SAL_WARN("connectivity.drivers", "ODbaseTable::seekRow: short read!");
            goto Error;
        }
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;
    }
    goto End;

Error:
    switch(eCursorPosition)
    {
        case IResultSetHelper::PRIOR:
        case IResultSetHelper::FIRST:
            m_nFilePos = 0;
            break;
        case IResultSetHelper::LAST:
        case IResultSetHelper::NEXT:
        case IResultSetHelper::ABSOLUTE1:
        case IResultSetHelper::RELATIVE1:
            if (nOffset > 0)
                m_nFilePos = nNumberOfRecords + 1;
            else if (nOffset < 0)
                m_nFilePos = 0;
            break;
        case IResultSetHelper::BOOKMARK:
            m_nFilePos = nTempPos;   // last position
    }
    return false;

End:
    nCurPos = m_nFilePos;
    return true;
}

bool ODbaseTable::ReadMemo(sal_Size nBlockNo, ORowSetValue& aVariable)
{
    m_pMemoStream->Seek(nBlockNo * m_aMemoHeader.db_size);
    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofield, ends with Ctrl-Z
        {
            const char cEOF = (char) DBF_EOL;
            OStringBuffer aBStr;
            static char aBuf[514];
            aBuf[512] = 0;          // avoid random value
            bool bReady = false;

            do
            {
                m_pMemoStream->Read(&aBuf,512);

                sal_uInt16 i = 0;
                while (aBuf[i] != cEOF && ++i < 512)
                    ;
                bReady = aBuf[i] == cEOF;

                aBuf[i] = 0;
                aBStr.append(aBuf);

            } while (!bReady && !m_pMemoStream->IsEof());

            aVariable = OStringToOUString(aBStr.makeStringAndClear(),
                m_eEncoding);

        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofield with length
        {
            bool bIsText = true;
            char sHeader[4];
            m_pMemoStream->Read(sHeader,4);
            // Foxpro stores text and binary data
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                bIsText = sHeader[3] != 0;
            }
            else if (((sal_uInt8)sHeader[0]) != 0xFF || ((sal_uInt8)sHeader[1]) != 0xFF || ((sal_uInt8)sHeader[2]) != 0x08)
            {
                return false;
            }

            sal_uInt32 nLength(0);
            (*m_pMemoStream).ReadUInt32( nLength );

            if (m_aMemoHeader.db_typ == MemodBaseIV)
                nLength -= 8;

            if ( nLength )
            {
                if ( bIsText )
                {
                    OStringBuffer aBuffer(read_uInt8s_ToOString(*m_pMemoStream, nLength));
                    //pad it out with ' ' to expected length on short read
                    sal_Int32 nRequested = sal::static_int_cast<sal_Int32>(nLength);
                    comphelper::string::padToLength(aBuffer, nRequested, ' ');
                    aVariable = OStringToOUString(aBuffer.makeStringAndClear(), m_eEncoding);
                } // if ( bIsText )
                else
                {
                    ::com::sun::star::uno::Sequence< sal_Int8 > aData(nLength);
                    m_pMemoStream->Read(aData.getArray(),nLength);
                    aVariable = aData;
                }
            } // if ( nLength )
        }
    }
    return true;
}

bool ODbaseTable::AllocBuffer()
{
    sal_uInt16 nSize = m_aHeader.db_slng;
    SAL_WARN_IF(nSize == 0, "connectivity.drivers", "Size too small");

    if (m_nBufferSize != nSize)
    {
        delete m_pBuffer;
        m_pBuffer = nullptr;
    }

    // if there is no buffer available: allocate:
    if (m_pBuffer == nullptr && nSize > 0)
    {
        m_nBufferSize = nSize;
        m_pBuffer = new sal_uInt8[m_nBufferSize+1];
    }

    return m_pBuffer != nullptr;
}

bool ODbaseTable::WriteBuffer()
{
    OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");

    // position on desired record:
    sal_Size nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);
    return m_pFileStream->Write(m_pBuffer, m_aHeader.db_slng) > 0;
}

sal_Int32 ODbaseTable::getCurrentLastPos() const
{
    return m_aHeader.db_anz;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
