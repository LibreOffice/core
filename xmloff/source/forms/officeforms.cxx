/*************************************************************************
 *
 *  $RCSfile: officeforms.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-20 13:41:05 $
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

#ifndef _XMLOFF_FORMS_OFFICEFORMS_HXX_
#include "officeforms.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::xml;

    //=========================================================================
    //= OFormsRootImport
    //=========================================================================
    TYPEINIT1(OFormsRootImport, SvXMLImportContext);
    //-------------------------------------------------------------------------
    OFormsRootImport::OFormsRootImport( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName )
        :SvXMLImportContext(rImport, nPrfx, rLocalName)
    {
    }

    //-------------------------------------------------------------------------
    OFormsRootImport::~OFormsRootImport()
    {
    }

    //-------------------------------------------------------------------------
    SvXMLImportContext* OFormsRootImport::CreateChildContext( USHORT _nPrefix, const ::rtl::OUString& _rLocalName,
            const Reference< sax::XAttributeList>& xAttrList )
    {
        return GetImport().GetFormImport()->createContext( _nPrefix, _rLocalName, xAttrList );
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::implImportBool(const Reference< sax::XAttributeList >& _rxAttributes, OfficeFormsAttributes _eAttribute,
            const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
            const ::rtl::OUString& _rPropName, sal_Bool _bDefault)
    {
        // the complete attribute name to look for
        ::rtl::OUString sCompleteAttributeName = GetImport().GetNamespaceMap().GetQNameByIndex(
            getOfficeFormsAttributeNamespace(_eAttribute),
            ::rtl::OUString::createFromAscii(getOfficeFormsAttributeName(_eAttribute)));

        // get and convert the value
        ::rtl::OUString sAttributeValue = _rxAttributes->getValueByName(sCompleteAttributeName);
        sal_Bool bValue = _bDefault;
        GetImport().GetMM100UnitConverter().convertBool(bValue, sAttributeValue);

        // set the property
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            _rxProps->setPropertyValue(_rPropName, ::cppu::bool2any(bValue));
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::StartElement( const Reference< sax::XAttributeList >& _rxAttrList )
    {
        SvXMLImportContext::StartElement( _rxAttrList );

        try
        {
            Reference< XPropertySet > xDocProperties(GetImport().GetModel(), UNO_QUERY);
            OSL_ENSURE(xDocProperties.is(), "OFormsRootImport::StartElement: invalid document model!");
            Reference< XPropertySetInfo > xDocPropInfo;
            if (xDocProperties.is())
                xDocPropInfo = xDocProperties->getPropertySetInfo();

            implImportBool(_rxAttrList, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, sal_False);
            implImportBool(_rxAttrList, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, sal_True);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormsRootImport::StartElement: caught an exception while setting the document properties!");
        }
    }

    //=====================================================================
    //= OFormsRootExport
    //=====================================================================
    //---------------------------------------------------------------------
    OFormsRootExport::OFormsRootExport( SvXMLExport& _rExp )
        :m_pImplElement(NULL)
    {
        addModelAttributes(_rExp);

        m_pImplElement = new SvXMLElementExport(_rExp, XML_NAMESPACE_OFFICE, sXML_forms, sal_True, sal_True);
    }

    //---------------------------------------------------------------------
    OFormsRootExport::~OFormsRootExport( )
    {
        delete m_pImplElement;
    }

    //-------------------------------------------------------------------------
    void OFormsRootExport::implExportBool(SvXMLExport& _rExp, OfficeFormsAttributes _eAttribute,
        const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
        const ::rtl::OUString& _rPropName, sal_Bool _bDefault)
    {
        // retrieve the property value
        sal_Bool bValue = _bDefault;
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            bValue = ::cppu::any2bool(_rxProps->getPropertyValue(_rPropName));

        // convert into a string
        ::rtl::OUStringBuffer aValue;
        _rExp.GetMM100UnitConverter().convertBool(aValue, bValue);

        // add the attribute
        _rExp.AddAttribute(
            getOfficeFormsAttributeNamespace(_eAttribute),
            getOfficeFormsAttributeName(_eAttribute),
            aValue.makeStringAndClear());
    }

    //-------------------------------------------------------------------------
    void OFormsRootExport::addModelAttributes(SvXMLExport& _rExp) SAL_THROW(())
    {
        try
        {
            Reference< XPropertySet > xDocProperties(_rExp.GetModel(), UNO_QUERY);
            OSL_ENSURE(xDocProperties.is(), "OFormsRootExport::addModelAttributes: invalid document model!");
            Reference< XPropertySetInfo > xDocPropInfo;
            if (xDocProperties.is())
                xDocPropInfo = xDocProperties->getPropertySetInfo();

            implExportBool(_rExp, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, sal_False);
            implExportBool(_rExp, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, sal_True);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormsRootExport::addModelAttributes: caught an exception while retrieving the document properties!");
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 20.03.01 13:12:35  fs
 ************************************************************************/

