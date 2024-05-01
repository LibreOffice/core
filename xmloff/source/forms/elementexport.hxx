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

#include <memory>
#include <o3tl/sorted_vector.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <xmloff/xmlexp.hxx>
#include "propertyexport.hxx"
#include "callbacks.hxx"
#include "controlelement.hxx"
#include "valueproperties.hxx"

class SvXMLElementExport;
namespace xmloff
{

    //= OElementExport
    class OElementExport : public OPropertyExport
    {
        css::uno::Sequence< css::script::ScriptEventDescriptor >
                                m_aEvents;

        std::unique_ptr<SvXMLElementExport> m_pXMLElement;          // XML element doing the concrete startElement etc.

    public:
        OElementExport(IFormsExportContext& _rContext,
            const css::uno::Reference< css::beans::XPropertySet >& _rxProps,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents);
        virtual ~OElementExport();

        void doExport();

    protected:
        /// get the name of the XML element
        virtual OUString getXMLElementName() const = 0;
        /// examine the element we're exporting
        virtual void examine();
        /// export the attributes
        virtual void exportAttributes();
        /// export any sub tags
        virtual void exportSubTags();

        /** exports the events (as script:events tag)
        */
        void exportEvents();

        /** add the service-name attribute to the export context
        */
        virtual void exportServiceNameAttribute();

        /// start the XML element
        virtual void implStartElement(const OUString& _pName);

        /// ends the XML element
        virtual void implEndElement();
    };

