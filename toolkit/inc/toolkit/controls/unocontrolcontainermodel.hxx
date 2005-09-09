/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocontrolcontainermodel.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:52:34 $
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

#ifndef _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_


#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>


//  ----------------------------------------------------
//  class ::com::sun::star::awt::UnoControlContainerModel
//  ----------------------------------------------------
class UnoControlContainerModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any                                                      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   getPropertySetInfo() const;
    ::cppu::IPropertyArrayHelper&                                                   SAL_CALL getInfoHelper();

public:
                        UnoControlContainerModel();
                        UnoControlContainerModel( const UnoControlContainerModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlContainerModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlContainerModel, UnoControlModel, szServiceName2_UnoControlContainerModel )
};



#endif // _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_

