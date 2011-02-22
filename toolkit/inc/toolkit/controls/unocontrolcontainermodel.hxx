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
                        UnoControlContainerModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
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

