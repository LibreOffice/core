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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_COMPTR_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_COMPTR_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <shobjidl.h>

template< class    T_INTERFACE          ,
          REFIID   P_IID   = IID_NULL   ,
          REFCLSID P_CLSID = CLSID_NULL >
class ComPtr
{
    public:


        /** initialize com ptr with null.
         */
        ComPtr()
        {
            m_pInterface = nullptr;
        }


        /** initialize com ptr with given interface.
         */
        explicit ComPtr(T_INTERFACE* pInterface)
        {
            m_pInterface = pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }


        /** copy ctor.
         */
        ComPtr(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }


        /** initialize object by quering external object for the right interface.
         */
        explicit ComPtr(IUnknown* pIUnknown)
        {
            if (pIUnknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
        }


        /** deinitialize com object right.
         */
        ~ComPtr()
        {
            release();
        }

    public:


        HRESULT create()
        {
            return CoCreateInstance(P_CLSID, nullptr, CLSCTX_ALL, P_IID, reinterpret_cast<void**>(&m_pInterface));
        }


        operator T_INTERFACE*() const
        {
            return m_pInterface;
        }


        T_INTERFACE& operator*() const
        {
            return *m_pInterface;
        }


        T_INTERFACE** operator&()
        {
            return &m_pInterface;
        }


        T_INTERFACE* operator->() const
        {
            return m_pInterface;
        }


        T_INTERFACE* operator=(T_INTERFACE* pInterface)
        {
            if ( equals(pInterface) )
                return m_pInterface;

            m_pInterface->Release();
            m_pInterface = pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();

            return m_pInterface;
        }


        T_INTERFACE* operator=(IUnknown* pIUnknown)
        {
            if (pIUnknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
            return m_pInterface;
        }


        T_INTERFACE* operator=(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();

            return m_pInterface;
        }


        T_INTERFACE* get() const
        {
            return m_pInterface;
        }


        void attach(T_INTERFACE* pInterface)
        {
            if (pInterface)
            {
                m_pInterface->Release();
                m_pInterface = pInterface;
            }
        }


        T_INTERFACE* detach()
        {
            T_INTERFACE* pInterface = m_pInterface;
            m_pInterface = NULL;
            return pInterface;
        }


        void release()
        {
            if (m_pInterface)
            {
                m_pInterface->Release();
                m_pInterface = nullptr;
            }
        }

        bool equals(IUnknown* pCheck)
        {
            if (
                ( ! m_pInterface ) &&
                ( ! pCheck       )
               )
                return sal_True;

            IUnknown* pCurrent = NULL;
            m_pInterface->QueryInterface(IID_IUnknown, (void**)&pCurrent);

            sal_Bool bEquals = (pCheck == pCurrent);
            pCurrent->Release();

            return bEquals;
        }


        bool is()
        {
            return (m_pInterface != nullptr);
        }

    private:
        T_INTERFACE* m_pInterface;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
