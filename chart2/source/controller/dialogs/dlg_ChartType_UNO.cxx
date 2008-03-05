/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ChartType_UNO.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:13:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_ChartType_UNO.hxx"
#include "dlg_ChartType.hxx"
#include "servicenames.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
uno::Reference< uno::XInterface >
            SAL_CALL ChartTypeUnoDlg::Create(const uno::Reference< uno::XComponentContext >& _xContext)
{
    return *(new ChartTypeUnoDlg(_xContext));
}
// -----------------------------------------------------------------------------
ChartTypeUnoDlg::ChartTypeUnoDlg( const uno::Reference< uno::XComponentContext >& _xContext )
                    : ChartTypeUnoDlg_BASE( _xContext )
{
}
// -----------------------------------------------------------------------------
ChartTypeUnoDlg::~ChartTypeUnoDlg()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
    // so this virtual-method-call the base class does does not work, we're already dead then ...
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
            destroyDialog();
    }
}
//-------------------------------------------------------------------------
// lang::XServiceInfo
//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL ChartTypeUnoDlg::getImplementationName() throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString ChartTypeUnoDlg::getImplementationName_Static() throw(uno::RuntimeException)
{
    return CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME;
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL ChartTypeUnoDlg::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------

uno::Sequence< rtl::OUString > ChartTypeUnoDlg::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_TYPE_DIALOG_SERVICE_NAME;
    return aSNS;
}
//-------------------------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL ChartTypeUnoDlg::getImplementationId( void ) throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}
//------------------------------------------------------------------------------
void ChartTypeUnoDlg::implInitialize(const uno::Any& _rValue)
{
    beans::PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (0 == aProperty.Name.compareToAscii("ChartModel"))
            m_xChartModel.set(aProperty.Value,uno::UNO_QUERY);
        else
            ChartTypeUnoDlg_BASE::implInitialize(_rValue);
    }
    else
        ChartTypeUnoDlg_BASE::implInitialize(_rValue);
}
//------------------------------------------------------------------------------
Dialog* ChartTypeUnoDlg::createDialog(Window* _pParent)
{
    return new ChartTypeDialog( _pParent, m_xChartModel, m_xContext );
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
uno::Reference<beans::XPropertySetInfo>  SAL_CALL ChartTypeUnoDlg::getPropertySetInfo() throw(uno::RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ChartTypeUnoDlg::getInfoHelper()
{
    return *const_cast<ChartTypeUnoDlg*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ChartTypeUnoDlg::createArrayHelper( ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//.............................................................................
} //namespace chart
//.............................................................................
