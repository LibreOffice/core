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

#include <string>
#include <stdexcept>
#include <type_traits>
#include <objbase.h>

namespace sal::systools
{
    /* Simple exception class for propagating COM errors */
    class ComError : public std::runtime_error
    {
    public:
        ComError(const std::string& message, HRESULT hr) :
            std::runtime_error(message),
            hr_(hr)
        {}

        HRESULT GetHresult() const { return hr_; }

    private:
        HRESULT hr_;
    };

    struct COM_QUERY_TAG {} constexpr COM_QUERY;
    struct COM_QUERY_THROW_TAG {} constexpr COM_QUERY_THROW;
    template <typename TAG>
    constexpr bool is_COM_query_tag
        = std::is_same_v<TAG, COM_QUERY_TAG> || std::is_same_v<TAG, COM_QUERY_THROW_TAG>;

    /* A simple COM smart pointer template */
    template <typename T>
    class COMReference
    {
    public:
        /* Explicitly controllable whether AddRef will be called or not */
        COMReference(T* comptr = nullptr, bool bAddRef = true) :
            com_ptr_(comptr)
        {
            if (bAddRef)
                addRef();
        }

        COMReference(const COMReference<T>& other) :
            com_ptr_(other.com_ptr_)
        {
            addRef();
        }

        // Query from IUnknown*, using COM_QUERY or COM_QUERY_THROW tags
        template <typename T2, typename TAG>
        COMReference(const COMReference<T2>& p, TAG t)
            : COMReference(p.template QueryInterface<T>(t))
        {
        }

        COMReference<T>& operator=(const COMReference<T>& other)
        {
            other.addRef();
            release();
            com_ptr_ = other.com_ptr_;
            return *this;
        }

        COMReference<T>& operator=(T* comptr)
        {
            release();
            com_ptr_ = comptr;
            addRef();
            return *this;
        }

        ~COMReference() { release(); }

        template <typename T2, typename TAG, std::enable_if_t<is_COM_query_tag<TAG>, int> = 0>
        COMReference<T2> QueryInterface(TAG) const
        {
            void* ip = nullptr;
            HRESULT hr = E_FAIL;
            if (com_ptr_)
                hr = com_ptr_->QueryInterface(__uuidof(T2), &ip);

            if constexpr (std::is_same_v<TAG, COM_QUERY_THROW_TAG>)
                if (FAILED(hr))
                    throw ComError("QueryInterface failed: Interface not supported!", hr);

            return { static_cast<T2*>(ip), false };
        }

        COMReference<T>& CoCreateInstance(REFCLSID clsid, IUnknown* pOuter = nullptr,
                                          DWORD nCtx = CLSCTX_ALL)
        {
            clear();
            HRESULT hr = ::CoCreateInstance(clsid, pOuter, nCtx, __uuidof(T),
                                            reinterpret_cast<void**>(&com_ptr_));
            if (FAILED(hr))
                throw ComError("CoCreateInstance failed!", hr);

            return *this;
        }

        T* operator->() const { return com_ptr_; }

        T& operator*() const { return *com_ptr_; }

        /* Necessary for assigning com_ptr_ from functions like
           CoCreateInstance which require a 'void**' */
        T** operator&()
        {
            clear();
            return &com_ptr_;
        }

        T* get() const { return com_ptr_; }

        void clear()
        {
            release();
            com_ptr_ = nullptr;
        }

        bool is() const { return (com_ptr_ != nullptr); }

    private:
        void addRef() const
        {
            if (com_ptr_)
                com_ptr_->AddRef();
        }

        void release() const
        {
            if (com_ptr_)
                com_ptr_->Release();
        }

        T* com_ptr_;
    };

} // sal::systools

/* Typedefs for some popular COM interfaces */
typedef sal::systools::COMReference<IDataObject> IDataObjectPtr;
typedef sal::systools::COMReference<IStream> IStreamPtr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
