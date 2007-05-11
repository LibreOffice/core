/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pppoptimizerdialog.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:59:59 $
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

#ifndef PPPOPTIMIZERDIALOG_HXX
#define PPPOPTIMIZERDIALOGOG_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTI_COMPONENT_FACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

// ----------------------
// - PPPOptimizerDialog -
// ----------------------

class OptimizerDialog;
class PPPOptimizerDialog : public   ::cppu::WeakImplHelper4<
                                    com::sun::star::lang::XInitialization,
                                    com::sun::star::lang::XServiceInfo,
                                    com::sun::star::frame::XDispatchProvider,
                                    com::sun::star::frame::XDispatch >
{
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxFrame;
    com::sun::star::uno::Reference< com::sun::star::frame::XController > mxController;

    OptimizerDialog*    mpOptimizerDialog;

public:

    PPPOptimizerDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF );
    virtual ~PPPOptimizerDialog();

    // XInitialization
    void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

    // XDispatchProvider
    virtual com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > SAL_CALL queryDispatch(
        const com::sun::star::util::URL& aURL, const rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags )
            throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches(
        const com::sun::star::uno::Sequence< com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw( com::sun::star::uno::RuntimeException );

    // XDispatch
    virtual void SAL_CALL dispatch( const com::sun::star::util::URL& aURL,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& lArguments )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException );
};

rtl::OUString PPPOptimizerDialog_getImplementationName();
com::sun::star::uno::Sequence< rtl::OUString > PPPOptimizerDialog_getSupportedServiceNames();
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > PPPOptimizerDialog_createInstance( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rSMgr )
    throw( com::sun::star::uno::Exception );

// -----------------------------------------------------------------------------

#endif // PPPOPTIMIZERDIALOG_HXX
