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
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#define _CONNECTIVITY_DBASE_INDEXNODE_HXX_

#include "file/fcode.hxx"
#include "file/FTable.hxx"
#include "dbase/DIndexPage.hxx"
#include "connectivity/FValue.hxx"
#include <tools/ref.hxx>

#define NODE_NOTFOUND 0xFFFF
#define PAGE_SIZE 512

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
            sal_uInt32          nRecord;                /* Satzzeiger               */
            ORowSetValue    xValue;                 /* Schluesselwert           */

        public:
            ONDXKey(sal_uInt32 nRec=0);
            ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec);
            ONDXKey(const rtl::OUString& aStr, sal_uInt32 nRec = 0);
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
            StringCompare Compare(const ONDXKey& rKey) const;
        };




        //==================================================================
        // Index Seitenverweis
        //==================================================================
        SV_DECL_REF(ONDXPage) // Basisklasse da weitere Informationen gehalten werden muessen


        class ONDXPagePtr : public ONDXPageRef
        {
            friend  SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            sal_uInt32  nPagePos;       // Position in der Indexdatei

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
        // Index Seite
        //==================================================================
        class ONDXPage : public SvRefBase
        {
            friend class ODbaseIndex;

            friend  SvStream& operator << (SvStream &rStream, const ONDXPage&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPage&);

            sal_uInt32      nPagePos;               // Position in der Indexdatei
            sal_Bool        bModified : 1;
            sal_uInt16      nCount;

            ONDXPagePtr aParent,            // VaterSeite
                        aChild;             // Zeiger auf rechte ChildPage
            ODbaseIndex& rIndex;
            ONDXNode*  ppNodes;             // array von Knoten

        public:
            // Knoten Operationen
            sal_uInt16  Count() const {return nCount;}

            sal_Bool    Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft = 0);
            sal_Bool    Insert(sal_uInt16 nIndex, ONDXNode& rNode);
            sal_Bool    Append(ONDXNode& rNode);
            sal_Bool    Delete(sal_uInt16);
            void    Remove(sal_uInt16);
            void    Release(sal_Bool bSave = sal_True);
            void    ReleaseFull(sal_Bool bSave = sal_True);

            // Aufteilen und Zerlegen
            ONDXNode Split(ONDXPage& rPage);
            void Merge(sal_uInt16 nParentNodePos, ONDXPagePtr xPage);

            // Zugriffsoperationen
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

            // Parent braucht nicht nachgeladen zu werden
            ONDXPagePtr GetParent();
            ODbaseIndex& GetIndex() {return rIndex;}
            const ODbaseIndex& GetIndex() const {return rIndex;}

            // Setzen des Childs, ueber Referenz, um die PagePos zu erhalten
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

            sal_Bool Find(const ONDXKey&);  // rek. Abstieg
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
        // Index Knoten
        //==================================================================
        class ONDXNode
        {
            friend class ONDXPage;
            ONDXPagePtr aChild;             /* naechster Seitenverweis  */
            ONDXKey   aKey;

        public:
            ONDXNode(){}
            ONDXNode(const ONDXKey& rKey,
                       ONDXPagePtr aPagePtr = ONDXPagePtr())
                       :aChild(aPagePtr),aKey(rKey) {}

            // verweist der Knoten auf eine Seite
            sal_Bool            HasChild() const {return aChild.HasPage();}
            // Ist ein Index angegeben, kann gegebenfalls die Seite nachgeladen werden
            ONDXPagePtr&    GetChild(ODbaseIndex* pIndex = NULL, ONDXPage* = NULL);

            const ONDXKey& GetKey() const   { return aKey;}
            ONDXKey&       GetKey()         { return aKey;}

            // Setzen des Childs, ueber Referenz, um die PagePos zu erhalten
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


//      inline ONDXKey::ONDXKey(const rtl::OUString& aStr, sal_uInt32 nRec)
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
            return Compare(rKey) == COMPARE_EQUAL;
        }
        inline sal_Bool ONDXKey::operator != (const ONDXKey& rKey) const
        {
            return !operator== (rKey);
        }
        inline sal_Bool ONDXKey::operator <  (const ONDXKey& rKey) const
        {
            return Compare(rKey) == COMPARE_LESS;
        }
        inline sal_Bool ONDXKey::operator >  (const ONDXKey& rKey) const
        {
            return Compare(rKey) == COMPARE_GREATER;
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
