/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasourceconnector.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:24:32 $
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

#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#define _DBAUI_DATASOURCECONNECTOR_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif

class Window;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ODatasourceConnector
    //=====================================================================
    class ODatasourceConnector
    {
    protected:
        Window*         m_pErrorMessageParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::rtl::OUString m_sContextInformation;

    public:
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent
        );
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent,
            const ::rtl::OUString& _rContextInformation
        );

        /// returns <TRUE/> if the object is able to create data source connections
        sal_Bool    isValid() const { return m_xORB.is(); }

        /// create a data source connection
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(const ::rtl::OUString& _rDataSourceName, sal_Bool _bShowError = sal_True) const;

        /// create a data source connection
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& _xDataSource
                            , sal_Bool _bShowError = sal_True) const;
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DATASOURCECONNECTOR_HXX_

