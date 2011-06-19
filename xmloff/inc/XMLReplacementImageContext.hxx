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

#ifndef _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX
#define _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlictxt.hxx>


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace io { class XOutputStream; } } } }

class XMLReplacementImageContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > m_xBase64Stream;
::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > m_xPropSet;

    ::rtl::OUString m_sHRef;
    const ::rtl::OUString m_sGraphicURL;

public:

    TYPEINFO();

    XMLReplacementImageContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rAttrList,
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet );
    virtual ~XMLReplacementImageContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

#endif  //  _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
