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

#ifndef _CONNECTIVITY_EVOAB_LEVOABFOLDERLIST_HXX_
#define _CONNECTIVITY_EVOAB_LEVOABFOLDERLIST_HXX_

#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/CommonTools.hxx"
#include <tools/urlobj.hxx>
#include "LTable.hxx"
#include "TResultSetHelper.hxx"


namespace connectivity
{
    namespace evoab
    {
        //==================================================================
        // Derivation of String with overloaded GetToken/GetTokenCount methods
        // Especially for FLAT FILE-Format: Strings can be quoted
        //==================================================================

        class OEvoabConnection;

        class OEvoabFolderList
        {
            // maps a row postion to a file position
            ::std::vector<sal_Int32>        m_aTypes;       // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32>        m_aPrecisions;  // same as aboth
            ::std::vector<sal_Int32>        m_aScales;
            QuotedTokenizedString           m_aCurrentLine;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > m_xNumberFormatter;
            sal_Int32                       m_nFilePos;                 // current IResultSetHelper::Movement
            SvStream*                       m_pFileStream;
            OEvoabConnection*               m_pConnection;
            ::rtl::Reference<OSQLColumns>       m_aColumns;
            OValueRow                       m_aRow;
            sal_Bool                        m_bIsNull;

        private:
            void fillColumns(const ::com::sun::star::lang::Locale& _aLocale);
            BOOL CreateFile(const INetURLObject& aFile, BOOL& bCreateMemo);

            sal_Bool fetchRow(OValueRow _rRow,const OSQLColumns& _rCols);
            sal_Bool seekRow(IResultSetHelper::Movement eCursorPosition);

        public:

            OEvoabFolderList( OEvoabConnection* _pConnection);

            OEvoabConnection* getConnection() const { return m_pConnection;}
            ::rtl::Reference<OSQLColumns> getTableColumns() const {return m_aColumns;}
            void construct(); // can throw any exception
            static SvStream* createStream_simpleError( const String& _rFileName, StreamMode _eOpenMode);
            void initializeRow(sal_Int32 _nColumnCount);
            void checkIndex(sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException);
            const ORowSetValue& getValue(sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException);
            ::rtl::OUString SAL_CALL getString( sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 SAL_CALL getInt( sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        };
    }
}
#endif // _CONNECTIVITY_EVOAB_LEVOABFOLDERLIST_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
