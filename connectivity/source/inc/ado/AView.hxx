/*************************************************************************
 *
 *  $RCSfile: AView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 13:44:21 $
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

#ifndef _CONNECTIVITY_ADO_VIEW_HXX_
#define _CONNECTIVITY_ADO_VIEW_HXX_

#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include "connectivity/sdbcx/VView.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#include "ado/Awrapadox.hxx"
#endif

namespace connectivity
{
    namespace ado
    {

        typedef sdbcx::OView OView_ADO;

        class OAdoView :     public OView_ADO
        {
            WpADOView       m_aView;

        protected:
            // OPropertySetHelper
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue)
                                 throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            DECLARE_CTY_DEFAULTS( OView_ADO);

            OAdoView(sal_Bool _bCase, ADOView* _pView=NULL);
            OAdoView(sal_Bool _bCase,  const ::rtl::OUString& _rName,
                    sal_Int32 _nCheckOption = 0,
                    const ::rtl::OUString& _rCommand = ::rtl::OUString(),
                    const ::rtl::OUString& _rSchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _rCatalogName = ::rtl::OUString());

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            WpADOView getImpl() const { return m_aView;}
        };
    }
}

#endif // _CONNECTIVITY_ADO_VIEW_HXX_

