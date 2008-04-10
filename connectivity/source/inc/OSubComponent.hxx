/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSubComponent.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#define _CONNECTIVITY_OSUBCOMPONENT_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>
namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XComponent;
            }
        }
    }
}
namespace connectivity
{

    void release(oslInterlockedCount& _refCount,
                 ::cppu::OBroadcastHelper& rBHelper,
                 ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                 ::com::sun::star::lang::XComponent* _pObject);
    //************************************************************
    // OSubComponent
    //************************************************************
    template <class SELF, class WEAK> class OSubComponent
    {
    protected:
        // the parent must support the tunnel implementation
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;
        SELF*   m_pDerivedImplementation;

    public:
        OSubComponent(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent,
                SELF* _pDerivedImplementation)
            :m_xParent(_xParent)
            ,m_pDerivedImplementation(_pDerivedImplementation)
        {
        }

    protected:
        void dispose_ChildImpl()
        {
            // avoid ambiguity
            ::osl::MutexGuard aGuard( m_pDerivedImplementation->WEAK::rBHelper.rMutex );
            m_xParent = NULL;
        }
        void relase_ChildImpl()
        {
            ::connectivity::release(m_pDerivedImplementation->m_refCount,
                                    m_pDerivedImplementation->WEAK::rBHelper,
                                    m_xParent,
                                    m_pDerivedImplementation);

            m_pDerivedImplementation->WEAK::release();
        }
    };
}
#endif // _CONNECTIVITY_OSUBCOMPONENT_HXX_

