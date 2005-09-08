/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DIndexIter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:04:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#define _CONNECTIVITY_DBASE_INDEXITER_HXX_

#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#include "file/fcode.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#include "dbase/dindexnode.hxx"
#endif

namespace connectivity
{
    namespace dbase
    {
        //==================================================================
        // IndexIterator
        //==================================================================
        class OIndexIterator
        {
        protected:
            file::OBoolOperator*    m_pOperator;
            const file::OOperand*   m_pOperand;
            ODbaseIndex*            m_pIndex;
            ONDXPagePtr             m_aRoot,
                                    m_aCurLeaf;
            USHORT                  m_nCurNode;

        protected:
            ULONG Find(BOOL bFirst);
            ULONG GetCompare(BOOL bFirst);
            ULONG GetLike(BOOL bFirst);
            ULONG GetNull(BOOL bFirst);
            ULONG GetNotNull(BOOL bFirst);

            ONDXKey* GetFirstKey(ONDXPage* pPage,
                                const file::OOperand& rKey);
            ONDXKey* GetNextKey();
            ODbaseIndex* GetIndex() const {return m_pIndex;}


        public:
            OIndexIterator(ODbaseIndex* pInd,
                            file::OBoolOperator* pOp,
                            const file::OOperand* pOper)
                :m_pOperator(pOp)
                ,m_pOperand(pOper)
                ,m_pIndex(pInd)
                ,m_nCurNode(NODE_NOTFOUND)
            {
                pInd->acquire();
            }

            virtual ~OIndexIterator();
            ULONG First();
            ULONG Next();

        };
    }
}
#endif // _CONNECTIVITY_DBASE_INDEXITER_HXX_

