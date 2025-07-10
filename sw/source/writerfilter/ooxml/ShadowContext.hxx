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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <sax/fastattribs.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/core/contexthandler.hxx>
#include <queue>

namespace writerfilter::ooxml
{
enum class CallDataType
{
    Init,
    ElementAttr,
    Char,
    EndElementAttr,
    Unknown,
    EndUnknown,
    ElementContext,
    UnknownContext
};

class CallData
{
    sal_uInt32 m_nLevel;
    CallDataType m_eType;
    sal_Int32 m_nElement;
    css::uno::Reference<css::xml::sax::XFastAttributeList> m_aAttributes;

    //char
    ::rtl::OUString m_aChars;

    //unknown
    ::rtl::OUString m_sNameSpace;
    ::rtl::OUString m_sElement;

public:
    //Start unknown element or context
    CallData(sal_uInt32 nLevel, const ::rtl::OUString& rNameSpace, const ::rtl::OUString& rElement,
             const css::uno::Reference<css::xml::sax::XFastAttributeList>& rAttributes,
             CallDataType eType)
        : m_nLevel(nLevel)
        , m_eType(eType)
        , m_nElement(0)
        , m_aAttributes(new sax_fastparser::FastAttributeList(rAttributes))
        , m_sNameSpace(rNameSpace)
        , m_sElement(rElement)
    {
    }

    //end unknown element
    CallData(sal_uInt32 nLevel, const ::rtl::OUString& rNameSpace, const ::rtl::OUString& rElement)
        : m_nLevel(nLevel)
        , m_eType(CallDataType::EndUnknown)
        , m_nElement(0)
        , m_sNameSpace(rNameSpace)
        , m_sElement(rElement)
    {
    }

    // start fast element
    CallData(sal_uInt32 nLevel, sal_Int32 nElement,
             const css::uno::Reference<css::xml::sax::XFastAttributeList>& rAttributes,
             CallDataType eType)
        : m_nLevel(nLevel)
        , m_eType(eType)
        , m_nElement(nElement)
        , m_aAttributes(new sax_fastparser::FastAttributeList(rAttributes))
    {
    }

    // end fast element
    CallData(sal_uInt32 nLevel, sal_Int32 nElement)
        : m_nLevel(nLevel)
        , m_eType(CallDataType::EndElementAttr)
        , m_nElement(nElement)
    {
    }

    //chars
    CallData(sal_uInt32 nLevel, const ::rtl::OUString& rChars)
        : m_nLevel(nLevel)
        , m_eType(CallDataType::Char)
        , m_nElement(0)
        , m_aChars(rChars)
    {
    }

    CallData(CallData const&) = default;

    sal_uInt32 getLevel() const { return m_nLevel; }
    CallDataType getType() const { return m_eType; }
    sal_Int32 getElement() const { return m_nElement; }
    const ::rtl::OUString& getChars() { return m_aChars; }
    css::uno::Reference<css::xml::sax::XFastAttributeList> getAttributes() const
    {
        return m_aAttributes;
    }
    const ::rtl::OUString& getUnknownNameSpace() const { return m_sNameSpace; }
    const ::rtl::OUString& getUnknownElement() const { return m_sElement; }
};
class ShadowContext : public ::oox::core::ContextHandler_BASE
{
public:
    explicit ShadowContext(::sal_Int32 Element,
                           const css::uno::Reference<css::xml::sax::XFastAttributeList>& rAttribs);
    virtual ~ShadowContext() override;

    //XFastContextHandler
    virtual void SAL_CALL startFastElement(
        ::sal_Int32 Element,
        const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual void SAL_CALL startUnknownElement(
        const ::rtl::OUString& Namespace, const ::rtl::OUString& Name,
        const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual void SAL_CALL endFastElement(::sal_Int32 Element) override;
    virtual void SAL_CALL endUnknownElement(const ::rtl::OUString& Namespace,
                                            const ::rtl::OUString& Name) override;
    virtual ::css::uno::Reference<::css::xml::sax::XFastContextHandler>
        SAL_CALL createFastChildContext(
            ::sal_Int32 Element,
            const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual ::css::uno::Reference<::css::xml::sax::XFastContextHandler>
        SAL_CALL createUnknownChildContext(
            const ::rtl::OUString& Namespace, const ::rtl::OUString& Name,
            const ::css::uno::Reference<::css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual void SAL_CALL characters(const ::rtl::OUString& aChars) override;

    sal_uInt16 getElementLevel() const { return m_nElementLevel; }
    bool isWriterFrame() const { return m_bImportAsWriterFrame; }

    std::deque<CallData>& getCallData() { return m_aCallDataDeque; }

private:
    std::deque<CallData> m_aCallDataDeque;
    sal_uInt16 m_nElementLevel;
    bool m_bImportAsWriterFrame;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
