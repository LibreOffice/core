/*************************************************************************
 *
 *  $RCSfile: FResultSet.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-30 07:28:24 $
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

#ifndef _CONNECTIVITY_FILE_FRESULTSET_HXX_
#define _CONNECTIVITY_FILE_FRESULTSET_HXX_

#ifndef _COM_SUN_STAR_SQLC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE11_HXX_
#include <cppuhelper/compbase11.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_OSTATEMENT_HXX_
#include "file/FStatement.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#include "file/fanalyzer.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif

namespace connectivity
{
    namespace file
    {
        class OKeySet : public ::std::vector<sal_Int32>
        {
            sal_Bool m_bFrozen;
        public:
            OKeySet(): ::std::vector<sal_Int32>(),m_bFrozen(sal_False){}
            OKeySet(size_type _nSize) : ::std::vector<sal_Int32>(_nSize),m_bFrozen(sal_False){}

            sal_Bool    isFrozen() const            { return m_bFrozen; }
            void        setFrozen(sal_Bool _bFrozen=sal_True) { m_bFrozen = _bFrozen; }
        };

        typedef union
        {
            double  aDouble;
            rtl::OUString* pString;
        } OFILEKey;

#define SQL_ORDERBYKEYS 10
#define SQL_COLUMN_NOTFOUND STRING_NOTFOUND

        class OFILEKeyValue
        {
        private:
            INT32 nValue;
            OFILEKey pKey[SQL_ORDERBYKEYS];

        public:
            OFILEKeyValue() { }
            OFILEKeyValue(INT32 nVal) : nValue(nVal) {}
            ~OFILEKeyValue(){}

            void SetKey(UINT16 i, double d) { pKey[i].aDouble = d; }
            void SetKey(UINT16 i, const rtl::OUString& rString) { pKey[i].pString = new rtl::OUString(rString); }
            void SetValue(INT32 nVal) { nValue = nVal; }

            rtl::OUString* GetKeyString(UINT16 i) const { return pKey[i].pString; }
            double GetKeyDouble(UINT16 i) const { return pKey[i].aDouble; }

            inline INT32 GetValue() const {return nValue;}
        };

        typedef OFILEKeyValue * OFILEKeyValuePtr;

        typedef enum
        {
            SQL_ORDERBYKEY_NONE,        // Nicht sortieren
            SQL_ORDERBYKEY_DOUBLE,      // Numerischer Key
            SQL_ORDERBYKEY_STRING       // String Key
        } OKeyType;

        class OFILESortIndex
        {
        private:
            INT32 nMaxCount;            // Maximal moegliche Anzahl Key/Value-Paare im Index (und damit Array-Groesse)
            INT32 nCount;               // Anzahl Key/Value-Paare im Index (und damit naechste freie Position)

            OFILEKeyValuePtr * ppKeyValueArray;
                                        // Zeiger auf Array der Groesse [nMaxCount]

            BOOL bFrozen;
            CharSet eCharSet;

        public: // nur fuer OFILECompare:
            static OFILESortIndex *pCurrentIndex;                   // Waehrend der Ausfuehrung von qsort ist hier der Zeiger
            static CharSet eCurrentCharSet;
                                                                    // auf den gerade zur Sortierung verwendeten Index hinterlegt
                                                                    // (wird von der Vergleichsfunktion OFILEKeyCompare verwendet).
            OKeyType eKeyType[SQL_ORDERBYKEYS];
            BOOL bAscending[SQL_ORDERBYKEYS];


        public:

            OFILESortIndex(const OKeyType eKeyType[],           // Art des Schluessels: numerisch/String/nicht sortieren (Genau 3 Eintraege!)
                             const BOOL bAscending[],               // TRUE = Aufsteigend sortieren (Genau 3 Eintraege!)
                             INT32 nMaxNumberOfRows,
                             CharSet eSet);

            ~OFILESortIndex();


            BOOL AddKeyValue(OFILEKeyValue * pKeyValue);
                                                                // TRUE, wenn erfolgreich hinzugefuegt, FALSE bei Ueberschreitung
                                                                // der Index-Kapazitaet.
                                                                // pKeyValue wird beim Zerstoeren des Index automatisch freigegeben.

            void Freeze();                                      // "Einfrieren" des Index:
                                                                // Vor "Freeze" duerfen Count() und Get() nicht gerufen werden,
                                                                // nach "Freeze" darf dafuer Add() nicht mehr gerufen werden.

            OKeySet* CreateKeySet();



            BOOL IsFrozen() { return bFrozen; }         // TRUE nach Aufruf von Freeze()

            INT32 Count() const { return nCount; }      // Anzahl Key/Value-Paare im Index
            INT32 GetValue(INT32 nPos) const;           // Value an Position nPos (1..n) [sortierter Zugriff].
        };

        static int
#if defined(WIN) || defined(WNT)
__cdecl
#endif
#if defined(ICC) && defined(OS2)
_Optlink
#endif
OFILEKeyCompare(const void * elem1, const void * elem2);

        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper11<  ::com::sun::star::sdbc::XResultSet,
                                                    ::com::sun::star::sdbc::XRow,
                                                    ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::sdbc::XResultSetUpdate,
                                                    ::com::sun::star::sdbc::XRowUpdate,
                                                    ::com::sun::star::sdbc::XCloseable,
                                                    ::com::sun::star::sdbc::XColumnLocate,
                                                    ::com::sun::star::lang::XServiceInfo,
                                                    ::com::sun::star::lang::XUnoTunnel> OResultSet_BASE;

        class OResultSet :  public  comphelper::OBaseMutex,
                            public  OResultSet_BASE,
                            public  ::comphelper::OPropertyContainer,
                            public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {

        protected:
            ::std::vector<void*>                                m_aBindVector;
            ::std::vector<sal_Int32>                            m_aColMapping; // pos 0 is unused so we don't have to decrement 1 everytime

            OValueRow                                           m_aRow;
            OValueRow                                           m_aEvaluateRow; // contains all values of a row
            OValueRow                                           m_aParameterRow;
            OValueRow                                           m_aInsertRow;   // needed for insert by cursor
            ORefAssignValues                                    m_aAssignValues; // needed for insert,update and parameters
                                                                    // to compare with the restrictions
            ::std::vector<sal_Int32>*                           m_pEvaluationKeySet;
            ::std::vector<sal_Int32>::iterator                  m_aEvaluateIter;

            ::std::map<sal_Int32,sal_Int32>                     m_aBookmarkToPos;

            OKeySet*                                            m_pFileSet;
            OKeySet::iterator                                   m_aFileSetIter;

            UINT16                                              nOrderbyColumnNumber[SQL_ORDERBYKEYS];
            BOOL                                                bOrderbyAscending[SQL_ORDERBYKEYS];

            OFILESortIndex*                                     m_pSortIndex;
            ::vos::ORef<connectivity::OSQLColumns>              m_xColumns; // this are the select columns
            ::vos::ORef<connectivity::OSQLColumns>              m_xParamColumns;
            OFileTable*                                         m_pTable;
            connectivity::OSQLParseNode*                        m_pParseTree;

            OFILEAnalyzer                                       m_aSQLAnalyzer;
            connectivity::OSQLParseTreeIterator&                m_aSQLIterator;

            sal_Int32                                           m_nFetchSize;
            sal_Int32                                           m_nResultSetType;
            sal_Int32                                           m_nFetchDirection;
            sal_Int32                                           m_nResultSetConcurrency;

            ::com::sun::star::uno::WeakReferenceHelper                                      m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>   m_xMetaData;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>    m_xDBMetaData;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xColNames; // table columns

            ::rtl::OUString                                     m_aTableRange;
            sal_Int32                                           m_nRowPos;
            sal_Int32                                           m_nFilePos;
            sal_Int32                                           m_nLastVisitedPos;
            sal_Int32                                           m_nRowCountResult;
            sal_Bool                                            m_bWasNull;
            sal_Bool                                            m_bBOF;                 // before first record
            sal_Bool                                            m_bEOF;                 // after last record
            sal_Bool                                            m_bLastRecord;
            sal_Bool                                            m_bFileSetFrozen;
            sal_Bool                                            m_bInserted;            // true when moveToInsertRow was called
                                                                                        // set to false when cursor moved or cancel
            sal_Bool                                            m_bRowUpdated;
            sal_Bool                                            m_bRowInserted;
            sal_Bool                                            m_bRowDeleted;

            void construct();
            sal_Bool evaluate();
            BOOL ExecuteRow(OFileTable::FilePosition eFirstCursorPosition,
                                INT32 nOffset = 1,
                                BOOL bRebind = TRUE,
                                BOOL bEvaluate = TRUE,
                                BOOL bRetrieveData = TRUE);

            OFILEKeyValue* GetOrderbyKeyValue(OValueRow _rRow);
            BOOL IsSorted() const {return nOrderbyColumnNumber[0] != SQL_COLUMN_NOTFOUND;}
            void anylizeSQL();
            void setOrderbyColumn(UINT16 nOrderbyColumnNo,
                                     connectivity::OSQLParseNode* pColumnRef,
                                     connectivity::OSQLParseNode* pAscendingDescending);
            void SetAssignValue(const String& aColumnName,
                                   const String& aValue,
                                   BOOL bSetNull = FALSE,
                                   UINT32 nParameter=SQL_NO_PARAMETER);
            void ParseAssignValues( const ::std::vector< String>& aColumnNameList,
                                    connectivity::OSQLParseNode* pRow_Value_Constructor_Elem,xub_StrLen nIndex);
                        UINT32 AddParameter(connectivity::OSQLParseNode * pParameter, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol);
            void GetAssignValues();

            void scanParameter(OSQLParseNode* pParseNode,::std::vector< OSQLParseNode*>& _rParaNodes);
            sal_Bool moveAbsolute(sal_Int32 _nOffset,sal_Bool _bRetrieveData);
        protected:

            using OResultSet_BASE::rBHelper;

            BOOL Move(OFileTable::FilePosition eCursorPosition, INT32 nOffset, BOOL bRetrieveData);
            BOOL SkipDeleted(OFileTable::FilePosition eCursorPosition, INT32 nOffset, BOOL bRetrieveData);

            virtual sal_Bool fillIndexValues(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> &_xIndex)
            { return sal_False; }

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        public:
            DECLARE_CTY_DEFAULTS(OResultSet_BASE);
            DECLARE_SERVICE_INFO();
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator);
            ~OResultSet();


            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(OResultSet_BASE*)this);
            }

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
            }
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetUpdate
            virtual void SAL_CALL insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRowUpdate
            virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // special methods
            inline sal_Int32 mapColumn(sal_Int32    column);
            BOOL OpenImpl();

            sal_Int32 getRowCountResult() const { return m_nRowCountResult; }
            void setParameterRow(const OValueRow& _rParaRow) { m_aParameterRow = _rParaRow; }
            void describeParameter();
        };
        // -------------------------------------------------------------------------
        inline sal_Int32 OResultSet::mapColumn (sal_Int32   column)
        {
            return column;
//          sal_Int32   map = column;
//
//          if (m_aColMapping.size())
//          {
//              // Validate column number
//              OSL_ENSHURE(column>0,"OResultSet::mapColumn column <= 0");
//              map = m_aColMapping[column];
//          }
//
//          return map;
        }
    }
}
#endif // _CONNECTIVITY_FILE_ORESULTSET_HXX_


