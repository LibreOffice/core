/*************************************************************************
 *
 *  $RCSfile: DIndex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 13:53:38 $
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

#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#define _CONNECTIVITY_DBASE_INDEX_HXX_

#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#include "dbase/dindexnode.hxx"
#endif

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
            SvFileStream    m_aFileStream;                  // Stream zum Lesen/Schreiben des Index
            NDXHeader       m_aHeader;
            ONDXPageList    m_aCollector;                   // Pool von nicht mehr benötigten Seiten
            ONDXPagePtr     m_aRoot,                        // Wurzel des b+ Baums
                            m_aCurLeaf;                 // aktuelles Blatt
            USHORT          m_nCurNode;                 // Position des aktuellen Knoten

            sal_uInt32          m_nPageCount,
                            m_nRootPage;

            ODbaseTable*    m_pTable;
            BOOL            m_bUseCollector : 1;                        // Verwenden des GarbageCollectors
            sal_Bool        m_bUnique;

            sal_Bool openIndexFile();
            INetURLObject getEntry();
        public:
            DECLARE_CTY_DEFAULTS( ODbaseIndex_BASE);

            ODbaseIndex(ODbaseTable* _pTable);
            ODbaseIndex(ODbaseTable* _pTable,const NDXHeader& _aHeader,const ::rtl::OUString& _Name);

            virtual void refreshColumns();

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(com::sun::star::uno::RuntimeException);
            static com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            const ODbaseTable* getTable() const { return m_pTable; }
            const NDXHeader& getHeader() const { return m_aHeader; }
            virtual OIndexIterator* createIterator(OBoolOperator_BASE* pOp,
                                                         const OOperand_BASE* pOperand = NULL);

            void SetRootPos(sal_uInt32 nPos)        {m_nRootPage = nPos;}
            void SetPageCount(sal_uInt32 nCount)    {m_nPageCount = nCount;}

            sal_uInt32 GetRootPos()                 {return m_nRootPage;}
            sal_uInt32 GetPageCount()               {return m_nPageCount;}

            BOOL IsText() const                 {return m_aHeader.db_keytype == 0;}
            USHORT GetMaxNodes() const          {return m_aHeader.db_maxkeys;}

            virtual BOOL Insert(sal_uInt32 nRec, const ORowSetValue_BASE& rValue);
            virtual BOOL Update(sal_uInt32 nRec, const ORowSetValue_BASE&, const ORowSetValue_BASE&);
            virtual BOOL Delete(sal_uInt32 nRec, const ORowSetValue_BASE& rValue);
            virtual BOOL Find(sal_uInt32 nRec, const ORowSetValue_BASE& rValue);

            void createINFEntry();
            BOOL CreateImpl();
            BOOL DropImpl();

            DECLARE_SERVICE_INFO();
        protected:

            ONDXPage* CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent = NULL, BOOL bLoad = FALSE);
            void Collect(ONDXPage*);
            ONDXPagePtr getRoot();

            sal_Bool isUnique() const { return m_bUnique; }
            BOOL UseCollector() const {return m_bUseCollector;}
            // Tree operationen
            void Insert(ONDXPagePtr aCurPage, ONDXNode& rNode);
            void Release(BOOL bSave = TRUE);
            BOOL ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue_BASE& rValue);
        };

        SvStream& operator << (SvStream &rStream, ODbaseIndex&);
        SvStream& operator >> (SvStream &rStream, ODbaseIndex&);
    }
}

#endif // _CONNECTIVITY_DBASE_INDEX_HXX_


