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
#include <set>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include "callbacks.hxx"
#include <rtl/ref.hxx>

class SvXMLExport;
class SvXMLNumFmtExport;
class XMLPropertyHandlerFactory;
class SvXMLExportPropertyMapper;

namespace xmloff
{

    typedef ::std::set  <   css::uno::Reference< css::beans::XPropertySet >
                        >   PropertySetBag;

    // maps objects (property sets) to strings, e.g. control ids.
    typedef ::std::map  <   css::uno::Reference< css::beans::XPropertySet >
                        ,   OUString
                        >   MapPropertySet2String;

    // map pages to maps (of property sets to strings)
    typedef ::std::map  <   css::uno::Reference< css::drawing::XDrawPage >
                        ,   MapPropertySet2String
                        >   MapPropertySet2Map;

    //= OFormLayerXMLExport_Impl
    /** the implementation class for OFormLayerXMLExport
    */
    class OFormLayerXMLExport_Impl
                :public IFormsExportContext
    {
        friend class OFormLayerXMLExport;

        SvXMLExport&        m_rContext;
        SvXMLNumFmtExport*  m_pControlNumberStyles;

        // ignore list for control models
        PropertySetBag      m_aIgnoreList;

        // style handling
        ::rtl::Reference< XMLPropertyHandlerFactory >   m_xPropertyHandlerFactory;
        ::rtl::Reference< SvXMLExportPropertyMapper >   m_xStyleExportMapper;

        // we need our own number formats supplier:
        // Controls which have a number formats do not work with the formats supplier of the document they reside
        // in, instead they use the formats of the data source their form is associated with. If there is no
        // such form or no such data source, they work with an own formatter.
        // Even more, time and date fields do not work with a central formatter at all, they have their own one
        // (which is shared internally, but this is a (hidden) implementation detail.)

        // To not contaminate the global (document) number formats supplier (which could be obtained from the context),
        // we have an own one.
        // (Contaminate means: If a user adds a user-defined format to a formatted field, this format is stored in
        // in the data source's formats supplier. To export this _and_ reuse existing structures, we would need to
        // add this format to the global (document) formats supplier.
        // In case of an export we could do some cleanup afterwards, but in case of an import, there is no such
        // chance, as (if other user-defined formats exist in the document as well) we can't distinguish
        // between user-defined formats really needed for the doc (i.e. in a calc cell) and formats only added
        // to the supplier because the controls needed it.
        css::uno::Reference< css::util::XNumberFormats >
                                                    m_xControlNumberFormats;

        MapPropertySet2Map  m_aControlIds;
            // the control ids of all controls on all pages we ever examined

        MapPropertySet2Map  m_aReferringControls;
            // for a given page (iter->first), and a given control (iter->second->first), this is the comma-separated
            // lists of ids of the controls referring to the control given.

        MapPropertySet2Map::iterator
                            m_aCurrentPageIds;
            // the iterator for the control id map for the page being handled
        MapPropertySet2Map::iterator
                            m_aCurrentPageReferring;
            // the same for the map of referring controls

        // TODO: To avoid this construct above, and to have a cleaner implementation, a class encapsulating the
        // export of a single page should be introduced.

        typedef std::map<css::uno::Reference<css::beans::XPropertySet>, sal_Int32> MapPropertySet2Int;
        MapPropertySet2Int  m_aControlNumberFormats;
            // maps controls to format keys, which are relative to our own formats supplier

        MapPropertySet2String   m_aGridColumnStyles;
            // style names of grid columns

    public:
        explicit OFormLayerXMLExport_Impl(SvXMLExport& _rContext);
        virtual ~OFormLayerXMLExport_Impl();

    private:
        /** exports one single grid column
        */
        void    exportGridColumn(
            const css::uno::Reference< css::beans::XPropertySet >& _rxColumn,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            );

        /** exports one single control
        */
        void    exportControl(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            );

        /** exports one single form
        */
        void    exportForm(const css::uno::Reference< css::beans::XPropertySet >& _rxProps,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            );

        /** seek to the page given.

            <p>This must be called before you can retrieve any ids for controls on the page.</p>

            @see
                getControlId
        */
        bool    seekPage(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** get the id of the given control.

            <p>You must have sought to the page of the control before calling this.</p>
        */
        OUString
                getControlId(const css::uno::Reference< css::beans::XPropertySet >& _rxControl);

        /** retrieves the style name for the control's number style.

            <p>For performance reasons, this method is allowed to be called for any controls, even those which
            do not have a number style. In this case, an empty string is returned.</p>
        */
        OUString
                getControlNumberStyle( const css::uno::Reference< css::beans::XPropertySet >& _rxControl );

        // IFormsExportContext
        virtual void                                        exportCollectionElements(const css::uno::Reference< css::container::XIndexAccess >& _rxCollection) override;
        virtual SvXMLExport&                                getGlobalContext() override;
        virtual OUString                             getObjectStyleName(
            const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) override;
        virtual ::rtl::Reference< SvXMLExportPropertyMapper >   getStylePropertyMapper() override;

        /** clear any structures which have been build in the recent <method>examine</method> calls.
        */
        void clear();

        /** examine a forms collection.

            <p>The method will collect control ids and add styles to the export context as necessary.</p>

            <p>Every control in the object hierarchy given will be assigned to a unique id, which is stored for later
            use.</p>

            <p>In addition, any references the controls may have between each other, are collected and stored for
            later use.</p>

            <p>Upon calling this method, the id map will be cleared before collecting the new ids, so any ids
            you collected previously will be lost</p>

            @param _rxDrawPage
                the draw page which's forms collection should be examined

            @see getControlId
            @see exportControl
            @see exportForms
        */
        void examineForms(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** export a forms collection of a draw page

            <p>The method will obtain the forms collection of the page and call
            <method>exportCollectionElements</method>.</p>
        */
        void exportForms(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** exports the XForms model data
        */
        void exportXForms() const;

        /** determines whether the given page contains logical forms
        */
        static bool pageContainsForms( const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage );

        /** determines whether the given page contains XForm instances
        */
        bool documentContainsXForms() const;

        /** exports the automatic control number styles
        */
        void    exportAutoControlNumberStyles();

        /** exports the auto-styles collected during the examineForms calls
        */
        void exportAutoStyles();

        static bool impl_isFormPageContainingForms(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage,
            css::uno::Reference< css::container::XIndexAccess >& _rxForms);

        /** moves the m_aCurrentPage* members to the positions specifying the given page.

            @return <TRUE/> if there already were structures for the given page
        */
        bool implMoveIterators(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage,
            bool _bClear);

        /** check the object given if it's a control, if so, examine it.
            @return <TRUE/> if the object has been handled
        */
        bool checkExamineControl(const css::uno::Reference< css::beans::XPropertySet >& _rxObject);

        /** examines the control's number format, so later the format style can be referred

            <p>remembers the format key for the control, so it can later be asked for in getControlNumberStyle</p>
        */
        void examineControlNumberFormat(const css::uno::Reference< css::beans::XPropertySet >& _rxControl);

        /** examines the control's number format, so later the format style can be referred

            <p>does not remember the information returned in any way</p>
        */
        sal_Int32 implExamineControlNumberFormat( const css::uno::Reference< css::beans::XPropertySet >& _rxObject );

        /** collects AutoStyles for grid columns
        */
        void collectGridColumnStylesAndIds( const css::uno::Reference< css::beans::XPropertySet >& _rxControl );

        /** ensures that the number format of the given control exist in our own formats supplier.

            <p>The given control is examined for its format (i.e. it's FormatKey/FormatsSupplier properties),
            and the format is added (if necessary) to m_xControlNumberFormats</p>.

            @return
                the format key of the control's format relative to our own formats supplier

        */
        sal_Int32   ensureTranslateFormat(const css::uno::Reference< css::beans::XPropertySet >& _rxFormattedControl);

        /// returns the instance exporting our control's number styles
        SvXMLNumFmtExport*  getControlNumberStyleExport();

        /// ensures that the instance exporting our control's number styles exists
        void                ensureControlNumberStyleExport();

        /** determines the number format style for the given object without remembering it
        */
        OUString
                getImmediateNumberStyle( const css::uno::Reference< css::beans::XPropertySet >& _rxObject );

        /** returns the prefix to be used for control number styles
        */
        static const OUString& getControlNumberStyleNamePrefix();

        /** exclude the given control (model) from export.

            <p>If your document contains form controls which are not to be exported for whatever reason,
            you need to announce the models of these controls (can be retrieved from XControlShape::getControl)
            to the form layer exporter.<br/>
            Of course you have to do this before calling <member>exportForms</member></p>
        */
        void excludeFromExport( const css::uno::Reference< css::awt::XControlModel >& _rxControl );
    };

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
