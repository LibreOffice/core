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
#ifndef _SV_XMLAUTOCORRECTEXPORT_HXX
#define _SV_XMLAUTOCORRECTEXPORT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/svxacorr.hxx>

class SvXMLAutoCorrectExport : public SvXMLExport
{
private:
    const SvxAutocorrWordList   *pAutocorr_List;
public:
    SvXMLAutoCorrectExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const SvxAutocorrWordList * pNewAutocorr_List,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SvXMLAutoCorrectExport ( void ) {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};

class SvStringsISortDtor;

class SvXMLExceptionListExport : public SvXMLExport
{
private:
    const SvStringsISortDtor & rList;
public:
    SvXMLExceptionListExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const SvStringsISortDtor &rNewList,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SvXMLExceptionListExport ( void ) {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
