/*************************************************************************
 *
 *  $RCSfile: datasourceconnector.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:56:46 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xDatabaseContext;
        ::rtl::OUString m_sContextInformation;
        ::rtl::OUString m_sContextDetails;

    public:
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent
        );
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent,
            const ::rtl::OUString& _rContextInformation,
            const ::rtl::OUString& _rContextDetails = ::rtl::OUString()
        );

        /// returns <TRUE/> if the object is able to create data source connections
        sal_Bool    isValid() const { return m_xDatabaseContext.is(); }

        /// create a data source connection
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(const ::rtl::OUString& _rDataSourceName, sal_Bool _bShowError = sal_True) const;

        /// create a data source connection
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& _xDataSource
                            , sal_Bool _bShowError = sal_True) const;

    private:
        void implConstruct();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DATASOURCECONNECTOR_HXX_

