/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addonstoolbarwrapper.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <helper/uielementwrapperbase.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

namespace framework
{

class AddonsToolBarManager;
class AddonsToolBarWrapper : public UIElementWrapperBase
{
    public:
        AddonsToolBarWrapper( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~AddonsToolBarWrapper();

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >                             m_xServiceManager;
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >                                       m_xToolBarManager;
        com::sun::star::uno::Reference< com::sun::star::awt::XWindow >                                           m_xToolBarWindow;
        com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > m_aConfigData;
};

}

#endif // __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_
