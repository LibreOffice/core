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

#ifndef _XMLOFF_FORMS_PROPERTYIMPORT_HXX_
#define _XMLOFF_FORMS_PROPERTYIMPORT_HXX_

#include <xmloff/xmlictxt.hxx>
#include "formattributes.hxx"
#include <rtl/ref.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "layerimport.hxx"

namespace com { namespace sun { namespace star { namespace util {
    struct Time;
    struct Date;
} } } }

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= PropertyConversion
    //=====================================================================
    class PropertyConversion
    {
    public:
        static ::com::sun::star::uno::Any convertString(
            SvXMLImport& _rImporter,
            const ::com::sun::star::uno::Type& _rExpectedType,
            const ::rtl::OUString& _rReadCharacters,
            const SvXMLEnumMapEntry* _pEnumMap = NULL,
            const sal_Bool _bInvertBoolean = sal_False
        );

        static ::com::sun::star::uno::Type xmlTypeToUnoType( const ::rtl::OUString& _rType );
    };

    class OFormLayerXMLImport_Impl;
    //=====================================================================
    //= OPropertyImport
    //=====================================================================
    /** Helper class for importing property values

        <p>This class imports properties which are stored as attributes as well as properties which
        are stored in </em>&lt;form:properties&gt;</em> elements.</p>
    */
    class OPropertyImport : public SvXMLImportContext
    {
        friend class OSinglePropertyContext;
        friend class OListPropertyContext;

    protected:
        typedef ::std::vector< ::com::sun::star::beans::PropertyValue > PropertyValueArray;
        PropertyValueArray          m_aValues;
        PropertyValueArray          m_aGenericValues;
            // the values which the instance collects between StartElement and EndElement

        DECLARE_STL_STDKEY_SET( ::rtl::OUString, StringSet );
        StringSet                   m_aEncounteredAttributes;

        OFormLayerXMLImport_Impl&       m_rContext;

        sal_Bool                    m_bTrackAttributes;

        // TODO: think about the restriction that the class does not know anything about the object it is importing.
        // Perhaps this object should be known to the class, so setting the properties ('normal' ones as well as
        // style properties) can be done in our own EndElement instead of letting derived classes do this.

    public:
        OPropertyImport(OFormLayerXMLImport_Impl& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName);

        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void Characters(const ::rtl::OUString& _rChars);

    protected:
        /** handle one single attribute.

            <p>This is called for every attribute of the element. This class' implementaion checks if the attribute
            describes a property, if so, it is added to <member>m_aValues</member>.</p>

            <p>All non-property attributes should be handled in derived classes.</p>

            @param _nNamespaceKey
                key of the namespace used in the attribute
            @param _rLocalName
                local (relative to the namespace) attribute name
            @param _rValue
                attribute value
        */
        virtual bool handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

        /** determine if the element imported by the object had an given attribute.
            <p>Please be aware of the fact that the name given must be a local name, i.e. not contain a namespace.
            All form relevant attributes are in the same namespace, so this would be an redundant information.</p>
        */
        sal_Bool    encounteredAttribute(const ::rtl::OUString& _rAttributeName) const;

        /** determine if the element imported by the object had an given attribute.
            <p>Please be aware of the fact that the name given must be a local name, i.e. not contain a namespace.
            All form relevant attributes are in the same namespace, so this would be an redundant information.</p>
        */
        sal_Bool    encounteredAttribute(const sal_Char* _pAttributeName) const { return encounteredAttribute(::rtl::OUString::createFromAscii(_pAttributeName)); }

        /** enables the tracking of the encountered attributes
            <p>The tracking will raise the import costs a little bit, but it's cheaper than
            derived classes tracking this themself.</p>
        */
        void        enableTrackAttributes() { m_bTrackAttributes = sal_True; }

        inline void implPushBackPropertyValue(const ::com::sun::star::beans::PropertyValue& _rProp)
        {
            m_aValues.push_back(_rProp);
        }

        inline void implPushBackPropertyValue( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Any& _rValue )
        {
            m_aValues.push_back( ::com::sun::star::beans::PropertyValue(
                _rName, -1, _rValue, ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) );
        }

        inline void implPushBackGenericPropertyValue(const ::com::sun::star::beans::PropertyValue& _rProp)
        {
            m_aGenericValues.push_back(_rProp);
        }
    };
    SV_DECL_IMPL_REF( OPropertyImport )

    //=====================================================================
    //= OPropertyElementsContext
    //=====================================================================
    /** helper class for importing the &lt;form:properties&gt; element
    */
    class OPropertyElementsContext : public SvXMLImportContext
    {
    protected:
        OPropertyImportRef  m_xPropertyImporter;    // to add the properties

    public:
        OPropertyElementsContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const OPropertyImportRef& _rPropertyImporter);

        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

#if OSL_DEBUG_LEVEL > 0
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void Characters(const ::rtl::OUString& _rChars);
#endif
    };

    //=====================================================================
    //= OSinglePropertyContext
    //=====================================================================
    /** helper class for importing a single &lt;form:property&gt; element
    */
    class OSinglePropertyContext : public SvXMLImportContext
    {
        OPropertyImportRef          m_xPropertyImporter;    // to add the properties

    public:
        OSinglePropertyContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const OPropertyImportRef& _rPropertyImporter);

        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OListPropertyContext
    //=====================================================================
    class OListPropertyContext : public SvXMLImportContext
    {
        OPropertyImportRef                  m_xPropertyImporter;
        ::rtl::OUString                     m_sPropertyName;
        ::rtl::OUString                     m_sPropertyType;
        ::std::vector< ::rtl::OUString >    m_aListValues;

    public:
        OListPropertyContext( SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const OPropertyImportRef& _rPropertyImporter );

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList );

        virtual void EndElement();

        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
    };

    //=====================================================================
    //= OListValueContext
    //=====================================================================
    class OListValueContext : public SvXMLImportContext
    {
        ::rtl::OUString& m_rListValueHolder;

    public:
        OListValueContext( SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            ::rtl::OUString& _rListValueHolder );

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList );
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_PROPERTYIMPORT_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
