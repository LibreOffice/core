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

#include <set>

#include "formattributes.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include "callbacks.hxx"
#include "strings.hxx"

enum class BoolAttrFlags {
    DefaultFalse          = 0x00,
    DefaultTrue           = 0x01,
    DefaultVoid           = 0x02,
    InverseSemantics      = 0x04,
};
namespace o3tl {
    template<> struct typed_flags<BoolAttrFlags> : is_typed_flags<BoolAttrFlags, 0x07> {};
}

namespace xmloff
{

    // if sal_True, indicates that the semantic of the property referred by <arg>_pPropertyName</arg>
    // is inverse to the semantic of the XML attribute.<br/>
    // I.e. if the property value is <TRUE/>, <FALSE/> has to be written and vice versa.
    // <p>Be careful with <arg>_bDefault</arg> and <arg>_bInverseSemantics</arg>: if <arg>_bInverseSemantics</arg>
    // is <TRUE/>, the current property value is inverted <em>before</em> comparing it to the default.</p>

    class IFormsExportContext;
    //= OPropertyExport
    /** provides export related tools for attribute handling

        <p>(The name is somewhat misleading. It's not only a PropertyExport, but in real an ElementExport.
        Anyway.)</p>
    */
    class OPropertyExport
    {
    private:
        std::set<OUString>      m_aRemainingProps;
            // see examinePersistence

        void exportRelativeTargetLocation(const OUString& _sPropertyName, CCAFlags _nProperty,bool _bAddType);

    protected:
        IFormsExportContext&    m_rContext;

        const css::uno::Reference< css::beans::XPropertySet >
                                m_xProps;
        const css::uno::Reference< css::beans::XPropertySetInfo >
                                m_xPropertyInfo;
        const css::uno::Reference< css::beans::XPropertyState >
                                m_xPropertyState;

        // caching
        OUString     m_sValueTrue;
        OUString     m_sValueFalse;

    public:
        /** constructs an object capable of handling attributes for export
            @param  _rContext
                the export context to which's attribute list the property translation should be added
            @param  m_xControl
                the property set to be exported
        */
        OPropertyExport(IFormsExportContext& _rContext,
            const css::uno::Reference< css::beans::XPropertySet >& _rxProps);

    protected:
        /** examines a property set given for all properties which's value are to made persistent

            <p>upon return the <method>m_aRemainingProps</method> will be filled with the names of all properties
            which need to be stored</p>
        */
        void examinePersistence();

        template< typename T > void exportRemainingPropertiesSequence(
            css::uno::Any const & value,
            token::XMLTokenEnum eValueAttName);

        void exportRemainingProperties();

        /** indicates that a property has been handled by a derived class, without using the helper methods of this
            class.

            <p>Calling this method is necessary in case you use the suggested mechanism for the generic export of
            properties. This means that you want to use <method>exportRemainingProperties</method>, which exports
            all properties which need to ('cause they haven't been exported with one of the other type-specific
            methods).</p>

            <p>In this case you should call exportedProperty for every property you export yourself, so the property
            will be flagged as <em>already handled</em></p>
        */
        void exportedProperty(const OUString& _rPropertyName)
            { m_aRemainingProps.erase(_rPropertyName); }

        /** add an attribute which is represented by a string property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace
            @param _pPropertyName
                the name of the property to ask the control for
        */
        void exportStringPropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName
        );

