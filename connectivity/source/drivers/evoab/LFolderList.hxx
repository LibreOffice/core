/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        // Ableitung von String mit ueberladenen GetToken/GetTokenCount-Methoden
        // Speziell fuer FLAT FILE-Format: Strings koennen gequotet sein
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
            sal_Int32                       m_nFilePos;                 // aktuelle IResultSetHelper::Movement
            SvStream*                       m_pFileStream;
            OEvoabConnection*               m_pConnection;
            ::vos::ORef<OSQLColumns>        m_aColumns;
            OValueRow                       m_aRow;
            sal_Bool                        m_bIsNull;

        private:
            void fillColumns(const ::com::sun::star::lang::Locale& _aLocale);
            sal_Bool CreateFile(const INetURLObject& aFile, sal_Bool& bCreateMemo);

            sal_Bool fetchRow(OValueRow _rRow,const OSQLColumns& _rCols);
            sal_Bool seekRow(IResultSetHelper::Movement eCursorPosition);

        public:

            OEvoabFolderList( OEvoabConnection* _pConnection);

            OEvoabConnection* getConnection() const { return m_pConnection;}
            ::vos::ORef<OSQLColumns> getTableColumns() const {return m_aColumns;}
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

