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

#ifndef _XMLOFF_DESCRIPTIONIMP_HXX
#define _XMLOFF_DESCRIPTIONIMP_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/drawing/XShape.hpp>

//////////////////////////////////////////////////////////////////////////////
// office:events inside a shape

class SdXMLDescriptionContext : public SvXMLImportContext
{
private:
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxShape;
    ::rtl::OUString msText;
public:
    TYPEINFO();

    SdXMLDescriptionContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rxShape );
    virtual ~SdXMLDescriptionContext();

    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const ::rtl::OUString& rChars );
};

#endif  //  _XMLOFF_EVENTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
