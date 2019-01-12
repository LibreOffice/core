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

#ifndef INCLUDED_XMLOFF_FORMLAYERIMPORT_HXX
#define INCLUDED_XMLOFF_FORMLAYERIMPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace xml { namespace sax { class XAttributeList; } } } } }

class SvXMLImport;
class SvXMLImportContext;
class SvXMLStylesContext;


namespace xmloff
{


    //= OFormLayerXMLImport

    class OFormLayerXMLImport_Impl;
    /** allows you to import a &lt;form:form&gt; element
    */
    class XMLOFF_DLLPUBLIC OFormLayerXMLImport
                :public ::salhelper::SimpleReferenceObject
    {
        std::unique_ptr<OFormLayerXMLImport_Impl>   m_pImpl;

    public:
        OFormLayerXMLImport(SvXMLImport& _rImporter);
        virtual ~OFormLayerXMLImport() override;

        /** start importing the forms of the given page

            <p>starting the import of a new page (by using this method) invalidates the current page position.</p>

            @see endPage
        */
        void startPage(
            const css::uno::Reference< css::drawing::XDrawPage >& _rxDrawPage);

        /** creates an import context for the office:forms element
        */
        static SvXMLImportContext* createOfficeFormsContext(
            SvXMLImport& _rImport,
            sal_uInt16 _nPrefix,
            const OUString& _rLocalName);

        /** create an SvXMLImportContext instance which is able to import the &lt;form:form&gt;
            element.

            <p>You must have called <method>startPage</method> before you can create such an import context.</p>

            @see startPage
            @see endPage
        */
        SvXMLImportContext* createContext(
            const sal_uInt16 _nPrefix,
            const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttribs);

        /** lookup a control given by id.

            <p>The control must be part of the page which is currently being imported.</p>

            <p>(And, of course, the control must have been imported already at the moment you call this.)</p>

            @see startPage
        */
        css::uno::Reference< css::beans::XPropertySet >
                lookupControl(const OUString& _rId);

        /** end importing the forms of the current page

            <p>You must call this method if you want the import to be finished correctly. Without calling
            it, you will lose information.</p>

            @see startPage
        */
        void endPage();

        /** announces the auto-style context to the form importer
        */
        void setAutoStyleContext(SvXMLStylesContext* _pNewContext);

        /** sets the given number style on the given control
            @param _rxControlModel
                the control model which's style is to be set
            @param _rControlNumerStyleName
                the style name for the control's number style
        */
        void applyControlNumberStyle(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const OUString& _rControlNumerStyleName
        );

        /** to be called when the document has been completely imported

            <p>For some documents (currently: only some spreadsheet documents) it's necessary
            do to a post processing, since not all information from the file can be processed
            if the document is not completed, yet.</p>
        */
        void documentDone( );
    };


}   // namespace xmloff


#endif // INCLUDED_XMLOFF_FORMLAYERIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
