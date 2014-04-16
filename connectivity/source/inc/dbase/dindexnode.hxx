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
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#define _CONNECTIVITY_DBASE_INDEXNODE_HXX_

#include "file/fcode.hxx"
#include "file/FTable.hxx"
#include "connectivity/FValue.hxx"
#include <rtl/ref.hxx>
#include <tools/ref.hxx>
#include <tools/stream.hxx>
#include <vector>

#define NODE_NOTFOUND 0xFFFF
#define DINDEX_PAGE_SIZE 512

namespace connectivity
{
    namespace dbase
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
            ONDXKey(sal_uInt32 nRec=0);
            ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec);
            ONDXKey(const OUString& aStr, sal_uInt32 nRec = 0);
            ONDXKey(double aVal, sal_uInt32 nRec = 0);

            inline ONDXKey(const ONDXKey& rKey);

            inline ONDXKey& operator= (const ONDXKey& rKey);
            virtual void setValue(const ORowSetValue& _rVal) SAL_OVERRIDE;

            virtual const ORowSetValue& getValue() const SAL_OVERRIDE;

            sal_uInt32 GetRecord() const        { return nRecord;   }
            void setRecord(sal_uInt32 _nRec)    { nRecord = _nRec;  }
            void   ResetRecord()            { nRecord = 0;      }

            bool operator == (const ONDXKey& rKey) const;
            bool operator != (const ONDXKey& rKey) const;
            bool operator <  (const ONDXKey& rKey) const;
            bool operator <= (const ONDXKey& rKey) const;
            bool operator >  (const ONDXKey& rKey) const;
            bool operator >= (const ONDXKey& rKey) const;

            bool Load (SvFileStream& rStream, bool bText);
            bool Write(SvFileStream& rStream, bool bText);

            static bool IsText(sal_Int32 eType);

        private:
            int Compare(const ONDXKey& rKey) const;
        };





        // Index Page Pointer

        class ONDXPage;
        typedef tools::SvRef<ONDXPage> ONDXPageRef; // Base class - because we need to store additional information


        class ONDXPagePtr : public ONDXPageRef
        {
            friend  SvStream& WriteONDXPagePtr(SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            sal_uInt32  nPagePos;       // Position in the index file

        public:
            ONDXPagePtr(sal_uInt32 nPos = 0):nPagePos(nPos){}
            ONDXPagePtr(const ONDXPagePtr& rRef);
            ONDXPagePtr(ONDXPage* pRefPage);

            ONDXPagePtr& operator=(const ONDXPagePtr& rRef);
            ONDXPagePtr& operator=(ONDXPage* pPageRef);

            sal_uInt32 GetPagePos() const {return nPagePos;}
            bool HasPage() const {return nPagePos != 0;}
            //  sal_Bool Is() const { return isValid(); }
        };

        // Index Page

        class ONDXPage : public SvRefBase
        {
            friend class ODbaseIndex;

            friend  SvStream& WriteONDXPage(SvStream &rStream, const ONDXPage&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPage&);

            sal_uInt32      nPagePos;       // Position in the index file
            bool        bModified : 1;
            sal_uInt16      nCount;

            ONDXPagePtr aParent,            // Parent page
                        aChild;             // Pointer to the right child page
            ODbaseIndex& rIndex;
            ONDXNode*  ppNodes;             // Array of nodes

        public:
            // Node operations
            sal_uInt16  Count() const {return nCount;}

            bool    Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft = 0);
            bool    Insert(sal_uInt16 nIndex, ONDXNode& rNode);
            bool    Append(ONDXNode& rNode);
            bool    Delete(sal_uInt16);
            void    Remove(sal_uInt16);
            void    Release(bool bSave = true);
            void    ReleaseFull(bool bSave = true);

            // Split and merge
            ONDXNode Split(ONDXPage& rPage);
            void Merge(sal_uInt16 nParentNodePos, ONDXPagePtr xPage);

            // Access operators
            ONDXNode& operator[] (sal_uInt16 nPos);
            const ONDXNode& operator[] (sal_uInt16 nPos) const;

            bool IsRoot() const;
            bool IsLeaf() const;
            bool IsModified() const;
            bool HasParent();
            bool HasChild() const;

            bool IsFull() const;

            sal_uInt32 GetPagePos() const {return nPagePos;}
            ONDXPagePtr& GetChild(ODbaseIndex* pIndex = 0);

            // Parent does not need to be reloaded
            ONDXPagePtr GetParent();
            ODbaseIndex& GetIndex() {return rIndex;}
            const ODbaseIndex& GetIndex() const {return rIndex;}

            // Setting the child, via reference to retain the PagePos
            void SetChild(ONDXPagePtr aCh);
            void SetParent(ONDXPagePtr aPa);

            sal_uInt16 Search(const ONDXKey& rSearch);
            sal_uInt16 Search(const ONDXPage* pPage);
            void   SearchAndReplace(const ONDXKey& rSearch, ONDXKey& rReplace);

        protected:
            ONDXPage(ODbaseIndex& rIndex, sal_uInt32 nPos, ONDXPage* = NULL);
            virtual ~ONDXPage();

            virtual void QueryDelete() SAL_OVERRIDE;

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
        inline bool ONDXPage::HasParent() {return aParent.Is();}
        inline bool ONDXPage::HasChild() const {return aChild.HasPage();}
        inline ONDXPagePtr ONDXPage::GetParent() {return aParent;}

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


        typedef ::std::vector<ONDXPage*>    ONDXPageList;


        // Index Node

        class ONDXNode
        {
            friend class ONDXPage;
            ONDXPagePtr aChild;             /* Next page reference */
            ONDXKey   aKey;

        public:
            ONDXNode(){}
            ONDXNode(const ONDXKey& rKey,
                       ONDXPagePtr aPagePtr = ONDXPagePtr())
                       :aChild(aPagePtr),aKey(rKey) {}

            // Does the node point to a page?
            bool            HasChild() const {return aChild.HasPage();}
            // If an index is provided, we may be able to retrieve the page
            ONDXPagePtr&    GetChild(ODbaseIndex* pIndex = NULL, ONDXPage* = NULL);

            const ONDXKey& GetKey() const   { return aKey;}
            ONDXKey&       GetKey()         { return aKey;}

            // Setting the child, via reference to retain the PagePos
            void            SetChild(ONDXPagePtr aCh = ONDXPagePtr(), ONDXPage* = NULL);
            void            SetKey(ONDXKey& rKey) {aKey = rKey;}

            void Write(SvStream &rStream, const ONDXPage& rPage) const;
            void Read(SvStream &rStream, ODbaseIndex&);
        };

        // inline implementation

//      inline ONDXKey::ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec)
//          : ONDXKey_BASE(eType)
//          , nRecord(nRec),xValue(rVal)
//      {
//      }


//      inline ONDXKey::ONDXKey(const OUString& aStr, sal_uInt32 nRec)
//                  : ONDXKey_BASE(::com::sun::star::sdbc::DataType::VARCHAR)
//                   ,nRecord(nRec)
//      {
//          if (aStr.len())
//              xValue = aStr;
//      }

//      inline ONDXKey::ONDXKey(double aVal, sal_uInt32 nRec)
//                   : ONDXKey_BASE(::com::sun::star::sdbc::DataType::DOUBLE)
//                   ,nRecord(nRec)
//                   ,xValue(aVal)
//      {
//      }

//      inline ONDXKey::ONDXKey(sal_uInt32 nRec)
//                   :nRecord(nRec)
//      {
//      }

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
        inline bool ONDXKey::operator >= (const ONDXKey& rKey) const
        {
            return !operator< (rKey);
        }

        inline void ONDXNode::SetChild(ONDXPagePtr aCh, ONDXPage* pParent)
        {
            aChild = aCh;
            if (aChild.Is())
                aChild->SetParent(pParent);
        }

    }

}




#endif // _CONNECTIVITY_DBASE_INDEXNODE_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
