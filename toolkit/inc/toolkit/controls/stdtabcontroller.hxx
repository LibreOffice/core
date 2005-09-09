/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdtabcontroller.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:50:16 $
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

#ifndef _TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX_
#define _TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX_


#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWINDOW.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif


class StdTabController :    public ::com::sun::star::awt::XTabController,
                            public ::com::sun::star::lang::XServiceInfo,
                            public ::com::sun::star::lang::XTypeProvider,
                            public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex            maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >    mxControlContainer;

protected:
    ::osl::Mutex&               GetMutex() { return maMutex; }
    sal_Bool                    ImplCreateComponentSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rControls, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rModels, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& rComponents, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>* pTabStops, sal_Bool bPeerComponent ) const;
    // wenn rModels kuerzer als rControls ist, werden nur die rModels entsprechenden Elemente geliefert und die korrespondierenden Elemente aus rControls entfernt
    void                        ImplActivateControl( sal_Bool bFirst ) const;

public:
                            StdTabController();
                            ~StdTabController();

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  FindControl( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rCtrls, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & rxCtrlModel );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XTabController
    void SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& Container ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SAL_CALL getContainer(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL getControls(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL autoTabOrder(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL activateTabOrder(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL activateFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL activateLast(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO( StdTabController, szServiceName2_TabController )
};



#endif // _TOOLKIT_AWT_STDTABCONTROLLER_HXX_

