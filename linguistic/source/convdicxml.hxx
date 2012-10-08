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

#ifndef _LINGUISTIC_CONVDICXML_HXX_
#define _LINGUISTIC_CONVDICXML_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
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
        SvXMLExport ( comphelper::getProcessServiceFactory(), rFileName,
                      ::com::sun::star::util::MeasureUnit::CM, rHandler ),
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
        SvXMLImport ( comphelper::getProcessServiceFactory(), IMPORT_ALL ),
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
