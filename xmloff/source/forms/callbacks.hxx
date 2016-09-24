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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_CALLBACKS_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_CALLBACKS_HXX

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <rtl/ref.hxx>

class SvXMLExport;
class SvXMLExportPropertyMapper;

namespace xmloff
{

    //= IFormsExportContext
    class IFormsExportContext
    {
    public:
        virtual SvXMLExport&                                getGlobalContext() = 0;
        virtual ::rtl::Reference< SvXMLExportPropertyMapper >   getStylePropertyMapper() = 0;

        /** steps through a collection and exports all children of this collection
        */
        virtual void    exportCollectionElements(
            const css::uno::Reference< css::container::XIndexAccess >& _rxCollection) = 0;

        virtual OUString                             getObjectStyleName(
            const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) = 0;

    protected:
        ~IFormsExportContext() {}
    };

    //= IEventAttacherManager
    class IEventAttacherManager
    {
    public:
        virtual void registerEvents(
            const css::uno::Reference< css::beans::XPropertySet >& _rxElement,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            ) = 0;

    protected:
        ~IEventAttacherManager() {}
    };

    //= IEventAttacher
    class IEventAttacher
    {
    public:
        virtual void registerEvents(
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            ) = 0;

    protected:
        ~IEventAttacher() {}
    };

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_CALLBACKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
