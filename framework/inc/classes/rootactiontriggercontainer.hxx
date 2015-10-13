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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_ROOTACTIONTRIGGERCONTAINER_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_ROOTACTIONTRIGGERCONTAINER_HXX

#include <helper/propertysetcontainer.hxx>
#include <vcl/menu.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <framework/fwedllapi.h>

#define IMPLEMENTATIONNAME_ROOTACTIONTRIGGERCONTAINER   "com.sun.star.comp.ui.RootActionTriggerContainer"

namespace framework
{

class FWE_DLLPUBLIC RootActionTriggerContainer :  public PropertySetContainer,
                                    public css::lang::XMultiServiceFactory,
                                    public css::lang::XServiceInfo,
                                    public css::lang::XUnoTunnel,
                                    public css::lang::XTypeProvider,
                                    public css::container::XNamed
{
    public:
        RootActionTriggerContainer( const Menu* pMenu, const OUString* pMenuIdentifier);
        virtual ~RootActionTriggerContainer();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XMultiServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element )
            throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element )
            throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw (css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XNamed
        virtual OUString SAL_CALL getName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        void FillContainer();

        bool            m_bContainerCreated;
        bool            m_bContainerChanged;
        bool            m_bInContainerCreation;
        const Menu*     m_pMenu;
        const OUString* m_pMenuIdentifier;
};

}

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_ROOTACTIONTRIGGERCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
