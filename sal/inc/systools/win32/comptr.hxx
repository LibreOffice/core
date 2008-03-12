/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comptr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:37:26 $
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
            if (pIUnknown)
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
        HRESULT query(REFIID rIID  ,
                      void** pQuery)
        {
            return m_pInterface->QueryInterface(rIID, pQuery);
        }

        //---------------------------------------------------------------------
        HRESULT unknown(IUnknown** pQuery)
        {
            return m_pInterface->QueryInterface(IID_IUnknown, (void**)pQuery);
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
