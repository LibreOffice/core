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

#include <sdbcx/VIndex.hxx>
#include <dbase/DTable.hxx>
#include <dbase/dindexnode.hxx>

#define dBASE_III_GROUP "dBase III"

namespace connectivity::dbase
    {
        class OIndexIterator;
        class ONDXKey;

        typedef sdbcx::OIndex ODbaseIndex_BASE;

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
                sal_uInt8   db_free[4];                     /* Reserved                         */
                sal_uInt16  db_keylen;                      /* Key length                       */
                sal_uInt16  db_maxkeys;                     /* Maximum number of keys per page  */
                sal_uInt16  db_keytype;                     /* Type of key:
                                                               0 = Text
                                                               1 = Numerical                    */
                sal_uInt16  db_keyrec;                      /* Length of an index record
                                                               RecordNumber + keylen            */
                sal_uInt8   db_free1[3];                    /* Reserved                         */
                sal_uInt8   db_unique;                      /* Unique                           */
                char        db_name[488];                   /* index_name (field name)          */
            };

        private:
            std::unique_ptr<SvStream> m_pFileStream;        // Stream to read/write the index
            NDXHeader       m_aHeader = {};
            std::vector<ONDXPage*>
                            m_aCollector;                   // Pool of obsolete pages
            ONDXPagePtr     m_aRoot,                        // Root of the B+ tree
                            m_aCurLeaf;                     // Current leaf
            sal_uInt16      m_nCurNode;                     // Position of the current node

            sal_uInt32      m_nPageCount,
                            m_nRootPage;

            ODbaseTable*    m_pTable;
            bool        m_bUseCollector : 1;            // Use the Garbage Collector

            OUString getCompletePath() const;
            void closeImpl();
            // Closes and kills the index file and throws an error
            void impl_killFileAndthrowError_throw(const char* pErrorId, const OUString& _sFile);
        protected:
            virtual ~ODbaseIndex() override;
        public:
            ODbaseIndex(ODbaseTable* _pTable);
            ODbaseIndex(ODbaseTable* _pTable,const NDXHeader& _aHeader,const OUString& Name);

            void openIndexFile();
            virtual void refreshColumns() override;

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            const ODbaseTable* getTable() const { return m_pTable; }
            const NDXHeader& getHeader() const { return m_aHeader; }
            std::unique_ptr<OIndexIterator> createIterator();

            void SetRootPos(sal_uInt32 nPos)        {m_nRootPage = nPos;}
            void SetPageCount(sal_uInt32 nCount)    {m_nPageCount = nCount;}

            sal_uInt32 GetPageCount() const         {return m_nPageCount;}

            sal_uInt16 GetMaxNodes() const          {return m_aHeader.db_maxkeys;}

            bool Insert(sal_uInt32 nRec, const ORowSetValue& rValue);
            bool Update(sal_uInt32 nRec, const ORowSetValue&, const ORowSetValue&);
            bool Delete(sal_uInt32 nRec, const ORowSetValue& rValue);
            bool Find(sal_uInt32 nRec, const ORowSetValue& rValue);

            void createINFEntry();
            void CreateImpl();
            void DropImpl();

            DECLARE_SERVICE_INFO();
        protected:

            ONDXPage* CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent = nullptr, bool bLoad = false);
            void Collect(ONDXPage*);
            ONDXPagePtr const & getRoot();

            bool isUnique() const { return m_IsUnique; }
            bool UseCollector() const {return m_bUseCollector;}
            // Tree operations
            void Release(bool bSave = true);
            bool ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue);
        };

        SvStream& WriteODbaseIndex(SvStream &rStream, ODbaseIndex&);
        SvStream& operator >> (SvStream &rStream, ODbaseIndex&);

        void ReadHeader(SvStream & rStream, ODbaseIndex::NDXHeader & rHeader);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
