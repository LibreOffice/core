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

#ifndef _XMLOFF_FORMS_LAYEREXPORT_HXX_
#define _XMLOFF_FORMS_LAYEREXPORT_HXX_

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include "callbacks.hxx"
#include "ifacecompare.hxx"
#include <rtl/ref.hxx>

class SvXMLExport;
class SvXMLNumFmtExport;
class XMLPropertyHandlerFactory;
class SvXMLExportPropertyMapper;

namespace xmloff
{

    typedef ::std::set  <   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        ,   OPropertySetCompare
                        >   PropertySetBag;

    // maps objects (property sets) to strings, e.g. control ids.
    typedef ::std::map  <   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        ,   OUString
                        ,   OPropertySetCompare
                        >   MapPropertySet2String;

    // map pages to maps (of property sets to strings)
    typedef ::std::map  <   ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        ,   MapPropertySet2String
                        ,   ODrawPageCompare
                        >   MapPropertySet2Map;

    //= OFormLayerXMLExport_Impl
    /** the implementation class for OFormLayerXMLExport
    */
    class OFormLayerXMLExport_Impl
                :public IFormsExportContext
    {
        friend class OFormLayerXMLExport;

    protected:
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
        //
        // To not contaminate the global (document) number formats supplier (which could be obtained from the context),
        // we have an own one.
        // (Contaminate means: If a user adds a user-defined format to a formatted field, this format is stored in
        // in the data source's formats supplier. To export this _and_ reuse existing structures, we would need to
        // add this format to the global (document) formats supplier.
        // In case of an export we could do some cleanup afterwards, but in case of an import, there is no such
        // chance, as (if other user-defined formats exist in the document as well) we can't distinguish
        // between user-defined formats really needed for the doc (i.e. in a calc cell) and formats only added
        // to the supplier because the controls needed it.
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >
                                                    m_xControlNumberFormats;

        MapPropertySet2Map  m_aControlIds;
            // the control ids of all controls on all pages we ever examined

        MapPropertySet2Map  m_aReferringControls;
            // for a given page (iter->first), and a given control (iter->second->first), this is the comma-separated
            // lists of ids of the controls refering to the control given.

        MapPropertySet2Map::iterator
                            m_aCurrentPageIds;
            // the iterator for the control id map for the page beeing handled
        MapPropertySet2Map::iterator
                            m_aCurrentPageReferring;
            // the same for the map of referring controls

        // TODO: To avoid this construct above, and to have a cleaner implementation, an class encapsulating the
        // export of a single page should be introduced.

        DECLARE_STL_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, sal_Int32, OPropertySetCompare, MapPropertySet2Int);
        MapPropertySet2Int  m_aControlNumberFormats;
            // maps controls to format keys, which are relative to our own formats supplier

        MapPropertySet2String   m_aGridColumnStyles;
            // style names of grid columns

    public:
        OFormLayerXMLExport_Impl(SvXMLExport& _rContext);
        virtual ~OFormLayerXMLExport_Impl();

    protected:
        /** exports one single grid column
        */
        void    exportGridColumn(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

        /** exports one single control
        */
        void    exportControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

        /** exports one single form
        */
        void    exportForm(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

        /** seek to the page given.

            <p>This must be called before you can retrieve any ids for controls on the page.</p>

            @see
                getControlId
        */
        sal_Bool    seekPage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** get the id of the given control.

            <p>You must have sought to the page of the control before calling this.</p>
        */
        OUString
                getControlId(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl);

        /** retrieves the style name for the control's number style.

            <p>For performance reasons, this method is allowed to be called for any controls, even those which
            do not have a number style. In this case, an empty string is returned.</p>
        */
        OUString
                getControlNumberStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl );

        // IFormsExportContext
        virtual void                                        exportCollectionElements(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxCollection);
        virtual SvXMLExport&                                getGlobalContext();
        virtual OUString                             getObjectStyleName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject );
        virtual ::rtl::Reference< SvXMLExportPropertyMapper >   getStylePropertyMapper();

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** export a forms collection of a draw page

            <p>The method will obtain the forms collection of the page and call
            <method>exportCollectionElements</method>.</p>
        */
        void exportForms(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** exports the XForms model data
        */
        void exportXForms() const;

        /** determines whether the given page contains logical forms
        */
        bool pageContainsForms( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage ) const;

        /** determines whether the given page contains XForm instances
        */
        bool documentContainsXForms() const;

        /** exports the automatic control number styles
        */
        void    exportAutoControlNumberStyles();

        /** exports the auto-styles collected during the examineForms calls
        */
        void exportAutoStyles();

    protected:
        sal_Bool impl_isFormPageContainingForms(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage,
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxForms);

        /** moves the m_aCurrentPage* members to the positions specifying the given page.

            @return <TRUE/> if there already were structures for the given page
        */
        sal_Bool implMoveIterators(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage,
            sal_Bool _bClear);

        /** check the object given if it's a control, if so, examine it.
            @return <TRUE/> if the object has been handled
        */
        sal_Bool checkExamineControl(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

        /** examines the control's number format, so later the format style can be referred

            <p>remembers the format key for the control, so it can later be asked for in getControlNumberStyle</p>
        */
        void examineControlNumberFormat(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl);

        /** examines the control's number format, so later the format style can be referred

            <p>does not remember the information returned in any way</p>
        */
        sal_Int32 implExamineControlNumberFormat( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject );

        /** collects AutoStyles for grid columns
        */
        void collectGridColumnStylesAndIds( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl );

        /** ensures that the number format of the given control exist in our own formats supplier.

            <p>The given control is examined for it's format (i.e. it's FormatKey/FormatsSupplier properties),
            and the format is added (if necessary) to m_xControlNumberFormats</p>.

            @return
                the format key of the control's format relative to our own formats supplier

        */
        sal_Int32   ensureTranslateFormat(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormattedControl);

        /// returns the instance exporting our control's number styles
        SvXMLNumFmtExport*  getControlNumberStyleExport();

        /// ensures that the instance exporting our control's number styles exists
        void                ensureControlNumberStyleExport();

        /** determines the number format style for the given object without remembering it
        */
        OUString
                getImmediateNumberStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject );

        /** returns the prefix to be used for control number styles
        */
        static const OUString& getControlNumberStyleNamePrefix();

        /** exclude the given control (model) from export.

            <p>If your document contains form controls which are not to be exported for whatever reason,
            you need to announce the models of these controls (can be retrieved from XControlShape::getControl)
            to the form layer exporter.<br/>
            Of course you have to do this before calling <member>exportForms</member></p>
        */
        void excludeFromExport( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > _rxControl );
    };

}   // namespace xmloff

#endif // _XMLOFF_FORMS_LAYEREXPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
