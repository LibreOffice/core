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

#include <osl/mutex.hxx>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>


namespace frm
{


    //= OLimitedFormats

    /** maintains translation tables format key <-> enum value
        <p>Used for controls which provide a limited number for (standard) formats, which
        should be available as format keys.</p>
    */
    class OLimitedFormats
    {
    private:
        static sal_Int32    s_nInstanceCount;
        static ::osl::Mutex s_aMutex;
        static css::uno::Reference< css::util::XNumberFormatsSupplier >
                            s_xStandardFormats;

    protected:
        sal_Int32           m_nFormatEnumPropertyHandle;
        const sal_Int16     m_nTableId;
        css::uno::Reference< css::beans::XFastPropertySet >
                            m_xAggregate;

    protected:
        /** ctor
            <p>The class id is used to determine the translation table to use. All instances which
            pass the same value here share one table.</p>
        */
        OLimitedFormats(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const sal_Int16 _nClassId
            );
        ~OLimitedFormats();

    protected:
        void setAggregateSet(
            const css::uno::Reference< css::beans::XFastPropertySet >& _rxAggregate,
            sal_Int32 _nOriginalPropertyHandle
        );

    protected:
        void        getFormatKeyPropertyValue( css::uno::Any& _rValue ) const;
        bool    convertFormatKeyPropertyValue(
                        css::uno::Any& _rConvertedValue,
                        css::uno::Any& _rOldValue,
                const   css::uno::Any& _rNewValue
            );
        void        setFormatKeyPropertyValue( const css::uno::Any& _rNewValue );
        // setFormatKeyPropertyValue should only be called with a value got from convertFormatKeyPropertyValue!

        const css::uno::Reference< css::util::XNumberFormatsSupplier >&
                    getFormatsSupplier() const { return s_xStandardFormats; }

    private:
        void acquireSupplier(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
        void releaseSupplier();

        static void ensureTableInitialized(const sal_Int16 _nTableId);
        static void clearTable(const sal_Int16 _nTableId);
    };


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
