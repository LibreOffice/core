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

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#define _XMLOFF_PAGEMASTERPROPHDL_HXX_

#include <xmloff/xmlprhdl.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>

// property handler for style:page-usage (style::PageStyleLayout)

class XMLPMPropHdl_PageStyleLayout : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_PageStyleLayout();
    virtual bool            equals(
                                const ::com::sun::star::uno::Any& rAny1,
                                const ::com::sun::star::uno::Any& rAny2
                                ) const;
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

// property handler for style:num-format (style::NumberingType)

class XMLPMPropHdl_NumFormat : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_NumFormat();
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

// property handler for style:num-letter-sync (style::NumberingType)

class XMLPMPropHdl_NumLetterSync : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_NumLetterSync();
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

// property handler for style:paper-tray-number

class XMLPMPropHdl_PaperTrayNumber : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_PaperTrayNumber();
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

// property handler for style:print

class XMLPMPropHdl_Print : public XMLPropertyHandler
{
protected:
    OUString         sAttrValue;

public:
                            XMLPMPropHdl_Print( enum ::xmloff::token::XMLTokenEnum eValue );
    virtual                 ~XMLPMPropHdl_Print();

    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

// property handler for style:table-centering

class XMLPMPropHdl_CenterHorizontal : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_CenterHorizontal();
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

class XMLPMPropHdl_CenterVertical : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_CenterVertical();
    virtual sal_Bool        importXML(
                                const OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
