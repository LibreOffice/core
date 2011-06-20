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
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SwXMLBlockListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
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
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    sal_Bool bTextOnly;
    String &m_rText;
    SwXMLTextBlockImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks &rBlocks, String &rNewText, sal_Bool bNewTextOnly );

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
