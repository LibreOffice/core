/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tkthrobber.hxx,v $
 * $Revision: 1.3 $
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
#ifndef TOOLKIT_CONTROLS_TKTHROBBER_HXX
#define TOOLKIT_CONTROLS_TKTHROBBER_HXX

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XThrobber.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= UnoThrobberControlModel
    //====================================================================
    class UnoThrobberControlModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

    public:
                            UnoThrobberControlModel();
                            UnoThrobberControlModel( const UnoThrobberControlModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoThrobberControlModel( *this ); }

        // XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= UnoThrobberControl
    //====================================================================

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XThrobber
                                >   UnoThrobberControl_Base;

    class UnoThrobberControl :public UnoControlBase
                             ,public UnoThrobberControl_Base
    {
    private:

    public:
                                    UnoThrobberControl();
        ::rtl::OUString             GetComponentServiceName();

        DECLARE_UNO3_AGG_DEFAULTS( UnoThrobberControl, UnoControlBase );
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XThrobber
        virtual void SAL_CALL start() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stop() throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_CONTROLS_TKTHROBBER_HXX
