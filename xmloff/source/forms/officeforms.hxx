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

#ifndef _XMLOFF_FORMS_OFFICEFORMS_HXX_
#define _XMLOFF_FORMS_OFFICEFORMS_HXX_

#include "formattributes.hxx"
#include <xmloff/xmlictxt.hxx>
#include "logging.hxx"

class SvXMLElementExport;
class SvXMLExport;

namespace xmloff
{

    //= OFormsRootImport
    class OFormsRootImport
                :public SvXMLImportContext
                ,public OStackedLogging
    {
    public:
        TYPEINFO();

        OFormsRootImport( SvXMLImport& _rImport, sal_uInt16 _nPrfx, const OUString& _rLocalName);
        virtual ~OFormsRootImport();

        // SvXMLImportContext overriabled
        virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
            const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
        virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList );
        virtual void EndElement();

    protected:
        void implImportBool(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttributes,
            OfficeFormsAttributes _eAttribute,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >& _rxPropInfo,
            const OUString& _rPropName,
            sal_Bool _bDefault
            );
    };

    //= OFormsRootExport
    class OFormsRootExport
    {
    private:
        SvXMLElementExport*     m_pImplElement;

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
            const OUString& _rPropName,
            sal_Bool _bDefault
            );
    };

}   // namespace xmloff

#endif // _XMLOFF_FORMS_OFFICEFORMS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
