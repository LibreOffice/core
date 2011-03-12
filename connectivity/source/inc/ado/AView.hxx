/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADO_VIEW_HXX_
#define _CONNECTIVITY_ADO_VIEW_HXX_

#include "connectivity/sdbcx/VView.hxx"
#include "ado/Awrapadox.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
