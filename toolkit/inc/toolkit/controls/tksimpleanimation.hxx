/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tksimpleanimation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 13:51:16 $
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
#ifndef TOOLKIT_CONTROLS_TKSIMPLEANIMATION_HXX
#define TOOLKIT_CONTROLS_TKSIMPLEANIMATION_HXX

#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif
#ifndef _TOOLKIT_AWT_UNOCONTROLBASE_HXX_
#include <toolkit/controls/unocontrolbase.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

#ifndef  _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef  _COM_SUN_STAR_AWT_XSIMPLEANIMATION_HPP_
#include <com/sun/star/awt/XSimpleAnimation.hpp>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= UnoSimpleAnimationControlModel
    //====================================================================
    class UnoSimpleAnimationControlModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

    public:
                            UnoSimpleAnimationControlModel();
                            UnoSimpleAnimationControlModel( const UnoSimpleAnimationControlModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoSimpleAnimationControlModel( *this ); }

        // XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= UnoSimpleAnimationControl
    //====================================================================

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSimpleAnimation
                                >   UnoSimpleAnimationControl_Base;

    class UnoSimpleAnimationControl :public UnoControlBase
                                    ,public UnoSimpleAnimationControl_Base
    {
    private:

    public:
                                    UnoSimpleAnimationControl();
        ::rtl::OUString             GetComponentServiceName();

        DECLARE_UNO3_AGG_DEFAULTS( UnoSimpleAnimationControl, UnoControlBase );
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XSimpleAnimation
        virtual void SAL_CALL start() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stop() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList )
                                            throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_CONTROLS_TKSIMPLEANIMATION_HXX