    //= OControlExport
    /** Helper class for handling xml elements representing a form control
    */
    class OControlExport
                :public OControlElement
                ,public OValuePropertiesMetaData
                ,public OElementExport
    {
    protected:
        typedef o3tl::sorted_vector<sal_Int16> Int16Set;
            // used below

        OUString                m_sControlId;           // the control id to use when exporting
        OUString                m_sReferringControls;   // list of referring controls (i.e. their id's)
        sal_Int16               m_nClassId;             // class id of the control we're representing
        ElementType             m_eType;                // (XML) type of the control we're representing
        CCAFlags                m_nIncludeCommon;       // common control attributes to include
        DAFlags                 m_nIncludeDatabase;     // common database attributes to include
        SCAFlags                m_nIncludeSpecial;      // special attributes to include
        EAFlags                 m_nIncludeEvents;       // events to include
        BAFlags                 m_nIncludeBindings;     // binding attributes to include

        std::unique_ptr<SvXMLElementExport> m_pOuterElement;        // XML element doing the concrete startElement etc. for the outer element

    public:
        /** constructs an object capable of exporting controls

            <p>You need at least two pre-requisites from outside: The control to be exported needs to have a class id
            assigned, and you need the list control-ids of all the controls referring to this one as LabelControl.<br/>
            This information can't be collected when known only the control itself and not it's complete context.</p>

            @param _rControlId
                the control id to use when exporting the control
            @param _rReferringControls
                the comma-separated list of control-ids of all the controls referring to this one as LabelControl
        */
        OControlExport(IFormsExportContext& _rContext,
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl,
            OUString _sControlId,
            OUString _sReferringControls,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rxEvents);

    protected:
        /// start the XML element
        virtual void implStartElement(const OUString& _pName) override;

        /// ends the XML element
        virtual void implEndElement() override;

        /// get the name of the outer XML element
        virtual OUString getOuterXMLElementName() const;

        // get the name of the XML element
        virtual OUString getXMLElementName() const override;

        /** examine the control. Some kind of CtorImpl.
        */
        virtual void examine() override;

        /// exports the attributes for the outer element
        void exportOuterAttributes();

        /// exports the attributes for the inner element
        void exportInnerAttributes();

        /// export the attributes
        virtual void exportAttributes() override;

        /** writes everything which needs to be represented as sub tag
        */
        void exportSubTags() override;

        /** adds the attributes which are handled via generic IPropertyHandlers

            <p>In the future, this really should be *all* attributes, instead of this shitload of
            hand-crafted code we have currently...</p>
        */
        void exportGenericHandlerAttributes();

        /** adds common control attributes to the XMLExport context given

            <p>The attribute list of the context is not cleared initially, this is the responsibility of the caller.</p>
        */
        void exportCommonControlAttributes();

        /** adds database attributes to the XMLExport context given

            <p>The attribute list of the context is not cleared initially, this is the responsibility of the caller.</p>
        */
        void exportDatabaseAttributes();

        /** adds the XML attributes which are related to binding controls to
            external values and/or list sources
        */
        void exportBindingAttributes();

        /** adds attributes which are special to a control type to the export context's attribute list
        */
        void exportSpecialAttributes();

        /** exports the ListSource property of a control as attribute

            The ListSource property may be exported in different ways: For a ComboBox, it is an attribute
            of the form:combobox element.

            For a ListBox, it's an attribute if the ListSourceType states that the ListBox does <em>not</em>
            display a value list. In case of a value list, the ListSource is not exported, and the pairs of
            StringItem/ValueItem are exported as sub-elements.

            This method does the attribute part: It exports the ListSource property as attribute, not caring
            about whether the object is a ComboBox or a ListBox.
        */
        void exportListSourceAsAttribute();

        /** exports the ListSource property of a control as XML elements

            @see exportListSourceAsAttribute
        */
        void exportListSourceAsElements();

        /** gets a Sequence&lt; sal_Int16 &gt; property value as set of sal_Int16's
            @param _rPropertyName
                the property name to use
            @param _rOut
                out parameter. The set of integers.
        */
        void getSequenceInt16PropertyAsSet(const OUString& _rPropertyName, Int16Set& _rOut);

        /** exports the attribute which descrives a cell value binding of a control
            in a spreadsheet document
        */
        void exportCellBindingAttributes( bool _bIncludeListLinkageType );

        /** exports the attribute(s) which bind this control to XForms */
        void exportXFormsBindAttributes();

        /** exports the attribute(s) which bind the list of a list
            control to XForms */
        void exportXFormsListAttributes();

        /** exports the attribute(s) for an XForms submission */
        void exportXFormsSubmissionAttributes();

        /** exports the attribute which descrives a cell range which acts as list source for
            a list-like control
        */
        void exportCellListSourceRange( );

        /** exports the attribute(s) for the ImagePosition property
        */
        void exportImagePositionAttributes();

        /** determines whether the control we're exporting has an active data binding.

            Bindings which count here are:
            <ul><li>an established connection to a database field</li>
                <li>a binding to an external value supplier (<type scope="css::form::binding">XValueBinding</type>)</li>
            </ul>
        */
        bool controlHasActiveDataBinding() const;

        /** retrieves the string specifying the ListSource of a list or combo box
        */
        OUString getScalarListSourceValue() const;

        /** determines whether the list entries (of a combo or list box) are supplied by the user

            List entries may be
            <ul><li>specified by the user</li>
                <li>specified by an external list source (<type scope="css::form::binding">XListEntrySource</type>)</li>
                <li>obtained from a database query (in various ways)</li>
            </ul>

            In the latter two cases, this method will return <FALSE/>
        */
        bool controlHasUserSuppliedListEntries() const;
    };

    //= OColumnExport
    /** Helper class for exporting a grid column
    */
    class OColumnExport : public OControlExport
    {
    public:
        /** ctor
            @see OColumnExport::OColumnExport
        */
        OColumnExport(IFormsExportContext& _rContext,
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl,
            const OUString& _rControlId,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rxEvents);

        virtual ~OColumnExport() override;

    protected:
        // OControlExport overridables
        virtual OUString getOuterXMLElementName() const override;
        virtual void exportServiceNameAttribute() override;
        virtual void exportAttributes() override;

        // OElementExport overridables
        virtual void examine() override;
    };

    //= OFormExport
    /** Helper class for handling xml elements representing a form

        <p>In opposite to the class <type>OControlExport</type>, OFormExport is unable to export a <em>complete</em>
        form. Instead the client has to care for sub elements of the form itself.</p>
    */
    class OFormExport
                :public OControlElement
                ,public OElementExport
    {
        bool m_bCreateConnectionResourceElement;
    public:
        /** constructs an object capable of exporting controls
        */
        OFormExport(IFormsExportContext& _rContext,
            const css::uno::Reference< css::beans::XPropertySet >& _rxForm,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rxEvents
            );

    protected:
        virtual OUString getXMLElementName() const override;
        virtual void exportSubTags() override;
        virtual void exportAttributes() override;
    };
}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
