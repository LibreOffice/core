/*************************************************************************
 *
 *  $RCSfile: elementimport.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-12-06 17:31:12 $
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

#ifndef _XMLOFF_FORMS_ELEMENTIMPORT_HXX_
#define _XMLOFF_FORMS_ELEMENTIMPORT_HXX_

#ifndef _XMLOFF_FORMS_PROPERTYIMPORT_HXX_
#include "propertyimport.hxx"
#endif
#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#include "controlelement.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OElementImport
    //=====================================================================
    /** implements common behaviour for importing forms, controls and columns
    */
    class OElementImport
                :public OPropertyImport
                ,public OAttributeMetaData
    {
    protected:
        ::rtl::OUString     m_sServiceName;     // the service name as extracted from the service-name attribute
        ::rtl::OUString     m_sName;            // the name of the object (redundant, already contained in the base class' array)

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        m_xParentContainer;
            // the parent container to insert the new element into

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xElement;             // the element we're creating. Valid after StartElement

    public:
        /** ctor
            @param _rImport
                the importer
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
            SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    protected:
        // SvXMLImportContext overridables
        virtual void StartElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void    EndElement();

        // OPropertyImport overridables
        virtual void    handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

        /** create the (uninitialized) element which is to represent the read data

            <p>The default implementation uses <member>m_xORB</member> to create a object with <member>m_sServiceName</member>.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();

    private:
        ::rtl::OUString implGetDefaultName() const;
    };

    //=====================================================================
    //= OControlImport
    //=====================================================================
    /** helper class for importing the description of a single control
    */
    class OControlImport : public OElementImport
    {
        ::rtl::OUString     m_sControlId;

    public:
        OControlImport(
            SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        // OPropertyImport overridables
        virtual void    handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);
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
        OControlElement::ElementType
                        m_eElementType;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aListSource;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aValueList;

        ::com::sun::star::uno::Sequence< sal_Int16 >
                        m_aSelectedSeq;
        ::com::sun::star::uno::Sequence< sal_Int16 >
                        m_aDefaultSelectedSeq;

    public:
        OListAndComboImport(
            SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            OControlElement::ElementType _eType,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);
        virtual void    EndElement();

        // OPropertyImport overridables
        virtual void    handleAttribute(sal_uInt16 _nNamespaceKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue);

    protected:
        void implPushBackLabel(const ::rtl::OUString& _rLabel);
        void implPushBackValue(const ::rtl::OUString& _rValue);

        void implSelectCurrentItem();
        void implDefaultSelectCurrentItem();
    };
    SV_DECL_IMPL_REF(OListAndComboImport);

    //=====================================================================
    //= OListOptionImport
    //=====================================================================
    /** helper class for importing a single &lt;form:option&gt; element.
    */
    class OListOptionImport : public SvXMLImportContext
    {
        OListAndComboImportRef  m_xListBoxImport;

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
    class OComboItemImport : public OAccumulateCharacters
    {
        OListAndComboImportRef  m_xListBoxImport;

    public:
        OComboItemImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const OListAndComboImportRef& _rListBox);

        // SvXMLImportContext overridables
        virtual void EndElement();
    };

    //=====================================================================
    //= OFormImport
    //=====================================================================
    /** helper class importing a single &lt;form:form&gt; element
    */
    class OFormImport : public OElementImport
    {
    public:
        OFormImport(
            SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        // SvXMLImportContext overridables
        virtual SvXMLImportContext* CreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList);

    protected:
        // OElementImport overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        createElement();
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_ELEMENTIMPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 04.12.00 14:52:28  fs
 ************************************************************************/

