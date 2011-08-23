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

#ifndef _XMLOFF_FORMS_OFFICEFORMS_HXX_
#define _XMLOFF_FORMS_OFFICEFORMS_HXX_

#ifndef _XMLOFF_FORMATTRIBUTES_HXX_
#include "formattributes.hxx"
#endif
#ifndef _XMLOFF_XMLICTXT_HXX 
#include "xmlictxt.hxx"
#endif
#ifndef XMLOFF_FORMS_LOGGING_HXX
#include "logging.hxx"
#endif
namespace binfilter {

class SvXMLElementExport;
class SvXMLExport;

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OFormsRootImport
    //=====================================================================
    class OFormsRootImport
                :public SvXMLImportContext
                ,public OAttributeMetaData
                ,public OStackedLogging
    {
    public:
        TYPEINFO();

        OFormsRootImport( SvXMLImport& _rImport, sal_uInt16 _nPrfx, const ::rtl::OUString& _rLocalName);
        virtual ~OFormsRootImport();

        // SvXMLImportContext overriabled
        virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
        virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList );
        virtual void EndElement();

    protected:
        void implImportBool(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttributes,
            OfficeFormsAttributes _eAttribute,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >& _rxPropInfo,
            const ::rtl::OUString& _rPropName,
            sal_Bool _bDefault
            );
    };

    //=====================================================================
    //= OFormsRootExport
    //=====================================================================
    class OFormsRootExport : public OAttributeMetaData
    {
    private:
        SvXMLElementExport*		m_pImplElement;

    public:
        OFormsRootExport( SvXMLExport& _rExp );
        ~OFormsRootExport();

    private:
        void addModelAttributes(SvXMLExport& _rExp) SAL_THROW(());

        void implExportBool(
            SvXMLExport& _rExp,
            OfficeFormsAttributes _eAttribute,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >& _rxPropInfo,
            const ::rtl::OUString& _rPropName,
            sal_Bool _bDefault
            );
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_OFFICEFORMS_HXX_