        /** add an attribute which is represented by a boolean property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the control for
            @param _nBooleanAttributeFlags
                specifies the default and the "alignment" (inverse semantics) of the boolean property
        */
        void exportBooleanPropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName,
            const BoolAttrFlags _nBooleanAttributeFlags);

        /** add an attribute which is represented by a sal_Int16 property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the control for
            @param _nDefault
                the default of the attribute. See force parameter.
            @param force
                if true and the property is not set or does not contain a sal_Int16,
                then _nDefault is written out.
                if false and the current property value equals _nDefault,
                then no attribute is added.
        */
        void exportInt16PropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName,
            const sal_Int16 _nDefault,
            const bool force = false);

        /** add an attribute which is represented by a sal_Int32 property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the control for
            @param _nDefault
                the default of the attribute. If the current property value equals this default, no
                attribute is added.
        */
        void exportInt32PropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName,
            const sal_Int32 _nDefault);

        /** add an attribute which is represented by an enum property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the control for
            @param _pValueMap
                the map to use when converting the property value to an attribute value
            @param _nDefault
                the default of the attribute. If the current property value equals this default, no
                attribute is added.
        */
        template<typename EnumT>
        void exportEnumPropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName,
            const SvXMLEnumMapEntry<EnumT>* _pValueMap,
            const EnumT _nDefault,
            const bool _bVoidDefault = false)
        {
            exportEnumPropertyAttributeImpl(_nNamespaceKey, _pAttributeName, _rPropertyName,
                            reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(_pValueMap),
                            static_cast<sal_Int16>(_nDefault), _bVoidDefault);
        }
        void exportEnumPropertyAttributeImpl(
            const sal_uInt16 _nNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName,
            const SvXMLEnumMapEntry<sal_uInt16>* _pValueMap,
            const sal_uInt16 _nDefault,
            const bool _bVoidDefault);

        // some very special methods for some very special attribute/property pairs

        /** add the hlink:target-frame attribute to the export context.

            <p>The value of this attribute is extracted from the TargetFrame property of the object given.</p>

            <p>The property needs a special handling because conflicts between the default values for the attribute
            and the property.</p>
        */
        void exportTargetFrameAttribute();

        /** add the form:href attribute to the export context.

            <p>The value of this attribute is extracted from the TargetURL property of the object given.</p>

            <p>The property needs a special handling because the URL's need to be made relative</p>

            <p>If _bAddType is set, an additional xlink:type="simple" attribute is also added.</p>
        */
        void exportTargetLocationAttribute(bool _bAddType) { exportRelativeTargetLocation(PROPERTY_TARGETURL,CCAFlags::TargetLocation,_bAddType); }

        /** add the form:image attribute to the export context.

            <p>The value of this attribute is extracted from the ImageURL property of the object given.</p>

            <p>The property needs a special handling because the URL's need to be made relative</p>
        */
        void exportImageDataAttribute() { exportRelativeTargetLocation(PROPERTY_GRAPHIC, CCAFlags::ImageData, false); }

        /** flag the style properties as 'already exported'

            <p>We don't have style support right now, so the only thing the method does is removing the style-relevant
            properties from the list of yet-to-be-exported properties (<member>m_aRemainingProps</member>)</p>
        */
        void flagStyleProperties();

        /** add an arbitrary attribute extracted from an arbitrary property to the export context

            <p>The current value of the property specified with <arg>_pPropertyName</arg> is taken and converted
            into a string, no matter what type it has. (Okay, there are the usual limitations: We know Date, Datetime,
            double, integer ... to name just a few).</p>

            <p>In case the property value is <NULL/> (void), no attribute is added</p>

            <p>In case the property value is an empty string, and the property is a not allowed to be <NULL/> (void),
            no attribute is added</p>

            <p>In case the property value is a sequence of any type, no attribute is added, 'cause sequences can't be
            transported as attribute. In the debug version, an additional assertion will occur if you nonetheless try
            to do this.</p>

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the object for
        */
        void exportGenericPropertyAttribute(
            const sal_uInt16 _nAttributeNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _pPropertyName);

        /** exports a property value, which is a string sequence, as attribute

            <p>The elements of the string sequence given are quoted and concatenated, with the characters used for
            this to be chosen by the caller</p>

            <p>If you use the quote character, no check (except assertions) is made if one of the list items
            contains the quote character</p>

            <p>If you don't use the quote character, no check (except assertions) is made if one of the list items
            contains the separator character (which would be deadly when reimporting the string)</p>

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the object for
        */
        void exportStringSequenceAttribute(
            const sal_uInt16 _nAttributeNamespaceKey,
            const OUString& _pAttributeName,
            const OUString& _rPropertyName);

        /** determines whether the given property is to be exported

            <p>Currently, the method simply checks whether the property's state is <em>not</em> PropertyState.DEFAULT,
            or whether the property is a dynamic property (i.e. added via an <code>XPropertyContainer</code>).
            So, take care when using the method - the heuristics is not applicable for all properties.</p>
        */
        bool shouldExportProperty( const OUString& i_propertyName ) const;

        /** tries to convert an arbitrary <type scope="com.sun:star.uno">Any</type> into an string

            <p>If the type contained in the Any is not supported, the returned string will be empty. In the
            debug version, an additional assertion occurs.</p>

            @param  _rValue
                the value to convert
        */
        OUString implConvertAny(
            const css::uno::Any& _rValue);

        /**
            @return
                token which can be used in the <code>form:property</code> element's <code>type</code> attribute
                to describe the type of a value.<br/>
                Possible types returned are
                <ul>
                    <li><b>boolean</b>: <arg>_rValue</arg> was interpreted as boolean value before converting
                        it into a string</li>
                    <li><b>float</b>: <arg>_rValue</arg> was interpreted as 64 bit floating point 16bit integer, 32bit integer or 64 bit integer value before
                        converting it into a string</li>
                    <li><b>string</b>: <arg>_rValue</arg> did not need any conversion as it already was a string</li>
                </ul>
                If the type is not convertible, float is returned
        */
        static ::xmloff::token::XMLTokenEnum implGetPropertyXMLType(const css::uno::Type& _rType);

