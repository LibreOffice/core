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

#ifndef _XMLOFF_FORMS_LAYERIMPORT_HXX_
#define _XMLOFF_FORMS_LAYERIMPORT_HXX_

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <rtl/ref.hxx>
#include "formattributes.hxx"
#include "callbacks.hxx"
#include "eventimport.hxx"
#include <tools/ref.hxx>
namespace binfilter {

class SvXMLImport;
class SvXMLImportContext;
class XMLPropertyHandlerFactory;
class SvXMLImportPropertyMapper;
class XMLPropStyleContext;

SV_DECL_REF( SvXMLStylesContext ) //STRIP007;
    // unfortunately, we can't put this into our namespace, as the macro expands to (amongst others) a forward
    // declaration of the class name, which then would be in the namespace, too

//.........................................................................
namespace xmloff
{
//.........................................................................

    class OAttribute2Property;

    //=====================================================================
    //= ControlReference
    //=====================================================================
    /// a structure containing a property set (the referred control) and a string (the list of referring controls)
/*	struct ControlReference
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    xReferredControl;
        ::rtl::OUString
                    sReferringControls;

        ControlReference(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxReferredControl,
                const ::rtl::OUString& _rReferringControls)
            :xReferredControl(_rxReferredControl)
            ,sReferringControls(_rReferringControls)
        {
        }
    };
*/

    //=====================================================================
    //= OFormLayerXMLImport_Impl
    //=====================================================================
    class OFormLayerXMLImport_Impl
                :public OAttributeMetaData
                ,public IControlIdMap
                ,public IFormsImportContext
                ,public ODefaultEventAttacherManager
    {
        friend class OFormLayerXMLImport;

    protected:
        SvXMLImport&						m_rImporter;
        OAttribute2Property					m_aAttributeMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                            m_xForms;	// the forms of the currently imported page
        SvXMLStylesContext*					m_pAutoStyles;

    protected:
        // style handling
        ::rtl::Reference< XMLPropertyHandlerFactory >	m_xPropertyHandlerFactory;
        ::rtl::Reference< SvXMLImportPropertyMapper >	m_xImportMapper;

        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, MapString2PropertySet );
        DECLARE_STL_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >, MapString2PropertySet, ODrawPageCompare, MapDrawPage2Map);

        MapDrawPage2Map			m_aControlIds;			// ids of the controls on all known page
        MapDrawPage2MapIterator	m_aCurrentPageIds;		// ifs of the controls on the current page

        typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, ::rtl::OUString >
                                ModelStringPair;
        ::std::vector< ModelStringPair >
                                m_aControlReferences;	// control reference descriptions for current page
        ::std::vector< ModelStringPair >
                                m_aCellValueBindings;   // information about controls bound to spreadsheet cells
        ::std::vector< ModelStringPair >
                                m_aCellRangeListSources;// information about controls bound to spreadsheet cell range list sources

    public:
        // IControlIdMap
        virtual void	registerControlId(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rId);
        virtual void	registerControlReferences(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rReferringControls);

        // IFormsImportContext
        virtual IControlIdMap&				getControlIdMap();
        virtual OAttribute2Property&		getAttributeMap();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            getServiceFactory();
        virtual SvXMLImport&				getGlobalContext();
        const SvXMLStyleContext*			getStyleElement(const ::rtl::OUString& _rStyleName) const;
        virtual void						enterEventContext();
        virtual void						leaveEventContext();
        void                                applyControlNumberStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rControlNumerStyleName
        );
        virtual void                        registerCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellAddress
        );

        virtual void                        registerCellRangeListSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellRangeAddress
        );

    protected:
        OFormLayerXMLImport_Impl(SvXMLImport& _rImporter);
        ~OFormLayerXMLImport_Impl();

        /** retrieves the property mapper form form related auto styles.
        */
        ::rtl::Reference< SvXMLImportPropertyMapper > getStylePropertyMapper() const;

        /** start importing the forms of the given page
        */
        void startPage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** end importing the forms of the current page
        */
        void endPage();

        /** creates an import context for the office:forms element
        */
        SvXMLImportContext* createOfficeFormsContext(
            SvXMLImport& _rImport,
            sal_uInt16 _nPrefix,
            const ::rtl::OUString& _rLocalName);

        /** create an <type>SvXMLImportContext</type> instance which is able to import the &lt;form:form&gt;
            element.
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

        /** get the control with the given id
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                lookupControlId(const ::rtl::OUString& _rControlId);

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

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_LAYERIMPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
