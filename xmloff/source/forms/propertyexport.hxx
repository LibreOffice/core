/*************************************************************************
 *
 *  $RCSfile: propertyexport.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 19:03:18 $
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
    protected:
        SvXMLExport&    m_rContext;

        DECLARE_STL_STDKEY_SET(::rtl::OUString, StringSet);
        StringSet       m_aRemainingProps;
            // see examinePersistence

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xProps;
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

            <p>upon returnm the <method>m_aRemainingProps</method> will be filled with the names of all properties
            which need to be stored</p>
        */
        void examinePersistence();

        /**
        */
        void exportRemainingProperties();

        /** add an attribute which is represented by a string property to the export context

            @param _nNamespaceKey
                the key of the namespace to use for the attribute name. Is used with the namespace map
                provided by the export context.
            @param _pAttributeName
                the name of the attribute to add. Must not contain any namespace
            @param _pPropertyName
                the name of the property to ask the control for
        */
        void implAddStringPropAttribute(
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
        void implAddBooleanPropAttribute(
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
        void implAddInt16PropAttribute(
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
        void implAddEnumPropAttribute(
            const sal_uInt16 _nNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName,
            const SvXMLEnumMapEntry* _pValueMap,
            const sal_Int32 _nDefault);

        /// some very special methods for some very special attribute/property pairs

        /** add the service-name attribute to the export context
        */
        void implExportServiceName();

        /** add the hlink:xref attribute to the export context.

            <p>The value of this attribute is extracted from the TargetFrame property of the object given.</p>

            <p>The property needs a special handling because conflicts between the default values for the attribute
            and the property.</p>
        */
        void implExportTargetFrame();

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
        void implExportGenericPropertyAttribute(
            const sal_uInt16 _nAttributeNamespaceKey,
            const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName);

        /** tries to convert an arbitrary <type scope="com.sun:star.uno">Any</type> into an string

            <p>If the type contained in the Any is not supported, the returned string will be empty. In the
            debug version, an additional assertion occurs.</p>
        */
        ::rtl::OUString implConvertAny(const ::com::sun::star::uno::Any& _rValue);
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
 *
 *  Revision 1.0 15.11.00 17:49:03  fs
 ************************************************************************/

