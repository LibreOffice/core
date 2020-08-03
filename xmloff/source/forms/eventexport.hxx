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

#include <sal/config.h>

#include <map>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>

namespace xmloff
{

    //= OEventDescriptorMapper
    typedef ::cppu::WeakImplHelper <   css::container::XNameReplace
                                    >   OEventDescriptorMapper_Base;
    /** helper class wrapping different script event representations

        <p>In the form layer, the script events are represented by <type scope="com.sun.star.script">ScriptEventDescriptor</type>
        instances. The office applications, on the other hand, represent their a single script event as sequence
        of <type scope="com.sun.star.beans">PropertyValue</type>s, where all events of a given object are
        accessible through a <type scope="com.sun.star.container">XNameReplace</type> interface.</p>
        <p>This class maps the first representation of events of a single object to the second one.</p>
        <p>This way, we can use the helper classes here in the project.</p>
    */
    class OEventDescriptorMapper : public OEventDescriptorMapper_Base
    {
        typedef std::map< OUString, css::uno::Sequence < css::beans::PropertyValue > > MapString2PropertyValueSequence;
        MapString2PropertyValueSequence m_aMappedEvents;

    public:
        explicit OEventDescriptorMapper(
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents);

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;
    };

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
