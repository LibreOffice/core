/*************************************************************************
 *
 *  $RCSfile: AView.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:13:55 $
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
#include "ado/AView.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#include "ado/adoimp.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

// -------------------------------------------------------------------------
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

//  IMPLEMENT_SERVICE_INFO(OAdoView,"com.sun.star.sdbcx.AView","com.sun.star.sdbcx.View");
// -------------------------------------------------------------------------
OAdoView::OAdoView(sal_Bool _bCase,ADOView* _pView) : OView_ADO(_bCase,NULL)
{
    construct();

    if(_pView)
        m_aView = WpADOView(_pView);
    else
        m_aView.Create();
}
// -------------------------------------------------------------------------
OAdoView::OAdoView(sal_Bool _bCase, const ::rtl::OUString& _Name,
                sal_Int32 _CheckOption,
                const ::rtl::OUString& _Command,
                const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName)
     : OView_ADO(  _bCase,
                    _Name,
                    NULL,
                    _CheckOption,
                    _Command,
                    _SchemaName,
                    _CatalogName)
{
    construct();
    m_aView.Create();
    WpADOCommand aCommand;
    aCommand.Create();
    aCommand.put_Name(_Name);
    aCommand.put_CommandText(_Command);

    m_aView.put_Command(OLEVariant((IDispatch*)aCommand));

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
                ?
            (sal_Int64)this
                :
            OView_ADO::getSomething(rId);
}

// -------------------------------------------------------------------------
void OAdoView::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
        throw Exception();
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
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoView::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OView_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoView::release() throw(::com::sun::star::uno::RuntimeException)
{
    OView_ADO::release();
}
// -----------------------------------------------------------------------------


