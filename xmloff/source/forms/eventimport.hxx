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

#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#define _XMLOFF_FORMS_EVENTIMPORT_HXX_

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
    protected:
        IEventAttacher& m_rEventAttacher;

    public:
        OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rLocalName,
            IEventAttacher& _rEventAttacher);

    protected:
        virtual void EndElement();
    };

    //= ODefaultEventAttacherManager
    class ODefaultEventAttacherManager : public IEventAttacherManager
    {
    protected:
        DECLARE_STL_MAP(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >,              // map from
            ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >,     // map to
            OPropertySetCompare,                                                                    // compare method
            MapPropertySet2ScriptSequence);                                                         // class name
        // usually an event attacher manager will need to collect all script events registered, 'cause
        // the _real_ XEventAttacherManager handles it's events by index, but out indicies are not fixed
        // until _all_ controls have been inserted.

        MapPropertySet2ScriptSequence   m_aEvents;

    public:
        // IEventAttacherManager
        virtual void registerEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

    protected:
        void setEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxContainer
            );

        virtual ~ODefaultEventAttacherManager();
    };

}   // namespace xmloff

#endif // _XMLOFF_FORMS_EVENTIMPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
