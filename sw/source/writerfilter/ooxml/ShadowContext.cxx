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
#include "ShadowContext.hxx"
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>

namespace writerfilter::ooxml
{
using namespace ::com::sun::star;
using namespace oox;
using namespace ::com::sun::star::xml::sax;

ShadowContext::ShadowContext(::sal_Int32 nElement,
                             const uno::Reference<XFastAttributeList>& rAttribs)
    : m_nElementLevel(0)
    , m_bImportAsWriterFrame(false)
{
    CallData callData(m_nElementLevel, nElement, rAttribs, CallDataType::Init);
    m_aCallDataDeque.push_back(callData);
}
ShadowContext::~ShadowContext() {}

void ShadowContext::startFastElement(
    ::sal_Int32 nElement,
    const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& rAttribs)
{
    ++m_nElementLevel;
    CallData callData(m_nElementLevel, nElement, rAttribs, CallDataType::ElementAttr);
    m_aCallDataDeque.push_back(callData);
    if (nElement == (oox::NMSP_doc | oox::XML_tbl))
    {
        m_bImportAsWriterFrame = true;
    }
}

void ShadowContext::startUnknownElement(
    const ::rtl::OUString& rNamespace, const ::rtl::OUString& rElement,
    const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& rAttribs)
{
    ++m_nElementLevel;
    CallData callData(m_nElementLevel, rNamespace, rElement, rAttribs, CallDataType::Unknown);
    m_aCallDataDeque.push_back(callData);
}
void ShadowContext::endFastElement(::sal_Int32 nElement)
{
    --m_nElementLevel;
    CallData callData(m_nElementLevel, nElement);
    m_aCallDataDeque.push_back(callData);
}
void ShadowContext::endUnknownElement(const ::rtl::OUString& rNamespace,
                                      const ::rtl::OUString& rElement)
{
    --m_nElementLevel;
    CallData callData(m_nElementLevel, rNamespace, rElement);
    m_aCallDataDeque.push_back(callData);
}
::css::uno::Reference<::css::xml::sax::XFastContextHandler> ShadowContext::createFastChildContext(
    ::sal_Int32 nElement,
    const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& rAttribs)
{
    CallData callData(m_nElementLevel, nElement, rAttribs, CallDataType::ElementContext);
    m_aCallDataDeque.push_back(callData);
    return this;
}
::css::uno::Reference<::css::xml::sax::XFastContextHandler>
ShadowContext::createUnknownChildContext(
    const ::rtl::OUString& rNamespace, const ::rtl::OUString& rElement,
    const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& rAttribs)
{
    CallData callData(m_nElementLevel, rNamespace, rElement, rAttribs,
                      CallDataType::UnknownContext);
    m_aCallDataDeque.push_back(callData);
    return this;
}
void ShadowContext::characters(const ::rtl::OUString& aChars)
{
    CallData callData(m_nElementLevel, aChars);
    m_aCallDataDeque.push_back(callData);
}
} //namespace
