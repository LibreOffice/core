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

#ifndef INCLUDED_LINGUISTIC_SOURCE_CONVDICXML_HXX
#define INCLUDED_LINGUISTIC_SOURCE_CONVDICXML_HXX

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <rtl/ustring.hxx>
#include "linguistic/misc.hxx"
#include "defs.hxx"


class ConvDic;


class ConvDicXMLExport : public SvXMLExport
{
    ConvDic     &rDic;
    bool    bSuccess;

public:
    ConvDicXMLExport( ConvDic &rConvDic,
        const OUString &rFileName,
        css::uno::Reference< css::xml::sax::XDocumentHandler > &rHandler) :
        SvXMLExport ( comphelper::getProcessComponentContext(), "com.sun.star.lingu2.ConvDicXMLExport", rFileName,
                      css::util::MeasureUnit::CM, rHandler ),
        rDic        ( rConvDic ),
        bSuccess    ( false )
    {
    }
    virtual ~ConvDicXMLExport()
    {
    }

    // SvXMLExport
    void ExportAutoStyles_() override    {}
    void ExportMasterStyles_() override  {}
    void ExportContent_() override;
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) override;

    bool    Export();
};


class ConvDicXMLImport : public SvXMLImport
{
    ConvDic        *pDic;       // conversion dictionary to be used
                                // if != NULL: whole file will be read and
                                //   all entries will be added to the dictionary
                                // if == NULL: no entries will be added
                                //   but the language and conversion type will
                                //   still be determined!

    sal_Int16       nLanguage;          // language of the dictionary
    sal_Int16       nConversionType;    // conversion type the dictionary is used for

public:

    //!!  see comment for pDic member
    explicit ConvDicXMLImport( ConvDic *pConvDic ) :
        SvXMLImport ( comphelper::getProcessComponentContext(), "com.sun.star.lingu2.ConvDicXMLImport", SvXMLImportFlags::ALL ),
        pDic        ( pConvDic )
    {
        nLanguage       = LANGUAGE_NONE;
        nConversionType = -1;
    }

    virtual ~ConvDicXMLImport() throw ()
    {
    }

    virtual void SAL_CALL startDocument() throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual SvXMLImportContext * CreateContext(
        sal_uInt16 nPrefix, const OUString &rLocalName,
        const css::uno::Reference < css::xml::sax::XAttributeList > &rxAttrList ) override;

    ConvDic *   GetDic()                    { return pDic; }
    sal_Int16   GetLanguage() const         { return nLanguage; }
    sal_Int16   GetConversionType() const   { return nConversionType; }

    void        SetLanguage( sal_Int16 nLang )              { nLanguage = nLang; }
    void        SetConversionType( sal_Int16 nType )    { nConversionType = nType; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
