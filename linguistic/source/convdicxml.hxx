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

#ifndef _LINGUISTIC_CONVDICXML_HXX_
#define _LINGUISTIC_CONVDICXML_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include "linguistic/misc.hxx"
#include "defs.hxx"


class ConvDic;


class ConvDicXMLExport : public SvXMLExport
{
    ConvDic     &rDic;
    sal_Bool    bSuccess;

public:
    ConvDicXMLExport( ConvDic &rConvDic,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler) :
        SvXMLExport ( utl::getProcessServiceFactory(), rFileName, rHandler ),
        rDic        ( rConvDic ),
        bSuccess    ( sal_False )
    {
    }
    virtual ~ConvDicXMLExport()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    // SvXMLExport
    void _ExportAutoStyles()    {}
    void _ExportMasterStyles()  {}
    void _ExportContent();
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );

    sal_Bool    Export();
};


class ConvDicXMLImport : public SvXMLImport
{
    ConvDic        *pDic;       // conversion dictionary to be used
                                // if != NULL: whole file will be read and
                                //   all entries will be added to the dictionary
                                // if == NULL: no entries will be added
                                //   but the language and conversion type will
                                //   still be determined!

    sal_Int16           nLanguage;          // language of the dictionary
    sal_Int16       nConversionType;    // conversion type the dictionary is used for
    sal_Bool        bSuccess;

public:

    //!!  see comment for pDic member
    ConvDicXMLImport( ConvDic *pConvDic, const rtl::OUString /*&rFileName*/ ) :
        SvXMLImport ( utl::getProcessServiceFactory(), IMPORT_ALL ),
        pDic        ( pConvDic )
    {
        nLanguage       = LANGUAGE_NONE;
        nConversionType = -1;
        bSuccess        = sal_False;
    }

    virtual ~ConvDicXMLImport() throw ()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual SvXMLImportContext * CreateContext(
        sal_uInt16 nPrefix, const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference < com::sun::star::xml::sax::XAttributeList > &rxAttrList );

    ConvDic *   GetDic()                    { return pDic; }
    sal_Int16       GetLanguage() const         { return nLanguage; }
    sal_Int16   GetConversionType() const   { return nConversionType; }
    sal_Bool    GetSuccess() const          { return bSuccess; }

    void        SetLanguage( sal_Int16 nLang )              { nLanguage = nLang; }
    void        SetConversionType( sal_Int16 nType )    { nConversionType = nType; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
