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

#ifndef _XMLOFF_FORMS_EVENTEXPORT_HXX_
#define _XMLOFF_FORMS_EVENTEXPORT_HXX_

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>

namespace xmloff
{

    //= OEventDescriptorMapper
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::container::XNameReplace
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
    protected:
        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >, MapString2PropertyValueSequence );
        MapString2PropertyValueSequence m_aMappedEvents;

    public:
        OEventDescriptorMapper(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents);

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
    };

}   // namespace xmloff

#endif // _XMLOFF_FORMS_EVENTEXPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
