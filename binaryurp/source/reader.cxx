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

#include <cassert>
#include <exception>
#include <memory>
#include <vector>

#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "rtl/byteseq.h"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "incomingreply.hxx"
#include "incomingrequest.hxx"
#include "outgoingrequest.hxx"
#include "reader.hxx"
#include "specialfunctionids.hxx"
#include "unmarshal.hxx"

#include <boost/scoped_ptr.hpp>

namespace binaryurp {

namespace {

namespace css = com::sun::star;

css::uno::Sequence< sal_Int8 > read(
    css::uno::Reference< css::connection::XConnection > const & connection,
    sal_uInt32 size, bool eofOk)
{
    assert(connection.is());
    if (size > SAL_MAX_INT32) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "binaryurp::Reader: block size too large")),
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Sequence< sal_Int8 > buf;
    sal_Int32 n = connection->read(buf, static_cast< sal_Int32 >(size));
    if (n == 0 && eofOk) {
        return css::uno::Sequence< sal_Int8 >();
    }
    if (n != static_cast< sal_Int32 >(size)) {
        throw css::io::IOException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "binaryurp::Reader: premature end of input")),
            css::uno::Reference< css::uno::XInterface >());
    }
    assert(buf.getLength() == static_cast< sal_Int32 >(size));
    return buf;
}

extern "C" void SAL_CALL request(void * pThreadSpecificData) {
    assert(pThreadSpecificData != 0);
    boost::scoped_ptr< IncomingRequest >(
        static_cast< IncomingRequest * >(pThreadSpecificData))->
        execute();
}

}

Reader::Reader(rtl::Reference< Bridge > const & bridge):
    Thread("binaryurpReader"), bridge_(bridge)
{
    assert(bridge.is());
}

Reader::~Reader() {}

void Reader::execute() {
    try {
        bridge_->sendRequestChangeRequest();
        css::uno::Reference< css::connection::XConnection > con(
            bridge_->getConnection());
        for (;;) {
            css::uno::Sequence< sal_Int8 > s(read(con, 8, true));
            if (s.getLength() == 0) {
                break;
            }
            Unmarshal header(bridge_, state_, s);
            sal_uInt32 size = header.read32();
            sal_uInt32 count = header.read32();
            header.done();
            if (count == 0) {
                throw css::io::IOException(
                    OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "binaryurp::Reader: block with zero message count"
                            " received")),
                    css::uno::Reference< css::uno::XInterface >());
            }
            Unmarshal block(bridge_, state_, read(con, size, false));
            for (sal_uInt32 i = 0; i != count; ++i) {
                readMessage(block);
            }
            block.done();
        }
    } catch (const css::uno::Exception & e) {
        SAL_WARN("binaryurp", "caught UNO exception '" << e.Message << '\'');
    } catch (const std::exception & e) {
        SAL_WARN("binaryurp", "caught C++ exception '" << e.what() << '\'');
    }
    bridge_->terminate(false);
}

