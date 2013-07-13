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

#ifndef _CONNECTIVITY_FLAT_TABLE_HXX_
#define _CONNECTIVITY_FLAT_TABLE_HXX_

#include "file/FTable.hxx"
#include "flat/EConnection.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/CommonTools.hxx"
#include <tools/urlobj.hxx>
#include "file/quotedstring.hxx"
#include <unotools/syslocale.hxx>

namespace connectivity
{
    namespace flat
    {
        typedef file::OFileTable OFlatTable_BASE;
        class OFlatConnection;

        typedef ::std::pair<sal_Int32, sal_Int32> TRowPositionInFile;

        class OFlatTable :  public OFlatTable_BASE
        {
            // maps a row position to a file position
            // row n is positions [m_aRowPosToFilePos[n]->first, m_aRowPosToFilePos[n]->second) in file
            // "real" row indexes start at 1; for the purposes of m_aRowPosToFilePos, row 0 is headers
            ::std::vector<TRowPositionInFile>
                                            m_aRowPosToFilePos;
            ::std::vector<sal_Int32>        m_aTypes;       // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32>        m_aPrecisions;  // same as aboth
            ::std::vector<sal_Int32>        m_aScales;
            QuotedTokenizedString           m_aCurrentLine;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > m_xNumberFormatter;
            ::com::sun::star::util::Date    m_aNullDate;
            sal_Int32                       m_nRowPos;
            sal_Int32                       m_nMaxRowCount; // will be set if stream is once eof
            sal_Unicode                     m_cStringDelimiter;     // delimiter for strings m_cStringDelimiter blabla m_cStringDelimiter
            sal_Unicode                     m_cFieldDelimiter;      // look at the name
            bool                            m_bNeedToReadLine;
        private:
            void fillColumns(const ::com::sun::star::lang::Locale& _aLocale);
            sal_Bool CreateFile(const INetURLObject& aFile, sal_Bool& bCreateMemo);
            bool readLine(sal_Int32 *pEndPos = NULL, sal_Int32 *pStartPos = NULL, bool nonEmpty = false);
            void setRowPos(::std::vector<TRowPositionInFile>::size_type rowNum, const TRowPositionInFile &rowPos);
            void impl_fillColumnInfo_nothrow(QuotedTokenizedString& aFirstLine, sal_Int32& nStartPosFirstLine, sal_Int32& nStartPosFirstLine2,
                                             sal_Int32& io_nType, sal_Int32& io_nPrecisions, sal_Int32& io_nScales, OUString& o_sTypeName,
                                             const sal_Unicode cDecimalDelimiter, const sal_Unicode cThousandDelimiter, const CharClass& aCharClass);
            OFlatConnection* getFlatConnection()
            {
#if OSL_DEBUG_LEVEL>1
                OFlatConnection* pConnection = dynamic_cast<OFlatConnection*>(m_pConnection);
                assert(pConnection);
#else
                OFlatConnection* pConnection = static_cast<OFlatConnection*>(m_pConnection);
#endif
                return pConnection;
            }
        public:
            virtual void refreshColumns();

        public:
            //  DECLARE_CTY_DEFAULTS( OFlatTable_BASE);
            OFlatTable( sdbcx::OCollection* _pTables,OFlatConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName = OUString(),
                    const OUString& _CatalogName = OUString()
                );

            void construct(); // can throw any exception

            virtual sal_Bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos);
            virtual sal_Bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, sal_Bool bIsTable, sal_Bool bRetrieveData);
            virtual void refreshHeader();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing(void);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            OUString getEntry();
        };
    }
}
#endif // _CONNECTIVITY_FLAT_TABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
