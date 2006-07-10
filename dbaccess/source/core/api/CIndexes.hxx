/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CIndexes.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:00:53 $
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

#ifndef DBACCESS_INDEXES_HXX_
#define DBACCESS_INDEXES_HXX_

#ifndef CONNECTIVITY_INDEXESHELPER_HXX
#include "connectivity/TIndexes.hxx"
#endif

namespace dbaccess
{
    class OIndexes : public connectivity::OIndexesHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xIndexes;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual connectivity::sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
    public:
        OIndexes(connectivity::OTableHelper* _pTable,
                 ::osl::Mutex& _rMutex,
                 const ::std::vector< ::rtl::OUString> &_rVector,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes
                 ) : connectivity::OIndexesHelper(_pTable,_rMutex,_rVector)
            ,m_xIndexes(_rxIndexes)
        {}

        virtual void SAL_CALL disposing(void);
    };
}

#endif // DBACCESS_INDEXES_HXX_
