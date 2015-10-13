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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_PROPERTYSETCONTAINER_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_PROPERTYSETCONTAINER_HXX

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC PropertySetContainer : public css::container::XIndexContainer    ,
                             public ::cppu::OWeakObject
{
    public:
        PropertySetContainer();
        virtual ~PropertySetContainer();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

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
            throw (css::uno::RuntimeException, std::exception) override
        {
            return cppu::UnoType<css::beans::XPropertySet>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

    private:
        typedef std::vector< css::uno::Reference< css::beans::XPropertySet > > PropertySetVector;
        PropertySetVector                                                               m_aPropertySetVector;

};

}

#endif // __FRAMEWORK_CLASSES_PROPERTYSETCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
