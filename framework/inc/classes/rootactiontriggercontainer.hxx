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
                                    public com::sun::star::lang::XMultiServiceFactory,
                                    public com::sun::star::lang::XServiceInfo,
                                    public com::sun::star::lang::XUnoTunnel,
                                    public com::sun::star::lang::XTypeProvider,
                                    public com::sun::star::container::XNamed
{
    public:
        RootActionTriggerContainer( const Menu* pMenu, const OUString* pMenuIdentifier);
        virtual ~RootActionTriggerContainer();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XMultiServiceFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XNamed
        virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        void FillContainer();

        bool    m_bContainerCreated;
        bool    m_bContainerChanged;
        bool    m_bInContainerCreation;
        const Menu* m_pMenu;
        const OUString* m_pMenuIdentifier;
};

}

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_ROOTACTIONTRIGGERCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
