/*************************************************************************
 *
 *  $RCSfile: propertyexport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-21 13:38:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#define _XMLOFF_FORMS_PROPERTYEXPORT_HXX_

#ifndef _XMLOFF_FORMATTRIBUTES_HXX_
#include "formattributes.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include "xmlexp.hxx"

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OPropertyExport
    //=====================================================================
    /** provides export related tools for attribute handling
    */
    class OPropertyExport : public OAttributeMetaData
    {
    private:
        DECLARE_STL_STDKEY_SET(::rtl::OUString, StringSet);
        StringSet       m_aRemainingProps;
            // see examinePersistence

    protected:
        SvXMLExport&    m_rContext;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xProps;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                        m_xPropertyInfo;

        // caching
        ::rtl::OUString     m_sValueTrue;
        ::rtl::OUString     m_sValueFalse;

    public:
        /** constructs an object capable of handling attributes for export
            @param  _rContext
                the export context to which's attribute list the property translation should be added
            @param  m_xControl
                the property set to be exported
        */
        OPropertyExport(SvXMLExport& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps);

    protected:
        /** examines a property set given for all properties which's value are to made persistent

            <p>upon return the <method>m_aRemainingProps</method> will be filled with the names of all properties
            which need to be stored</p>
        */
        void examinePersistence();

        /**
        */
        void exportRemainingProperties();

        /** indicates that a property has been by a derived class, without using the helper methods of this class.

            <p>Calling this method is necessary in case you use the suggested mechanism for the generic export of
            properties. This means that you want to use <method>exportRemainingProperties</method>, which exports
            all properties which need to ('cause they haven't been exported with one of the other type-specific
            methods).</p>

            <p>In this case you should call exportedProperty for every property you export yourself, so the property
            will be flagged as <em>already handled</em></p>
        */
        void exportedProperty(const ::rtl::OUString& _rPropertyName)
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
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName);

        /** add an attribute which is represented by a boolean property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the control for
            @param _bDefault
                the default for the attribute. If the current property value equals this default, no
                attribute is added. See also <arg>_bInverseSemantics</arg>
            @param _bInverseSemantics
                if sal_True, indicates that the semantic of the property refered by <arg>_pPropertyName</arg>
                is inverse to the semantic of the XML attribute.<br/>
                I.e. if the property value is <TRUE/>, <FALSE/> has to be written and vice versa.
                <p>Be careful with <arg>_bDefault</arg> and <arg>_bInverseSemantics</arg>: if <arg>_bInverseSemantics</arg>
                is <TRUE/>, the current property value is inverted <em>before</em> comparing it to the default.</p>
        */
        void exportBooleanPropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName,
            const sal_Bool _bDefault,
            const sal_Bool _bInverseSemantics);

        /** add an attribute which is represented by a sal_Int16 property to the export context

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
        void exportInt16PropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName,
            const sal_Int16 _nDefault);

        /** add an attribute which is represented by a enum property to the export context

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
        void exportEnumPropertyAttribute(
            const sal_uInt16 _nNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName,
            const SvXMLEnumMapEntry* _pValueMap,
            const sal_Int32 _nDefault);

        // some very special methods for some very special attribute/property pairs

        /** add the service-name attribute to the export context
        */
        void exportServiceNameAttribute();

        /** add the hlink:xref attribute to the export context.

            <p>The value of this attribute is extracted from the TargetFrame property of the object given.</p>

            <p>The property needs a special handling because conflicts between the default values for the attribute
            and the property.</p>
        */
        void exportTargetFrameAttribute();

        /** just a dummy at the moment.

            <p>We don't have style support right now, so the only thing the method does is removing the style-relevant
            properties from the list of yet-to-be-exported properties (<member>m_aRemainingProps</member>)</p>
        */
        void implExportStyleReference();

        /** add an arbitrary attribute extracted from an arbitrary property to the export context

            <p>The current value of the property specified wiht <arg>_pPropertyName</arg> is taken and converted
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
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName);

        /** exports a property value, which is a string sequence, as attribute

            <p>The elements of the string sequence given are quoted and concatenated, with the characters used for
            this to be choosen by the caller</p>

            <p>If you use the quote character, no check (except assertions) is made if one of the list items
            containes the quote character</p>

            <p>If you don't use the quote character, no check (except assertions) is made if one of the list items
            containes the separator character (which would be deadly when reimporting the string)</p>

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace (it's added automatically)
            @param _pPropertyName
                the name of the property to ask the object for
            @param _aQuoteCharacter
                the character to use to quote the sequence elements with. May be 0, in this case no quoting happens
            @param _aListSeparator
                the character to use to separate the list entries
        */
        void exportStringSequenceAttribute(
            const sal_uInt16 _nAttributeNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName,
            const sal_Unicode _aQuoteCharacter = '"',
            const sal_Unicode _aListSeparator = ',');

        /** tries to convert an arbitrary <type scope="com.sun:star.uno">Any</type> into an string

            <p>If the type contained in the Any is not supported, the returned string will be empty. In the
            debug version, an additional assertion occurs.</p>

            @param  _rValue
                the value to convert
        */
        ::rtl::OUString implConvertAny(
            const ::com::sun::star::uno::Any& _rValue);

        /**
            @return
                string which can be used in the <code>form:property</code> element's <code>type</code> attribute
                to describe the type of a value.<br/>
                Possible types returned are
                <ul>
                    <li><b>boolean</b>: <arg>_rValue</arg> was interpreted as boolean value before converting
                        it into a string</li>
                    <li><b>short</b>: <arg>_rValue</arg> was interpreted as 16 bit integer value before
                        converting it into a string</li>
                    <li><b>int</b>: <arg>_rValue</arg> was interpreted as 32 bit integer value before
                        converting it into a string</li>
                    <li><b>long</b>: <arg>_rValue</arg> was interpreted as 64 bit integer value before
                        converting it into a string</li>
                    <li><b>double</b>: <arg>_rValue</arg> was interpreted as 64 bit floating point value before
                        converting it into a string</li>
                    <li><b>string</b>: <arg>_rValue</arg> did not need any conversion as it already was a string</li>
                </ul>
                If the type is not convertable, an empty string is returned
        */
        ::rtl::OUString implGetPropertyXMLType(const ::com::sun::star::uno::Type& _rType);

