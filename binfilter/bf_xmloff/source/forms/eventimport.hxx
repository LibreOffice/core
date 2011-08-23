/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#define _XMLOFF_FORMS_EVENTIMPORT_HXX_

#include "XMLEventsImportContext.hxx"
#include "callbacks.hxx"
#include "ifacecompare.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
namespace binfilter {

class SvXMLImport;
//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OFormEventsImportContext
    //=====================================================================
    class OFormEventsImportContext : public XMLEventsImportContext
    {
    protected:
        IEventAttacher&	m_rEventAttacher;

    public:
        OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            IEventAttacher& _rEventAttacher);

    protected:
        virtual void EndElement();
    };

    //=====================================================================
    //= ODefaultEventAttacherManager
    //=====================================================================
    class ODefaultEventAttacherManager : public IEventAttacherManager
    {
    protected:
        DECLARE_STL_MAP(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >,				// map from
            ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >,		// map to
            OPropertySetCompare,																	// compare method
            MapPropertySet2ScriptSequence);															// class name
        // usually an event attacher manager will need to collect all script events registered, 'cause
        // the _real_ XEventAttacherManager handles it's events by index, but out indicies are not fixed
        // until _all_ controls have been inserted.

        MapPropertySet2ScriptSequence	m_aEvents;

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
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_EVENTIMPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
