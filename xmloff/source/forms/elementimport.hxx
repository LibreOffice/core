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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_ELEMENTIMPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_ELEMENTIMPORT_HXX

#include <sal/config.h>

#include <map>

#include "propertyimport.hxx"
#include "controlelement.hxx"
#include "valueproperties.hxx"
#include "eventimport.hxx"
#include "logging.hxx"
#include "property_description.hxx"

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>

class XMLTextStyleContext;
namespace xmloff
{

    class OFormLayerXMLImport_Impl;

    //= OElementNameMap
    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e);

    /** helper class which allows fast translation of xml tag names into element types.
    */
    class OElementNameMap : public OControlElement
    {
    protected:
        typedef std::map<OUString, ElementType> MapString2Element;
        static MapString2Element    s_sElementTranslations;

    protected:
        OElementNameMap() { }

    public:
        static ElementType getElementType(const OUString& _rName);
    };

    //= OElementImport
    /** implements common behaviour for importing forms, controls and columns
    */
    class OElementImport
                :public OPropertyImport
                ,public IEventAttacher
                ,public OStackedLogging
    {
    protected:
        OUString             m_sServiceName;     // the service name as extracted from the service-name attribute
        OUString             m_sName;            // the name of the object (redundant, already contained in the base class' array)
        OFormLayerXMLImport_Impl&   m_rFormImport;      // the form import context
        IEventAttacherManager&      m_rEventManager;    // the event attacher manager

        const XMLTextStyleContext*  m_pStyleElement;    // the XML element which describes the style we encountered
                                                        // while reading our element

        /// the parent container to insert the new element into
        css::uno::Reference< css::container::XNameContainer >
                                    m_xParentContainer;

        /// the element we're creating. Valid after StartElement
        css::uno::Reference< css::beans::XPropertySet >
                                    m_xElement;
        css::uno::Reference< css::beans::XPropertySetInfo >
                                    m_xInfo;

        bool                        m_bImplicitGenericAttributeHandling;

    public:
        /** ctor
            @param _rImport
                the importer
            @param _rEventManager
                the event attacher manager for the control being imported
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
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer
        );
        virtual ~OElementImport();

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    EndElement() override;

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;

        // IEventAttacher
        virtual void registerEvents(
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& _rEvents
            ) override;

        /** create the (uninitialized) element which is to represent the read data

            <p>The default implementation uses <member>m_xORB</member> to create a object with <member>m_sServiceName</member>.
        */
        virtual css::uno::Reference< css::beans::XPropertySet >
                        createElement();

    protected:
        /** can be used to handle properties where the attribute default and the property default differ.
            <p>In such case, if the property had the attribute default upon writing, nothing is read, so upon reading,
            the property is still at it's own default (which is not the attribute default).<p/>
            <p>This method, if told the attribute and the property, and the (implied) attribute default, sets the
            property value as if the attribute was encountered.</p>
            @see encounteredAttribute
        */
        void        simulateDefaultedAttribute(const sal_Char* _pAttributeName, const OUString& _rPropertyName, const sal_Char* _pAttributeDefault);

        /** to be called from within handleAttribute, checks whether the given attribute is covered by our generic
            attribute handler mechanisms
        */
        bool        tryGenericAttribute( sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue );

        /** controls whether |handleAttribute| implicitly calls |tryGenericAttribute|, or whether the derived class
            must do this explicitly at a suitable place in its own |handleAttribute|
        */
        void        disableImplicitGenericAttributeHandling() { m_bImplicitGenericAttributeHandling = false; }

    private:
        OUString implGetDefaultName() const;
        void implApplyGenericProperties();
        void implApplySpecificProperties();

        PropertyGroups::const_iterator impl_matchPropertyGroup( const PropertyGroups& i_propertyGroups ) const;

        virtual OUString determineDefaultServiceName() const;
    };

    //= OControlImport
    /** helper class for importing the description of a single control
    */
    class OControlImport
                :public OElementImport
                ,public OValuePropertiesMetaData
    {
    protected:
        OUString                 m_sControlId;
        OControlElement::ElementType    m_eElementType;

        PropertyValueArray              m_aValueProperties;
        // the value properties (value, current-value, min-value, max-value) require some special
        // handling

        // we fake the attributes our base class gets: we add the attributes of the outer wrapper
        // element which encloses us
        css::uno::Reference< css::xml::sax::XAttributeList >
                                        m_xOuterAttributes;

        /** the address of the calc cell which the control model should be bound to,
            if applicable
        */
        OUString                 m_sBoundCellAddress;

        /** name of a value binding (xforms:bind attribute) */
        OUString                 m_sBindingID;

        /** name of a list binding (form:xforms-list-source attribute) */
        OUString                 m_sListBindingID;

        /** name of a submission (xforms:submission attribute) */
        OUString                 m_sSubmissionID;

    protected:
        // for use by derived classes only
        OControlImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer
            );

    public:
        OControlImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    EndElement() override;

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;

        void addOuterAttributes(const css::uno::Reference< css::xml::sax::XAttributeList >& _rxOuterAttribs);

    protected:
        void setElementType(OControlElement::ElementType _eType) { m_eElementType = _eType; }

    protected:
        static void implTranslateValueProperty(
            const css::uno::Reference< css::beans::XPropertySetInfo >& _rxPropInfo,
            css::beans::PropertyValue& /* [in/out] */ _rPropValue);

        virtual OUString determineDefaultServiceName() const override;

        /** registers the given cell address as value binding address for our element

            <p>The default implementation simply calls registerCellValueBinding at our import
            context, but you may want to override this behaviour.</p>

            @param _rBoundCellAddress
                the cell address to register for our element. Must not be <NULL/>.
            @precond
                we have a valid element (m_xElement)
        */
        virtual void doRegisterCellValueBinding( const OUString& _rBoundCellAddress );

        /** register the given XForms binding */
        void doRegisterXFormsValueBinding( const OUString& );

        /** register the given XForms list binding */
        void doRegisterXFormsListBinding( const OUString& );

        /** register the given XForms submission */
        void doRegisterXFormsSubmission( const OUString& );

    protected:

        // OElementImport overridables
        virtual css::uno::Reference< css::beans::XPropertySet >
                        createElement() override;
    };

    // TODO:
    // this whole mechanism doesn't scale. Instead of deriving even more classes for every new attribute,
    // we should have dedicated attribute handlers
    // The rest of xmloff implements it this way - why don't we do, too?

    //= OImagePositionImport
    class OImagePositionImport : public OControlImport
    {
        sal_Int16   m_nImagePosition;
        sal_Int16   m_nImageAlign;
        bool    m_bHaveImagePosition;

    public:
        OImagePositionImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;

        // OPropertyImport overridables
        virtual bool    handleAttribute( sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue
       ) override;
    };

    //= OReferredControlImport
    class OReferredControlImport : public OControlImport
    {
    protected:
        OUString m_sReferringControls;   // the list of ids of controls referring to the one being imported

    public:
        OReferredControlImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;
    };

    //= OPasswordImport
    class OPasswordImport : public OControlImport
    {
    public:
        OPasswordImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;
    };

    //= ORadioImport
    class ORadioImport : public OImagePositionImport
    {
    public:
        ORadioImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;
    };

    //= OURLReferenceImport
    /** a specialized version of the <type>OControlImport</type> class, which is able
        to handle attributes which denote URLs (and stored relative)
    */
    class OURLReferenceImport : public OImagePositionImport
    {
    public:
        OURLReferenceImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;
    };

    //= OButtonImport
    /** A specialized version of the <type>OControlImport</type> class, which handles
        the target frame for image and command buttons
    */
    class OButtonImport : public OURLReferenceImport
    {
    public:
        OButtonImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
    };

    //= OValueRangeImport
    /** A specialized version of the <type>OControlImport</type> class, which imports
        the value-range elements
    */
    class OValueRangeImport : public OControlImport
    {
    private:
        sal_Int32   m_nStepSizeValue;

    public:
        OValueRangeImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList ) override;

        // OPropertyImport overridables
        virtual bool    handleAttribute( sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue ) override;
    };

    //= OTextLikeImport
    /** A specialized version of the <type>OControlImport</type> class, which handles
        text like controls which have the convert-empty-to-null attribute</p>
    */
    class OTextLikeImport : public OControlImport
    {
    private:
        css::uno::Reference< css::text::XTextCursor >   m_xCursor;
        css::uno::Reference< css::text::XTextCursor >   m_xOldCursor;
        bool                                                                    m_bEncounteredTextPara;

    public:
        OTextLikeImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    EndElement() override;

    private:
        void    adjustDefaultControlProperty();
        void    removeRedundantCurrentValue();
    };

    //= OListAndComboImport
    /** A specialized version of the <type>OControlImport</type> class, which handles
        attributes / sub elements which are special to list and combo boxes
    */
    class OListAndComboImport : public OControlImport
    {
        friend class OListOptionImport;
        friend class OComboItemImport;

    protected:
        css::uno::Sequence< OUString >
                        m_aListSource;
        css::uno::Sequence< OUString >
                        m_aValueList;

        css::uno::Sequence< sal_Int16 >
                        m_aSelectedSeq;
        css::uno::Sequence< sal_Int16 >
                        m_aDefaultSelectedSeq;

        OUString m_sCellListSource;      /// the cell range which acts as list source for the control

        sal_Int32       m_nEmptyListItems;      /// number of empty list items encountered during reading
        sal_Int32       m_nEmptyValueItems;     /// number of empty value items encountered during reading

        bool        m_bEncounteredLSAttrib;
        bool        m_bLinkWithIndexes;     /** <TRUE/> if and only if we should use a cell value binding
                                                    which exchanges the selection index (instead of the selection text
                                                */

    public:
        OListAndComboImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType
        );

        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    EndElement() override;

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;

        // OControlImport ovrridables
        virtual void doRegisterCellValueBinding( const OUString& _rBoundCellAddress ) override;

    protected:
        void implPushBackLabel(const OUString& _rLabel);
        void implPushBackValue(const OUString& _rValue);

        void implEmptyLabelFound();
        void implEmptyValueFound();

        void implSelectCurrentItem();
        void implDefaultSelectCurrentItem();
    };
    typedef tools::SvRef<OListAndComboImport> OListAndComboImportRef;

    //= OListOptionImport
    /** helper class for importing a single &lt;form:option&gt; element.
    */
    class OListOptionImport
                :public SvXMLImportContext
    {
        OListAndComboImportRef  m_xListBoxImport;

    public:
        OListOptionImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rName,
            const OListAndComboImportRef& _rListBox);

        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
    };

    //= OComboItemImport
    /** helper class for importing a single &lt;form:item&gt; element.
    */
    class OComboItemImport
                :public SvXMLImportContext
    {
        OListAndComboImportRef  m_xListBoxImport;

    public:
        OComboItemImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rName,
            const OListAndComboImportRef& _rListBox);

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
    };

    //= OContainerImport
    // BASE must be a derivee of OElementImport
    template <class BASE>
    class OContainerImport
                :public BASE
                ,public ODefaultEventAttacherManager
    {
    protected:
        css::uno::Reference< css::container::XNameContainer >
                        m_xMeAsContainer;
        OUString m_sWrapperElementName;

    protected:
        OContainerImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
                const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
                const sal_Char* _pWrapperElementName)
            :BASE(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
            ,m_sWrapperElementName(OUString::createFromAscii(_pWrapperElementName))
        {
        }

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) SAL_OVERRIDE;
        virtual void EndElement() SAL_OVERRIDE;

    protected:
        // OElementImport overridables
        virtual css::uno::Reference< css::beans::XPropertySet >
                        createElement() SAL_OVERRIDE;

        // create the child context for the given control type
        virtual SvXMLImportContext* implCreateControlWrapper(
            sal_uInt16 _nPrefix, const OUString& _rLocalName) = 0;
    };

    //= OColumnImport
    /** helper class importing a single grid column (without the &lt;form:column&gt; element wrapping
        the column).

        <p>BASE (the template argument) must be a derivee of OControlImport</p>
    */
    template <class BASE>
    class OColumnImport : public BASE
    {
    protected:
        css::uno::Reference< css::form::XGridColumnFactory >
                    m_xColumnFactory;

    public:
        OColumnImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
                const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
                OControlElement::ElementType _eType);

    protected:
        // OElementImport overridables
        virtual css::uno::Reference< css::beans::XPropertySet >
                        createElement() SAL_OVERRIDE;
    };

    //= OColumnWrapperImport
    class OColumnWrapperImport : public SvXMLImportContext
    {
    protected:
        css::uno::Reference< css::xml::sax::XAttributeList >
                                m_xOwnAttributes;
        css::uno::Reference< css::container::XNameContainer >
                                m_xParentContainer;
        OFormLayerXMLImport_Impl&   m_rFormImport;
        IEventAttacherManager&  m_rEventManager;

    public:
        OColumnWrapperImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
                const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer);

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
    protected:
        OControlImport* implCreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            OControlElement::ElementType _eType);
    };

    //= OGridImport
    typedef OContainerImport< OControlImport >  OGridImport_Base;
    /** helper class importing a single &lt;form:grid&gt; element
    */
    class OGridImport : public OGridImport_Base
    {
    public:
        OGridImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType);

    protected:
        // OContainerImport overridables
        virtual SvXMLImportContext* implCreateControlWrapper(
            sal_uInt16 _nPrefix, const OUString& _rLocalName) override;
    };

    //= OFormImport
    typedef OContainerImport< OElementImport >  OFormImport_Base;
    /** helper class importing a single &lt;form:form&gt; element
    */
    class OFormImport : public OFormImport_Base
    {
    public:
        OFormImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer
        );

    protected:
        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    StartElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList) override;
        virtual void    EndElement() override;

        // OContainerImport overridables
        virtual SvXMLImportContext* implCreateControlWrapper(
            sal_uInt16 _nPrefix, const OUString& _rLocalName) override;

        // OPropertyImport overridables
        virtual bool    handleAttribute(sal_uInt16 _nNamespaceKey,
            const OUString& _rLocalName,
            const OUString& _rValue) override;

        OControlImport* implCreateChildContext(
                sal_uInt16 _nPrefix, const OUString& _rLocalName,
                OControlElement::ElementType _eType );

        virtual OUString determineDefaultServiceName() const override;
        void implTranslateStringListProperty(const OUString& _rPropertyName, const OUString& _rValue);
    };

    //= OXMLDataSourceImport
    class OXMLDataSourceImport : public SvXMLImportContext
    {
    public:
        OXMLDataSourceImport( SvXMLImport& _rImport
                    ,sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList
                    ,const css::uno::Reference< css::beans::XPropertySet >& _xElement);
    };

#define _INCLUDING_FROM_ELEMENTIMPORT_HXX_
#include "elementimport_impl.hxx"
#undef _INCLUDING_FROM_ELEMENTIMPORT_HXX_

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_ELEMENTIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
