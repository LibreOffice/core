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

#include "sal/config.h"

#include <exception>
#include <vector>

#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/mutex.hxx"
#include "rtl/memory.h"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "currentcontext.hxx"
#include "specialfunctionids.hxx"
#include "writer.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

Writer::Item::Item() {}

Writer::Item::Item(
    rtl::ByteSequence const & theTid, OUString const & theOid,
    css::uno::TypeDescription const & theType,
    css::uno::TypeDescription const & theMember,
    std::vector< BinaryAny > const & inArguments,
    css::uno::UnoInterfaceReference const & theCurrentContext):
    request(true), tid(theTid), oid(theOid), type(theType), member(theMember),
    arguments(inArguments), currentContext(theCurrentContext)
{}

Writer::Item::Item(
    rtl::ByteSequence const & theTid,
    css::uno::TypeDescription const & theMember, bool theSetter,
    bool theException, BinaryAny const & theReturnValue,
    std::vector< BinaryAny > const & outArguments,
    bool theSetCurrentContextMode):
    request(false), tid(theTid), member(theMember), setter(theSetter),
    arguments(outArguments), exception(theException),
    returnValue(theReturnValue), setCurrentContextMode(theSetCurrentContextMode)
{}

Writer::Writer(rtl::Reference< Bridge > const  & bridge):
    Thread("binaryurpWriter"), bridge_(bridge), marshal_(bridge, state_),
    stop_(false)
{
    OSL_ASSERT(bridge.is());
}

void Writer::sendDirectRequest(
    rtl::ByteSequence const & tid, OUString const & oid,
    css::uno::TypeDescription const & type,
    css::uno::TypeDescription const & member,
    std::vector< BinaryAny > const & inArguments)
{
    OSL_ASSERT(!unblocked_.check());
    sendRequest(
        tid, oid, type, member, inArguments, false,
        css::uno::UnoInterfaceReference());
}

void Writer::sendDirectReply(
    rtl::ByteSequence const & tid, css::uno::TypeDescription const & member,
    bool exception, BinaryAny const & returnValue,
    std::vector< BinaryAny > const & outArguments)
{
    OSL_ASSERT(!unblocked_.check());
    sendReply(tid, member, false, exception, returnValue,outArguments);
}

void Writer::queueRequest(
    rtl::ByteSequence const & tid, OUString const & oid,
    css::uno::TypeDescription const & type,
    css::uno::TypeDescription const & member,
    std::vector< BinaryAny > const & inArguments)
{
    css::uno::UnoInterfaceReference cc(current_context::get());
    osl::MutexGuard g(mutex_);
    queue_.push_back(Item(tid, oid, type, member, inArguments, cc));
    items_.set();
}

void Writer::queueReply(
    rtl::ByteSequence const & tid,
    com::sun::star::uno::TypeDescription const & member, bool setter,
    bool exception, BinaryAny const & returnValue,
    std::vector< BinaryAny > const & outArguments, bool setCurrentContextMode)
{
    osl::MutexGuard g(mutex_);
    queue_.push_back(
        Item(
            tid, member, setter, exception, returnValue, outArguments,
            setCurrentContextMode));
    items_.set();
}

void Writer::unblock() {
    // Assumes that osl::Condition::set works as a memory barrier, so that
    // changes made by preceeding sendDirectRequest/Reply calls are visible to
    // subsequent sendRequest/Reply calls:
    unblocked_.set();
}

void Writer::stop() {
    {
        osl::MutexGuard g(mutex_);
        stop_ = true;
    }
    unblocked_.set();
    items_.set();
}

Writer::~Writer() {}

