/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AView.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:55:04 $
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
            virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            OAdoView(sal_Bool _bCase, ADOView* _pView=NULL);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            WpADOView getImpl() const { return m_aView;}
        };
    }
}

#endif // _CONNECTIVITY_ADO_VIEW_HXX_

