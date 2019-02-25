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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_EVENTIMPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_EVENTIMPORT_HXX

#include <sal/config.h>

#include <map>

#include <xmloff/XMLEventsImportContext.hxx>
#include "callbacks.hxx"
#include "ifacecompare.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>

class SvXMLImport;
namespace xmloff
{

    //= OFormEventsImportContext
    class OFormEventsImportContext : public XMLEventsImportContext
    {
        IEventAttacher& m_rEventAttacher;

    public:
        OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rLocalName,
            IEventAttacher& _rEventAttacher);

    protected:
        virtual void EndElement() override;
    };

    //= ODefaultEventAttacherManager
    class ODefaultEventAttacherManager : public IEventAttacherManager
    {
        typedef std::map<
            css::uno::Reference< css::beans::XPropertySet >,
            css::uno::Sequence< css::script::ScriptEventDescriptor >,
            OPropertySetCompare>
            MapPropertySet2ScriptSequence;
        // usually an event attacher manager will need to collect all script events registered, 'cause
        // the _real_ XEventAttacherManager handles it's events by index, but out indices are not fixed
        // until _all_ controls have been inserted.

        MapPropertySet2ScriptSequence   m_aEvents;

    public:
        // IEventAttacherManager
        virtual void registerEvents(
            const css::uno::Reference< css::beans::XPropertySet >& _rxElement,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            ) override;

    protected:
        void setEvents(
            const css::uno::Reference< css::container::XIndexAccess >& _rxContainer
            );

        virtual ~ODefaultEventAttacherManager();
    };

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_EVENTIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