void Writer::execute() {
    try {
        unblocked_.wait();
        for (;;) {
            items_.wait();
            Item item;
            {
                osl::MutexGuard g(mutex_);
                if (stop_) {
                    return;
                }
                OSL_ASSERT(!queue_.empty());
                item = queue_.front();
                queue_.pop_front();
                if (queue_.empty()) {
                    items_.reset();
                }
            }
            if (item.request) {
                sendRequest(
                    item.tid, item.oid, item.type, item.member, item.arguments,
                    (item.oid != "UrpProtocolProperties" &&
                     !item.member.equals(
                         css::uno::TypeDescription(
                             OUString(
                                 RTL_CONSTASCII_USTRINGPARAM(
                                     "com.sun.star.uno.XInterface::"
                                     "release")))) &&
                     bridge_->isCurrentContextMode()),
                    item.currentContext);
            } else {
                sendReply(
                    item.tid, item.member, item.setter, item.exception,
                    item.returnValue, item.arguments);
                if (item.setCurrentContextMode) {
                    bridge_->setCurrentContextMode();
                }
            }
        }
    } catch (const css::uno::Exception & e) {
        OSL_TRACE(
            OSL_LOG_PREFIX "caught UNO exception '%s'",
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    } catch (const std::exception & e) {
        OSL_TRACE(OSL_LOG_PREFIX "caught C++ exception '%s'", e.what());
    }
    bridge_->terminate(false);
}

void Writer::sendRequest(
    rtl::ByteSequence const & tid, OUString const & oid,
    css::uno::TypeDescription const & type,
    css::uno::TypeDescription const & member,
    std::vector< BinaryAny > const & inArguments, bool currentContextMode,
    css::uno::UnoInterfaceReference const & currentContext)
{
    OSL_ASSERT(tid.getLength() != 0 && !oid.isEmpty() && member.is());
    css::uno::TypeDescription t(type);
    sal_Int32 functionId = 0;
    bool forceSynchronous = false;
    member.makeComplete();
    switch (member.get()->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription * atd =
                reinterpret_cast< typelib_InterfaceAttributeTypeDescription * >(
                    member.get());
            OSL_ASSERT(atd->pInterface != 0);
            if (!t.is()) {
                t = css::uno::TypeDescription(&atd->pInterface->aBase);
            }
            t.makeComplete();
            functionId = atd->pInterface->pMapMemberIndexToFunctionIndex[
                atd->aBase.nPosition];
            if (!inArguments.empty()) { // setter
                ++functionId;
            }
            break;
        }
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription * mtd =
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    member.get());
            OSL_ASSERT(mtd->pInterface != 0);
            if (!t.is()) {
                t = css::uno::TypeDescription(&mtd->pInterface->aBase);
            }
            t.makeComplete();
            functionId = mtd->pInterface->pMapMemberIndexToFunctionIndex[
                mtd->aBase.nPosition];
            forceSynchronous = mtd->bOneWay &&
                functionId != SPECIAL_FUNCTION_ID_RELEASE;
            break;
        }
    default:
        OSL_ASSERT(false); // this cannot happen
        break;
    }
    OSL_ASSERT(functionId >= 0);
    if (functionId > SAL_MAX_UINT16) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM("function ID too large for URP")),
            css::uno::Reference< css::uno::XInterface >());
    }
    std::vector< unsigned char > buf;
    bool newType = !(lastType_.is() && t.equals(lastType_));
    bool newOid = oid != lastOid_;
    bool newTid = tid != lastTid_;
    if (newType || newOid || newTid || forceSynchronous || functionId > 0x3FFF)
        // > 14 bit function ID
    {
        Marshal::write8(
            &buf,
            (0xC0 | (newType ? 0x20 : 0) | (newOid ? 0x10 : 0) |
             (newTid ? 0x08 : 0) | (functionId > 0xFF ? 0x04 : 0) |
             (forceSynchronous ? 0x01 : 0)));
            // bit 7: LONGHEADER, bit 6: REQUEST, bit 5: NEWTYPE, bit 4: NEWOID,
            // bit 3: NEWTID, bit 2: FUNCTIONID16, bit 0: MOREFLAGS
        if (forceSynchronous) {
            Marshal::write8(&buf, 0xC0); // bit 7: MUSTREPLY, bit 6: SYNCHRONOUS
        }
        if (functionId <= 0xFF) {
            Marshal::write8(&buf, static_cast< sal_uInt8 >(functionId));
        } else {
            Marshal::write16(&buf, static_cast< sal_uInt16 >(functionId));
        }
        if (newType) {
            marshal_.writeType(&buf, t);
        }
        if (newOid) {
            marshal_.writeOid(&buf, oid);
        }
        if (newTid) {
            marshal_.writeTid(&buf, tid);
        }
    } else if (functionId <= 0x3F) { // <= 6 bit function ID
        Marshal::write8(&buf, static_cast< sal_uInt8 >(functionId));
            // bit 7: !LONGHEADER, bit 6: !FUNCTIONID14
    } else {
        Marshal::write8(
            &buf, static_cast< sal_uInt8 >(0x40 | (functionId >> 8)));
            // bit 7: !LONGHEADER, bit 6: FUNCTIONID14
        Marshal::write8(&buf, functionId & 0xFF);
    }
    if (currentContextMode) {
        css::uno::UnoInterfaceReference cc(currentContext);
        marshal_.writeValue(
            &buf,
            css::uno::TypeDescription(
                cppu::UnoType<
                    css::uno::Reference< css::uno::XCurrentContext > >::get()),
            BinaryAny(
                css::uno::TypeDescription(
                    cppu::UnoType<
                        css::uno::Reference<
                            css::uno::XCurrentContext > >::get()),
                &cc.m_pUnoI));
    }
    switch (member.get()->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        if (!inArguments.empty()) { // setter
            OSL_ASSERT(inArguments.size() == 1);
            marshal_.writeValue(
                &buf,
                css::uno::TypeDescription(
                    reinterpret_cast<
                        typelib_InterfaceAttributeTypeDescription * >(
                            member.get())->
                    pAttributeTypeRef),
                inArguments.front());
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription * mtd =
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    member.get());
            std::vector< BinaryAny >::const_iterator i(inArguments.begin());
            for (sal_Int32 j = 0; j != mtd->nParams; ++j) {
                if (mtd->pParams[j].bIn) {
                    marshal_.writeValue(
                        &buf,
                        css::uno::TypeDescription(mtd->pParams[j].pTypeRef),
                        *i++);
                }
            }
            OSL_ASSERT(i == inArguments.end());
            break;
        }
    default:
        OSL_ASSERT(false); // this cannot happen
        break;
    }
    sendMessage(buf);
    lastType_ = t;
    lastOid_ = oid;
    lastTid_ = tid;
}

