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

#ifndef _XMLOFF_FORMS_ELEMENTIMPORT_HXX_
#define _XMLOFF_FORMS_ELEMENTIMPORT_HXX_

#ifndef _XMLOFF_FORMS_PROPERTYIMPORT_HXX_
#include "propertyimport.hxx"
#endif
#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#include "controlelement.hxx"
#endif
#ifndef _XMLOFF_FORMS_VALUEPROPERTIES_HXX_
#include "valueproperties.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#include "eventimport.hxx"
#endif
#ifndef XMLOFF_FORMS_LOGGING_HXX
#include "logging.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    class IControlIdMap;
    class IFormsImportContext;
    class OControlStyleContext;

    //=====================================================================
    //= OElementNameMap
    //=====================================================================
    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e);

    /** helper class which allows fast translation of xml tag names into element types.
    */
    class OElementNameMap : public OControlElement
    {
    protected:
        DECLARE_STL_USTRINGACCESS_MAP( ElementType, MapString2Element );
        static MapString2Element	s_sElementTranslations;

    protected:
        OElementNameMap() { }

    public:
        static ElementType getElementType(const ::rtl::OUString& _rName);
    };

    //=====================================================================
    //= OElementImport
    //=====================================================================
    /** implements common behaviour for importing forms, controls and columns
    */
    class OElementImport
                :public OPropertyImport
                ,public OAttributeMetaData
                ,public IEventAttacher
                ,public OStackedLogging
    {
    protected:
        ::rtl::OUString			m_sServiceName;		// the service name as extracted from the service-name attribute
        ::rtl::OUString			m_sName;			// the name of the object (redundant, already contained in the base class' array)
        IFormsImportContext&	m_rFormImport;		// the form import context
        IEventAttacherManager&	m_rEventManager;	// the event attacher manager

        const OControlStyleContext*	m_pStyleElement;	// the XML element which describes the style we encountered
                                                        // while reading our element

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        m_xParentContainer;
            // the parent container to insert the new element into

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xElement;				// the element we're creating. Valid after StartElement

    public:
        /** ctor
            @param _rImport
                the importer
            @param _rEventManager
                the event attacher manager for the control beeing imported
            @param _nPrefix
                the namespace prefix
            @param _rName
                the element name
            @param _rAttributeMap
                the attribute map to be used for translating attributes into properties
            @param _rxParentContainer
                the container in which the new element should be inserted
        */
        OElementImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer
        );

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void	EndElement();

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

        // IEventAttacher
        virtual void registerEvents(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

        /** create the (uninitialized) element which is to represent the read data

            <p>The default implementation uses <member>m_xORB</member> to create a object with <member>m_sServiceName</member>.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();

    protected:
        /** can be used to handle properties where the attribute default and the property default differ.
            <p>In such case, if the property had the attribute default upon writing, nothing is read, so upon reading,
            the property is still at it's own default (which is not the attribute default).<p/>
            <p>This method, if told the attribute and the property, and the (implied) attribute default, sets the
            property value as if the attribute was encountered.</p>
            @see encounteredAttribute
        */
        void		simulateDefaultedAttribute(const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName, const sal_Char* _pAttributeDefault);

    private:
        ::rtl::OUString implGetDefaultName() const;

        /** sets the style properties which have been read for the element (if any)
        */
        void implSetStyleProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject );
    };

    //=====================================================================
    //= OControlImport
    //=====================================================================
    /** helper class for importing the description of a single control
    */
    class OControlImport
                :public OElementImport
                ,public OValuePropertiesMetaData
    {
    protected:
        ::rtl::OUString					m_sControlId;
        OControlElement::ElementType	m_eElementType;

        PropertyValueArray				m_aValueProperties;
        // the value properties (value, current-value, min-value, max-value) require some special
        // handling

        // we fake the attributes our base class gets: we add the attributes of the outer wrapper
        // element which encloses us
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >
                                        m_xOuterAttributes;

        /** the address of the calc cell which the control model should be bound to,
            if applicable
        */
        ::rtl::OUString                 m_sBoundCellAddress;

    protected:
        // for use by derived classes only
        OControlImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer
            );

    public:
        OControlImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void	EndElement();

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

        void addOuterAttributes(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxOuterAttribs);

    protected:
        void setElementType(OControlElement::ElementType _eType) { m_eElementType = _eType; }

    protected:
        void implTranslateValueProperty(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >& _rxPropInfo,
            ::com::sun::star::beans::PropertyValue& /* [in/out] */ _rPropValue);

        /** registers the given cell address as value binding address for our element

            <p>The default implementation simply calls registerCellValueBinding at our import
            context, but you may want to override this behaviour.</p>

            @param _rBoundCellAddress
                the cell address to register for our element. Must not be <NULL/>.
            @precond
                we have a valid element (m_xElement)
        */
        virtual void doRegisterCellValueBinding( const ::rtl::OUString& _rBoundCellAddress );
    
    protected:
        //added by BerryJia for fixing bug102407 2002-11-5
        // OElementImport overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();

    };

    //=====================================================================
    //= OControlWrapperImport
    //=====================================================================
    /** helper class importing a &lt;form:column&gt; or &lt;form:control&gt; element.
    */
    class OControlWrapperImport : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >
                                m_xOwnAttributes;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                m_xParentContainer;
        IFormsImportContext&	m_rFormImport;
        IEventAttacherManager&	m_rEventManager;

    public:
        OControlWrapperImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer);

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

    protected:
        virtual OControlImport* implCreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            OControlElement::ElementType _eType);
    };

    //=====================================================================
    //= OReferredControlImport
    //=====================================================================
    class OReferredControlImport : public OControlImport
    {
    protected:
        ::rtl::OUString	m_sReferringControls;	// the list of ids of controls referring to the one beeing imported

    public:
        OReferredControlImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);
    };

    //=====================================================================
    //= OPasswordImport
    //=====================================================================
    class OPasswordImport : public OControlImport
    {
    public:
        OPasswordImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);
    };

    //=====================================================================
    //= ORadioImport
    //=====================================================================
    class ORadioImport : public OControlImport
    {
    public:
        ORadioImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);
    };

    //=====================================================================
    //= OURLReferenceImport
    //=====================================================================
    /** a specialized version of the <type>OControlImport</type> class, which is able
        to handle attributes which denote URLs (and stored relative)
    */
    class OURLReferenceImport : public OControlImport
    {
    public:
        OURLReferenceImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);
    };

    //=====================================================================
    //= OButtonImport
    //=====================================================================
    /** A specialized version of the <type>OControlImport</type> class, which handles
        the target frame for image and command buttons
    */
    class OButtonImport : public OURLReferenceImport
    {
    public:
        OButtonImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OTextLikeImport
    //=====================================================================
    /** A specialized version of the <type>OControlImport</type> class, which handles
        text like controls which have the convert-empty-to-null attribute</p>
    */
    class OTextLikeImport : public OControlImport
    {
    public:
        OTextLikeImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OListAndComboImport
    //=====================================================================
    /** A specialized version of the <type>OControlImport</type> class, which handles
        attributes / sub elements which are special to list and combo boxes
    */
    class OListAndComboImport : public OControlImport
    {
        friend class OListOptionImport;
        friend class OComboItemImport;

    protected:
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aListSource;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aValueList;

        ::com::sun::star::uno::Sequence< sal_Int16 >
                        m_aSelectedSeq;
        ::com::sun::star::uno::Sequence< sal_Int16 >
                        m_aDefaultSelectedSeq;

        ::rtl::OUString m_sCellListSource;      /// the cell range which acts as list source for the control

        sal_Int32		m_nEmptyListItems;      /// number of empty list items encountered during reading
        sal_Int32		m_nEmptyValueItems;     /// number of empty value items encountered during reading

        sal_Bool		m_bEncounteredLSAttrib;
        sal_Bool        m_bLinkWithIndexes;     /** <TRUE/> if and only if we should use a cell value binding
                                                    which exchanges the selection index (instead of the selection text
                                                */

    public:
        OListAndComboImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void	EndElement();

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

        // OControlImport ovrridables
        virtual void doRegisterCellValueBinding( const ::rtl::OUString& _rBoundCellAddress );

    protected:
        void implPushBackLabel(const ::rtl::OUString& _rLabel);
        void implPushBackValue(const ::rtl::OUString& _rValue);

        void implEmptyLabelFound();
        void implEmptyValueFound();

        void implSelectCurrentItem();
        void implDefaultSelectCurrentItem();
    };
    SV_DECL_IMPL_REF(OListAndComboImport);

    //=====================================================================
    //= OListOptionImport
    //=====================================================================
    /** helper class for importing a single &lt;form:option&gt; element.
    */
    class OListOptionImport
                :public SvXMLImportContext
                ,public OAttributeMetaData
    {
        OListAndComboImportRef	m_xListBoxImport;

    public:
        OListOptionImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const OListAndComboImportRef& _rListBox);

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OComboItemImport
    //=====================================================================
    /** helper class for importing a single &lt;form:item&gt; element.
    */
    class OComboItemImport
                :public SvXMLImportContext
                ,public OAttributeMetaData
    {
        OListAndComboImportRef	m_xListBoxImport;

    public:
        OComboItemImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const OListAndComboImportRef& _rListBox);

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OContainerImport
    //=====================================================================
    // BASE must be a derivee of OElementImport
    template <class BASE>
    class OContainerImport
                :public BASE
                ,public ODefaultEventAttacherManager
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        m_xMeAsContainer;
        ::rtl::OUString	m_sWrapperElementName;

    protected:
        OContainerImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
                const sal_Char* _pWrapperElementName)
            :BASE(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
            ,m_sWrapperElementName(::rtl::OUString::createFromAscii(_pWrapperElementName))
        {
        }

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void EndElement();

    protected:
        // OElementImport overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();

        // create the child context for the given control type
        virtual SvXMLImportContext*	implCreateControlWrapper(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName) = 0;
    };

    //=====================================================================
    //= OColumnImport
    //=====================================================================
    /** helper class importing a single grid column (without the &lt;form:column&gt; element wrapping
        the column).

        <p>BASE (the template argument) must be a derivee of OControlImport</p>
    */
    template <class BASE>
    class OColumnImport : public BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridColumnFactory >
                    m_xColumnFactory;

    public:
        OColumnImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
                OControlElement::ElementType _eType,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxOuterAttribs);

    protected:
        // OElementImport overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();
    };

    //=====================================================================
    //= OColumnWrapperImport
    //=====================================================================
    class OColumnWrapperImport : public OControlWrapperImport
    {
    public:
        OColumnWrapperImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer);
    protected:
        virtual OControlImport* implCreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            OControlElement::ElementType _eType);
    };

    //=====================================================================
    //= OGridImport
    //=====================================================================
    typedef OContainerImport< OControlImport >	OGridImport_Base;
    /** helper class importing a single &lt;form:grid&gt; element
    */
    class OGridImport : public OGridImport_Base
    {
    public:
        OGridImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType);

    protected:
        // OContainerImport overridables
        virtual SvXMLImportContext*	implCreateControlWrapper(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName);
    };

    //=====================================================================
    //= OFormImport
    //=====================================================================
    typedef OContainerImport< OElementImport >	OFormImport_Base;
    /** helper class importing a single &lt;form:form&gt; element
    */
    class OFormImport : public OFormImport_Base
    {
    public:
        OFormImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer
        );

    protected:
        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void	StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void	EndElement();

        // OContainerImport overridables
        virtual SvXMLImportContext*	implCreateControlWrapper(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName);

        // OPropertyImport overridables
        virtual void	handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);


        void implTranslateStringListProperty(const ::rtl::OUString& _rPropertyName, const ::rtl::OUString& _rValue);
    };

#define _INCLUDING_FROM_ELEMENTIMPORT_HXX_
#include "elementimport_impl.hxx"
#undef _INCLUDING_FROM_ELEMENTIMPORT_HXX_

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_ELEMENTIMPORT_HXX_

