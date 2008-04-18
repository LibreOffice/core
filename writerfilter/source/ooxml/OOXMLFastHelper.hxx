/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOXMLFastHelper.hxx,v $
 * $Revision: 1.5 $
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
#ifndef INCLUDED_FAST_HELPER_HXX
#define INCLUDED_FAST_HELPER_HXX

#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include "OOXMLFastContextHandler.hxx"

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

    static uno::Reference<XFastContextHandler> createAndSetParentRef
    (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken,
     const uno::Reference < xml::sax::XFastAttributeList > & Attribs);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId,
                            const ::rtl::OUString & rValue);

    static void newProperty(OOXMLFastContextHandler * pHandler,
                            Id nId, sal_Int32 nValue);

    static void mark(OOXMLFastContextHandler * pHandler,
                     Id nId,
                     const ::rtl::OUString & rValue);

    static void attributes
    (OOXMLFastContextHandler * pContext,
     const uno::Reference < xml::sax::XFastAttributeList > & Attribs);
};

template <class T>
uno::Reference<XFastContextHandler>
OOXMLFastHelper<T>::createAndSetParent
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId)
{
    OOXMLFastContextHandler * pTmp = new T(pHandler);
    OOXMLFastContextHandler::RefAndPointer_t aResult(pTmp);

    pTmp->setToken(nToken);
    pTmp->setId(nId);

#ifdef DEBUG_CREATE
    XMLTag aTag("createAndSetParent");

    aTag.addAttr("context", pHandler->getType());
    aTag.addAttr("token", fastTokenToId(pTmp->getToken()));
    aTag.addAttr("id", (*QNameToString::Instance())(nId));
    if (pTmp->isFallback())
        aTag.addAttr("fallback", "yes");

    XMLTag::Pointer_t pCreatedTag(new XMLTag("created"));
    pCreatedTag->addTag(pTmp->toTag());

    aTag.addTag(pCreatedTag);

    logger("DEBUG", aTag.toString());
#endif

    return aResult;
}

template <class T>
uno::Reference<XFastContextHandler>
OOXMLFastHelper<T>::createAndSetParentRef
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken,
 const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
{
    boost::shared_ptr<OOXMLFastContextHandler> pTmp(new T(pHandler));

    uno::Reference<XFastContextHandler> xChild =
        pTmp->createFastChildContext(nToken, Attribs);

    OOXMLFastContextHandler * pResult = NULL;
    if (xChild.is())
    {
        pResult = dynamic_cast<OOXMLFastContextHandler *>(xChild.get());
        pResult->setToken(nToken);
        pResult->setParent(pHandler);
    }


#ifdef DEBUG_CREATE
    XMLTag aTag("createAndSetParentRef");

    aTag.addAttr("context", pHandler->getType());
    aTag.addAttr("type", fastTokenToId(nToken));
    if (pTmp->isFallback())
        aTag.addAttr("fallback", "yes");

    XMLTag::Pointer_t pTag(new XMLTag("created"));

    pTag->chars(pTmp->getType());

    aTag.addTag(pTag);

    logger("DEBUG", aTag.toString());
#endif

    return xChild;
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler,
                                     Id nId,
                                     const ::rtl::OUString & rValue)
{
    OOXMLValue::Pointer_t pVal(new T(rValue));

    string aStr = (*QNameToString::Instance())(nId);

    if (aStr.size() == 0)
        logger("DEBUG", "unknown QName");

#ifdef DEBUG_PROPERTIES
    XMLTag aTag("newProperty");

    aTag.addAttr("name", aStr);
    aTag.addAttr("value",
                 ::rtl::OUStringToOString
                 (rValue, RTL_TEXTENCODING_ASCII_US).getStr());
    logger("DEBUG", aTag.toString());
#endif

    pHandler->newProperty(nId, pVal);
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler,
                                     Id nId,
                                     sal_Int32 nVal)
{
    OOXMLValue::Pointer_t pVal(new T(nVal));

    string aStr = (*QNameToString::Instance())(nId);

    if (aStr.size() == 0)
        logger("DEBUG", "unknown QName");

#ifdef DEBUG_PROPERTIES
    XMLTag aTag("newProperty");

    aTag.addAttr("name", aStr);
    aTag.addAttr("value", pVal->toString());

    logger("DEBUG", aTag.toString());
#endif

    pHandler->newProperty(nId, pVal);
}

template <class T>
void OOXMLFastHelper<T>::mark(OOXMLFastContextHandler * pHandler,
                              Id nId,
                              const ::rtl::OUString & rValue)
{
    OOXMLValue::Pointer_t pVal(new T(rValue));

    string aStr = (*QNameToString::Instance())(nId);

    if (aStr.size() == 0)
        logger("DEBUG", "unknown QName");

#ifdef DEBUG_PROPERTIES
    XMLTag aTag("mark");

    aTag.addAttr("name", aStr);
    aTag.addAttr("value",
                 ::rtl::OUStringToOString
                 (rValue, RTL_TEXTENCODING_ASCII_US).getStr());

    logger("DEBUG", aTag.toString());
#endif

    pHandler->mark(nId, pVal);
}

template <class T>
void OOXMLFastHelper<T>::attributes
(OOXMLFastContextHandler * pContext,
 const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
{
    T aContext(pContext);

    aContext.setPropertySet(pContext->getPropertySet());
    aContext.attributes(Attribs);
}

}}
#endif // INCLUDED_FAST_HELPER_HXX
