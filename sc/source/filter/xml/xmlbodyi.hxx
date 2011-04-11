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

#ifndef SC_XMLBODYI_HXX
#define SC_XMLBODYI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include "tabprotection.hxx"

class ScXMLImport;
class ScXMLChangeTrackingImportHelper;

class ScXMLBodyContext : public SvXMLImportContext
{
    rtl::OUString   sPassword;
    ScPasswordHash  meHash1;
    ScPasswordHash  meHash2;
    sal_Bool        bProtected;
    sal_Bool        bHadCalculationSettings;

    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLBodyContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual ~ScXMLBodyContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
