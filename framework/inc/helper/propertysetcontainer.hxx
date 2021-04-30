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

#pragma once

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace framework
{

class PropertySetContainer : public css::container::XIndexContainer    ,
                             public ::cppu::OWeakObject
{
    public:
        PropertySetContainer();
        virtual ~PropertySetContainer() override;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override
        {
            return cppu::UnoType<css::beans::XPropertySet>::get();
        }

        virtual sal_Bool SAL_CALL hasElements() override;

    private:
        typedef std::vector< css::uno::Reference< css::beans::XPropertySet > > PropertySetVector;
        PropertySetVector                                                               m_aPropertySetVector;

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
