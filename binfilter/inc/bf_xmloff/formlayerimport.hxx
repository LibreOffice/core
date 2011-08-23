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

#ifndef _XMLOFF_FORMLAYERIMPORT_HXX_
#define _XMLOFF_FORMLAYERIMPORT_HXX_

#include <salhelper/simplereferenceobject.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ref.hxx>
#include <bf_xmloff/xmlimppr.hxx>
namespace binfilter {

class SvXMLImport;
class SvXMLImportContext;
class SvXMLStylesContext;
class XMLPropStyleContext;

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OFormLayerXMLImport
    //=====================================================================
    class OFormLayerXMLImport_Impl;
    /** allows you to import a &lt;form:form&gt; element
    */
    class OFormLayerXMLImport
                :public ::salhelper::SimpleReferenceObject
    {
        OFormLayerXMLImport_Impl*	m_pImpl;

    public:
        OFormLayerXMLImport(SvXMLImport& _rImporter);
        ~OFormLayerXMLImport();

        /** retrieves the property mapper form form related auto styles.
        */
        ::rtl::Reference< SvXMLImportPropertyMapper >
                getStylePropertyMapper() const;

        /** start importing the forms of the given page

            <p>starting the import of a new page (by using this method) invalidates the current page position
            you may have set with <method>seekPage</method>.</p>

            @see endPage
            @see seekPage
        */
        void startPage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** creates an import context for the office:forms element
        */
        SvXMLImportContext* createOfficeFormsContext(
            SvXMLImport& _rImport,
            sal_uInt16 _nPrefix,
            const ::rtl::OUString& _rLocalName);

        /** create an <type>SvXMLImportContext</type> instance which is able to import the &lt;form:form&gt;
            element.

            <p>You must have called <method>startPage</method> before you can create such an import context.</p>

            @see startPage
            @see endPage
        */
        SvXMLImportContext* createContext(
            const sal_uInt16 _nPrefix,
            const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttribs);

        /**
        */
        XMLPropStyleContext* createControlStyleContext(
            sal_uInt16 _nPrefix, 
            const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList,
            SvXMLStylesContext& _rParentStyles,
            sal_uInt16 _nFamily = 0,
            sal_Bool _bDefaultStyle = sal_False
        );

        /** lookup a control given by id.

            <p>The control must be part of the page which is currently beeing imported, or you must have used
            seekPage for the page which the control belongs to.</p>

            <p>(And, of course, the control must have been imported already at the moment you call this.)</p>

            @see startPage
            @see seekPage
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                lookupControl(const ::rtl::OUString& _rId);

        /** end importing the forms of the current page

            <p>You must call this method if you want the import to be finished correctly. Without calling
            it, you will lose information.</p>

            @see startPage
            @see seekPage
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rControlNumerStyleName
        );

        /** to be called when the document has been completely imported

            <p>For some documents (currently: only some spreadsheet documents) it's necessary
            do to a post processing, since not all information from the file can be processed
            if the document is not completed, yet.</p>
        */
        void documentDone( );
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMLAYERIMPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