void Reader::readMessage(Unmarshal & unmarshal) {
    sal_uInt8 flags1 = unmarshal.read8();
    bool newType;
    bool newOid;
    bool newTid;
    bool forceSynchronous;
    sal_uInt16 functionId;
    if ((flags1 & 0x80) != 0) { // bit 7: LONGHEADER
        if ((flags1 & 0x40) == 0) { // bit 6: REQUEST
            readReplyMessage(unmarshal, flags1);
            return;
        }
        newType = (flags1 & 0x20) != 0; // bit 5: NEWTYPE
        newOid = (flags1 & 0x10) != 0; // bit 4: NEWOID
        newTid = (flags1 & 0x08) != 0; // bit 3: NEWTID
        if ((flags1 & 0x01) != 0) { // bit 0: MOREFLAGSS
            sal_uInt8 flags2 = unmarshal.read8();
            forceSynchronous = (flags2 & 0x80) != 0; // bit 7: MUSTREPLY
            if (((flags2 & 0x40) != 0) != forceSynchronous) {
                    // bit 6: SYNCHRONOUS
                throw css::uno::RuntimeException(
                    OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "URP: request message with MUSTREPLY != SYNCHRONOUS"
                            " received")),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else {
            forceSynchronous = false;
        }
        functionId = ((flags1 & 0x04) != 0) // bit 2: FUNCTIONID16
            ? unmarshal.read16() : unmarshal.read8();
    } else {
        newType = false;
        newOid = false;
        newTid = false;
        forceSynchronous = false;
        functionId = ((flags1 & 0x40) != 0) // bit 6: FUNCTIONID14
            ? ((flags1 & 0x3F) << 8) | unmarshal.read8() : flags1 & 0x3F;
    }
    css::uno::TypeDescription type;
    if (newType) {
        type = unmarshal.readType();
        lastType_ = type;
    } else {
        if (!lastType_.is()) {
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "URP: request message with NEWTYPE received when last"
                        " interface type has not yet been set")),
                css::uno::Reference< css::uno::XInterface >());
        }
        type = lastType_;
    }
    OUString oid;
    if (newOid) {
        oid = unmarshal.readOid();
        if (oid.isEmpty()) {
            throw css::io::IOException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "binaryurp::Unmarshal: emtpy OID")),
                css::uno::Reference< css::uno::XInterface >());
        }
        lastOid_ = oid;
    } else {
        if (lastOid_.isEmpty()) {
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "URP: request message with NEWOID received when last"
                        " OID has not yet been set")),
                css::uno::Reference< css::uno::XInterface >());
        }
        oid = lastOid_;
    }
    rtl::ByteSequence tid(getTid(unmarshal, newTid));
    lastTid_ = tid;
    type.makeComplete();
    if (type.get()->eTypeClass != typelib_TypeClass_INTERFACE) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: request message with non-interface interface type"
                    " received")),
            css::uno::Reference< css::uno::XInterface >());
    }
    typelib_InterfaceTypeDescription * itd =
        reinterpret_cast< typelib_InterfaceTypeDescription * >(type.get());
    if (functionId >= itd->nMapFunctionIndexToMemberIndex) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: request message with unknown function ID received")),
            css::uno::Reference< css::uno::XInterface >());
    }
    sal_Int32 memberId = itd->pMapFunctionIndexToMemberIndex[functionId];
    css::uno::TypeDescription memberTd(itd->ppAllMembers[memberId]);
    memberTd.makeComplete();
    assert(memberTd.is());
    bool protProps = bridge_->isProtocolPropertiesRequest(oid, type);
    bool ccMode = !protProps && functionId != SPECIAL_FUNCTION_ID_RELEASE &&
        bridge_->isCurrentContextMode();
    css::uno::UnoInterfaceReference cc;
    if (ccMode) {
        css::uno::TypeDescription t(
            cppu::UnoType< css::uno::Reference< css::uno::XCurrentContext > >::
            get());
        cc.set(
            *static_cast< uno_Interface ** >(
                unmarshal.readValue(t).getValue(t)));
    }
    bool synchronous;
    if (memberTd.get()->eTypeClass == typelib_TypeClass_INTERFACE_METHOD &&
        (reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
            memberTd.get())->
         bOneWay))
    {
        synchronous = forceSynchronous;
    } else {
        if (forceSynchronous) {
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "URP: synchronous request message with non-oneway"
                        " function ID received")),
                css::uno::Reference< css::uno::XInterface >());
        }
        synchronous = true;
    }
    bool setter = false;
    std::vector< BinaryAny > inArgs;
    switch (memberTd.get()->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        setter = itd->pMapMemberIndexToFunctionIndex[memberId] != functionId;
            // pMapMemberIndexToFunctionIndex contains function index of
            // attribute getter
        if (setter) {
            inArgs.push_back(
                unmarshal.readValue(
                    css::uno::TypeDescription(
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription * >(
                                memberTd.get())->
                        pAttributeTypeRef)));
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription * mtd =
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    memberTd.get());
            for (sal_Int32 i = 0; i != mtd->nParams; ++i) {
                if (mtd->pParams[i].bIn) {
                    inArgs.push_back(
                        unmarshal.readValue(
                            css::uno::TypeDescription(
                                mtd->pParams[i].pTypeRef)));
                }
            }
            break;
        }
    default:
        assert(false); // this cannot happen
        break;
    }
    bridge_->incrementCalls(
        !protProps && functionId != SPECIAL_FUNCTION_ID_RELEASE);
    if (protProps) {
        switch (functionId) {
        case SPECIAL_FUNCTION_ID_REQUEST_CHANGE:
            bridge_->handleRequestChangeRequest(tid, inArgs);
            break;
        case SPECIAL_FUNCTION_ID_COMMIT_CHANGE:
            bridge_->handleCommitChangeRequest(tid, inArgs);
            break;
        default:
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "URP: request message with UrpProtocolProperties OID"
                        " and unknown function ID received")),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        css::uno::UnoInterfaceReference obj;
        switch (functionId) {
        case SPECIAL_FUNCTION_ID_QUERY_INTERFACE:
            obj = bridge_->findStub(oid, type);
            if (!obj.is()) {
                assert(
                    inArgs.size() == 1
                    && inArgs[0].getType().equals(
                        css::uno::TypeDescription(
                            cppu::UnoType< css::uno::Type >::get())));
                if (!(type.equals(
                          css::uno::TypeDescription(
                              cppu::UnoType<
                                  css::uno::Reference<
                                      css::uno::XInterface > >::get()))
                      && (css::uno::TypeDescription(
                              *static_cast<
                                  typelib_TypeDescriptionReference ** >(
                                      inArgs[0].getValue(inArgs[0].getType()))).
                          equals(
                              css::uno::TypeDescription(
                                  cppu::UnoType<
                                      css::uno::Reference<
                                          css::uno::XInterface > >::get())))))
                {
                    throw css::uno::RuntimeException(
                        OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "URP: queryInterface request message with"
                                " unknown OID received")),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            break;
        case SPECIAL_FUNCTION_ID_RESERVED:
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "URP: request message with unknown function ID 1"
                        " received")),
                css::uno::Reference< css::uno::XInterface >());
        case SPECIAL_FUNCTION_ID_RELEASE:
            break;
        default:
            obj = bridge_->findStub(oid, type);
            if (!obj.is()) {
                throw css::uno::RuntimeException(
                    OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "URP: request message with unknown OID received")),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        }
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< IncomingRequest > req(
            new IncomingRequest(
                bridge_, tid, oid, obj, type, functionId, synchronous, memberTd,
                setter, inArgs, ccMode, cc));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if (synchronous) {
            bridge_->incrementActiveCalls();
        }
        uno_threadpool_putJob(
            bridge_->getThreadPool(), tid.getHandle(), req.get(), &request,
            !synchronous);
        req.release();
    }
}