void Writer::sendReply(
    rtl::ByteSequence const & tid,
    com::sun::star::uno::TypeDescription const & member, bool setter,
    bool exception, BinaryAny const & returnValue,
    std::vector< BinaryAny > const & outArguments)
{
    OSL_ASSERT(tid.getLength() != 0 && member.is() && member.get()->bComplete);
    std::vector< unsigned char > buf;
    bool newTid = tid != lastTid_;
    Marshal::write8(&buf, 0x80 | (exception ? 0x20 : 0) | (newTid ? 0x08 : 0));
        // bit 7: LONGHEADER; bit 6: !REQUEST; bit 5: EXCEPTION; bit 3: NEWTID
    if (newTid) {
        marshal_.writeTid(&buf, tid);
    }
    if (exception) {
        marshal_.writeValue(
            &buf,
            css::uno::TypeDescription(cppu::UnoType< css::uno::Any >::get()),
            returnValue);
    } else {
        switch (member.get()->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            if (!setter) {
                marshal_.writeValue(
                    &buf,
                    css::uno::TypeDescription(
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription * >(
                                member.get())->
                        pAttributeTypeRef),
                    returnValue);
            }
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            {
                typelib_InterfaceMethodTypeDescription * mtd =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription * >(
                            member.get());
                marshal_.writeValue(
                    &buf, css::uno::TypeDescription(mtd->pReturnTypeRef),
                    returnValue);
                std::vector< BinaryAny >::const_iterator i(
                    outArguments.begin());
                for (sal_Int32 j = 0; j != mtd->nParams; ++j) {
                    if (mtd->pParams[j].bOut) {
                        marshal_.writeValue(
                            &buf,
                            css::uno::TypeDescription(mtd->pParams[j].pTypeRef),
                            *i++);
                    }
                }
                OSL_ASSERT(i == outArguments.end());
                break;
            }
        default:
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
    sendMessage(buf);
    lastTid_ = tid;
    bridge_->decrementCalls();
}

void Writer::sendMessage(std::vector< unsigned char > const & buffer) {
    std::vector< unsigned char > header;
    if (buffer.size() > SAL_MAX_UINT32) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM("message too large for URP")),
            css::uno::Reference< css::uno::XInterface >());
    }
    Marshal::write32(&header, static_cast< sal_uInt32 >(buffer.size()));
    Marshal::write32(&header, 1);
    OSL_ASSERT(!buffer.empty());
    unsigned char const * p = &buffer[0];
    std::vector< unsigned char >::size_type n = buffer.size();
    OSL_ASSERT(header.size() <= SAL_MAX_INT32 && SAL_MAX_INT32 <= SAL_MAX_SIZE);
    sal_Size k = SAL_MAX_INT32 - header.size();
    if (n < k) {
        k = static_cast< sal_Size >(n);
    }
    css::uno::Sequence< sal_Int8 > s(
        static_cast< sal_Int32 >(header.size() + k));
    OSL_ASSERT(!header.empty());
    rtl_copyMemory(
        s.getArray(), &header[0], static_cast< sal_Size >(header.size()));
    for (;;) {
        rtl_copyMemory(s.getArray() + s.getLength() - k, p, k);
        try {
            bridge_->getConnection()->write(s);
        } catch (const css::io::IOException & e) {
            css::uno::Any exc(cppu::getCaughtException());
            throw css::lang::WrappedTargetRuntimeException(
                (OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Binary URP write raised IO exception: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >(), exc);
        }
        n = static_cast< std::vector< unsigned char >::size_type >(n - k);
        if (n == 0) {
            break;
        }
        p += k;
        k = SAL_MAX_INT32;
        if (n < k) {
            k = static_cast< sal_Size >(n);
        }
        s.realloc(k);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
