/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comtools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:16:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

