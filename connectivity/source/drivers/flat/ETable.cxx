/*************************************************************************
 *
 *  $RCSfile: ETable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-19 11:55:41 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _CONNECTIVITY_FLAT_TABLE_HXX_
#include "flat/ETable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _SV_CONVERTER_HXX_
#include <svtools/converter.hxx>
#endif
#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_COLUMNS_HXX_
#include "flat/EColumns.hxx"
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#include <stdio.h>      //sprintf
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _CONNECTIVITY_FLAT_EDRIVER_HXX_
#include "flat/EDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace connectivity;
using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ucb;
using namespace comphelper;
using namespace cppu;
using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//------------------------------------------------------------------
xub_StrLen OFlatString::GetTokenCount( sal_uInt8 cTok, sal_uInt8 cStrDel ) const
{
    if ( !Len() )
        return 0;

    xub_StrLen nTokCount = 1;
    BOOL bStart = TRUE;     // Stehen wir auf dem ersten Zeichen im Token?
    BOOL bInString = FALSE; // Befinden wir uns INNERHALB eines (cStrDel delimited) String?

    // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
    for( xub_StrLen i = 0; i < Len(); i++ )
    {
        if (bStart)
        {
            bStart = FALSE;
            // Erstes Zeichen ein String-Delimiter?
            if ((*this).GetChar(i) == cStrDel)
            {
                bInString = TRUE;   // dann sind wir jetzt INNERHALB des Strings!
                continue;           // dieses Zeichen ueberlesen!
            }
        }

        if (bInString) {
            // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
            if ( (*this).GetChar(i) == cStrDel )
            {
                if ((i+1 < Len()) && ((*this).GetChar(i+1) == cStrDel))
                {
                    // Verdoppeltes String-Delimiter-Zeichen:
                    i++;    // kein String-Ende, naechstes Zeichen ueberlesen.
                }
                else
                {
                    // String-Ende
                    bInString = FALSE;
                }
            }
        } else {
            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( (*this).GetChar(i) == cTok )
            {
                nTokCount++;
                bStart = TRUE;
            }
        }
    }

    return nTokCount;
}

//------------------------------------------------------------------
ByteString OFlatString::GetToken( xub_StrLen nToken, sal_uInt8 cTok, sal_uInt8 cStrDel ) const
{
    if ( !Len() )
        return ByteString();

    xub_StrLen nTok = 0;
    BOOL bStart = TRUE;     // Stehen wir auf dem ersten Zeichen im Token?
    BOOL bInString = FALSE; // Befinden wir uns INNERHALB eines (cStrDel delimited) String?
    ByteString aResult;         // Ergebnisstring

    // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
    for( xub_StrLen i = 0; i < Len(); i++ )
    {
        if (bStart) {
            bStart = FALSE;
            // Erstes Zeichen ein String-Delimiter?
            if ((*this).GetChar(i) == cStrDel) {
                bInString = TRUE;   // dann sind wir jetzt INNERHALB des Strings!
                continue;           // dieses Zeichen ueberlesen!
            }
        }

        if (bInString) {
            // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
            if ( (*this).GetChar(i) == cStrDel ) {
                if ((i+1 < Len()) && ((*this).GetChar(i+1) == cStrDel))
                {
                    // Verdoppeltes String-Delimiter-Zeichen:
                    i++;    // kein String-Ende, naechstes Zeichen ueberlesen.

                    if (nTok == nToken)
                    {
                        aResult += (*this).GetChar(i);  // Zeichen gehoert zum Resultat-String
                    }
                }
                else
                {
                    // String-Ende
                    bInString = FALSE;
                }
            } else {
                if (nTok == nToken) {
                    aResult += (*this).GetChar(i);  // Zeichen gehoert zum Resultat-String
                }
            }

        } else {
            // Stimmt das Tokenzeichen ueberein, dann erhoehe nTok
            if ( (*this).GetChar(i) == cTok ) {
                nTok++;
                bStart = TRUE;

                if ( nTok > nToken )
                {
                    // Vorzeitiger Abbruch der Schleife moeglich, denn
                    // wir haben, was wir wollten.
                    return aResult;
                }
            } else {
                if (nTok == nToken) {
                    aResult += (*this).GetChar(i);  // Zeichen gehoert zum Resultat-String
                }
            }
        }
    }

    return aResult;
}

// -------------------------------------------------------------------------
void OFlatTable::fillColumns()
{
    BOOL bRead = TRUE;

    OFlatString aHeaderLine;
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    if (pConnection->isHeaderLine())
    {
        while(bRead && !aHeaderLine.Len())
        {
            bRead = m_aFileStream.ReadLine(aHeaderLine);
        }
    }

    // read first row
    OFlatString aFirstLine;

    bRead = m_aFileStream.ReadLine(aFirstLine);

    if (!pConnection->isHeaderLine() || !aHeaderLine.Len())
    {
        while(bRead && !aFirstLine.Len())
        {
            bRead = m_aFileStream.ReadLine(aFirstLine);
        }
        // use first row as headerline because we need the number of columns
        aHeaderLine = aFirstLine;
    }
    // column count
    xub_StrLen nFieldCount = aHeaderLine.GetTokenCount(pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
    // read description
    char cDecimalDelimiter  = pConnection->getDecimalDelimiter();
    char cThousandDelimiter = pConnection->getThousandDelimiter();
    ByteString aColumnName;
    ::rtl::OUString aTypeName;
    for (xub_StrLen i = 0; i < nFieldCount; i++)
    {
        if (pConnection->isHeaderLine())
        {
            aColumnName = aHeaderLine.GetToken(i,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
            aColumnName.Convert(pConnection->getTextEncoding(),gsl_getSystemTextEncoding());
        }
        else
        {
            // no column name so ...
            aColumnName = 'C';
            aColumnName += ByteString::CreateFromInt32(i+1);
        }
        sal_Int32 eType;
        UINT16 nPrecision = 0;
        UINT16 nScale = 0;

        BOOL bNumeric = FALSE;
        double nRes=0.0;
        ULONG  nIndex = 0;

        // first without fielddelimiter
        ByteString aField(aFirstLine.GetToken(i,pConnection->getFieldDelimiter(),'\0'));
        if (aField.Len() == 0 ||
            (pConnection->getStringDelimiter() && pConnection->getStringDelimiter() == aField.GetChar(0)))
        {
            bNumeric = FALSE;
        }
        else
        {
            ByteString aField(aFirstLine.GetToken(i,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter()));

            if (aField.Len() == 0)
            {
                bNumeric = FALSE;
            }
            else
            {
                bNumeric = TRUE;
                xub_StrLen nDot = 0;
                for (xub_StrLen j = 0; j < aField.Len(); j++)
                {
                    char c = aField.GetChar(j);
                    // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
                    if ((!cDecimalDelimiter || c != cDecimalDelimiter) &&
                        (!cThousandDelimiter || c != cThousandDelimiter) &&
                        !isdigit(c))
                    {
                        bNumeric = FALSE;
                        break;
                    }
                    if (cDecimalDelimiter && c == cDecimalDelimiter)
                    {
                        nPrecision = 15; // we have an decimal value
                        nScale = 2;
                        nDot++;
                    }
                }

                if (nDot > 1) // if there is more than one dot it isn't a number
                    bNumeric = FALSE;
                if (bNumeric && cThousandDelimiter)
                {
                    // Ist der Trenner richtig angegeben?
                    ByteString aValue = aField.GetToken(0,cDecimalDelimiter);
                    for (xub_StrLen j = aValue.Len() - 4; j >= 0; j -= 4)
                    {
                        char c = aField.GetChar(j);
                        // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
                        if (c == cThousandDelimiter && j)
                            continue;
                        else
                        {
                            bNumeric = FALSE;
                            break;
                        }
                    }
                }

                // jetzt könnte es noch ein Datumsfeld sein
                if (!bNumeric)
                {
                    nIndex = m_xNumberFormatter->detectNumberFormat(::com::sun::star::util::NumberFormat::ALL,String(aField,pConnection->getTextEncoding()));
                }
            }
        }

        sal_Int32 nFlags = 0;
        if (bNumeric)
        {
            if (cDecimalDelimiter)
            {
                if(nPrecision)
                {
                    eType = DataType::DECIMAL;
                    aTypeName = ::rtl::OUString::createFromAscii("DECIMAL");
                }
                else
                {
                    eType = DataType::DOUBLE;
                    aTypeName = ::rtl::OUString::createFromAscii("DOUBLE");
                }
            }
            else
                eType = DataType::INTEGER;
            nFlags = ColumnSearch::BASIC;
        }
        else
        {

            switch (comphelper::getNumberFormatType(m_xNumberFormatter,nIndex))
            {
                case NUMBERFORMAT_DATE:
                    eType = DataType::DATE;
                    aTypeName = ::rtl::OUString::createFromAscii("DATE");
                    break;
                case NUMBERFORMAT_DATETIME:
                    eType = DataType::TIMESTAMP;
                    aTypeName = ::rtl::OUString::createFromAscii("TIMESTAMP");
                    break;
                case NUMBERFORMAT_TIME:
                    eType = DataType::TIME;
                    aTypeName = ::rtl::OUString::createFromAscii("TIME");
                    break;
                default:
                    eType = DataType::VARCHAR;
                    nPrecision = 0; // nyi: Daten koennen aber laenger sein!
                    nScale = 0;
                    aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
            };
            nFlags |= ColumnSearch::CHAR;
        }

        String aAlias(aColumnName,pConnection->getTextEncoding());
//      const SdbColumn* pPrevCol = aOriginalColumns->Column(aAlias);
//
//      ULONG nExprCnt(0);
//      while(pPrevCol)
//      {
//          (aAlias = String(aColumnName,pConnection->getTextEncoding())) += String::CreateFromInt32(++nExprCnt);
//          pPrevCol = aOriginalColumns->Column(aAlias);
//      }
//      SdbColumn* pColumn = new SdbFILEColumn(aAlias,eType,nPrecision,nScale,nFlags);
//
//      if (!bNumeric)
//          // CHARACTER-Felder koennen leer (NULL) sein.
//          pColumn->SetFlag(SDB_FLAGS_NULLALLOWED);
//
//      // Column-Beschreibung hinzufuegen.
//      aOriginalColumns->AddColumn(pColumn);
//  }
        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias,aTypeName,::rtl::OUString(),
                                                ColumnValue::NULLABLE,nPrecision,nScale,eType,sal_False,sal_False,sal_False,
                                                getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers());
        Reference< XFastPropertySet> xCol = pColumn;
        m_aColumns->push_back(xCol);
    }
    m_aFileStream.Seek(STREAM_SEEK_TO_BEGIN);
}
// -------------------------------------------------------------------------
OFlatTable::OFlatTable(OFlatConnection* _pConnection) : OFlatTable_BASE(_pConnection)
{

}
// -------------------------------------------------------------------------
OFlatTable::OFlatTable(OFlatConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OFlatTable_BASE(_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
    ,m_nRowPos(0)
    ,m_nMaxRowCount(0)
{

    Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
    LanguageType eLanguage = ConvertIsoStringToLanguage(comphelper::getString(aValue),'_');
    String sLanguage, sCountry;
    ConvertLanguageToIsoNames(eLanguage, sLanguage, sCountry);
    ::com::sun::star::lang::Locale aAppLocale(sLanguage,sCountry,rtl::OUString());
    Sequence< ::com::sun::star::uno::Any > aArg(1);
    aArg[0] <<= aAppLocale;

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(_pConnection->getDriver()->getFactory()->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier"),aArg),UNO_QUERY);
    m_xNumberFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(_pConnection->getDriver()->getFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")),UNO_QUERY);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);

    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetSmartURL(getEntry(), INetURLObject::ENCODE_ALL);

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    //  Content aContent(aURL.GetMainURL());

    m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
    if(!m_aFileStream.IsOpen())
        m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE );

    if(m_aFileStream.IsOpen())
    {
        m_aFileStream.Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = m_aFileStream.Tell();
        m_aFileStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize abhaengig von der Filegroesse
        m_aFileStream.SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);

        fillColumns();
        AllocBuffer();

        refreshColumns();
    }
}
// -------------------------------------------------------------------------
String OFlatTable::getEntry()
{
    ::rtl::OUString aURL;
    Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
    Reference< XRow> xRow(xDir,UNO_QUERY);
    ::rtl::OUString aName;
    sal_Int32 nLen = m_pConnection->getExtension().Len()+1;
    xDir->beforeFirst();
    while(xDir->next())
    {
        aName = xRow->getString(1);
        aName = aName.replaceAt(aName.getLength()-nLen,nLen,::rtl::OUString());
        if(aName == m_Name)
        {
            Reference< XContentAccess > xContentAccess( xDir, UNO_QUERY );
            aURL = xContentAccess->queryContentIdentfierString();
            break;
        }
    }
    xDir->beforeFirst(); // move back to before first record
    return aURL.getStr();
}
// -------------------------------------------------------------------------
void OFlatTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< ::rtl::OUString> aVector;

    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        delete m_pColumns;
    m_pColumns  = new OFlatColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OFlatTable::refreshIndexes()
{
}

// -------------------------------------------------------------------------
void SAL_CALL OFlatTable::disposing(void)
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns->clear();
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OFlatTable::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    Sequence< Type > aRet(aTypes.getLength()-3);
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    sal_Int32 i=0;
    for(;pBegin != pEnd;++pBegin,++i)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XKeysSupplier>*)0)   ||
            *pBegin == ::getCppuType((const Reference<XRename>*)0)          ||
            *pBegin == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XAlterTable>*)0)      ||
            *pBegin == ::getCppuType((const Reference<XDataDescriptorFactory>*)0)))
        {
            aRet.getArray()[i] = *pBegin;
        }
    }
    aRet.getArray()[i] = ::getCppuType( (const Reference< ::com::sun::star::lang::XUnoTunnel > *)0 );

    return aRet;
}

// -------------------------------------------------------------------------
Any SAL_CALL OFlatTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0)      ||
        rType == ::getCppuType((const Reference<XIndexesSupplier>*)0)   ||
        rType == ::getCppuType((const Reference<XRename>*)0)            ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0)        ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;

    return OTable_TYPEDEF::queryInterface(rType);
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OFlatTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
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
//------------------------------------------------------------------
sal_Int64 OFlatTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return OFlatTable_BASE::getSomething(rId);
}
// -------------------------------------------------------------------------
sal_Bool OFlatTable::checkHeaderLine()
{
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    if (m_nFilePos == 0 && pConnection->isHeaderLine())
    {
        BOOL bRead2;
        do
        {
            bRead2 = m_aFileStream.ReadLine(m_aCurrentLine);
        }
        while(bRead2 && !m_aCurrentLine.Len());

        m_nFilePos = m_aFileStream.Tell();
        if (m_aFileStream.IsEof())
            return sal_False;
    }
    return sal_True;
}
//------------------------------------------------------------------
sal_Bool OFlatTable::seekRow(FilePosition eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    // ----------------------------------------------------------
    // Positionierung vorbereiten:

    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case FILE_FIRST:
            m_nFilePos = 0;
            m_nRowPos = 1;
            // run through
        case FILE_NEXT:
            if(eCursorPosition != FILE_FIRST)
                ++m_nRowPos;
            m_aFileStream.Seek(m_nFilePos);
            if (m_aFileStream.IsEof() || !checkHeaderLine())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }

            m_aRowToFilePos[m_nRowPos] = m_nFilePos;

            m_aFileStream.ReadLine(m_aCurrentLine);
            if (m_aFileStream.IsEof())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }
            nCurPos = m_aFileStream.Tell();
            break;
        case FILE_PRIOR:
            --m_nRowPos;
            if(m_nRowPos > 0)
            {
                m_nFilePos = m_aRowToFilePos.find(m_nRowPos)->second;
                m_aFileStream.Seek(m_nFilePos);
                if (m_aFileStream.IsEof() || !checkHeaderLine())
                    return sal_False;
                m_aFileStream.ReadLine(m_aCurrentLine);
                if (m_aFileStream.IsEof())
                    return sal_False;
                nCurPos = m_aFileStream.Tell();
            }
            else
                m_nRowPos = 0;

            break;

            break;
        case FILE_LAST:
            if(m_nMaxRowCount)
            {
                m_nFilePos = m_aRowToFilePos.rbegin()->second;
                m_nRowPos  = m_aRowToFilePos.rbegin()->first;
                m_aFileStream.Seek(m_nFilePos);
                if (m_aFileStream.IsEof() || !checkHeaderLine())
                    return sal_False;
                m_aFileStream.ReadLine(m_aCurrentLine);
                if (m_aFileStream.IsEof())
                    return sal_False;
                nCurPos = m_aFileStream.Tell();
            }
            else
            {
                while(seekRow(FILE_NEXT,1,nCurPos)) ; // run through after last row
                // now I know all
                seekRow(FILE_PRIOR,1,nCurPos);
            }
            break;
        case FILE_RELATIVE:
            if(nOffset > 0)
            {
                for(sal_Int32 i = 0;i<nOffset;++i)
                    seekRow(FILE_NEXT,1,nCurPos);
            }
            else if(nOffset < 0)
            {
                for(sal_Int32 i = nOffset;i;++i)
                    seekRow(FILE_PRIOR,1,nCurPos);
            }
            break;
        case FILE_ABSOLUTE:
            {
                if(nOffset < 0)
                    nOffset = m_nRowPos + nOffset;
                ::std::map<sal_Int32,sal_Int32>::const_iterator aIter = m_aRowToFilePos.find(nOffset);
                if(aIter != m_aRowToFilePos.end())
                {
                    m_nFilePos = aIter->second;
                    m_aFileStream.Seek(m_nFilePos);
                    if (m_aFileStream.IsEof() || !checkHeaderLine())
                        return sal_False;
                    m_aFileStream.ReadLine(m_aCurrentLine);
                    if (m_aFileStream.IsEof())
                        return sal_False;
                    nCurPos = m_aFileStream.Tell();
                }
                else if(m_nMaxRowCount && nOffset > m_nMaxRowCount) // offset is outside the table
                {
                    m_nRowPos = m_nMaxRowCount;
                    return sal_False;
                }
                else
                {
                    aIter = m_aRowToFilePos.upper_bound(nOffset);
                    if(aIter == m_aRowToFilePos.end())
                    {
                        m_nRowPos   = m_aRowToFilePos.rbegin()->first;
                        nCurPos = m_nFilePos = m_aRowToFilePos.rbegin()->second;
                        while(m_nRowPos != nOffset)
                            seekRow(FILE_NEXT,1,nCurPos);
                    }
                    else
                    {
                        --aIter;
                        m_nRowPos   = aIter->first;
                        m_nFilePos  = aIter->second;
                        m_aFileStream.Seek(m_nFilePos);
                        if (m_aFileStream.IsEof() || !checkHeaderLine())
                            return sal_False;
                        m_aFileStream.ReadLine(m_aCurrentLine);
                        if (m_aFileStream.IsEof())
                            return sal_False;
                        nCurPos = m_aFileStream.Tell();
                    }
                }
            }

            break;
        case FILE_BOOKMARK:
            m_aFileStream.Seek(nOffset);
            if (m_aFileStream.IsEof())
                return sal_False;

            m_nFilePos = m_aFileStream.Tell();  // Byte-Position in der Datei merken (am ZeilenANFANG)
            m_aFileStream.ReadLine(m_aCurrentLine);
            if (m_aFileStream.IsEof())
                return sal_False;
            nCurPos  = m_aFileStream.Tell();
            break;
    }


    return sal_True;
}
//------------------------------------------------------------------
sal_Bool OFlatTable::fetchRow(file::OValueRow _rRow,const OSQLColumns & _rCols, sal_Bool bRetrieveData)
{
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    (*_rRow)[0] = m_nFilePos;

    if (!bRetrieveData)
        return TRUE;

    sal_Int32 nByteOffset = 1;
    // Felder:
    OSQLColumns::const_iterator aIter = _rCols.begin();
    for (sal_Int32 i = 1; aIter != _rCols.end();++aIter, i++)
    {
        Reference< XFastPropertySet> xColumn = *aIter;

        // Laengen je nach Datentyp:
        // nyi: eine zentrale Funktion, die die Laenge liefert!
        sal_Int32 nLen;
        xColumn->getFastPropertyValue(PROPERTY_ID_PRECISION) >>= nLen;
        sal_Int32 nType = getINT32(xColumn->getFastPropertyValue(PROPERTY_ID_TYPE));
        ByteString aStr(m_aCurrentLine.GetToken(i-1,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter()));

        if (aStr.Len() == 0)
            (*_rRow)[i].setNull();
        else switch(nType)
        {
            case DataType::TIMESTAMP:
            case DataType::DATE:
            case DataType::TIME:
            {
                double nRes = 0.0;
                ULONG  nIndex;
                try
                {
                    nRes = m_xNumberFormatter->convertStringToNumber(::com::sun::star::util::NumberFormat::ALL,String(aStr, pConnection->getTextEncoding()));
                    Reference<XPropertySet> xProp(m_xNumberFormatter->getNumberFormatsSupplier()->getNumberFormatSettings(),UNO_QUERY);
                    com::sun::star::util::Date aDate;
                    xProp->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")) >>= aDate;

                    switch(nType)
                    {
                        case DataType::DATE:
                            (*_rRow)[i] = DateConversion::toDouble(DateConversion::toDate(nRes,aDate));
                            break;
                        case DataType::TIMESTAMP:
                            (*_rRow)[i] = DateConversion::toDouble(DateConversion::toDateTime(nRes,aDate));
                            break;
                        default:
                            (*_rRow)[i] = DateConversion::toDouble(DateConversion::toTime(nRes));
                    }
                }
                catch(...)
                {
                    (*_rRow)[i].setNull();
                }
            }   break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::DOUBLE:
            case DataType::INTEGER:
            {
                char cDecimalDelimiter = pConnection->getDecimalDelimiter();
                char cThousandDelimiter = pConnection->getThousandDelimiter();
                ByteString aStrConverted;

                OSL_ENSHURE(cDecimalDelimiter && nType != DataType::INTEGER ||
                           !cDecimalDelimiter && nType == DataType::INTEGER,
                           "FalscherTyp");

                // In Standard-Notation (DezimalPUNKT ohne Tausender-Komma) umwandeln:
                for (xub_StrLen j = 0; j < aStr.Len(); j++)
                {
                    if (cDecimalDelimiter && aStr.GetChar(j) == cDecimalDelimiter)
                        aStrConverted += '.';
                    else if (cThousandDelimiter && aStr.GetChar(j) == cThousandDelimiter)
                    {
                        // weglassen
                    }
                    else
                        aStrConverted += aStr.GetChar(j) ;
                }
                (*_rRow)[i] = aStrConverted.ToDouble();
            } break;
            default:
            {
                // Wert als String in Variable der Row uebernehmen
                (*_rRow)[i] = String(aStr, pConnection->getTextEncoding());
            }
            break;
        }
    }
    return sal_True;
}
// -------------------------------------------------------------------------
void OFlatTable::FileClose()
{
    // falls noch nicht alles geschrieben wurde
    if (m_aFileStream.IsOpen() && m_aFileStream.IsWritable())
        m_aFileStream.Flush();

    m_aFileStream.Close();

    if (m_pBuffer != NULL)
    {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }
}
// -------------------------------------------------------------------------
BOOL OFlatTable::CreateImpl()
{
    return TRUE;
}

//------------------------------------------------------------------
BOOL OFlatTable::DropImpl()
{
    return TRUE;
}
//------------------------------------------------------------------
BOOL OFlatTable::InsertRow(OValueVector& rRow, BOOL bFlush,const Reference<XIndexAccess>& _xCols)
{
    return sal_True;;
}

//------------------------------------------------------------------
BOOL OFlatTable::UpdateRow(file::OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    return sal_True;
}

//------------------------------------------------------------------
BOOL OFlatTable::DeleteRow(const OSQLColumns& _rCols)
{
    return sal_True;;
}

//------------------------------------------------------------------
void OFlatTable::AllocBuffer()
{
}
//------------------------------------------------------------------
double toDouble(const ByteString& rString)
{
    static International aInter(LANGUAGE_ENGLISH);
    static int nErrno=0;
    BOOL bInitialized = sal_False;
    if (!bInitialized)
    {   // ensure that the two members we're interested in are really set
        // (if the system doesn't know the locale en_US aIntl would be initialized with the
        // system language which may be anything - which we don't want ...)
        // 74342 - 21.03.00 - FS
        aInter.SetNumThousandSep(',');
        aInter.SetNumDecimalSep('.');
        bInitialized = TRUE;
    }
    return SolarMath::StringToDouble(UniString(rString,gsl_getSystemTextEncoding()).GetBuffer(),aInter,nErrno);
}

//------------------------------------------------------------------
BOOL OFlatTable::UpdateBuffer(OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    return sal_True;
}


