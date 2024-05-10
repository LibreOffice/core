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

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <rtl/ustring.hxx>


class ConvDic;
using namespace css::xml::sax;
using namespace ::xmloff::token;


class ConvDicXMLExport : public SvXMLExport
{
    ConvDic     &rDic;
    bool    bSuccess;

public:
    ConvDicXMLExport( ConvDic &rConvDic,
        const OUString &rFileName,
        css::uno::Reference< css::xml::sax::XDocumentHandler > const &rHandler) :
        SvXMLExport ( comphelper::getProcessComponentContext(), u"com.sun.star.lingu2.ConvDicXMLExport"_ustr, rFileName,
                      css::util::MeasureUnit::CM, rHandler ),
        rDic        ( rConvDic ),
        bSuccess    ( false )
    {
    }

    // SvXMLExport
    void ExportAutoStyles_() override    {}
    void ExportMasterStyles_() override  {}
    void ExportContent_() override;
    ErrCode exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) override;

    bool    Export();
};


enum ConvDicXMLToken : sal_Int32
{
    TEXT_CONVERSION_DICTIONARY = FastToken::NAMESPACE | XML_NAMESPACE_TCD | XML_BLOCK_LIST,
    RIGHT_TEXT = FastToken::NAMESPACE | XML_NAMESPACE_TCD | XML_RIGHT_TEXT,
    ENTRY = FastToken::NAMESPACE | XML_NAMESPACE_TCD | XML_ENTRY,
};

class ConvDicXMLImport : public SvXMLImport
{
    ConvDic        *pDic;       // conversion dictionary to be used
                                // if != NULL: whole file will be read and
                                //   all entries will be added to the dictionary
                                // if == NULL: no entries will be added
                                //   but the language and conversion type will
                                //   still be determined!

    LanguageType    nLanguage;          // language of the dictionary
    sal_Int16       nConversionType;    // conversion type the dictionary is used for

public:

    //!!  see comment for pDic member
    explicit ConvDicXMLImport( ConvDic *pConvDic );

    ConvDic *    GetDic()                    { return pDic; }
    LanguageType GetLanguage() const         { return nLanguage; }
    sal_Int16    GetConversionType() const   { return nConversionType; }

    void         SetLanguage( LanguageType nLang )              { nLanguage = nLang; }
    void         SetConversionType( sal_Int16 nType )    { nConversionType = nType; }

private:
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
