/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BIndexColumns.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:32:15 $
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

#ifndef _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_
#define _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#include "adabas/BIndex.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        class OIndexColumns : public sdbcx::OCollection
        {
            OAdabasIndex* m_pIndex;
        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        public:
            OIndexColumns(  OAdabasIndex* _pIndex,
                            ::osl::Mutex& _rMutex,
                            const TStringVector &_rVector)
                        : sdbcx::OCollection(*_pIndex,sal_True,_rMutex,_rVector)
                        ,m_pIndex(_pIndex)
            {
            }
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_

