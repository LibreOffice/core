/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:04:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    logger("DEBUG", "<createAndSetParent>");
    logger("DEBUG", "<context>");
    logger("DEBUG", pHandler->getType());
    logger("DEBUG", "</context>");
    logger("DEBUG", "<token>");
    logger("DEBUG", fastTokenToId(nToken));
    logger("DEBUG", "</token>");
    logger("DEBUG", "<created>");
    logger("DEBUG", pTmp->getType());
    logger("DEBUG", "</created>");
    if (pTmp->isFallback())
        logger("DEBUG", "<fallback/>");
    logger("DEBUG", "</createAndSetParent>");
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
    logger("DEBUG", "<createAndSetParentRef>");
    logger("DEBUG", "<context>");
    logger("DEBUG", pHandler->getType());
    logger("DEBUG", "</context>");
    logger("DEBUG", "<token>");
    logger("DEBUG", fastTokenToId(nToken));
    logger("DEBUG", "</token>");
    if (pResult != NULL)
    {
        logger("DEBUG", "<created>");
        logger("DEBUG", pResult->getType());
        logger("DEBUG", "</created>");
        if (pResult->isFallback())
            logger("DEBUG", "<fallback/>");
    }
    logger("DEBUG", "</createAndSetParentRef>");
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
    logger("DEBUG", "<newProperty>");
    logger("DEBUG", "<name>");
    logger("DEBUG", aStr);
    logger("DEBUG", "</name>");
    logger("DEBUG", "<value>");
    logger("DEBUG", ::rtl::OUStringToOString(rValue, RTL_TEXTENCODING_ASCII_US).getStr());
    logger("DEBUG", "</value>");
    logger("DEBUG", "</newProperty>");
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
    logger("DEBUG", "<newProperty>");
    logger("DEBUG", "<name>");
    logger("DEBUG", aStr);
    logger("DEBUG", "</name>");
    logger("DEBUG", "<value>");
    logger("DEBUG", pVal->toString());
    logger("DEBUG", "</value>");
    logger("DEBUG", "</newProperty>");
#endif

    pHandler->newProperty(nId, pVal);
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
