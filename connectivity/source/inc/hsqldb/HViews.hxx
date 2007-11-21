/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HViews.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:06:30 $
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
#ifndef _CONNECTIVITY_HSQLDB_VIEWS_HXX_
#define _CONNECTIVITY_HSQLDB_VIEWS_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
namespace connectivity
{
    namespace hsqldb
    {
        class HViews : public sdbcx::OCollection
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;
            sal_Bool m_bInDrop;

        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

            void createView( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        public:
            HViews(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, const TStringVector &_rVector );

            // only the name is identical to ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);

            void appendNew(const ::rtl::OUString& _rsNewTable);
            void dropByNameImpl(const ::rtl::OUString& elementName);
        };
    }
}
#endif // _CONNECTIVITY_HSQLDB_VIEWS_HXX_

