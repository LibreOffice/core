/*************************************************************************
 *
 *  $RCSfile: dindexnode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-17 08:40:53 $
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
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#define _CONNECTIVITY_DBASE_INDEXNODE_HXX_

#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#include "file/fcode.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXPAGE_HXX_
#include "dbase/DIndexPage.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "file/FValue.hxx"
#endif

namespace connectivity
{
    namespace dbase
    {

        class ONDXNode;
        class ODbaseIndex;
        typedef file::ORowSetValue ORowSetValue_BASE;
        //==================================================================
        // Index Key
        //==================================================================
        typedef file::OOperand ONDXKey_BASE;
        class ONDXKey : public ONDXKey_BASE
        {
            friend class ONDXNode;
            UINT32              nRecord;                /* Satzzeiger               */
            ORowSetValue_BASE   xValue;                 /* Schluesselwert           */

        public:
            ONDXKey():nRecord(0){}
            inline ONDXKey(const ORowSetValue_BASE& rVal, sal_Int32 eType, UINT32 nRec)
                : ONDXKey_BASE(eType)
                , nRecord(nRec)
                , xValue(rVal)
            {}
            ONDXKey(const rtl::OUString& aStr, UINT32 nRec = 0)
                : ONDXKey_BASE(::com::sun::star::sdbc::DataType::VARCHAR)
                 ,nRecord(nRec)
            {
                if (aStr.len())
                    xValue = aStr;
            }
            ONDXKey(double aVal, UINT32 nRec = 0)
                : ONDXKey_BASE(::com::sun::star::sdbc::DataType::DOUBLE)
                 ,nRecord(nRec)
                 ,xValue(aVal)
            {
            }
            ONDXKey(UINT32 nRec)
                : nRecord(nRec){}
            inline ONDXKey(const ONDXKey& rKey);

            inline ONDXKey& operator= (const ONDXKey& rKey);
            virtual void setValue(const ::com::sun::star::uno::Any& _rVal)
            {
                xValue = _rVal;
            }

            virtual ::com::sun::star::uno::Any getValue() const
            {
                return xValue.makeAny();
            }

            UINT32 GetRecord() const {return nRecord;}
            void   ResetRecord() {nRecord = 0;}

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

        class ONDXPagePtr;
        //==================================================================
        // Index Seite
        //==================================================================
        class ONDXPage //: public SvRefBase
        {
            friend class ODbaseIndex;

            friend  SvStream& operator << (SvStream &rStream, const ONDXPage&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPage&);

            UINT32      nPagePos;               // Position in der Indexdatei
            BOOL        bModified : 1;
            USHORT      nCount;

            ONDXPagePtr *aParent,           // VaterSeite
                    *aChild;                // Zeiger auf rechte ChildPage
            ODbaseIndex& rIndex;
            ONDXNode*  ppNodes;             // array von Knoten

            oslInterlockedCount         m_refCount;

        public:
            void acquire()
            {
                osl_incrementInterlockedCount( &m_refCount );
            }
            void release();

            // Knoten Operationen
            USHORT  Count() const {return nCount;}

            BOOL    Insert(ONDXNode& rNode, ULONG nRowsLeft = 0);
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
            ONDXPage(ODbaseIndex& rIndex, ULONG nPos, ONDXPage* = NULL);
            ~ONDXPage();

            virtual void QueryDelete();

            void SetModified(BOOL bMod) {bModified = bMod;}
            void SetPagePos(UINT32 nPage) {nPagePos = nPage;}

            BOOL Find(const ONDXKey&);  // rek. Abstieg
            USHORT FindPos(const ONDXKey& rKey) const;

        #ifdef DEBUG
            void PrintPage();
        #endif
        };

        //==================================================================
        // Index Seitenverweis
        //==================================================================
        //  SV_DECL_REF(ONDXPage); // Basisklasse da weitere Informationen gehalten werden muessen
        typedef vos::ORef<ONDXPage>         ONDXPagePtr_BASE;

        class ONDXPagePtr : public ONDXPagePtr_BASE //ONDXPageRef
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
            sal_Bool Is() const { return isValid(); }
            void Clear()
            {
                unbind();
            }
        };

        SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
        SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

        inline BOOL ONDXPage::IsRoot() const {return !aParent->Is();}
        inline BOOL ONDXPage::IsLeaf() const {return !aChild->HasPage();}
        inline BOOL ONDXPage::IsModified() const {return bModified;}
        inline BOOL ONDXPage::HasParent() {return aParent->Is();}
        inline BOOL ONDXPage::HasChild() const {return aChild->HasPage();}
        inline ONDXPagePtr ONDXPage::GetParent() {return *aParent;}

        inline void ONDXPage::SetParent(ONDXPagePtr aPa = ONDXPagePtr())
        { *aParent = aPa;}

        inline void ONDXPage::SetChild(ONDXPagePtr aCh = ONDXPagePtr())
        {
            *aChild = aCh;
            if (aChild->Is())
                (*aChild)->SetParent(this);
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
                       :aKey(rKey), aChild(aPagePtr) {}

            // verweist der Knoten auf eine Seite
            BOOL            HasChild() const {return aChild.HasPage();}
            // Ist ein Index angegeben, kann gegebenfalls die Seite nachgeladen werden
            ONDXPagePtr&    GetChild(ODbaseIndex* pIndex = NULL, ONDXPage* = NULL);

            const ONDXKey& GetKey() const {return aKey;}
            ONDXKey&         GetKey() {return aKey;}

            // Setzen des Childs, ueber Referenz, um die PagePos zu erhalten
            void            SetChild(ONDXPagePtr aCh = ONDXPagePtr(), ONDXPage* = NULL);
            void            SetKey(ONDXKey& rKey) {aKey = rKey;}

            void Write(SvStream &rStream, const ONDXPage& rPage) const;
            void Read(SvStream &rStream, ODbaseIndex&);
        };
        //==================================================================
        // inline implementation
        //==================================================================
//      inline ONDXKey::ONDXKey(const ORowSetValue_BASE& rVal, sal_Int32 eType, UINT32 nRec)
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
                     ,nRecord(rKey.nRecord),xValue(rKey.xValue)
        {
        }

        inline ONDXKey& ONDXKey::operator=(const ONDXKey& rKey)
        {
            xValue = rKey.xValue;
            nRecord = rKey.nRecord;
            m_eDBType = rKey.getDBType();
            return *this;
        }

        inline BOOL ONDXKey::operator == (const ONDXKey& rKey) const
        {
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


