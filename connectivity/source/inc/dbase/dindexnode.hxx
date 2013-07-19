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
        //==================================================================
        // Index Key
        //==================================================================
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
            virtual void setValue(const ORowSetValue& _rVal);

            virtual const ORowSetValue& getValue() const;

            sal_uInt32 GetRecord() const        { return nRecord;   }
            void setRecord(sal_uInt32 _nRec)    { nRecord = _nRec;  }
            void   ResetRecord()            { nRecord = 0;      }

            sal_Bool operator == (const ONDXKey& rKey) const;
            sal_Bool operator != (const ONDXKey& rKey) const;
            sal_Bool operator <  (const ONDXKey& rKey) const;
            sal_Bool operator <= (const ONDXKey& rKey) const;
            sal_Bool operator >  (const ONDXKey& rKey) const;
            sal_Bool operator >= (const ONDXKey& rKey) const;

            sal_Bool Load (SvFileStream& rStream, sal_Bool bText);
            sal_Bool Write(SvFileStream& rStream, sal_Bool bText);

            static sal_Bool IsText(sal_Int32 eType);

        private:
            int Compare(const ONDXKey& rKey) const;
        };




        //==================================================================
        // Index Page Pointer
        //==================================================================
        SV_DECL_REF(ONDXPage) // Base class - because we need to store additional information


        class ONDXPagePtr : public ONDXPageRef
        {
            friend  SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            sal_uInt32  nPagePos;       // Position in the index file

        public:
            ONDXPagePtr(sal_uInt32 nPos = 0):nPagePos(nPos){}
            ONDXPagePtr(const ONDXPagePtr& rRef);
            ONDXPagePtr(ONDXPage* pRefPage);

            ONDXPagePtr& operator=(const ONDXPagePtr& rRef);
            ONDXPagePtr& operator=(ONDXPage* pPageRef);

            sal_uInt32 GetPagePos() const {return nPagePos;}
            sal_Bool HasPage() const {return nPagePos != 0;}
            //  sal_Bool Is() const { return isValid(); }
        };
        //==================================================================
        // Index Page
        //==================================================================
        class ONDXPage : public SvRefBase
        {
            friend class ODbaseIndex;

            friend  SvStream& operator << (SvStream &rStream, const ONDXPage&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPage&);

            sal_uInt32      nPagePos;       // Position in the index file
            sal_Bool        bModified : 1;
            sal_uInt16      nCount;

            ONDXPagePtr aParent,            // Parent page
                        aChild;             // Pointer to the right child page
            ODbaseIndex& rIndex;
            ONDXNode*  ppNodes;             // Array of nodes

        public:
            // Node operations
            sal_uInt16  Count() const {return nCount;}

            sal_Bool    Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft = 0);
            sal_Bool    Insert(sal_uInt16 nIndex, ONDXNode& rNode);
            sal_Bool    Append(ONDXNode& rNode);
            sal_Bool    Delete(sal_uInt16);
            void    Remove(sal_uInt16);
            void    Release(sal_Bool bSave = sal_True);
            void    ReleaseFull(sal_Bool bSave = sal_True);

            // Split and merge
            ONDXNode Split(ONDXPage& rPage);
            void Merge(sal_uInt16 nParentNodePos, ONDXPagePtr xPage);

            // Access operators
            ONDXNode& operator[] (sal_uInt16 nPos);
            const ONDXNode& operator[] (sal_uInt16 nPos) const;

            sal_Bool IsRoot() const;
            sal_Bool IsLeaf() const;
            sal_Bool IsModified() const;
            sal_Bool HasParent();
            sal_Bool HasChild() const;

            sal_Bool IsFull() const;

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
            ~ONDXPage();

            virtual void QueryDelete();

            void SetModified(sal_Bool bMod) {bModified = bMod;}
            void SetPagePos(sal_uInt32 nPage) {nPagePos = nPage;}

            sal_Bool Find(const ONDXKey&);  // Descend recursively
            sal_uInt16 FindPos(const ONDXKey& rKey) const;

#if OSL_DEBUG_LEVEL > 1
            void PrintPage();
#endif
        };

        SV_IMPL_REF(ONDXPage);

        SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
        SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

        inline sal_Bool ONDXPage::IsRoot() const {return !aParent.Is();}
        inline sal_Bool ONDXPage::IsLeaf() const {return !aChild.HasPage();}
        inline sal_Bool ONDXPage::IsModified() const {return bModified;}
        inline sal_Bool ONDXPage::HasParent() {return aParent.Is();}
        inline sal_Bool ONDXPage::HasChild() const {return aChild.HasPage();}
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
        SvStream& operator << (SvStream &rStream, const ONDXPage& rPage);


        typedef ::std::vector<ONDXPage*>    ONDXPageList;

        //==================================================================
        // Index Node
        //==================================================================
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
            sal_Bool            HasChild() const {return aChild.HasPage();}
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
        //==================================================================
        // inline implementation
        //==================================================================
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

        inline sal_Bool ONDXKey::operator == (const ONDXKey& rKey) const
        {
            if(&rKey == this)
                return sal_True;
            return Compare(rKey) == 0;
        }
        inline sal_Bool ONDXKey::operator != (const ONDXKey& rKey) const
        {
            return !operator== (rKey);
        }
        inline sal_Bool ONDXKey::operator <  (const ONDXKey& rKey) const
        {
            return Compare(rKey) < 0;
        }
        inline sal_Bool ONDXKey::operator >  (const ONDXKey& rKey) const
        {
            return Compare(rKey) > 0;
        }
        inline sal_Bool ONDXKey::operator <= (const ONDXKey& rKey) const
        {
            return !operator > (rKey);
        }
        inline sal_Bool ONDXKey::operator >= (const ONDXKey& rKey) const
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