void Reader::readReplyMessage(Unmarshal & unmarshal, sal_uInt8 flags1) {
    rtl::ByteSequence tid(getTid(unmarshal, (flags1 & 0x08) != 0));
        // bit 3: NEWTID
    lastTid_ = tid;
    OutgoingRequest req(bridge_->lastOutgoingRequest(tid));
    bool exc = (flags1 & 0x20) != 0; // bit 5: EXCEPTION
    BinaryAny ret;
    std::vector< BinaryAny > outArgs;
    if (exc) {
        ret = unmarshal.readValue(
            css::uno::TypeDescription(cppu::UnoType< css::uno::Any >::get()));
        if (!typelib_typedescription_isAssignableFrom(
                (css::uno::TypeDescription(
                    cppu::UnoType< css::uno::RuntimeException >::get()).
                 get()),
                ret.getType().get()))
        {
            sal_Int32 n = 0;
            typelib_TypeDescriptionReference ** p = 0;
            switch (req.member.get()->eTypeClass) {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
                {
                    typelib_InterfaceAttributeTypeDescription * atd =
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription * >(
                                req.member.get());
                    n = req.setter ? atd->nSetExceptions : atd->nGetExceptions;
                    p = req.setter
                        ? atd->ppSetExceptions : atd->ppGetExceptions;
                    break;
                }
            case typelib_TypeClass_INTERFACE_METHOD:
                {
                    typelib_InterfaceMethodTypeDescription * mtd =
                        reinterpret_cast<
                            typelib_InterfaceMethodTypeDescription * >(
                                req.member.get());
                    n = mtd->nExceptions;
                    p = mtd->ppExceptions;
                    break;
                }
            default:
                assert(false); // this cannot happen
                break;
            }
            bool ok = false;
            for (sal_Int32 i = 0; i != n; ++i) {
                if (typelib_typedescriptionreference_isAssignableFrom(
                        p[i],
                        reinterpret_cast< typelib_TypeDescriptionReference * >(
                            ret.getType().get())))
                {
                    ok = true;
                    break;
                }
            }
            if (!ok) {
                throw css::uno::RuntimeException(
                    OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "URP: reply message with bad exception type"
                            " received")),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    } else {
        switch (req.member.get()->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            if (!req.setter) {
                ret = unmarshal.readValue(
                    css::uno::TypeDescription(
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription * >(
                                req.member.get())->
                        pAttributeTypeRef));
            }
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            {
                typelib_InterfaceMethodTypeDescription * mtd =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription * >(
                            req.member.get());
                ret = unmarshal.readValue(
                    css::uno::TypeDescription(mtd->pReturnTypeRef));
                for (sal_Int32 i = 0; i != mtd->nParams; ++i) {
                    if (mtd->pParams[i].bOut) {
                        outArgs.push_back(
                            unmarshal.readValue(
                                css::uno::TypeDescription(
                                    mtd->pParams[i].pTypeRef)));
                    }
                }
                break;
            }
        default:
            assert(false); // this cannot happen
            break;
        }
    }
    switch (req.kind) {
    case OutgoingRequest::KIND_NORMAL:
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            std::auto_ptr< IncomingReply > resp(
                new IncomingReply(exc, ret, outArgs));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            uno_threadpool_putJob(
                bridge_->getThreadPool(), tid.getHandle(), resp.get(), 0,
                false);
            resp.release();
            break;
        }
    case OutgoingRequest::KIND_REQUEST_CHANGE:
        assert(outArgs.empty());
        bridge_->handleRequestChangeReply(exc, ret);
        break;
    case OutgoingRequest::KIND_COMMIT_CHANGE:
        assert(outArgs.empty());
        bridge_->handleCommitChangeReply(exc, ret);
        break;
    default:
        assert(false); // this cannot happen
        break;
    }
}

rtl::ByteSequence Reader::getTid(Unmarshal & unmarshal, bool newTid) const {
    if (newTid) {
        return unmarshal.readTid();
    }
    if (lastTid_.getLength() == 0) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: message with NEWTID received when last TID has not"
                    " yet been set")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return lastTid_;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
