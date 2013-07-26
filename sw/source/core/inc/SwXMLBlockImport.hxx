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

#ifndef _SW_XMLBLOCKIMPORT_HXX
#define _SW_XMLBLOCKIMPORT_HXX

#include <xmloff/xmlimp.hxx>

class SwXMLTextBlocks;
class SwXMLBlockListImport : public SvXMLImport
{
private:
    SwXMLTextBlocks &rBlockList;

protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:
    SwXMLBlockListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        SwXMLTextBlocks &rBlocks );

    SwXMLTextBlocks& getBlockList ( void )
    {
        return rBlockList;
    }
    virtual ~SwXMLBlockListImport ( void )
        throw();
};

class SwXMLTextBlockImport : public SvXMLImport
{
private:
    SwXMLTextBlocks &rBlockList;

protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:
    sal_Bool bTextOnly;
    OUString &m_rText;
    SwXMLTextBlockImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        SwXMLTextBlocks &rBlocks, OUString &rNewText, sal_Bool bNewTextOnly );

    SwXMLTextBlocks& getBlockList ( void )
    {
        return rBlockList;
    }
    virtual ~SwXMLTextBlockImport ( void )
        throw();
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
