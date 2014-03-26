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
            friend SvStream& WriteODbaseIndex(SvStream &rStream, ODbaseIndex&);
            friend SvStream& operator >> (SvStream &rStream, ODbaseIndex&);

            friend class ONDXNode;
            friend class ONDXPage;
            friend class ONDXPagePtr;
            friend class OIndexIterator;

        public:

            // Header struct - stays in memory

            struct NDXHeader
            {
                sal_uInt32  db_rootpage;                    /* Rootpage position                */
                sal_uInt32  db_pagecount;                   /* Page count                       */
                sal_uInt8   db_frei[4];                     /* Reserved                         */
                sal_uInt16  db_keylen;                      /* Key length                       */
                sal_uInt16  db_maxkeys;                     /* Maximum number of keys per page  */
                sal_uInt16  db_keytype;                     /* Type of key:
                                                               0 = Text
                                                               1 = Numerical                    */
                sal_uInt16  db_keyrec;                      /* Length of an index record
                                                               RecordNumber + keylen            */
                sal_uInt8   db_frei1[3];                    /* Reserved                         */
                sal_uInt8   db_unique;                      /* Unique                           */
                char        db_name[488];                   /* index_name (field name)          */
            };

        private:
            SvStream*       m_pFileStream;                  // Stream to read/write the index
            NDXHeader       m_aHeader;
            ONDXPageList    m_aCollector;                   // Pool of obsolete pages
            ONDXPagePtr     m_aRoot,                        // Root of the B+ tree
                            m_aCurLeaf;                     // Current leaf
            sal_uInt16      m_nCurNode;                     // Position of the current node

            sal_uInt32      m_nPageCount,
                            m_nRootPage;

            ODbaseTable*    m_pTable;
            sal_Bool        m_bUseCollector : 1;            // Use the Garbage Collector

            OUString getCompletePath();
            void closeImpl();
            // Closes and kills the index file and throws an error
            void impl_killFileAndthrowError_throw(sal_uInt16 _nErrorId,const OUString& _sFile);
        protected:
            virtual ~ODbaseIndex();
        public:
            ODbaseIndex(ODbaseTable* _pTable);
            ODbaseIndex(ODbaseTable* _pTable,const NDXHeader& _aHeader,const OUString& _Name);

            sal_Bool openIndexFile();
            virtual void refreshColumns() SAL_OVERRIDE;

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            static com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
            virtual void SAL_CALL release() throw() SAL_OVERRIDE;

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
            // Tree operations
            void Insert(ONDXPagePtr aCurPage, ONDXNode& rNode);
            void Release(sal_Bool bSave = sal_True);
            sal_Bool ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue);
        };

        SvStream& WriteODbaseIndex(SvStream &rStream, ODbaseIndex&);
        SvStream& operator >> (SvStream &rStream, ODbaseIndex&);
    }
}

#endif // _CONNECTIVITY_DBASE_INDEX_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
