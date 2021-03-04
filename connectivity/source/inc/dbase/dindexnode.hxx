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

#include <file/fcode.hxx>
#include <connectivity/FValue.hxx>
#include <memory>

#define NODE_NOTFOUND 0xFFFF
#define DINDEX_PAGE_SIZE 512

class SvStream;

namespace connectivity::dbase
    {

        class ONDXNode;
        class ODbaseIndex;

        // Index Key

        typedef file::OOperand ONDXKey_BASE;
        class ONDXKey : public ONDXKey_BASE
        {
            friend class ONDXNode;
            sal_uInt32      nRecord;                /* Record pointer */
            ORowSetValue    xValue;                 /* Key values     */

        public:
            ONDXKey();
            ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec);
            ONDXKey(const OUString& aStr, sal_uInt32 nRec);
            ONDXKey(double aVal, sal_uInt32 nRec);

            inline ONDXKey(const ONDXKey& rKey);

            inline ONDXKey& operator= (const ONDXKey& rKey);
            virtual void setValue(const ORowSetValue& _rVal) override;

            virtual const ORowSetValue& getValue() const override;

            sal_uInt32 GetRecord() const        { return nRecord;   }
            void setRecord(sal_uInt32 _nRec)    { nRecord = _nRec;  }
            void   ResetRecord()            { nRecord = 0;      }

            bool operator == (const ONDXKey& rKey) const;
            bool operator != (const ONDXKey& rKey) const;
            bool operator <  (const ONDXKey& rKey) const;
            bool operator <= (const ONDXKey& rKey) const;
            bool operator >  (const ONDXKey& rKey) const;

            static bool IsText(sal_Int32 eType);

        private:
            int Compare(const ONDXKey& rKey) const;
        };


        class ONDXPage;

        // Index Page Pointer
        // This is  ref-count pointer class
        class ONDXPagePtr
        {
            friend  SvStream& WriteONDXPagePtr(SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            ONDXPage*   mpPage;
            sal_uInt32  nPagePos;       // Position in the index file

        public:
            ONDXPagePtr();
            ONDXPagePtr(ONDXPagePtr&& rObj) noexcept;
            ONDXPagePtr(ONDXPagePtr const & rRef);
            ONDXPagePtr(ONDXPage* pRefPage);
            ~ONDXPagePtr();
            void Clear();
            ONDXPagePtr& operator=(ONDXPagePtr const & rRef);
            ONDXPagePtr& operator=(ONDXPagePtr && rRef);
            bool Is() const { return mpPage != nullptr; }

            ONDXPage * operator ->() const { assert(mpPage != nullptr); return mpPage; }
            operator ONDXPage *() const { return mpPage; }

            sal_uInt32 GetPagePos() const {return nPagePos;}
            bool HasPage() const {return nPagePos != 0;}
        };

        // Index Page
        // This is a ref-counted class, with re-cycling
        class ONDXPage
        {
            friend class ODbaseIndex;
            friend class ONDXPagePtr;

            friend  SvStream& WriteONDXPage(SvStream &rStream, const ONDXPage&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPage&);

            // work around a clang 3.5 optimization bug: if the bNoDelete is *first*
            // it mis-compiles "if (--nRefCount == 0)" and never deletes any object
            unsigned int    nRefCount : 31;
            // the only reason this is not bool is because MSVC cannot handle mixed type bitfields
            unsigned int    bNoDelete : 1;
            sal_uInt32      nPagePos;       // Position in the index file
            bool            bModified : 1;
            sal_uInt16      nCount;

            ONDXPagePtr     aParent,            // Parent page
                            aChild;             // Pointer to the right child page
            ODbaseIndex&    rIndex;
            std::unique_ptr<ONDXNode[]>
                            ppNodes;             // Array of nodes

        public:
            // Node operations
            sal_uInt16  Count() const {return nCount;}

            bool    Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft = 0);
            bool    Insert(sal_uInt16 nIndex, ONDXNode& rNode);
            bool    Append(ONDXNode& rNode);
            void    Delete(sal_uInt16);
            void    Remove(sal_uInt16);
            void    Release(bool bSave = true);
            void    ReleaseFull();

            // Split and merge
            ONDXNode Split(ONDXPage& rPage);
            void Merge(sal_uInt16 nParentNodePos, const ONDXPagePtr& xPage);

            // Access operators
            ONDXNode& operator[] (sal_uInt16 nPos);
            const ONDXNode& operator[] (sal_uInt16 nPos) const;

            bool IsRoot() const;
            bool IsLeaf() const;
            bool IsModified() const;
            bool HasParent() const;

            bool IsFull() const;

            sal_uInt32 GetPagePos() const {return nPagePos;}
            ONDXPagePtr& GetChild(ODbaseIndex const * pIndex = nullptr);

            // Parent does not need to be reloaded
            const ONDXPagePtr& GetParent() const;
            ODbaseIndex& GetIndex() {return rIndex;}
            const ODbaseIndex& GetIndex() const {return rIndex;}

            // Setting the child, via reference to retain the PagePos
            void SetChild(ONDXPagePtr aCh);
            void SetParent(ONDXPagePtr aPa);

            sal_uInt16 Search(const ONDXKey& rSearch);
            sal_uInt16 Search(const ONDXPage* pPage);
            void   SearchAndReplace(const ONDXKey& rSearch, ONDXKey const & rReplace);

        protected:
            ONDXPage(ODbaseIndex& rIndex, sal_uInt32 nPos, ONDXPage*);
            ~ONDXPage();

            void ReleaseRef();
            void QueryDelete();
            void AddNextRef()
                    {
                        assert( nRefCount < (1 << 30) && "Do not add refs to dead objects" );
                        ++nRefCount;
                    }
            void AddFirstRef()
                    {
                        assert( nRefCount < (1 << 30) && "Do not add refs to dead objects" );
                        if( bNoDelete )
                            bNoDelete = 0;
                        ++nRefCount;
                    }

            void SetModified(bool bMod) {bModified = bMod;}
            void SetPagePos(sal_uInt32 nPage) {nPagePos = nPage;}

            bool Find(const ONDXKey&);  // Descend recursively
            sal_uInt16 FindPos(const ONDXKey& rKey) const;

#if OSL_DEBUG_LEVEL > 1
            void PrintPage();
#endif
        };

        SvStream& WriteONDXPagePtr(SvStream &rStream, const ONDXPagePtr&);
        SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

        inline bool ONDXPage::IsRoot() const {return !aParent.Is();}
        inline bool ONDXPage::IsLeaf() const {return !aChild.HasPage();}
        inline bool ONDXPage::IsModified() const {return bModified;}
        inline bool ONDXPage::HasParent() const {return aParent.Is();}
        inline const ONDXPagePtr& ONDXPage::GetParent() const {return aParent;}

        inline void ONDXPage::SetParent(ONDXPagePtr aPa = ONDXPagePtr())
        {
            aParent = aPa;
        }

        inline void ONDXPage::SetChild(ONDXPagePtr aCh = ONDXPagePtr())
        {
            aChild = aCh;
            if (aChild.Is())
                aChild->SetParent(this);
        }
        SvStream& operator >> (SvStream &rStream, ONDXPage& rPage);
        SvStream& WriteONDXPage(SvStream &rStream, const ONDXPage& rPage);


        // Index Node

        class ONDXNode
        {
            friend class ONDXPage;
            ONDXPagePtr aChild;             /* Next page reference */
            ONDXKey   aKey;

        public:
            ONDXNode(){}
            ONDXNode(const ONDXKey& rKey)
                       :aKey(rKey) {}

            // Does the node point to a page?
            bool            HasChild() const {return aChild.HasPage();}
            // If an index is provided, we may be able to retrieve the page
            ONDXPagePtr&    GetChild(ODbaseIndex* pIndex = nullptr, ONDXPage* = nullptr);

            const ONDXKey& GetKey() const   { return aKey;}
            ONDXKey&       GetKey()         { return aKey;}

            // Setting the child, via reference to retain the PagePos
            void            SetChild(ONDXPagePtr aCh = ONDXPagePtr(), ONDXPage* = nullptr);

            void Write(SvStream &rStream, const ONDXPage& rPage) const;
            void Read(SvStream &rStream, ODbaseIndex const &);
        };

        inline ONDXKey::ONDXKey(const ONDXKey& rKey)
                     : ONDXKey_BASE(rKey.getDBType())
                     ,nRecord(rKey.nRecord)
                     ,xValue(rKey.xValue)
        {
        }

        inline ONDXKey& ONDXKey::operator=(const ONDXKey& rKey)
        {
            if(&rKey == this)
                return *this;

            xValue = rKey.xValue;
            nRecord = rKey.nRecord;
            m_eDBType = rKey.getDBType();
            return *this;
        }

        inline bool ONDXKey::operator == (const ONDXKey& rKey) const
        {
            if(&rKey == this)
                return true;
            return Compare(rKey) == 0;
        }
        inline bool ONDXKey::operator != (const ONDXKey& rKey) const
        {
            return !operator== (rKey);
        }
        inline bool ONDXKey::operator <  (const ONDXKey& rKey) const
        {
            return Compare(rKey) < 0;
        }
        inline bool ONDXKey::operator >  (const ONDXKey& rKey) const
        {
            return Compare(rKey) > 0;
        }
        inline bool ONDXKey::operator <= (const ONDXKey& rKey) const
        {
            return !operator > (rKey);
        }

        inline void ONDXNode::SetChild(ONDXPagePtr aCh, ONDXPage* pParent)
        {
            aChild = aCh;
            if (aChild.Is())
                aChild->SetParent(pParent);
        }

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
