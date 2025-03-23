/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "net_bridge.hxx"
#include "net_proxy.hxx"

#include <cstring>
#include <vector>

#include <frozen/unordered_map.h>

#include <bridges/net_uno/net_context.hxx>

namespace net_uno
{
namespace
{
size_t net_sizeof(typelib_TypeClass eTypeClass)
{
    static constexpr frozen::unordered_map<typelib_TypeClass, size_t, 19> s_sizes{
        { typelib_TypeClass_BOOLEAN, sizeof(sal_Bool) },
        { typelib_TypeClass_BYTE, sizeof(sal_Int8) },
        { typelib_TypeClass_CHAR, sizeof(sal_Unicode) },
        { typelib_TypeClass_SHORT, sizeof(sal_Int16) },
        { typelib_TypeClass_UNSIGNED_SHORT, sizeof(sal_uInt16) },
        { typelib_TypeClass_LONG, sizeof(sal_Int32) },
        { typelib_TypeClass_UNSIGNED_LONG, sizeof(sal_uInt32) },
        { typelib_TypeClass_HYPER, sizeof(sal_Int64) },
        { typelib_TypeClass_UNSIGNED_HYPER, sizeof(sal_uInt64) },
        { typelib_TypeClass_FLOAT, sizeof(float) },
        { typelib_TypeClass_DOUBLE, sizeof(double) },
        { typelib_TypeClass_ENUM, sizeof(sal_Int32) },
        { typelib_TypeClass_STRING, sizeof(IntPtr) },
        { typelib_TypeClass_TYPE, sizeof(IntPtr) },
        { typelib_TypeClass_ANY, sizeof(IntPtr) + sizeof(IntPtr) },
        { typelib_TypeClass_SEQUENCE, sizeof(IntPtr) + sizeof(sal_Int32) },
        { typelib_TypeClass_INTERFACE, sizeof(IntPtr) },
        { typelib_TypeClass_EXCEPTION, sizeof(IntPtr) },
        { typelib_TypeClass_STRUCT, sizeof(IntPtr) },
    };
    return s_sizes.at(eTypeClass);
}

IntPtr alloc_net_string(const OUString& str)
{
    const sal_Unicode* src = str.getStr();
    sal_Int32 len = str.getLength();

    void* dst = std::malloc((len + 1) * sizeof(sal_Unicode));
    std::memcpy(dst, src, len * sizeof(sal_Unicode));
    static_cast<sal_Unicode*>(dst)[len] = u'\0';

    return dst;
}

uno_Sequence* alloc_uno_sequence(sal_Int32 nElements, sal_Int32 nElementSize, void* data)
{
    void* mem = std::malloc(SAL_SEQUENCE_HEADER_SIZE + nElements * nElementSize);

    uno_Sequence* seq = static_cast<uno_Sequence*>(mem);
    seq->nRefCount = 1;
    seq->nElements = nElements;

    if (data)
        std::memcpy(seq->elements, data, nElements * nElementSize);

    return seq;
}

void marshal_data(void* pUnoData, void* pNetData, typelib_TypeDescriptionReference* pTDRef,
                  bool bDestructValue, Bridge& bridge)
{
    switch (pTDRef->eTypeClass)
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
        case typelib_TypeClass_ENUM:
            std::memcpy(pNetData, pUnoData, net_sizeof(pTDRef->eTypeClass));
            break;
        case typelib_TypeClass_STRING:
        {
            IntPtr* ppNetStr = static_cast<IntPtr*>(pNetData);
            rtl_uString* pUnoStr = *static_cast<rtl_uString**>(pUnoData);

            if (bDestructValue && pNetData)
                std::free(pNetData);

            *ppNetStr = alloc_net_string(OUString::unacquired(&pUnoStr));
            break;
        }
        case typelib_TypeClass_TYPE:
        {
            IntPtr* ppNetType = static_cast<IntPtr*>(pNetData);
            typelib_TypeDescriptionReference* pUnoType
                = *static_cast<typelib_TypeDescriptionReference**>(pUnoData);

            if (bDestructValue && pNetData)
                std::free(pNetData);

            *ppNetType = alloc_net_string(map_uno_type_to_net(pUnoType));
            break;
        }
        case typelib_TypeClass_ANY:
        {
            Value::Any* ppNetAny = static_cast<Value::Any*>(pNetData);
            uno_Any* pUnoAny = static_cast<uno_Any*>(pUnoData);

            if (bDestructValue && ppNetAny->type)
                std::free(ppNetAny->type);
            if (bDestructValue && ppNetAny->data)
                std::free(ppNetAny->data);

            ppNetAny->type = alloc_net_string(map_uno_type_to_net(pUnoAny->pType));

            switch (pUnoAny->pType->eTypeClass)
            {
                case typelib_TypeClass_VOID:
                    ppNetAny->data = nullptr;
                    break;
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_ENUM:
                    std::memcpy(&ppNetAny->data, pUnoAny->pData, sizeof(IntPtr));
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                case typelib_TypeClass_DOUBLE:
                {
                    size_t size = net_sizeof(pUnoAny->pType->eTypeClass);
                    if (size <= sizeof(IntPtr))
                    {
                        std::memcpy(&ppNetAny->data, pUnoAny->pData, sizeof(IntPtr));
                    }
                    else
                    {
                        IntPtr mem = std::malloc(size);
                        std::memcpy(mem, pUnoAny->pData, size);
                        std::free(ppNetAny->data);
                        ppNetAny->data = mem;
                    }
                    break;
                }
                case typelib_TypeClass_ANY:
                case typelib_TypeClass_SEQUENCE:
                {
                    IntPtr mem = std::malloc(net_sizeof(pUnoAny->pType->eTypeClass));
                    marshal_data(pUnoAny->pData, mem, pUnoAny->pType, bDestructValue, bridge);
                    std::free(ppNetAny->data);
                    ppNetAny->data = mem;
                    break;
                }
                case typelib_TypeClass_STRING:
                case typelib_TypeClass_TYPE:
                case typelib_TypeClass_INTERFACE:
                case typelib_TypeClass_EXCEPTION:
                case typelib_TypeClass_STRUCT:
                    marshal_data(pUnoAny->pData, &ppNetAny->data, pUnoAny->pType, bDestructValue,
                                 bridge);
                    break;
                default:
                {
                    throw BridgeRuntimeError(SAL_WHERE,
                                             "could not map "
                                                 + OUString::unacquired(&pUnoAny->pType->pTypeName)
                                                 + " out of an UNO any");
                }
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            Value::Sequence* ppNetSeq = static_cast<Value::Sequence*>(pNetData);
            uno_Sequence* pUnoSeq = *static_cast<uno_Sequence**>(pUnoData);

            if (bDestructValue && ppNetSeq->data)
                std::free(ppNetSeq->data);

            ppNetSeq->length = pUnoSeq->nElements;

            TypeDescHolder type(pTDRef);
            typelib_TypeDescriptionReference* pElemTDRef
                = reinterpret_cast<typelib_IndirectTypeDescription*>(type.get())->pType;

            size_t nNetElemSize = net_sizeof(pElemTDRef->eTypeClass);

            switch (pElemTDRef->eTypeClass)
            {
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                case typelib_TypeClass_ENUM:
                {
                    ppNetSeq->data = std::malloc(nNetElemSize * ppNetSeq->length);
                    std::memcpy(ppNetSeq->data, pUnoSeq->elements, nNetElemSize * ppNetSeq->length);
                    break;
                }
                case typelib_TypeClass_ANY:
                case typelib_TypeClass_SEQUENCE:
                case typelib_TypeClass_STRING:
                case typelib_TypeClass_TYPE:
                case typelib_TypeClass_INTERFACE:
                case typelib_TypeClass_EXCEPTION:
                case typelib_TypeClass_STRUCT:
                {
                    TypeDescHolder elemType(pElemTDRef);
                    sal_Int32 nUnoElemSize = elemType.get()->nSize;

                    ppNetSeq->data = std::malloc(nUnoElemSize * ppNetSeq->length);

                    // Convert each element
                    for (int nPos = 0; nPos < ppNetSeq->length; ++nPos)
                    {
                        void* pNetElem = static_cast<char*>(ppNetSeq->data) + (nPos * nNetElemSize);
                        void* pUnoElem = pUnoSeq->elements + (nPos * nUnoElemSize);
                        marshal_data(pUnoElem, pNetElem, pElemTDRef, bDestructValue, bridge);
                    }
                    break;
                }
                default:
                {
                    throw BridgeRuntimeError(
                        SAL_WHERE, "could not map " + OUString::unacquired(&pElemTDRef->pTypeName)
                                       + " out of an UNO sequence");
                }
            }
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            IntPtr* ppNetI = static_cast<IntPtr*>(pNetData);
            uno_Interface* pUnoI = *static_cast<uno_Interface**>(pUnoData);

            if (pUnoI)
            {
                Context* pCtx = static_cast<Context*>(bridge.m_net_env->pContext);

                // Get oid and type name
                rtl_uString* pOid = nullptr;
                (*bridge.m_uno_env->getObjectIdentifier)(bridge.m_uno_env, &pOid, pUnoI);
                const OUString& sOid = OUString::unacquired(&pOid);

                OUString sTypeName = map_uno_type_to_net(pTDRef);

                // Get the proxy if already created, else create new
                *ppNetI = pCtx->lookupInterface(sOid.getStr(), sTypeName.getStr());
                if (!*ppNetI)
                {
                    TypeDescHolder type(pTDRef);
                    typelib_InterfaceTypeDescription* pTD
                        = reinterpret_cast<typelib_InterfaceTypeDescription*>(type.get());

                    // TODO: check whether lifetime control is correct
                    bridge.acquire();
                    (*pUnoI->acquire)(pUnoI);
                    typelib_typedescription_acquire(&pTD->aBase);
                    (*bridge.m_uno_env->registerInterface)(
                        bridge.m_uno_env, reinterpret_cast<void**>(&pUnoI), pOid, pTD);
                    *ppNetI
                        = pCtx->createProxy(sOid.getStr(), sTypeName.getStr(), &bridge, pUnoI, pTD);
                }
            }
            else
            {
                *ppNetI = nullptr;
            }
            break;
        }
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        {
            IntPtr* ppNetStruct = static_cast<IntPtr*>(pNetData);
            void* pUnoStruct = pUnoData;

            if (bDestructValue && *ppNetStruct)
                std::free(*ppNetStruct);

            TypeDescHolder type(pTDRef);
            typelib_CompoundTypeDescription* pCompTD
                = reinterpret_cast<typelib_CompoundTypeDescription*>(type.get());
            if (!type.get()->bComplete)
                typelib_typedescription_complete(
                    reinterpret_cast<typelib_TypeDescription**>(&pCompTD));

            size_t nBytes = 0;
            std::vector<std::pair<typelib_TypeDescriptionReference*, sal_Int32>> vecMembers;
            for (typelib_CompoundTypeDescription* pHierTD = pCompTD; pHierTD;
                 pHierTD = pHierTD->pBaseTypeDescription)
            {
                for (int n = pHierTD->nMembers - 1; n >= 0; n--)
                {
                    vecMembers.emplace_back(pHierTD->ppTypeRefs[n], pHierTD->pMemberOffsets[n]);
                    nBytes += net_sizeof(pHierTD->ppTypeRefs[n]->eTypeClass);
                }
            }

            *ppNetStruct = std::malloc(nBytes);

            size_t nNetOffset = 0;
            int nPos = vecMembers.size() - 1;
            for (; nPos >= 0; nPos--)
            {
                auto[pMemberTDRef, nUnoOffset] = vecMembers[nPos];

                void* pUnoField = static_cast<char*>(pUnoStruct) + nUnoOffset;
                void* pNetField = static_cast<char*>(*ppNetStruct) + nNetOffset;

                switch (pMemberTDRef->eTypeClass)
                {
                    case typelib_TypeClass_BOOLEAN:
                    case typelib_TypeClass_BYTE:
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_FLOAT:
                    case typelib_TypeClass_DOUBLE:
                    case typelib_TypeClass_ENUM:
                        std::memcpy(pNetField, pUnoField, net_sizeof(pMemberTDRef->eTypeClass));
                        break;
                    case typelib_TypeClass_ANY:
                    case typelib_TypeClass_SEQUENCE:
                    case typelib_TypeClass_STRING:
                    case typelib_TypeClass_TYPE:
                    case typelib_TypeClass_INTERFACE:
                    case typelib_TypeClass_EXCEPTION:
                    case typelib_TypeClass_STRUCT:
                        marshal_data(pUnoField, pNetField, pMemberTDRef, bDestructValue, bridge);
                        break;
                    default:
                    {
                        throw BridgeRuntimeError(
                            SAL_WHERE,
                            "could not map " + OUString::unacquired(&pMemberTDRef->pTypeName)
                                + " out of an UNO " + OUString::unacquired(&pTDRef->pTypeName));
                    }
                }

                nNetOffset += net_sizeof(pMemberTDRef->eTypeClass);
            }
            break;
        }
        default:
        {
            throw BridgeRuntimeError(SAL_WHERE, "could not map "
                                                    + OUString::unacquired(&pTDRef->pTypeName)
                                                    + " value to .NET");
        }
    }
}

void unmarshal_data(void* pUnoData, void* pNetData, typelib_TypeDescriptionReference* pTDRef,
                    bool bDestructObject, bool bAssignData, Bridge& bridge)
{
    switch (pTDRef->eTypeClass)
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
        case typelib_TypeClass_ENUM:
            if (bAssignData)
                std::memcpy(pUnoData, pNetData, net_sizeof(pTDRef->eTypeClass));
            break;
        case typelib_TypeClass_STRING:
        {
            rtl_uString** ppUnoStr = static_cast<rtl_uString**>(pUnoData);
            IntPtr pNetStr = *static_cast<IntPtr*>(pNetData);

            if (bDestructObject && ppUnoStr)
                rtl_uString_release(*ppUnoStr);

            if (bAssignData)
            {
                *ppUnoStr = nullptr;
                if (pNetStr)
                    rtl_uString_newFromStr(ppUnoStr, static_cast<String>(pNetStr));
                else
                    rtl_uString_new(ppUnoStr);
            }

            std::free(pNetStr);
            break;
        }
        case typelib_TypeClass_TYPE:
        {
            typelib_TypeDescriptionReference** ppUnoType
                = static_cast<typelib_TypeDescriptionReference**>(pUnoData);
            IntPtr pNetType = *static_cast<IntPtr*>(pNetData);

            if (bDestructObject && ppUnoType)
                typelib_typedescriptionreference_release(*ppUnoType);

            if (bAssignData)
                *ppUnoType = map_net_type_to_uno(OUString(static_cast<String>(pNetType)));

            std::free(pNetType);
            break;
        }
        case typelib_TypeClass_ANY:
        {
            uno_Any* pUnoAny = static_cast<uno_Any*>(pUnoData);
            Value::Any* pNetAny = static_cast<Value::Any*>(pNetData);

            if (bDestructObject && pUnoData)
                uno_any_destruct(pUnoAny, nullptr);

            typelib_TypeDescriptionReference* pValueTDRef
                = map_net_type_to_uno(OUString(static_cast<String>(pNetAny->type)));
            std::free(pNetAny->type);

            if (bAssignData)
            {
                switch (pValueTDRef->eTypeClass)
                {
                    case typelib_TypeClass_VOID:
                    {
                        pUnoAny->pType = pValueTDRef;
                        pUnoAny->pData = &pUnoAny->pReserved;
                        break;
                    }
                    case typelib_TypeClass_BOOLEAN:
                    case typelib_TypeClass_BYTE:
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                    case typelib_TypeClass_FLOAT:
                    case typelib_TypeClass_ENUM:
                    {
                        pUnoAny->pType = pValueTDRef;
                        pUnoAny->pData = &pUnoAny->pReserved;
                        std::memcpy(pUnoAny->pData, &pNetAny->data, sizeof(IntPtr));
                        break;
                    }
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                    {
                        pUnoAny->pType = pValueTDRef;
                        size_t size = net_sizeof(pValueTDRef->eTypeClass);
                        if (size <= sizeof(IntPtr))
                        {
                            pUnoAny->pData = &pUnoAny->pReserved;
                            std::memcpy(pUnoAny->pData, &pNetAny->data, sizeof(IntPtr));
                        }
                        else
                        {
                            void* mem = std::malloc(size);
                            std::memcpy(mem, pNetAny->data, size);
                            pUnoAny->pData = mem;
                            std::free(pNetAny->data);
                        }
                        break;
                    }
                    case typelib_TypeClass_STRING:
                    case typelib_TypeClass_TYPE:
                    case typelib_TypeClass_INTERFACE:
                    {
                        if (pNetAny->data)
                        {
                            pUnoAny->pType = pValueTDRef;
                            pUnoAny->pData = &pUnoAny->pReserved;
                            pUnoAny->pReserved = nullptr;
                            unmarshal_data(pUnoAny->pData, &pNetAny->data, pValueTDRef,
                                           bDestructObject, true, bridge);
                        }
                        else
                        {
                            uno_any_construct(pUnoAny, nullptr, nullptr, nullptr);
                        }
                        break;
                    }
                    case typelib_TypeClass_ANY:
                    case typelib_TypeClass_SEQUENCE:
                    {
                        if (pNetAny->data)
                        {
                            pUnoAny->pType = pValueTDRef;
                            pUnoAny->pData = &pUnoAny->pReserved;
                            pUnoAny->pReserved = nullptr;
                            unmarshal_data(pUnoAny->pData, &pNetAny->data, pValueTDRef,
                                           bDestructObject, true, bridge);
                        }
                        else
                        {
                            uno_any_construct(pUnoAny, nullptr, nullptr, nullptr);
                        }
                        break;
                    }
                    case typelib_TypeClass_STRUCT:
                    case typelib_TypeClass_EXCEPTION:
                    {
                        if (pNetAny->data)
                        {
                            pUnoAny->pType = pValueTDRef;
                            TypeDescHolder valueType(pValueTDRef);
                            void* mem = std::malloc(valueType.get()->nSize);
                            unmarshal_data(mem, &pNetAny->data, pValueTDRef, bDestructObject, true,
                                           bridge);
                            pUnoAny->pData = mem;
                        }
                        else
                        {
                            uno_any_construct(pUnoAny, nullptr, nullptr, nullptr);
                        }
                        break;
                    }
                    default:
                    {
                        throw BridgeRuntimeError(SAL_WHERE,
                                                 "could not map "
                                                     + OUString::unacquired(&pValueTDRef->pTypeName)
                                                     + " into an UNO any");
                    }
                }
            }
            else
            {
                switch (pValueTDRef->eTypeClass)
                {
                    case typelib_TypeClass_VOID:
                    case typelib_TypeClass_BOOLEAN:
                    case typelib_TypeClass_BYTE:
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                    case typelib_TypeClass_FLOAT:
                    case typelib_TypeClass_ENUM:
                        break;
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                    {
                        size_t size = net_sizeof(pValueTDRef->eTypeClass);
                        if (pNetAny->data && size > sizeof(IntPtr))
                            std::free(pNetAny->data);
                        break;
                    }
                    case typelib_TypeClass_ANY:
                    case typelib_TypeClass_SEQUENCE:
                    {
                        if (pNetAny->data)
                        {
                            unmarshal_data(pUnoAny->pData, &pNetAny->data, pValueTDRef,
                                           bDestructObject, false, bridge);
                            std::free(pNetAny->data);
                        }
                        break;
                    }
                    case typelib_TypeClass_STRING:
                    case typelib_TypeClass_TYPE:
                    case typelib_TypeClass_INTERFACE:
                    case typelib_TypeClass_STRUCT:
                    case typelib_TypeClass_EXCEPTION:
                    {
                        if (pNetAny->data)
                        {
                            unmarshal_data(pUnoAny->pData, &pNetAny->data, pValueTDRef,
                                           bDestructObject, false, bridge);
                        }
                        break;
                    }
                    default:
                    {
                        throw BridgeRuntimeError(SAL_WHERE,
                                                 "could not map "
                                                     + OUString::unacquired(&pValueTDRef->pTypeName)
                                                     + " into an UNO any");
                    }
                }
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            uno_Sequence** ppUnoSeq = static_cast<uno_Sequence**>(pUnoData);
            Value::Sequence* pNetSeq = static_cast<Value::Sequence*>(pNetData);

            TypeDescHolder type(pTDRef);
            if (bDestructObject && ppUnoSeq)
                uno_destructData(ppUnoSeq, type.get(), nullptr);

            typelib_TypeDescriptionReference* pElemTDRef
                = reinterpret_cast<typelib_IndirectTypeDescription*>(type.get())->pType;
            size_t nNetElemSize = net_sizeof(pElemTDRef->eTypeClass);

            switch (pElemTDRef->eTypeClass)
            {
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                case typelib_TypeClass_ENUM:
                    if (bAssignData)
                        *ppUnoSeq
                            = alloc_uno_sequence(pNetSeq->length, nNetElemSize, pNetSeq->data);
                    break;
                case typelib_TypeClass_ANY:
                case typelib_TypeClass_SEQUENCE:
                case typelib_TypeClass_STRING:
                case typelib_TypeClass_TYPE:
                case typelib_TypeClass_INTERFACE:
                case typelib_TypeClass_EXCEPTION:
                case typelib_TypeClass_STRUCT:
                {
                    TypeDescHolder elemType(pElemTDRef);
                    sal_Int32 nUnoElemSize = elemType.get()->nSize;

                    *ppUnoSeq = alloc_uno_sequence(pNetSeq->length, nUnoElemSize, nullptr);
                    int nPos = 0;
                    try
                    {
                        // Convert each element
                        for (; nPos < pNetSeq->length; ++nPos)
                        {
                            void* pNetElem
                                = static_cast<char*>(pNetSeq->data) + (nPos * nNetElemSize);
                            void* pUnoElem = (*ppUnoSeq)->elements + (nPos * nUnoElemSize);
                            unmarshal_data(pUnoElem, pNetElem, pElemTDRef, bDestructObject,
                                           bAssignData, bridge);
                        }
                    }
                    catch (...)
                    {
                        if (bAssignData)
                        {
                            // Clean up already converted elements
                            for (int nClean = 0; nClean < nPos; ++nClean)
                            {
                                void* pUnoElem = (*ppUnoSeq)->elements + (nClean * nUnoElemSize);
                                uno_destructData(pUnoElem, elemType.get(), nullptr);
                            }
                        }
                        throw;
                    }

                    break;
                }
                default:
                {
                    throw BridgeRuntimeError(
                        SAL_WHERE, "could not map " + OUString::unacquired(&pElemTDRef->pTypeName)
                                       + " into an UNO sequence");
                }
            }

            std::free(pNetSeq->data);
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            uno_Interface** ppUnoI = static_cast<uno_Interface**>(pUnoData);
            IntPtr pNetI = *static_cast<IntPtr*>(pNetData);

            TypeDescHolder type(pTDRef);
            if (bDestructObject && ppUnoI)
                uno_destructData(ppUnoI, type.get(), nullptr);

            if (bAssignData)
            {
                if (pNetI)
                {
                    Context* pCtx = static_cast<Context*>(bridge.m_net_env->pContext);

                    // Get oid and type description
                    OUString sOid(pCtx->lookupObjectId(pNetI));

                    typelib_InterfaceTypeDescription* pInterfaceTD
                        = reinterpret_cast<typelib_InterfaceTypeDescription*>(type.get());

                    // Get the proxy if already created, else create new
                    *ppUnoI = nullptr;
                    (*bridge.m_uno_env->getRegisteredInterface)(bridge.m_uno_env,
                                                                reinterpret_cast<void**>(ppUnoI),
                                                                sOid.pData, pInterfaceTD);

                    if (!*ppUnoI)
                        *ppUnoI = new NetProxy(bridge, pNetI, pInterfaceTD, sOid);
                }
                else
                {
                    *ppUnoI = nullptr;
                }
            }

            break;
        }
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        {
            void* pUnoStruct = pUnoData;
            IntPtr pNetStruct = *static_cast<IntPtr*>(pNetData);

            TypeDescHolder type(pTDRef);
            if (bDestructObject && pNetStruct)
                uno_destructData(pNetStruct, type.get(), nullptr);

            typelib_CompoundTypeDescription* pCompTD
                = reinterpret_cast<typelib_CompoundTypeDescription*>(type.get());
            if (!type.get()->bComplete)
                typelib_typedescription_complete(
                    reinterpret_cast<typelib_TypeDescription**>(&pCompTD));

            std::vector<std::pair<typelib_TypeDescriptionReference*, sal_Int32>> vecMembers;
            for (typelib_CompoundTypeDescription* pHierTD = pCompTD; pHierTD;
                 pHierTD = pHierTD->pBaseTypeDescription)
            {
                for (int n = pHierTD->nMembers - 1; n >= 0; n--)
                {
                    vecMembers.emplace_back(pHierTD->ppTypeRefs[n], pHierTD->pMemberOffsets[n]);
                }
            }

            size_t nNetOffset = 0;
            int nPos = vecMembers.size() - 1;
            try
            {
                for (; nPos >= 0; nPos--)
                {
                    auto[pMemberTDRef, nUnoOffset] = vecMembers[nPos];

                    void* pUnoField = static_cast<char*>(pUnoStruct) + nUnoOffset;
                    void* pNetField = static_cast<char*>(pNetStruct) + nNetOffset;

                    switch (pMemberTDRef->eTypeClass)
                    {
                        case typelib_TypeClass_BOOLEAN:
                        case typelib_TypeClass_BYTE:
                        case typelib_TypeClass_CHAR:
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
                        case typelib_TypeClass_FLOAT:
                        case typelib_TypeClass_DOUBLE:
                        case typelib_TypeClass_ENUM:
                            if (bAssignData)
                                std::memcpy(pUnoField, pNetField,
                                            net_sizeof(pMemberTDRef->eTypeClass));
                            break;
                        case typelib_TypeClass_ANY:
                        case typelib_TypeClass_SEQUENCE:
                        case typelib_TypeClass_STRING:
                        case typelib_TypeClass_TYPE:
                        case typelib_TypeClass_INTERFACE:
                        case typelib_TypeClass_EXCEPTION:
                        case typelib_TypeClass_STRUCT:
                            unmarshal_data(pUnoField, pNetField, pMemberTDRef, bDestructObject,
                                           bAssignData, bridge);
                            break;
                        default:
                        {
                            throw BridgeRuntimeError(
                                SAL_WHERE,
                                "could not map " + OUString::unacquired(&pMemberTDRef->pTypeName)
                                    + " into an UNO " + OUString::unacquired(&pTDRef->pTypeName));
                        }
                    }

                    nNetOffset += net_sizeof(pMemberTDRef->eTypeClass);
                }
            }
            catch (...)
            {
                if (bAssignData)
                {
                    // Clean up already converted fields
                    for (int nClean = vecMembers.size() - 1; nClean > nPos; nClean--)
                    {
                        auto[pMemberTDRef, nUnoOffset] = vecMembers[nClean];
                        void* pUnoField = static_cast<char*>(pUnoStruct) + nUnoOffset;
                        uno_type_destructData(pUnoField, pMemberTDRef, nullptr);
                    }
                }
                throw;
            }

            std::free(pNetStruct);
            break;
        }
        default:
        {
            throw BridgeRuntimeError(SAL_WHERE, "could not map "
                                                    + OUString::unacquired(&pTDRef->pTypeName)
                                                    + " value to UNO");
        }
    }
}
}

void Bridge::map_uno_to_net_value(void* pUnoData, Value* pValue,
                                  typelib_TypeDescriptionReference* pTDRef, bool bDestructValue)
{
    marshal_data(pUnoData, pValue, pTDRef, bDestructValue, *this);
}

void Bridge::map_net_value_to_uno(void* pUnoData, Value* pValue,
                                  typelib_TypeDescriptionReference* pTDRef, bool bDestructObject,
                                  bool bAssignObject)
{
    unmarshal_data(pUnoData, pValue, pTDRef, bDestructObject, bAssignObject, *this);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
