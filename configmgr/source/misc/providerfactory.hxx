/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerfactory.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _CONFIGMGR_PROVIDER_FACTORY_HXX_
#define _CONFIGMGR_PROVIDER_FACTORY_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
/*
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/stl_types.hxx>
*/
//------------------------------------------------------------------------
namespace configmgr
{
    //------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;
    using rtl::OUString;
    //------------------------------------------------------------------------
    class ContextReader;
    class ArgumentHelper;
    //------------------------------------------------------------------------
    //= OProviderFactory
    //------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper1< lang::XSingleComponentFactory > ProviderFactory_Base;

    /** a special factory for the configuration provider,
        which maps creation arguments into a context.
    */
    class ProviderFactory : public ProviderFactory_Base
    {
        OUString const m_aImplementationName;
        bool m_bAdmin;

    public:
        typedef uno::Reference< uno::XComponentContext > Context;
        typedef uno::Sequence < uno::Any >          Arguments;
        typedef uno::Sequence < beans::NamedValue > NamedValues;
    public:
        explicit
        ProviderFactory(OUString const & aImplementationName, bool bAdmin);
        ~ProviderFactory();

        virtual uno::Reference< uno::XInterface >
            SAL_CALL createInstanceWithContext(Context const & xContext )
                throw (uno::Exception, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstanceWithArgumentsAndContext( Arguments const & aArguments, Context const & xContext )
                throw (uno::Exception, uno::RuntimeException);

    private:
        uno::Reference< uno::XInterface > getProviderFromContext(Context const & aContext);
        uno::Reference< uno::XInterface > getProviderAlways(Context const & xContext);
        uno::Reference< uno::XInterface > createProviderWithArguments(Context const & xContext, Arguments const & _aArguments);
        uno::Reference< uno::XInterface > createProvider(Context const & xContext,bool bAdmin);
        uno::Reference< uno::XInterface > createProvider(Context const & xContext);
        sal_Int32 parseArguments(ArgumentHelper & aParser, NamedValues & rValues, Arguments const & _aArguments);
    };
//------------------------------------------------------------------------
}   // namespace configmgr
//------------------------------------------------------------------------

#endif // _CONFIGMGR_PROVIDER_FACTORY_HXX_

