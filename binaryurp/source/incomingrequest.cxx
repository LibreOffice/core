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
#include <list>
#include <vector>

#include "com/sun/star/bridge/XInstanceProvider.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "o3tl/runtimetooustring.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "currentcontext.hxx"
#include "incomingrequest.hxx"
#include "specialfunctionids.hxx"

namespace binaryurp {

IncomingRequest::IncomingRequest(
    rtl::Reference< Bridge > const & bridge, rtl::ByteSequence const & tid,
    OUString const & oid, css::uno::UnoInterfaceReference const & object,
    css::uno::TypeDescription const & type, sal_uInt16 functionId,
    bool synchronous, css::uno::TypeDescription const & member, bool setter,
    std::vector< BinaryAny > const & inArguments, bool currentContextMode,
    css::uno::UnoInterfaceReference const & currentContext):
    bridge_(bridge), tid_(tid), oid_(oid), object_(object), type_(type),
    functionId_(functionId), synchronous_(synchronous), member_(member),
    setter_(setter), inArguments_(inArguments),
    currentContextMode_(currentContextMode), currentContext_(currentContext)
{
    assert(bridge.is());
    assert(member.is());
    assert(member.get()->bComplete);
}

IncomingRequest::~IncomingRequest() {}

void IncomingRequest::execute() const {
    BinaryAny ret;
    std::vector< BinaryAny > outArgs;
    bool isExc;
    try {
        bool resetCc = false;
        css::uno::UnoInterfaceReference oldCc;
        if (currentContextMode_) {
            oldCc = current_context::get();
            current_context::set(currentContext_);
            resetCc = true;
        }
        try {
            try {
                isExc = !execute_throw(&ret, &outArgs);
            } catch (const std::exception & e) {
                throw css::uno::RuntimeException(
                    "caught C++ exception: "
                    + o3tl::runtimeToOUString(e.what()));
            }
        } catch (const css::uno::RuntimeException &) {
            css::uno::Any exc(cppu::getCaughtException());
            ret = bridge_->mapCppToBinaryAny(exc);
            isExc = true;
        }
        if (resetCc) {
            current_context::set(oldCc);
        }
    } catch (const css::uno::RuntimeException &) {
        css::uno::Any exc(cppu::getCaughtException());
        ret = bridge_->mapCppToBinaryAny(exc);
        isExc = true;
    }
    if (synchronous_) {
        bridge_->decrementActiveCalls();
        try {
            bridge_->getWriter()->queueReply(
                tid_, member_, setter_, isExc, ret, outArgs, false);
            return;
        } catch (const css::uno::RuntimeException & e) {
            SAL_INFO("binaryurp", "caught UNO runtime exception " << e.Message);
        } catch (const std::exception & e) {
            SAL_INFO("binaryurp", "caught C++ exception " << e.what());
        }
        bridge_->terminate(false);
    } else {
        if (isExc) {
            SAL_INFO("binaryurp", "oneway method raised exception");
        }
        bridge_->decrementCalls();
    }
}

static size_t size_t_round(size_t val)
{
    return (val + (sizeof(size_t)-1)) & ~(sizeof(size_t)-1);
}

bool IncomingRequest::execute_throw(
    BinaryAny * returnValue, std::vector< BinaryAny > * outArguments) const
{
    assert(returnValue != nullptr);
    assert(
        returnValue->getType().equals(
            css::uno::TypeDescription(cppu::UnoType<void>::get())));
    assert(outArguments != nullptr);
    assert(outArguments->empty());
    bool isExc = false;
    switch (functionId_) {
    case SPECIAL_FUNCTION_ID_RESERVED:
        assert(false); // this cannot happen
        break;
    case SPECIAL_FUNCTION_ID_RELEASE:
        bridge_->releaseStub(oid_, type_);
        break;
    case SPECIAL_FUNCTION_ID_QUERY_INTERFACE:
        if (!object_.is()) {
            css::uno::Reference< css::uno::XInterface > ifc;
            css::uno::Reference< css::bridge::XInstanceProvider > prov(
                bridge_->getProvider());
            if (prov.is()) {
                try {
                    ifc = prov->getInstance(oid_);
                } catch (const css::container::NoSuchElementException & e) {
                    SAL_INFO(
                        "binaryurp",
                        "initial element " << oid_
                            << ": NoSuchElementException " << e.Message);
                }
            }
            if (ifc.is()) {
                css::uno::UnoInterfaceReference unoIfc(
                    static_cast< uno_Interface * >(
                        bridge_->getCppToBinaryMapping().mapInterface(
                            ifc.get(),
                            (css::uno::TypeDescription(
                                cppu::UnoType<
                                    css::uno::Reference<
                                        css::uno::XInterface > >::get()).
                             get()))),
                    SAL_NO_ACQUIRE);
                *returnValue = BinaryAny(
                    css::uno::TypeDescription(
                        cppu::UnoType<
                            css::uno::Reference<
                                css::uno::XInterface > >::get()),
                    &unoIfc.m_pUnoI);
            }
            break;
        }
        SAL_FALLTHROUGH;
    default:
        {
            assert(object_.is());
            css::uno::TypeDescription retType;
            std::list< std::vector< char > > outBufs;
            std::vector< void * > args;
            switch (member_.get()->eTypeClass) {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
                {
                    css::uno::TypeDescription t(
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription * >(
                                member_.get())->
                        pAttributeTypeRef);
                    if (setter_) {
                        assert(inArguments_.size() == 1);
                        args.push_back(inArguments_[0].getValue(t));
                    } else {
                        assert(inArguments_.empty());
                        retType = t;
                    }
                    break;
                }
            case typelib_TypeClass_INTERFACE_METHOD:
                {
                    typelib_InterfaceMethodTypeDescription * mtd =
                        reinterpret_cast<
                            typelib_InterfaceMethodTypeDescription * >(
                                member_.get());
                    retType = css::uno::TypeDescription(mtd->pReturnTypeRef);
                    std::vector< BinaryAny >::const_iterator i(
                        inArguments_.begin());
                    for (sal_Int32 j = 0; j != mtd->nParams; ++j) {
                        void * p;
                        if (mtd->pParams[j].bIn) {
                            p = i++->getValue(
                                css::uno::TypeDescription(
                                    mtd->pParams[j].pTypeRef));
                        } else {
                            outBufs.push_back(
                                std::vector< char >(size_t_round(
                                    css::uno::TypeDescription(
                                        mtd->pParams[j].pTypeRef).
                                    get()->nSize)));
                            p = &outBufs.back()[0];
                        }
                        args.push_back(p);
                        if (mtd->pParams[j].bOut) {
                            outArguments->push_back(BinaryAny());
                        }
                    }
                    assert(i == inArguments_.end());
                    break;
                }
            default:
                assert(false); // this cannot happen
                break;
            }
            size_t nSize = 0;
            if (retType.is())
                nSize = size_t_round(retType.get()->nSize);
            std::vector< char > retBuf(nSize);
            uno_Any exc;
            uno_Any * pexc = &exc;
            (*object_.get()->pDispatcher)(
                object_.get(), member_.get(), retBuf.empty() ? nullptr : &retBuf[0],
                args.empty() ? nullptr : &args[0], &pexc);
            isExc = pexc != nullptr;
            if (isExc) {
                *returnValue = BinaryAny(
                    css::uno::TypeDescription(
                        cppu::UnoType< css::uno::Any >::get()),
                    &exc);
                uno_any_destruct(&exc, nullptr);
            } else {
                if (!retBuf.empty()) {
                    *returnValue = BinaryAny(retType, &retBuf[0]);
                    uno_destructData(&retBuf[0], retType.get(), nullptr);
                }
                if (!outArguments->empty()) {
                    assert(
                        member_.get()->eTypeClass ==
                        typelib_TypeClass_INTERFACE_METHOD);
                    typelib_InterfaceMethodTypeDescription * mtd =
                        reinterpret_cast<
                            typelib_InterfaceMethodTypeDescription * >(
                                member_.get());
                    std::vector< BinaryAny >::iterator i(outArguments->begin());
                    std::list< std::vector< char > >::iterator j(
                        outBufs.begin());
                    for (sal_Int32 k = 0; k != mtd->nParams; ++k) {
                        if (mtd->pParams[k].bOut) {
                            *i++ = BinaryAny(
                                css::uno::TypeDescription(
                                    mtd->pParams[k].pTypeRef),
                                args[k]);
                        }
                        if (!mtd->pParams[k].bIn) {
                            uno_type_destructData(
                                &(*j++)[0], mtd->pParams[k].pTypeRef, nullptr);
                        }
                    }
                    assert(i == outArguments->end());
                    assert(j == outBufs.end());
                }
            }
            break;
        }
    }
    return !isExc;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
