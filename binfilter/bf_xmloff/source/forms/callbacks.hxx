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

#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#define _XMLOFF_FORMS_CALLBACKS_HXX_

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <rtl/ref.hxx>
namespace binfilter {

class SvXMLImport;
class SvXMLExport;
class SvXMLExportPropertyMapper;
class SvXMLStyleContext;

//.........................................................................
namespace xmloff
{
//.........................................................................

    class OAttribute2Property;

    //=====================================================================
    //= IFormsExportContext
    //=====================================================================
    class IFormsExportContext
    {
    public:
        virtual SvXMLExport&								getGlobalContext() = 0;
        virtual ::rtl::Reference< SvXMLExportPropertyMapper >	getStylePropertyMapper() = 0;

        /** steps through a collection and exports all children of this collection
        */
        virtual void	exportCollectionElements(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxCollection) = 0;

        virtual ::rtl::OUString								getObjectStyleName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject ) = 0;
    };

    //=====================================================================
    //= IControlIdMap
    //=====================================================================
    class IControlIdMap
    {
    public:
        /** register a control id
        */
        virtual void	registerControlId(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rId) = 0;

        /** register references to a control.

            <p>In the XML representation, the control which is refered by others stores the ids of these other
            controls, but in "real life" :) the referring controls store the referred one as property.</p>

            <p>This method allows a referred control to announce to ids of the referring ones.</p>

            @param _rxControl
                the referred control
            @param _rReferringControls
                a (comma separated) list of control ids of referring controls
        */
        virtual void	registerControlReferences(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rReferringControls) = 0;
    };

    //=====================================================================
    //= IFormsImportContext
    //=====================================================================
    class IFormsImportContext
    {
    public:
        virtual IControlIdMap&				getControlIdMap() = 0;
        virtual OAttribute2Property&		getAttributeMap() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            getServiceFactory() = 0;
        virtual SvXMLImport&				getGlobalContext() = 0;

        virtual const SvXMLStyleContext*	getStyleElement(const ::rtl::OUString& _rStyleName) const = 0;

        virtual void						enterEventContext() = 0;
        virtual void						leaveEventContext() = 0;

        /** applies the given number style to the given object
        */
        virtual void applyControlNumberStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rControlNumerStyleName
        ) = 0;

        /** registers a control model for later binding to a spreadsheet cell value
        */
        virtual void                        registerCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellAddress
        ) = 0;

        /** registers a list-like control model for later binding to a spreadsheet cell range as list source
        */
        virtual void                        registerCellRangeListSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellRangeAddress
        ) = 0;
    };

    //=====================================================================
    //= IEventAttacherManager
    //=====================================================================
    class IEventAttacherManager
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

    //=====================================================================
    //= IEventAttacher
    //=====================================================================
    class IEventAttacher
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_CALLBACKS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
