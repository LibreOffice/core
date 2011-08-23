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
    debug_logger->endElement("createAndSetParent");
#endif

    uno::Reference<XFastContextHandler> aResult(pTmp);
    
    return aResult;
}

template <class T>
uno::Reference<XFastContextHandler> 
OOXMLFastHelper<T>::createAndSetParentAndDefine
(OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId, Id nDefine)
{
    OOXMLFastContextHandler * pTmp = new T(pHandler);
    
    pTmp->setToken(nToken);
    pTmp->setId(nId);
    pTmp->setDefine(nDefine);
    
#ifdef DEBUG_CREATE
    debug_logger->startElement("createAndSetParentAndDefine");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("token", fastTokenToId(pTmp->getToken()));
    debug_logger->attribute("id", (*QNameToString::Instance())(nId));
    
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "0x%08" SAL_PRIxUINT32, nId);
     
    debug_logger->attribute("idnum", buffer);
    
    debug_logger->startElement("created");
    debug_logger->addTag(pTmp->toTag());
    debug_logger->endElement("created");
    debug_logger->endElement("createAndSetParentAndDefine");
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
    debug_logger->startElement("createAndSetParentRef");
    debug_logger->attribute("context", pHandler->getType());
    debug_logger->attribute("type", fastTokenToId(nToken));
    
    debug_logger->startElement("created");
    debug_logger->chars(pTmp->getType());
    debug_logger->endElement("created");
    debug_logger->endElement("createAndSetParentRef");
#endif

    return xChild;
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler, 
                                     Id nId, 
                                     const ::rtl::OUString & rValue)
{
    OOXMLValue::Pointer_t pVal(new T(rValue));

#ifdef DEBUG_PROPERTIES
    string aStr = (*QNameToString::Instance())(nId); 

    debug_logger->startElement("newProperty-from-string");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute
        ("value", 
         ::rtl::OUStringToOString
         (rValue, RTL_TEXTENCODING_ASCII_US).getStr());

    if (aStr.size() == 0)
        debug_logger->addTag(XMLTag::Pointer_t(new XMLTag("unknown-qname")));
    
    debug_logger->endElement("newProperty-from-string");
#endif

    pHandler->newProperty(nId, pVal);
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler, 
                                     Id nId, 
                                     sal_Int32 nVal)
{
    OOXMLValue::Pointer_t pVal(new T(nVal));

#ifdef DEBUG_PROPERTIES
    string aStr = (*QNameToString::Instance())(nId); 

    debug_logger->startElement("newProperty-from-int");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute("value", pVal->toString());
    
    if (aStr.size() == 0)
        debug_logger->addTag(XMLTag::Pointer_t(new XMLTag("unknown-qname")));
    
    debug_logger->endElement("newProperty-from-int");
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

#ifdef DEBUG_PROPERTIES
    debug_logger->startElement("mark");
    debug_logger->attribute("name", aStr);
    debug_logger->attribute
    ("value",
     ::rtl::OUStringToOString
     (rValue, RTL_TEXTENCODING_ASCII_US).getStr());

    if (aStr.size() == 0)
        debug_logger->addTag(XMLTag::Pointer_t(new XMLTag("unknown-qname")));

    debug_logger->endElement("mark");
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
