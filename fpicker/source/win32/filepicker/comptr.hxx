/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: comptr.hxx,v $
 * $Revision: 1.3 $
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

#ifndef COMPTR_HXX
#define COMPTR_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <shobjidl.h>

template< class    T_INTERFACE          ,
          REFIID   P_IID   = IID_NULL   ,
          REFCLSID P_CLSID = CLSID_NULL >
class ComPtr
{
    public:

        //---------------------------------------------------------------------
        /** initialize com ptr with null.
         */
        ComPtr()
        {
            m_pInterface = NULL;
        }

        //---------------------------------------------------------------------
        /** initialize com ptr with given interface.
         */
        ComPtr(T_INTERFACE* pInterface)
        {
            m_pInterface = pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }

        //---------------------------------------------------------------------
        /** copy ctor.
         */
        ComPtr(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }

        //---------------------------------------------------------------------
        /** initialize object by quering external object for the right interface.
         */
        ComPtr(IUnknown* pIUnknown)
        {
            if (pIUnknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
        }

        //---------------------------------------------------------------------
        /** deinitialize com object right.
         */
        ~ComPtr()
        {
            release();
        }

    public:

        //---------------------------------------------------------------------
        HRESULT create()
        {
            return CoCreateInstance(P_CLSID, NULL, CLSCTX_ALL, P_IID, (void**)&m_pInterface);
        }

        //---------------------------------------------------------------------
        operator T_INTERFACE*() const
        {
            return m_pInterface;
        }

        //---------------------------------------------------------------------
        T_INTERFACE& operator*() const
        {
            return *m_pInterface;
        }

        //---------------------------------------------------------------------
        T_INTERFACE** operator&()
        {
            return &m_pInterface;
        }

        //---------------------------------------------------------------------
        T_INTERFACE* operator->() const
        {
            return m_pInterface;
        }

        //---------------------------------------------------------------------
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

        //---------------------------------------------------------------------
        T_INTERFACE* operator=(IUnknown* pIUnknown)
        {
            if (pIUknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
            return m_pInterface;
        }

        //---------------------------------------------------------------------
        T_INTERFACE* operator=(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();

            return m_pInterface;
        }

        //---------------------------------------------------------------------
        T_INTERFACE* get() const
        {
            return m_pInterface;
        }

        //---------------------------------------------------------------------
        void attach(T_INTERFACE* pInterface)
        {
            if (pInterface)
            {
                m_pInterface->Release();
                m_pInterface = pInterface;
            }
        }

        //---------------------------------------------------------------------
        T_INTERFACE* detach()
        {
            T_INTERFACE* pInterface = m_pInterface;
            m_pInterface = NULL;
            return pInterface;
        }

        //---------------------------------------------------------------------
        void release()
        {
            if (m_pInterface)
            {
                m_pInterface->Release();
                m_pInterface = NULL;
            }
        }

        //---------------------------------------------------------------------
        template< class T_QUERYINTERFACE >
        HRESULT query(T_QUERYINTERFACE** pQuery)
        {
            return m_pInterface->QueryInterface(__uuidof(T_QUERYINTERFACE), (void**)pQuery);
        }

        //---------------------------------------------------------------------
        ::sal_Bool equals(IUnknown* pCheck)
        {
            if (
                ( ! m_pInterface ) &&
                ( ! pCheck       )
               )
                return sal_True;

            IUnknown* pCurrent = NULL;
            m_pInterface->QueryInterface(IID_IUnknown, (void**)&pCurrent);

            ::sal_Bool bEquals = (pCheck == pCurrent);
            pCurrent->Release();

            return bEquals;
        }

        //---------------------------------------------------------------------
        ::sal_Bool is()
        {
            return (m_pInterface != 0);
        }

    private:
        T_INTERFACE* m_pInterface;
};

#endif
