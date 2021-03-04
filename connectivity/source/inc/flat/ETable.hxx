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

#pragma once

#include <file/FTable.hxx>
#include <flat/EConnection.hxx>
#include <connectivity/CommonTools.hxx>
#include <file/quotedstring.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/util/XNumberFormatter.hpp>

namespace connectivity::flat
    {
        typedef file::OFileTable OFlatTable_BASE;
        class OFlatConnection;

        typedef std::pair<sal_Int32, sal_Int32> TRowPositionInFile;

        class OFlatTable :  public OFlatTable_BASE
        {
            // maps a row position to a file position
            // row n is positions [m_aRowPosToFilePos[n]->first, m_aRowPosToFilePos[n]->second) in file
            // "real" row indexes start at 1; for the purposes of m_aRowPosToFilePos, row 0 is headers
            std::vector<TRowPositionInFile>
                                            m_aRowPosToFilePos;
            std::vector<sal_Int32>        m_aTypes;       // holds all type for columns just to avoid to ask the propertyset
            std::vector<sal_Int32>        m_aPrecisions;  // same as aboth
            std::vector<sal_Int32>        m_aScales;
            QuotedTokenizedString           m_aCurrentLine;
            css::uno::Reference< css::util::XNumberFormatter > m_xNumberFormatter;
            css::util::Date                 m_aNullDate;
            sal_Int32                       m_nRowPos;
            sal_Int32                       m_nMaxRowCount; // will be set if stream is once eof
            sal_Unicode                     m_cStringDelimiter;     // delimiter for strings m_cStringDelimiter blabla m_cStringDelimiter
            sal_Unicode                     m_cFieldDelimiter;      // look at the name
            bool                            m_bNeedToReadLine;
        private:
            void fillColumns(const css::lang::Locale& _aLocale);
            bool readLine(sal_Int32 *pEndPos, sal_Int32 *pStartPos, bool nonEmpty = false);
            void setRowPos(std::vector<TRowPositionInFile>::size_type rowNum, const TRowPositionInFile &rowPos);
            void impl_fillColumnInfo_nothrow(QuotedTokenizedString const & aFirstLine, sal_Int32& nStartPosFirstLine, sal_Int32& nStartPosFirstLine2,
                                             sal_Int32& io_nType, sal_Int32& io_nPrecisions, sal_Int32& io_nScales, OUString& o_sTypeName,
                                             const sal_Unicode cDecimalDelimiter, const sal_Unicode cThousandDelimiter, const CharClass& aCharClass);
            OFlatConnection* getFlatConnection()
            {
#if OSL_DEBUG_LEVEL > 0
                OFlatConnection* pConnection = dynamic_cast<OFlatConnection*>(m_pConnection);
                assert(pConnection);
#else
                OFlatConnection* pConnection = static_cast<OFlatConnection*>(m_pConnection);
#endif
                return pConnection;
            }
        public:
            virtual void refreshColumns() override;

        public:
            //  DECLARE_CTY_DEFAULTS( OFlatTable_BASE);
            OFlatTable( sdbcx::OCollection* _pTables,OFlatConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description = OUString(),
                    const OUString& SchemaName = OUString(),
                    const OUString& CatalogName = OUString()
                );

            void construct() override; // can throw any exception

            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) override;
            virtual bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, bool bRetrieveData) override;
            virtual void refreshHeader() override;

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            virtual void SAL_CALL disposing() override;

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            OUString getEntry() const;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
