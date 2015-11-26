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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_LAYERIMPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_LAYERIMPORT_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <rtl/ref.hxx>
#include "formattributes.hxx"
#include "callbacks.hxx"
#include "eventimport.hxx"
#include <tools/ref.hxx>

class SvXMLImport;
class SvXMLImportContext;
class SvXMLStyleContext;
class SvXMLStylesContext;

    // unfortunately, we can't put this into our namespace, as the macro expands to (amongst others) a forward
    // declaration of the class name, which then would be in the namespace, too

namespace xmloff
{

    class OAttribute2Property;

    //= OFormLayerXMLImport_Impl
    class OFormLayerXMLImport_Impl
                : public ODefaultEventAttacherManager
    {
        friend class OFormLayerXMLImport;

    protected:
        SvXMLImport&                        m_rImporter;
        OAttribute2Property                 m_aAttributeMetaData;

        /// the supplier for the forms of the currently imported page
        css::uno::Reference< css::form::XFormsSupplier2 >
                                            m_xCurrentPageFormsSupp;
        SvXMLStylesContext*                 m_pAutoStyles;

    protected:
        typedef std::map< OUString, css::uno::Reference< css::beans::XPropertySet > > MapString2PropertySet;
        typedef std::map<css::uno::Reference<css::drawing::XDrawPage>, MapString2PropertySet, ODrawPageCompare> MapDrawPage2Map;

        MapDrawPage2Map         m_aControlIds;          // ids of the controls on all known page
        MapDrawPage2Map::iterator m_aCurrentPageIds;      // ifs of the controls on the current page

        typedef ::std::pair< css::uno::Reference< css::beans::XPropertySet >, OUString >
                                ModelStringPair;
        ::std::vector< ModelStringPair >
                                m_aControlReferences;   // control reference descriptions for current page
        ::std::vector< ModelStringPair >
                                m_aCellValueBindings;   // information about controls bound to spreadsheet cells
        ::std::vector< ModelStringPair >
                                m_aCellRangeListSources;// information about controls bound to spreadsheet cell range list sources

        ::std::vector< ModelStringPair >
                                m_aXFormsValueBindings; // collect xforms:bind attributes to be resolved

        ::std::vector< ModelStringPair >
                                m_aXFormsListBindings; // collect forms:xforms-list-source attributes to be resolved

        ::std::vector< ModelStringPair >
                                m_aXFormsSubmissions;   // collect xforms:submission attributes to be resolved

    public:
        // IControlIdMap
        void    registerControlId(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl,
            const OUString& _rId);
        void    registerControlReferences(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl,
            const OUString& _rReferringControls);

        // OFormLayerXMLImport_Impl
        inline OAttribute2Property&         getAttributeMap()   { return m_aAttributeMetaData; }
        inline SvXMLImport&                 getGlobalContext()  { return m_rImporter; }
        const SvXMLStyleContext*            getStyleElement(const OUString& _rStyleName) const;
        void                                enterEventContext();
        void                                leaveEventContext();
        void                                applyControlNumberStyle(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rControlNumerStyleName
        );
        void                        registerCellValueBinding(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rCellAddress
        );

        void                        registerCellRangeListSource(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rCellRangeAddress
        );

        void                        registerXFormsValueBinding(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rBindingID
        );

        void                        registerXFormsListBinding(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rBindingID
        );

        void                        registerXFormsSubmission(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rSubmissionID
        );

        virtual ~OFormLayerXMLImport_Impl();
    protected:
        explicit OFormLayerXMLImport_Impl(SvXMLImport& _rImporter);

        /** start importing the forms of the given page
        */
        void startPage(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** end importing the forms of the current page
        */
        void endPage();

        /** creates an import context for the office:forms element
        */
        static SvXMLImportContext* createOfficeFormsContext(
            SvXMLImport& _rImport,
            sal_uInt16 _nPrefix,
            const OUString& _rLocalName);

        /** create an <type>SvXMLImportContext</type> instance which is able to import the &lt;form:form&gt;
            element.
        */
        SvXMLImportContext* createContext(
            const sal_uInt16 _nPrefix,
            const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttribs);

        /** get the control with the given id
        */
        css::uno::Reference< css::beans::XPropertySet >
                lookupControlId(const OUString& _rControlId);

        /** announces the auto-style context to the form importer
        */
        void setAutoStyleContext(SvXMLStylesContext* _pNewContext);

        /** to be called when the document has been completely imported

            <p>For some documents (currently: only some spreadsheet documents) it's necessary
            do to a post processing, since not all information from the file can be processed
            if the document is not completed, yet.</p>
        */
        void documentDone( );
    };

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_LAYERIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
