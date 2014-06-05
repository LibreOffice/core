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
#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTHELPER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTHELPER_HXX

#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include "OOXMLFastContextHandler.hxx"
#include "ooxmlLoggers.hxx"
namespace writerfilter {

namespace ooxml
{

template <class T>
class OOXMLFastHelper
{
public:
    static css::uno::Reference<css::xml::sax::XFastContextHandler> createAndSetParentAndDefine
    (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId, Id nDefine);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId,
                            const OUString & rValue);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId, sal_Int32 nValue);

    static void attributes
    (OOXMLFastContextHandler * pContext,
     const css::uno::Reference < css::xml::sax::XFastAttributeList > & Attribs);
};

template <class T>
uno::Reference<css::xml::sax::XFastContextHandler>
OOXMLFastHelper<T>::createAndSetParentAndDefine
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId, Id nDefine)
{
#ifdef DEBUG_HELPER
    debug_logger->startElement("helper.createAndSetParentAndDefine");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("id", (*QNameToString::Instance())(nId));

    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "0x%08" SAL_PRIxUINT32, nId);

    debug_logger->attribute("idnum", buffer);
#endif

    OOXMLFastContextHandler * pTmp = new T(pHandler);

    pTmp->setToken(nToken);
    pTmp->setId(nId);
    pTmp->setDefine(nDefine);


#ifdef DEBUG_HELPER
    debug_logger->startElement("created");
    debug_logger->addTag(pTmp->toTag());
    debug_logger->endElement("created");
    debug_logger->endElement("helper.createAndSetParentAndDefine");
#endif

    css::uno::Reference<css::xml::sax::XFastContextHandler> aResult(pTmp);

    return aResult;
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler,
                                     Id nId,
                                     const OUString & rValue)
{
    OOXMLValue::Pointer_t pVal(new T(rValue));

#ifdef DEBUG_HELPER
    string aStr = (*QNameToString::Instance())(nId);

    debug_logger->startElement("newProperty-from-string");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute
        ("value",
         OUStringToOString
         (rValue, RTL_TEXTENCODING_ASCII_US).getStr());

    if (aStr.empty())
        debug_logger->element( "unknown-qname" );
#endif

    pHandler->newProperty(nId, pVal);

#ifdef DEBUG_HELPER
    debug_logger->endElement();
#endif

}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler,
                                     Id nId,
                                     sal_Int32 nVal)
{
    OOXMLValue::Pointer_t pVal(new T(nVal));

#ifdef DEBUG_HELPER
    string aStr = (*QNameToString::Instance())(nId);

    debug_logger->startElement("helper.newProperty-from-int");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute("value", pVal->toString());

    if (aStr.empty())
        debug_logger->element("unknown-qname");

    debug_logger->endElement();
#endif

    pHandler->newProperty(nId, pVal);
}

}}
#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
