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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "ado/AView.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "ado/adoimp.hxx"
#include <cppuhelper/typeprovider.hxx>
#include "ado/Awrapado.hxx"
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include "TConnection.hxx"

// -------------------------------------------------------------------------
using namespace comphelper;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

//  IMPLEMENT_SERVICE_INFO(OAdoView,"com.sun.star.sdbcx.AView","com.sun.star.sdbcx.View");
// -------------------------------------------------------------------------
OAdoView::OAdoView(sal_Bool _bCase,ADOView* _pView) : OView_ADO(_bCase,NULL)
,m_aView(_pView)
{
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoView::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoView::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OView_ADO::getSomething(rId);
}

// -------------------------------------------------------------------------
void OAdoView::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aView.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aView.get_Name();
                break;
            case PROPERTY_ID_CATALOGNAME:
                break;
            case PROPERTY_ID_SCHEMANAME:
                //  rValue <<= m_aView.get_Type();
                break;
            case PROPERTY_ID_COMMAND:
                {
                    OLEVariant aVar;
                    m_aView.get_Command(aVar);
                    if(!aVar.isNull() && !aVar.isEmpty())
                    {
                        ADOCommand* pCom = (ADOCommand*)aVar.getIDispatch();
                        OLEString aBSTR;
                        pCom->get_CommandText(&aBSTR);
                        rValue <<= (::rtl::OUString) aBSTR;
                    }
                }
                break;
        }
    }
    else
        OView_ADO::getFastPropertyValue(rValue,nHandle);
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoView::acquire() throw()
{
    OView_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoView::release() throw()
{
    OView_ADO::release();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
