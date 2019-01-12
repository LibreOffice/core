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

#ifndef INCLUDED_XMLOFF_FORMLAYEREXPORT_HXX
#define INCLUDED_XMLOFF_FORMLAYEREXPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace awt {
    class XControlModel;
} } } }

namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

class SvXMLExport;

namespace xmloff
{


    class OFormLayerXMLExport_Impl;
    class OFormsRootExport;


    //= OFormLayerXMLExport

    /** provides functionality for exporting a complete form layer.
    */
    class XMLOFF_DLLPUBLIC OFormLayerXMLExport
                :public ::salhelper::SimpleReferenceObject
    {
        // impl class
        std::unique_ptr<OFormLayerXMLExport_Impl> m_pImpl;

    protected:
        virtual ~OFormLayerXMLExport() override;

    public:
        OFormLayerXMLExport(SvXMLExport& _rContext);

        /** initializes some internal structures for fast access to the given page

            <p>This method has to be called before you use getControlId for controls on the given page.
            This way a performance optimization can be done for faster access to the control ids</p>

            @return
                <TRUE/> if the page has been examined before. If <FALSE/> is returned, this is a serious error.

            @see getControlId
            @see examineForms
        */
        bool seekPage(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** get the id for the given control

            <p>The page the control belongs to must have been examined and sought to.</p>

            @param _rxControl
                the control which's id should be retrieved. Must not be <NULL/>.

            @see examineForms
            @see seekPage

        */
        OUString getControlId(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl);

        /** retrieves the style name for the control's number style.

            <p>For performance reasons, this method is allowed to be called for any controls, even those which
            do not have a number style. In this case, an empty string is returned.</p>

            @param _rxControl
                the control which's id should be retrieved. Must not be <NULL/>.

            @see examineForms
            @see seekPage
        */
        OUString getControlNumberStyle(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControl );

        /** examines the forms collection given.

            <p>This method will collect all form layer related data of the given draw page</p>

            @param _rxDrawPage
                the draw page to examine. The object will be queried for a com.sun.star.form::XFormsSupplier
                interface to obtain the forms container.
        */
        void examineForms(const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** exports the structure of a form layer

            <p>This method does not export styles (e.g. font properties of the controls), or any references
            external instances may have to the control models contained in the form layer (e.g. uno control
            shapes in the drawing layer may have such references)</p>

            <p>No top level element describing the whole collection is inserted. I.e. if within your document, you
            expect the forms collection to be stored like
                <listing>
                    &lt;Forms&gt;
                        ....    // all the forms stuff here
                    &lt;/Forms&gt;
                </listing>
            you have to start the Forms element yourself.</p>

            @param  _rxDrawPage
                the draw page to examine. The object will be queried for a com.sun.star.form::XFormsSupplier
                interface to obtain the forms container.
        */
        void exportForms(const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** exports the XForms model data
        */
        void exportXForms() const;

        /** determines whether the given page contains logical forms
        */
        static bool pageContainsForms( const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage );

        /** determines whether the given page contains XForm instances
        */
        bool documentContainsXForms() const;

        /** exports the automatic controls number styles
        */
        void exportAutoControlNumberStyles();

        /** exports the auto-styles collected during the examineForms calls
        */
        void exportAutoStyles();

        /** exclude the given control (model) from export.

            <p>If your document contains form controls which are not to be exported for whatever reason,
            you need to announce the models of these controls (can be retrieved from XControlShape::getControl)
            to the form layer exporter.<br/>
            Of course you have to do this before calling <member>exportForms</member></p>
        */
        void excludeFromExport( const css::uno::Reference< css::awt::XControlModel >& _rxControl );
    };


    //= OOfficeFormsExport

    /// export helper for the office::forms element
    class XMLOFF_DLLPUBLIC OOfficeFormsExport
    {
    private:
        std::unique_ptr<OFormsRootExport>  m_pImpl;

    public:
        OOfficeFormsExport( SvXMLExport& _rExp );
        ~OOfficeFormsExport();
    };


}   // namespace xmloff


#endif // INCLUDED_XMLOFF_FORMLAYEREXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
