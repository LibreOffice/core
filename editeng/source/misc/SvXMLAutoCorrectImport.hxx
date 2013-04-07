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
#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#define _SV_XMLAUTOCORRECTIMPORT_HXX

#include <sot/storage.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/svxacorr.hxx>

class SvXMLAutoCorrectImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvxAutocorrWordList     *pAutocorr_List;
    SvxAutoCorrect          &rAutoCorrect;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStorage;

    SvXMLAutoCorrectImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        SvxAutocorrWordList *pNewAutocorr_List,
        SvxAutoCorrect &rNewAutoCorrect,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage);

    ~SvXMLAutoCorrectImport ( void ) throw ();
};

class SvXMLWordListContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordListContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordListContext ( void );
};

class SvXMLWordContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordContext ( void );
};


class SvXMLExceptionListImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvStringsISortDtor  &rList;

    SvXMLExceptionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        SvStringsISortDtor & rNewList );

    ~SvXMLExceptionListImport ( void ) throw ();
};

class SvXMLExceptionListContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionListContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionListContext ( void );
};

class SvXMLExceptionContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionContext ( void );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
