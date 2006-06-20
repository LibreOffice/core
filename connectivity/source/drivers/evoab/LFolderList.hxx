/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LFolderList.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:23:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_LEVOABFOLDERLIST_HXX_
#define _CONNECTIVITY_EVOAB_LEVOABFOLDERLIST_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_EVOAB_LTABLE_HXX_
#include "LTable.hxx"
#endif
#ifndef CONNECTIVITY_TRESULTSETHELPER_HXX
#include "TResultSetHelper.hxx"
#endif


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
            BOOL CreateFile(const INetURLObject& aFile, BOOL& bCreateMemo);

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

