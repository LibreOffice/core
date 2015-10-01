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
#include <ooxml/QNameToString.hxx>
#include "OOXMLFastContextHandler.hxx"

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
                            Id nId, sal_Int32 nValue);
};

template <class T>
css::uno::Reference<css::xml::sax::XFastContextHandler> OOXMLFastHelper<T>::createAndSetParentAndDefine (OOXMLFastContextHandler * pHandler, sal_uInt32 nToken, Id nId, Id nDefine)
{
    OOXMLFastContextHandler * pTmp = new T(pHandler);

    pTmp->setToken(nToken);
    pTmp->setId(nId);
    pTmp->setDefine(nDefine);

    css::uno::Reference<css::xml::sax::XFastContextHandler> aResult(pTmp);

    return aResult;
}

template <class T>
void OOXMLFastHelper<T>::newProperty(OOXMLFastContextHandler * pHandler,
                                     Id nId,
                                     sal_Int32 nVal)
{
    OOXMLValue::Pointer_t pVal(T::Create(nVal));

    pHandler->newProperty(nId, pVal);
}

}}
#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
