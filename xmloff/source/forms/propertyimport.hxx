/*************************************************************************
 *
 *  $RCSfile: propertyimport.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-28 13:59:54 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_PROPERTYIMPORT_HXX_
#define _XMLOFF_FORMS_PROPERTYIMPORT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_FORMATTRIBUTES_HXX_
#include "formattributes.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

namespace com { namespace sun { namespace star { namespace util {
    struct Time;
    struct Date;
} } } }

class XMLPropStyleContext;
//.........................................................................
namespace xmloff
{
//.........................................................................

    class IFormsImportContext;
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

    protected:
        DECLARE_STL_VECTOR( ::com::sun::star::beans::PropertyValue, PropertyValueArray );
        PropertyValueArray      m_aValues;
            // the values which the instance collects between StartElement and EndElement

        DECLARE_STL_STDKEY_SET( ::rtl::OUString, StringSet );
        StringSet               m_aEncounteredAttributes;

        IFormsImportContext&        m_rContext;

        // TODO: think about the restriction that the class does not know anything about the object it is importing.
        // Perhaps this object should be known to the class, so setting the properties ('normal' ones as well as
        // style properties) can be done in our own EndElement instead of letting derived classes do this.

    public:
        OPropertyImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName);

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
        virtual void handleAttribute(sal_uInt16 _nNamespaceKey,
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

        /** can be used to handle properties where the attribute default and the property default differ.
            <p>In such case, if the property had the attribute default upon writing, nothing is read, so upon reading,
            the property is still at it's own default (which is not the attribute default).<p/>
            <p>This method, if told the attribute and the property, and the (implied) attribute default, sets the
            property value as if the attribute was encountered.</p>
            @see encounteredAttribute
        */
        void        simluateDefaultedAttribute(const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName, const sal_Char* _pAttributeDefault);

        void implPushBackPropertyValue(const ::com::sun::star::beans::PropertyValue& _rProp)
        { m_aValues.push_back(_rProp); }

        static ::com::sun::star::uno::Any convertString(
            SvXMLImport& _rImporter,
            const ::com::sun::star::uno::Type& _rExpectedType,
            const ::rtl::OUString& _rReadCharacters,
            const SvXMLEnumMapEntry* _pEnumMap = NULL,
            const sal_Bool _bInvertBoolean = sal_False
            );

    private:
        static ::com::sun::star::util::Time implGetTime(double _nValue);
        static ::com::sun::star::util::Date implGetDate(double _nValue);
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

#ifdef _DEBUG
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void Characters(const ::rtl::OUString& _rChars);
#endif
    };

    //=====================================================================
    //= OSinglePropertyContext
    //=====================================================================
    SV_DECL_REF( OAccumulateCharacters )
    /** helper class for importing a single &lt;form:property&gt; element
    */
    class OSinglePropertyContext : public SvXMLImportContext
    {
        OPropertyImportRef          m_xPropertyImporter;    // to add the properties
        OAccumulateCharactersRef    m_xValueReader;         // the class reading the characters
        ::com::sun::star::beans::PropertyValue
                                    m_aPropValue;           // the property the instance imports currently
        ::com::sun::star::uno::Type m_aPropType;            // the type of the property the instance imports currently

    public:
        OSinglePropertyContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
                const OPropertyImportRef& _rPropertyImporter);

        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
#ifdef _DEBUG
        virtual void Characters(const ::rtl::OUString& _rChars);
#endif
        virtual void EndElement();
    };

    //=====================================================================
    //= OAccumulateCharacters
    //=====================================================================
    /** helper class which accumulates the characters it gets
    */
    class OAccumulateCharacters : public SvXMLImportContext
    {
    protected:
        ::rtl::OUString m_sCharacters;
    public:
        OAccumulateCharacters(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName);

        // SvXMLImportContext overridables
        virtual void Characters(const ::rtl::OUString& _rChars);

        ::rtl::OUString getCharacters() const { return m_sCharacters; }
    };
    SV_IMPL_REF( OAccumulateCharacters )

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_PROPERTYIMPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2001/03/28 09:59:38  fs
 *  #85097# correctly import boolean properties with inverse semantics
 *
 *  Revision 1.5  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.4  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.3  2000/12/13 10:40:15  fs
 *  new import related implementations - at this version, we should be able to import everything we export (which is all except events and styles)
 *
 *  Revision 1.2  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.1  2000/12/06 17:32:01  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 15:10:13  fs
 ************************************************************************/

