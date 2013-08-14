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
#ifndef INCLUDED_FAST_HELPER_HXX
#define INCLUDED_FAST_HELPER_HXX

#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include "OOXMLFastContextHandler.hxx"
#include "ooxmlLoggers.hxx"
namespace writerfilter {

namespace ooxml
{
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;

template <class T>
class OOXMLFastHelper
{
public:
    static uno::Reference<XFastContextHandler> createAndSetParent
    (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId);

    static uno::Reference<XFastContextHandler> createAndSetParentAndDefine
    (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId, Id nDefine);

    static uno::Reference<XFastContextHandler> createAndSetParentRef
    (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken,
     const uno::Reference < xml::sax::XFastAttributeList > & Attribs);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId,
                            const OUString & rValue);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId, sal_Int32 nValue);

    static void mark(OOXMLFastContextHandler * pHandler,
                     Id nId,
                     const OUString & rValue);

    static void attributes
    (OOXMLFastContextHandler * pContext,
     const uno::Reference < xml::sax::XFastAttributeList > & Attribs);
};

template <class T>
uno::Reference<XFastContextHandler>
OOXMLFastHelper<T>::createAndSetParent
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId)
{
#ifdef DEBUG_HELPER
    debug_logger->startElement("helper.createAndSetParent");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("id", (*QNameToString::Instance())(nId));
#endif

    OOXMLFastContextHandler * pTmp = new T(pHandler);

    pTmp->setToken(nToken);
    pTmp->setId(nId);

#ifdef DEBUG_CREATE
    debug_logger->startElement("createAndSetParent");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("token", fastTokenToId(pTmp->getToken()));
    debug_logger->attribute("id", (*QNameToString::Instance())(nId));

    debug_logger->startElement("created");
    debug_logger->addTag(pTmp->toTag());
    debug_logger->endElement("created");
    debug_logger->endElement("helper.createAndSetParent");
#endif

    uno::Reference<XFastContextHandler> aResult(pTmp);

    return aResult;
}

template <class T>
uno::Reference<XFastContextHandler>
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

    uno::Reference<XFastContextHandler> aResult(pTmp);

    return aResult;
}

template <class T>
uno::Reference<XFastContextHandler>
OOXMLFastHelper<T>::createAndSetParentRef
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken,
 const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
{
#ifdef DEBUG_HELPER
    debug_logger->startElement("helper.createAndSetParentRef");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("type", fastTokenToId(nToken));
#endif

    boost::shared_ptr<OOXMLFastContextHandler> pTmp(new T(pHandler));

    uno::Reference<XFastContextHandler> xChild =
        pTmp->createFastChildContext(nToken, Attribs);

    if (xChild.is())
    {
        OOXMLFastContextHandler* pResult = dynamic_cast<OOXMLFastContextHandler *>(xChild.get());
        pResult->setToken(nToken);
        pResult->setParent(pHandler);
    }


#ifdef DEBUG_HELPER
    debug_logger->startElement("created");
    debug_logger->addTag(pTmp->toTag());
    debug_logger->endElement("created");
    debug_logger->endElement("helper.createAndSetParentRef");
#endif

    return xChild;
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

template <class T>
void OOXMLFastHelper<T>::mark(OOXMLFastContextHandler * pHandler,
                              Id nId,
                              const OUString & rValue)
{
    OOXMLValue::Pointer_t pVal(new T(rValue));

#ifdef DEBUG_HELPER
    string aStr = (*QNameToString::Instance())(nId);

    debug_logger->startElement("helper.mark");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute("value",
        OUStringToOString
        (rValue, RTL_TEXTENCODING_ASCII_US).getStr());

    if (aStr.empty())
        debug_logger->element("unknown-qname");

    debug_logger->endElement();
#endif

    pHandler->mark(nId, pVal);
}
}}
#endif // INCLUDED_FAST_HELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
