/*************************************************************************
 *
 *  $RCSfile: dynload.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-28 11:02:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SALHELPER_DYNLOAD_HXX_
#define _SALHELPER_DYNLOAD_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

namespace salhelper
{

class ORealDynamicLoader
{
public:

    static ORealDynamicLoader* SAL_CALL newInstance(
            ORealDynamicLoader ** ppSetToZeroInDestructor,
            const ::rtl::OUString& strModuleName,
            const ::rtl::OUString& strInitFunction );

    sal_uInt32 SAL_CALL acquire();
    sal_uInt32 SAL_CALL release();

    void* SAL_CALL getApi() const;

protected:
    ORealDynamicLoader( ORealDynamicLoader ** ppSetToZeroInDestructor,
                        const ::rtl::OUString& strModuleName,
                        const ::rtl::OUString& strInitFunction,
                        void* pApi,
                        oslModule pModule );

    virtual ~ORealDynamicLoader();

    void*                   m_pApi;
    sal_uInt32              m_refCount;
    oslModule               m_pModule;
    ::rtl::OUString         m_strModuleName;
    ::rtl::OUString         m_strInitFunction;
    ORealDynamicLoader **   ppSetToZeroInDestructor;
};

template<class API>
class ODynamicLoader //: public OObject
{
public:
    ODynamicLoader() SAL_THROW(())
    {
        m_pLoader = 0;
    }

    ODynamicLoader( const ::rtl::OUString& strModuleName,
                       const ::rtl::OUString& strInitFunction ) SAL_THROW(())
    {
        if (!m_pStaticLoader)
        {
            m_pStaticLoader = ORealDynamicLoader::newInstance(
               &m_pStaticLoader,
               strModuleName,
               strInitFunction);
        }
        else
        {
            m_pStaticLoader->acquire();
        }

        m_pLoader = m_pStaticLoader;
    }


    ODynamicLoader(const ODynamicLoader<API>& toCopy) SAL_THROW(())
    {
        m_pLoader = toCopy.m_pLoader;
        if( m_pLoader )
            m_pLoader->acquire();
    }

    ~ODynamicLoader() SAL_THROW(())
    {
        if( m_pLoader )
            m_pLoader->release();
    }

    ODynamicLoader<API>& SAL_CALL operator = (const ODynamicLoader<API>& toAssign) SAL_THROW(())
    {
        if( m_pLoader != toAssign.m_pLoader )
        {
            if( toAssign.m_pLoader )
            toAssign.m_pLoader->acquire();
            if( m_pLoader )
            m_pLoader->release();
            m_pLoader = toAssign.m_pLoader;
        }

        return (*this);
    }

    API* SAL_CALL getApi() const SAL_THROW(())
    {
        return (API*)m_pLoader->getApi();
    }

    API* SAL_CALL operator->() const SAL_THROW(())
    {
        return (API*)m_pLoader->getApi();
    }

    sal_Bool SAL_CALL isLoaded() const SAL_THROW(())
    {
        return (m_pLoader != NULL);
    }

protected:
    static ORealDynamicLoader*  m_pStaticLoader;
    ORealDynamicLoader*         m_pLoader;
};


template<class API>
ORealDynamicLoader* ODynamicLoader<API>::m_pStaticLoader = NULL;

}

#endif

