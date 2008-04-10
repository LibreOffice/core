/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dindexnode.hxx,v $
 * $Revision: 1.17 $
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
            UINT32          nRecord;                /* Satzzeiger               */
            ORowSetValue    xValue;                 /* Schluesselwert           */

        public:
            ONDXKey(UINT32 nRec=0);
            ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, UINT32 nRec);
            ONDXKey(const rtl::OUString& aStr, UINT32 nRec = 0);
            ONDXKey(double aVal, UINT32 nRec = 0);

            inline ONDXKey(const ONDXKey& rKey);

            inline ONDXKey& operator= (const ONDXKey& rKey);
            virtual void setValue(const ORowSetValue& _rVal);

            virtual const ORowSetValue& getValue() const;

            UINT32 GetRecord() const        { return nRecord;   }
            void setRecord(UINT32 _nRec)    { nRecord = _nRec;  }
            void   ResetRecord()            { nRecord = 0;      }

            BOOL operator == (const ONDXKey& rKey) const;
            BOOL operator != (const ONDXKey& rKey) const;
            BOOL operator <  (const ONDXKey& rKey) const;
            BOOL operator <= (const ONDXKey& rKey) const;
            BOOL operator >  (const ONDXKey& rKey) const;
            BOOL operator >= (const ONDXKey& rKey) const;

            BOOL Load (SvFileStream& rStream, BOOL bText);
            BOOL Write(SvFileStream& rStream, BOOL bText);

            static BOOL IsText(sal_Int32 eType);

        private:
            StringCompare Compare(const ONDXKey& rKey) const;
        };

        #define NODE_NOTFOUND 0xFFFF


        //==================================================================
        // Index Seitenverweis
        //==================================================================
        SV_DECL_REF(ONDXPage) // Basisklasse da weitere Informationen gehalten werden muessen


        class ONDXPagePtr : public ONDXPageRef
        {
            friend  SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            UINT32  nPagePos;       // Position in der Indexdatei

        public:
            ONDXPagePtr(UINT32 nPos = 0):nPagePos(nPos){}
            ONDXPagePtr(const ONDXPagePtr& rRef);
            ONDXPagePtr(ONDXPage* pRefPage);

            ONDXPagePtr& operator=(const ONDXPagePtr& rRef);
            ONDXPagePtr& operator=(ONDXPage* pPageRef);

            UINT32 GetPagePos() const {return nPagePos;}
            BOOL HasPage() const {return nPagePos != 0;}
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

            UINT32      nPagePos;               // Position in der Indexdatei
            BOOL        bModified : 1;
            USHORT      nCount;

            ONDXPagePtr aParent,            // VaterSeite
                        aChild;             // Zeiger auf rechte ChildPage
            ODbaseIndex& rIndex;
            ONDXNode*  ppNodes;             // array von Knoten

        public:
            // Knoten Operationen
            USHORT  Count() const {return nCount;}

            BOOL    Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft = 0);
            BOOL    Insert(USHORT nIndex, ONDXNode& rNode);
            BOOL    Append(ONDXNode& rNode);
            BOOL    Delete(USHORT);
            void    Remove(USHORT);
            void    Release(BOOL bSave = TRUE);
            void    ReleaseFull(BOOL bSave = TRUE);

            // Aufteilen und Zerlegen
            ONDXNode Split(ONDXPage& rPage);
            void Merge(USHORT nParentNodePos, ONDXPagePtr xPage);

            // Zugriffsoperationen
            ONDXNode& operator[] (USHORT nPos);
            const ONDXNode& operator[] (USHORT nPos) const;

            BOOL IsRoot() const;
            BOOL IsLeaf() const;
            BOOL IsModified() const;
            BOOL HasParent();
            BOOL HasChild() const;

            BOOL IsFull() const;

            UINT32 GetPagePos() const {return nPagePos;}
            ONDXPagePtr& GetChild(ODbaseIndex* pIndex = 0);

            // Parent braucht nicht nachgeladen zu werden
            ONDXPagePtr GetParent();
            ODbaseIndex& GetIndex() {return rIndex;}
            const ODbaseIndex& GetIndex() const {return rIndex;}

            // Setzen des Childs, ueber Referenz, um die PagePos zu erhalten
            void SetChild(ONDXPagePtr aCh);
            void SetParent(ONDXPagePtr aPa);

            USHORT Search(const ONDXKey& rSearch);
            USHORT Search(const ONDXPage* pPage);
            void   SearchAndReplace(const ONDXKey& rSearch, ONDXKey& rReplace);

        protected:
            ONDXPage(ODbaseIndex& rIndex, sal_uInt32 nPos, ONDXPage* = NULL);
            ~ONDXPage();

            virtual void QueryDelete();

            void SetModified(BOOL bMod) {bModified = bMod;}
            void SetPagePos(UINT32 nPage) {nPagePos = nPage;}

            BOOL Find(const ONDXKey&);  // rek. Abstieg
            USHORT FindPos(const ONDXKey& rKey) const;

#if OSL_DEBUG_LEVEL > 1
            void PrintPage();
#endif
        };

        SV_IMPL_REF(ONDXPage);

        SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
        SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

        inline BOOL ONDXPage::IsRoot() const {return !aParent.Is();}
        inline BOOL ONDXPage::IsLeaf() const {return !aChild.HasPage();}
        inline BOOL ONDXPage::IsModified() const {return bModified;}
        inline BOOL ONDXPage::HasParent() {return aParent.Is();}
        inline BOOL ONDXPage::HasChild() const {return aChild.HasPage();}
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
            BOOL            HasChild() const {return aChild.HasPage();}
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
//      inline ONDXKey::ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, UINT32 nRec)
//          : ONDXKey_BASE(eType)
//          , nRecord(nRec),xValue(rVal)
//      {
//      }


//      inline ONDXKey::ONDXKey(const rtl::OUString& aStr, UINT32 nRec)
//                  : ONDXKey_BASE(::com::sun::star::sdbc::DataType::VARCHAR)
//                   ,nRecord(nRec)
//      {
//          if (aStr.len())
//              xValue = aStr;
//      }

//      inline ONDXKey::ONDXKey(double aVal, UINT32 nRec)
//                   : ONDXKey_BASE(::com::sun::star::sdbc::DataType::DOUBLE)
//                   ,nRecord(nRec)
//                   ,xValue(aVal)
//      {
//      }

//      inline ONDXKey::ONDXKey(UINT32 nRec)
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

        inline BOOL ONDXKey::operator == (const ONDXKey& rKey) const
        {
            if(&rKey == this)
                return sal_True;
            return Compare(rKey) == COMPARE_EQUAL;
        }
        inline BOOL ONDXKey::operator != (const ONDXKey& rKey) const
        {
            return !operator== (rKey);
        }
        inline BOOL ONDXKey::operator <  (const ONDXKey& rKey) const
        {
            return Compare(rKey) == COMPARE_LESS;
        }
        inline BOOL ONDXKey::operator >  (const ONDXKey& rKey) const
        {
            return Compare(rKey) == COMPARE_GREATER;
        }
        inline BOOL ONDXKey::operator <= (const ONDXKey& rKey) const
        {
            return !operator > (rKey);
        }
        inline BOOL ONDXKey::operator >= (const ONDXKey& rKey) const
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


