/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: framectr.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _BIB_FRAMECTR_HXX
#define _BIB_FRAMECTR_HXX
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <cppuhelper/implbase5.hxx>
#include <svtools/svarray.hxx>

#include "bibmod.hxx"
class BibDataManager;
class BibFrameCtrl_Impl;
namespace com{namespace sun{namespace star{
    namespace form{
        class XFormController;
    }
}}}
class BibStatusDispatch
{
public:
    ::com::sun::star::util::URL                 aURL;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >    xListener;
                        BibStatusDispatch( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef )
                            : aURL( rURL )
                            , xListener( xRef )
                        {}
};

typedef BibStatusDispatch* BibStatusDispatchPtr;
SV_DECL_PTRARR_DEL( BibStatusDispatchArr, BibStatusDispatchPtr, 4, 4 )

class BibFrameController_Impl : public cppu::WeakImplHelper5 <
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::frame::XController,
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::frame::XDispatchProvider,
    ::com::sun::star::frame::XDispatchInformationProvider
>
{
friend class BibFrameCtrl_Impl;
    BibFrameCtrl_Impl*          pImp;
    BibStatusDispatchArr        aStatusListeners;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  xWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 xFrame;
    sal_Bool                        bDisposing;
    sal_Bool                        bHierarchical;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >               m_xDatMan;
    BibDataManager*             pDatMan;
    HdlBibModul                 pBibMod;

    DECL_STATIC_LINK( BibFrameController_Impl, DisposeHdl, void* );

    sal_Bool                    SaveModified(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController);
public:
                                BibFrameController_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xComponent);
                                BibFrameController_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xComponent,
                                                        BibDataManager* pDatMan);
                                ~BibFrameController_Impl();


    void                        activate();
    void                        deactivate();

    BibDataManager*             GetDataManager();
    void                        ChangeDataSource(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs);
    void                        RemoveFilter();

                                // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

                                // ::com::sun::star::frame::XController
    virtual void                SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool            SAL_CALL attachModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool            SAL_CALL suspend( sal_Bool bSuspend ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any  SAL_CALL    getViewData() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL                           restoreViewData( const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException);

                                // ::com::sun::star::lang::XComponent
    virtual void                SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void                SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void                SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException);

                                // ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw (::com::sun::star::uno::RuntimeException);

                                //class ::com::sun::star::frame::XDispatch
    virtual void                SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs) throw (::com::sun::star::uno::RuntimeException);
    virtual void                SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw (::com::sun::star::uno::RuntimeException);
    virtual void                SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw (::com::sun::star::uno::RuntimeException);

                                // ::com::sun::star::frame::XDispatchInformationProvider
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (::com::sun::star::uno::RuntimeException);
 };

#endif

