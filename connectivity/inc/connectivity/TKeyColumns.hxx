/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TKeyColumns.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:15:47 $
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

#ifndef CONNECTIVITY_TKEYCOLUMNS_HXX
#define CONNECTIVITY_TKEYCOLUMNS_HXX

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef CONNECTIVITY_TKEY_HXX
#include "connectivity/TKey.hxx"
#endif


namespace connectivity
{
    class OKeyColumnsHelper : public connectivity::sdbcx::OCollection
    {
        OTableKeyHelper* m_pKey;
    protected:
        virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
    public:
        OKeyColumnsHelper(  OTableKeyHelper* _pKey,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector);
    };
}
#endif // CONNECTIVITY_TKEYCOLUMNS_HXX
