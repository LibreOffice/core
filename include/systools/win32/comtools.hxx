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

#include <sal/config.h>

#include <source_location>
#include <string>
#include <string_view>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <prewin.h>
#include <objbase.h>
#include <postwin.h>

namespace sal::systools
{
    /* Simple exception class for propagating COM errors */
    class ComError : public std::runtime_error
    {
    public:
        ComError(std::string_view message, HRESULT hr,
                 const std::source_location& loc = std::source_location::current())
            : std::runtime_error(std::string(message))
            , hr_(hr)
            , loc_(loc)
        {}

        HRESULT GetHresult() const { return hr_; }
        const std::source_location& GetLocation() const { return loc_; }

    private:
        HRESULT hr_;
        std::source_location loc_;
    };

    /* Convert failed HRESULT to thrown ComError */
    inline void ThrowIfFailed(HRESULT hr, std::string_view msg,
                              std::source_location loc = std::source_location::current())
    {
        if (FAILED(hr))
            throw ComError(msg, hr, loc);
    }

    /* A guard class to call CoInitializeEx/CoUninitialize in proper pairs
     * See also: o3tl::safeCoInitializeEx doing dangerous re-initialization
     */
    class CoInitializeGuard
    {
    public:
        enum class WhenFailed
        {
            NoThrow, // do not throw
            Throw, // throw on failure
            Abort, // std::abort on failure
        };
        explicit CoInitializeGuard(DWORD dwCoInit, bool failChangeMode = false,
                                   WhenFailed whenFailed = WhenFailed::Throw)
        {
            HRESULT hr = ::CoInitializeEx(nullptr, dwCoInit);
            if (whenFailed != WhenFailed::NoThrow && FAILED(hr)
                && (failChangeMode || hr != RPC_E_CHANGED_MODE))
            {
                if (whenFailed == WhenFailed::Throw)
                    throw ComError("CoInitializeEx failed", hr);
                else // if (whenFailed == Abort)
                    std::abort();
            }
            mbUninit = SUCCEEDED(hr);
        }
        CoInitializeGuard(const CoInitializeGuard&) = delete; // non-construction-copyable
        void operator=(const CoInitializeGuard&) = delete; // non-copyable
        ~CoInitializeGuard()
        {
            if (mbUninit)
                ::CoUninitialize();
        }

    private:
        bool mbUninit;
    };

    struct COM_QUERY_TAG {} constexpr COM_QUERY;
    struct COM_QUERY_THROW_TAG : public COM_QUERY_TAG {} constexpr COM_QUERY_THROW;

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
                addRef(com_ptr_);
        }

        COMReference(const COMReference<T>& other) :
            COMReference(other.com_ptr_)
        {
        }

        COMReference(COMReference<T>&& other) :
            COMReference(std::exchange(other.com_ptr_, nullptr), false)
        {
        }

        // Query from IUnknown*, using COM_QUERY or COM_QUERY_THROW tags
        template <typename T2, typename TAG>
        COMReference(const COMReference<T2>& p, TAG t)
            : COMReference(p.template QueryInterface<T>(t))
        {
        }

        // Using CoCreateInstance
        COMReference(REFCLSID clsid, IUnknown* pOuter = nullptr, DWORD nCtx = CLSCTX_ALL)
            : com_ptr_(nullptr)
        {
            ThrowIfFailed(CoCreateInstance(clsid, pOuter, nCtx), "CoCreateInstance failed");
        }

        COMReference<T>& operator=(const COMReference<T>& other)
        {
            return operator=(other.com_ptr_);
        }

        COMReference<T>& operator=(COMReference<T>&& other)
        {
            if (com_ptr_ != other.com_ptr_)
            {
                clear();
                std::swap(com_ptr_, other.com_ptr_);
            }
            return *this;
        }

        COMReference<T>& operator=(T* comptr)
        {
            assign(comptr);
            return *this;
        }

        ~COMReference() { release(com_ptr_); }

        template <typename T2, typename TAG>
            requires std::is_base_of_v<COM_QUERY_TAG, TAG>
        COMReference<T2> QueryInterface(TAG) const
        {
            T2* ip = nullptr;
            HRESULT hr = E_POINTER;
            if (com_ptr_)
                hr = com_ptr_->QueryInterface(&ip);

            if constexpr (std::is_same_v<TAG, COM_QUERY_THROW_TAG>)
                ThrowIfFailed(hr, "QueryInterface failed");

            return { ip, false };
        }

        template <typename T2, typename TAG>
        COMReference<T>& set(const COMReference<T2>& p, TAG t)
        {
            return operator=(p.template QueryInterface<T>(t));
        }

        HRESULT CoCreateInstance(REFCLSID clsid, IUnknown* pOuter = nullptr,
                                 DWORD nCtx = CLSCTX_ALL)
        {
            T* ip;
            HRESULT hr = ::CoCreateInstance(clsid, pOuter, nCtx, IID_PPV_ARGS(&ip));
            if (SUCCEEDED(hr))
                release(std::exchange(com_ptr_, ip));
            return hr;
        }

        HRESULT CoGetClassObject(REFCLSID clsid, DWORD nCtx = CLSCTX_ALL)
        {
            T* ip;
            HRESULT hr = ::CoGetClassObject(clsid, nCtx, nullptr, IID_PPV_ARGS(&ip));
            if (SUCCEEDED(hr))
                release(std::exchange(com_ptr_, ip));
            return hr;
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
        operator T* () const { return get(); }

        void clear() { assign(nullptr); }

        bool is() const { return (com_ptr_ != nullptr); }
        operator bool() const { return is(); }

    private:
        static void addRef(T* ptr)
        {
            if (ptr)
                ptr->AddRef();
        }

        static void release(T* ptr)
        {
            if (ptr)
                ptr->Release();
        }

        void assign(T* ptr)
        {
            if (com_ptr_ == ptr)
                return;
            addRef(ptr);
            release(std::exchange(com_ptr_, ptr));
        }

        T* com_ptr_;
    };

    // A class to use with functions taking an out pointer argument,
    // that needs to be freed with CoTaskMemFree - like SHGetKnownFolderPath
    template <typename T> class CoTaskMemAllocated
    {
    public:
        ~CoTaskMemAllocated() { CoTaskMemFree(m_pv); }

        T** operator&()
        {
            CoTaskMemFree(std::exchange(m_pv, nullptr));
            return &m_pv;
        };

        operator T*() { return m_pv; }

    private:
        T* m_pv = nullptr;
    };

} // sal::systools

/* Typedefs for some popular COM interfaces */
typedef sal::systools::COMReference<IDataObject> IDataObjectPtr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
