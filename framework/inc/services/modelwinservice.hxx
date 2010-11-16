/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urltransformer.hxx,v $
 * $Revision: 1.8 $
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

#ifndef __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_
#define __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <cppuhelper/weak.hxx>
#include <vcl/window.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

class IModelWin
{
    public:
        virtual void registerModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow, const css::uno::Reference< css::awt::XControlModel >& rModel ) = 0;
        virtual void deregisterModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow ) = 0;
};

class ModelWinService : public css::lang::XTypeProvider
                     ,  public css::lang::XServiceInfo
                     ,  public css::container::XNameAccess
                     ,  public IModelWin
                     ,  public ::cppu::OWeakObject
{
    public:
        ModelWinService(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
        virtual ~ModelWinService();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  IModelWin
        //---------------------------------------------------------------------------------------------------------
        virtual void registerModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow, const css::uno::Reference< css::awt::XControlModel >& rModel );
        virtual void deregisterModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow );

        //---------------------------------------------------------------------------------------------------------
        //  XNameAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& sName ) throw( css::container::NoSuchElementException  ,
                                                                                        css::lang::WrappedTargetException       ,
                                                                                        css::uno::RuntimeException              );
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException );

    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager;
};

}

#endif // __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_
