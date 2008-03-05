/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ChartType_UNO.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:13:43 $
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

#ifndef _CHART2_CREATION_TYPE_UNO_HXX
#define _CHART2_CREATION_TYPE_UNO_HXX

#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _VCL_VCLEVENT_HXX
#include <vcl/vclevent.hxx>
#endif

#include <svtools/genericunodialog.hxx>
//.............................................................................
namespace chart
{
//.............................................................................
typedef ::svt::OGenericUnoDialog ChartTypeUnoDlg_BASE;
class ChartType;
class ChartTypeUnoDlg : public ChartTypeUnoDlg_BASE
                        ,public ::comphelper::OPropertyArrayUsageHelper< ChartTypeUnoDlg >
{
public:
    ChartTypeUnoDlg( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

    // XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void);
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >&);
private:
    virtual ~ChartTypeUnoDlg();

    // OGenericUnoDialog overridables
    virtual void implInitialize(const com::sun::star::uno::Any& _rValue);
    virtual Dialog* createDialog(Window* _pParent);

    // XTypeProvider
    virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException);
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    //no default constructor
    ChartTypeUnoDlg();
    ChartTypeUnoDlg(const ChartTypeUnoDlg&); // no defined
    void operator =(const ChartTypeUnoDlg&); // no defined

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >           m_xChartModel;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
