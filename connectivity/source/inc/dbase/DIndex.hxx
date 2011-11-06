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



#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#define _CONNECTIVITY_DBASE_INDEX_HXX_

#include "connectivity/sdbcx/VIndex.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "dbase/DTable.hxx"
#include "dbase/dindexnode.hxx"

#define dBASE_III_GROUP "dBase III"

namespace connectivity
{
    namespace dbase
    {
        class OIndexIterator;
        class ONDXKey;

        typedef sdbcx::OIndex ODbaseIndex_BASE;
        typedef file::OBoolOperator OBoolOperator_BASE;
        typedef file::OOperand OOperand_BASE;

        class ODbaseIndex : public ODbaseIndex_BASE
        {
            friend SvStream& operator << (SvStream &rStream, ODbaseIndex&);
            friend SvStream& operator >> (SvStream &rStream, ODbaseIndex&);

            friend class ONDXNode;
            friend class ONDXPage;
            friend class ONDXPagePtr;
            friend class OIndexIterator;

        public:
            //==================================================================
            // Kopfsatz-Struktur, verbleibt im Speicher
            //==================================================================
            struct NDXHeader
            {
                sal_uInt32  db_rootpage;                    /* Position der Rootpage        */
                sal_uInt32  db_pagecount;                   /* Anzahl Pages                 */
                sal_uInt8   db_frei[4];                     /* reserviert                   */
                sal_uInt16  db_keylen;                      /* Laenge des Schluessels       */
                sal_uInt16  db_maxkeys;                     /* Max. # keys pro Seite        */
                sal_uInt16  db_keytype;                     /* Art des Schluessels
                                                           (0-Text)
                                                           (1-Numerisch)                */
                sal_uInt16  db_keyrec;                      /* Laenge eines IndexSatzes
                                                           SatzNr + keylen              */
                sal_uInt8   db_frei1[3];                    /* reserviert                   */
                sal_uInt8   db_unique;                      /* eindeutig                    */
                char        db_name[488];                   /* index_name   (Feldname)      */
            };

        private:
            SvStream*       m_pFileStream;                  // Stream zum Lesen/Schreiben des Index
            NDXHeader       m_aHeader;
            ONDXPageList    m_aCollector;                   // Pool von nicht mehr benoetigten Seiten
            ONDXPagePtr     m_aRoot,                        // Wurzel des b+ Baums
                            m_aCurLeaf;                 // aktuelles Blatt
            sal_uInt16          m_nCurNode;                 // Position des aktuellen Knoten

            sal_uInt32          m_nPageCount,
                            m_nRootPage;

            ODbaseTable*    m_pTable;
            sal_Bool            m_bUseCollector : 1;                        // Verwenden des GarbageCollectors

            ::rtl::OUString getCompletePath();
            void closeImpl();
            /* closes and kill the index file and throws an error.
            */
            void impl_killFileAndthrowError_throw(sal_uInt16 _nErrorId,const ::rtl::OUString& _sFile);
        protected:
            virtual ~ODbaseIndex();
        public:
            ODbaseIndex(ODbaseTable* _pTable);
            ODbaseIndex(ODbaseTable* _pTable,const NDXHeader& _aHeader,const ::rtl::OUString& _Name);

            sal_Bool openIndexFile();
            virtual void refreshColumns();

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(com::sun::star::uno::RuntimeException);
            static com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            const ODbaseTable* getTable() const { return m_pTable; }
            const NDXHeader& getHeader() const { return m_aHeader; }
            virtual OIndexIterator* createIterator(OBoolOperator_BASE* pOp,
                                                         const OOperand_BASE* pOperand = NULL);

            void SetRootPos(sal_uInt32 nPos)        {m_nRootPage = nPos;}
            void SetPageCount(sal_uInt32 nCount)    {m_nPageCount = nCount;}

            sal_uInt32 GetRootPos()                 {return m_nRootPage;}
            sal_uInt32 GetPageCount()               {return m_nPageCount;}

            sal_Bool IsText() const                 {return m_aHeader.db_keytype == 0;}
            sal_uInt16 GetMaxNodes() const          {return m_aHeader.db_maxkeys;}

            virtual sal_Bool Insert(sal_uInt32 nRec, const ORowSetValue& rValue);
            virtual sal_Bool Update(sal_uInt32 nRec, const ORowSetValue&, const ORowSetValue&);
            virtual sal_Bool Delete(sal_uInt32 nRec, const ORowSetValue& rValue);
            virtual sal_Bool Find(sal_uInt32 nRec, const ORowSetValue& rValue);

            void createINFEntry();
            sal_Bool CreateImpl();
            sal_Bool DropImpl();

            DECLARE_SERVICE_INFO();
        protected:

            ONDXPage* CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent = NULL, sal_Bool bLoad = sal_False);
            void Collect(ONDXPage*);
            ONDXPagePtr getRoot();

            sal_Bool isUnique() const { return m_IsUnique; }
            sal_Bool UseCollector() const {return m_bUseCollector;}
            // Tree operationen
            void Insert(ONDXPagePtr aCurPage, ONDXNode& rNode);
            void Release(sal_Bool bSave = sal_True);
            sal_Bool ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue);
        };

        SvStream& operator << (SvStream &rStream, ODbaseIndex&);
        SvStream& operator >> (SvStream &rStream, ODbaseIndex&);
    }
}

#endif // _CONNECTIVITY_DBASE_INDEX_HXX_