#ifdef DBG_UTIL
                void AddAttribute(sal_uInt16 _nPrefix, const sal_Char* _pName, const ::rtl::OUString& _rValue);
#else
        //  in the product version, inline this, so it does not cost us extra time calling into our method
        inline  void AddAttribute(sal_uInt16 _nPrefix, const sal_Char* _pName, const ::rtl::OUString& _rValue)
            { m_rContext.AddAttribute(_nPrefix, _pName, _rValue); }
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
            const ::rtl::OUString& _rPropertyName,
            const ::com::sun::star::uno::Type* _pType);

        void dbg_implCheckProperty(
            const sal_Char* _rPropertyName,
            const ::com::sun::star::uno::Type* _pType)
        {
            dbg_implCheckProperty(::rtl::OUString::createFromAscii(_rPropertyName), _pType);
        }
#endif
    };

    //=====================================================================
    //= helper
    //=====================================================================
#ifdef DBG_UTIL
    #define DBG_CHECK_PROPERTY(name, type)  \
        dbg_implCheckProperty(name, &::getCppuType(static_cast< type* >(NULL)))
    #define DBG_CHECK_PROPERTY_NO_TYPE(name)    \
        dbg_implCheckProperty(name, NULL)
#else
    #define DBG_CHECK_PROPERTY(name, type)
    #define DBG_CHECK_PROPERTY_NO_TYPE(name)
#endif

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_PROPERTYEXPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/11/19 15:41:32  fs
 *  extended the export capabilities - generic controls / grid columns / generic properties / some missing form properties
 *
 *  Revision 1.1  2000/11/17 19:03:18  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 15.11.00 17:49:03  fs
 ************************************************************************/