#ifdef DBG_UTIL
                void AddAttribute( sal_uInt16 _nPrefix, const OUString& _rName, const OUString& _rValue );
                void AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, const OUString& _rValue);
                void AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, ::xmloff::token::XMLTokenEnum _eValue );
#else
        //  in the product version, inline this, so it does not cost us extra time calling into our method
        void AddAttribute( sal_uInt16 _nPrefix, const OUString& _rName, const OUString& _rValue )
            { m_rContext.getGlobalContext().AddAttribute( _nPrefix, _rName, _rValue ); }
        void AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, const OUString& _rValue)
            { m_rContext.getGlobalContext().AddAttribute(_nPrefix, _eName, _rValue); }
        void AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, ::xmloff::token::XMLTokenEnum _eValue )
            { m_rContext.getGlobalContext().AddAttribute(_nPrefix, _eName, _eValue); }
#endif

#ifdef DBG_UTIL
    protected:
        /** check a given property set for the existence and type correctness of a given property

            <p>This method is available in the non-product version only.</p>

            @param _rPropertyName
                the name of the property to ask the control model for
            @param _pType
                the expected type of the property. May be NULL, in this case no type check is made.
            @return sal_True, if the property exists and is of the correct type
        */
        void dbg_implCheckProperty(
            const OUString& _rPropertyName,
            const css::uno::Type* _pType);

//      void dbg_implCheckProperty(
//          const char* _rPropertyName,
//          const css::uno::Type* _pType)
//      {
//          dbg_implCheckProperty(OUString::createFromAscii(_rPropertyName), _pType);
//      }
#endif
    };

    //= helper
#ifdef DBG_UTIL
    #define DBG_CHECK_PROPERTY(name, type)  \
        dbg_implCheckProperty(name, &cppu::UnoType<type>::get())

    #define DBG_CHECK_PROPERTY_NO_TYPE(name)    \
        dbg_implCheckProperty(name, nullptr)

    #define DBG_CHECK_PROPERTY_ASCII_NO_TYPE( name ) \
        dbg_implCheckProperty( OUString::createFromAscii( name ), nullptr )
#else
    #define DBG_CHECK_PROPERTY(name, type)
    #define DBG_CHECK_PROPERTY_NO_TYPE(name)
    #define DBG_CHECK_PROPERTY_ASCII_NO_TYPE( name )
#endif

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
