/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BGroups.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:32:05 $
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

#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#define _CONNECTIVITY_ADABAS_GROUPS_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
namespace connectivity
{
    namespace sdbcx
    {
        class IRefreshableGroups;
    }
    namespace adabas
    {
        class OAdabasConnection;
        class OGroups : public sdbcx::OCollection
        {
        protected:
            OAdabasConnection*                          m_pConnection;
            connectivity::sdbcx::IRefreshableGroups*    m_pParent;

            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
            virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
        public:
            OGroups(::cppu::OWeakObject& _rParent,
                     ::osl::Mutex& _rMutex,
                     const TStringVector &_rVector,
                     OAdabasConnection* _pConnection,
                     connectivity::sdbcx::IRefreshableGroups* _pParent) : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
                ,m_pConnection(_pConnection)
                ,m_pParent(_pParent)
            {}
        };
    }
}

#endif // _CONNECTIVITY_ADABAS_GROUPS_HXX_

