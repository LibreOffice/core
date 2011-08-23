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

#ifndef _SC_XMLCONSOLIDATIONCONTEXT_HXX
#define _SC_XMLCONSOLIDATIONCONTEXT_HXX

#include "global.hxx"

#include <bf_xmloff/xmlimp.hxx>
namespace binfilter {

class ScXMLImport;


//___________________________________________________________________

class ScXMLConsolidationContext : public SvXMLImportContext
{
private:
    ::rtl::OUString				sSourceList;
    ::rtl::OUString				sUseLabel;
    ScAddress					aTargetAddr;
    ScSubTotalFunc				eFunction;
    sal_Bool					bLinkToSource : 1;
    sal_Bool					bTargetAddr : 1;

protected:
    const ScXMLImport&			GetScImport() const	{ return (const ScXMLImport&)GetImport(); }
    ScXMLImport&				GetScImport()		{ return (ScXMLImport&)GetImport(); }

public:
                                ScXMLConsolidationContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual						~ScXMLConsolidationContext();

    virtual SvXMLImportContext*	CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void				EndElement();
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
