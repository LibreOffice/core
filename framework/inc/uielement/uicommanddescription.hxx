/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_UICOMMANDDESCRIPTION_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_UICOMMANDDESCRIPTION_HXX

#include <unordered_map>
#include <stdtypes.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
typedef ::cppu::WeakComponentImplHelper< css::lang::XServiceInfo,
        css::container::XNameAccess > UICommandDescription_BASE;

class UICommandDescription : private cppu::BaseMutex,
                             public UICommandDescription_BASE
{
    public:
        UICommandDescription( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~UICommandDescription();

        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return OUString("com.sun.star.comp.framework.UICommandDescription");
        }

        virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
            throw (css::uno::RuntimeException, std::exception) override
        {
            return cppu::supportsService(this, ServiceName);
        }

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override
        {
            css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.UICommandDescription" };
            return aSeq;
        }

private:
        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
            throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
            throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

public:
        typedef std::unordered_map< OUString,
                                    OUString,
                                    OUStringHash,
                                    std::equal_to< OUString > > ModuleToCommandFileMap;

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::container::XNameAccess >,
                                    OUStringHash,
                                    std::equal_to< OUString > > UICommandsHashMap;

    protected:
        UICommandDescription( const css::uno::Reference< css::uno::XComponentContext>& rxContext, bool  );
        void impl_fillElements(const sal_Char* _pName);

        OUString                                                  m_aPrivateResourceURL;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        ModuleToCommandFileMap                                    m_aModuleToCommandFileMap;
        UICommandsHashMap                                         m_aUICommandsHashMap;
        css::uno::Reference< css::container::XNameAccess >        m_xGenericUICommands;
        css::uno::Reference< css::frame::XModuleManager2 >        m_xModuleManager;
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_UICOMMANDDESCRPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
