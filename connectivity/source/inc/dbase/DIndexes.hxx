/*************************************************************************
 *
 *  $RCSfile: DIndexes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-04 06:36:43 $
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

#ifndef _CONNECTIVITY_DBASE_INDEXES_HXX_
#define _CONNECTIVITY_DBASE_INDEXES_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif

namespace connectivity
{
    namespace dbase
    {
        class ODbaseTable;

        typedef sdbcx::OCollection ODbaseIndexes_BASE;

        class ODbaseIndexes : public ODbaseIndexes_BASE
        {
            ODbaseTable*    m_pTable;
        protected:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject();
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > cloneObject(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
            virtual void appendObject( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
        public:
            ODbaseIndexes(ODbaseTable* _pTable, ::osl::Mutex& _rMutex,
                const TStringVector &_rVector) : ODbaseIndexes_BASE(*_pTable,_pTable->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers(),_rMutex,_rVector)
                , m_pTable(_pTable)
            {}

            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t nSize,const void* _pHint ) SAL_THROW( () )
                { return const_cast<void*>(_pHint); }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * pMem,const void* _pHint ) SAL_THROW( () )
                {  }
        };
    }
}
#endif // _CONNECTIVITY_DBASE_INDEXES_HXX_

