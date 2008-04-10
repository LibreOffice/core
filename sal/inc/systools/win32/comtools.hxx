/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: comtools.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#pragma once

#include <string>
#include <stdexcept>
#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

namespace sal
{
namespace systools
{
    typedef int HRESULT;

    /* Simple exception class for propagating COM errors */
    class ComError : public std::runtime_error
    {
    public:
        ComError(const std::string& message, HRESULT hr) :
            std::runtime_error(message),
            hr_(hr)
        {}

        HRESULT GetHresult() const
        {
            return hr_;
        }

    private:
        HRESULT hr_;
    };

    /* A simple COM smart pointer template */
    template <typename T>
    class COMReference
    {
    public:
        COMReference() :
            com_ptr_(NULL)
        {
        }

        explicit COMReference(T* comptr) :
            com_ptr_(comptr)
        {
            addRef();
        }

        /* Explicitly controllable whether AddRef will be called or not */
        COMReference(T* comptr, bool bAddRef) :
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

        COMReference<T>& operator=(const COMReference<T>& other)
        {
            if (other.com_ptr_)
                other.com_ptr_->AddRef();
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

        ~COMReference()
        {
            release();
        }

        template<typename InterfaceType>
        COMReference<InterfaceType> QueryInterface(REFIID iid)
        {
            COMReference<InterfaceType> ip;
            HRESULT hr = E_FAIL;
            if (com_ptr_)
                hr = com_ptr_->QueryInterface(iid, reinterpret_cast<LPVOID*>(&ip));

            if (FAILED(hr))
                throw ComError("QueryInterface failed: Interface not supported!", hr);

            return ip;
        }

        T* operator->() const
        {
            return com_ptr_;
        }

        T& operator*() const
        {
            return *com_ptr_;
        }

        /* Necessary for assigning com_ptr_ from functions like
           CoCreateInstance which require a 'void**' */
        T** operator&()
        {
            release();
            com_ptr_ = NULL;
            return &com_ptr_;
        }

        T* get() const
        {
            return com_ptr_;
        }

        COMReference<T>& clear()
        {
            release();
            com_ptr_ = NULL;
            return *this;
        }

        bool is() const
        {
            return (com_ptr_ != NULL);
        }

    private:
        ULONG addRef()
        {
            ULONG cnt = 0;
            if (com_ptr_)
                cnt = com_ptr_->AddRef();
            return cnt;
        }

        ULONG release()
        {
            ULONG cnt = 0;
            if (com_ptr_)
                cnt = com_ptr_->Release();
            return cnt;
        }

    private:
        T* com_ptr_;
    };

} // systools
} // sal

/* Typedefs for some popular COM interfaces */
typedef sal::systools::COMReference<IDataObject> IDataObjectPtr;
typedef sal::systools::COMReference<IStream> IStreamPtr;
typedef sal::systools::COMReference<IEnumFORMATETC> IEnumFORMATETCPtr;

