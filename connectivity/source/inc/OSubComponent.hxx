/*************************************************************************
 *
 *  $RCSfile: OSubComponent.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:24 $
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
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#define _CONNECTIVITY_OSUBCOMPONENT_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/Weak.hxx>
#endif

namespace connectivity
{

    //************************************************************
    // OSubComponent
    //************************************************************
    template <class T > class OSubComponent
    {
    protected:
        // the parent must support the tunnel implementation
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;
        T*  m_pChildImpl;

    public:
        OSubComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent) : m_xParent(_xParent)
        {
            m_pChildImpl = (T*)this;
        }


    protected:
        void dispose_ChildImpl()
        {
            ::osl::MutexGuard aGuard( m_pChildImpl->rBHelper.rMutex );
            m_xParent = NULL;
        }
        void relase_ChildImpl()
        {
            if (osl_decrementInterlockedCount( &m_pChildImpl->m_refCount ) == 0)
            {
                if (! m_pChildImpl->rBHelper.bDisposed)
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xHoldAlive( *m_pChildImpl );

                    // remember the parent
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xParent;
                    {
                        ::osl::MutexGuard aGuard( m_pChildImpl->rBHelper.rMutex );
                        xParent = m_xParent;
                        m_xParent = NULL;
                    }

                    // First dispose
                    m_pChildImpl->dispose();

                    // only the alive ref holds the object
                    OSL_ASSERT( m_pChildImpl->m_refCount == 1 );

                    // release the parent in the ~
                    if (xParent.is())
                    {
                        ::osl::MutexGuard aGuard( m_pChildImpl->rBHelper.rMutex );
                        m_xParent = xParent;
                    }

                    // destroy the object if xHoldAlive decrement the refcount to 0
                    return;
                }
            }
            // restore the reference count
            osl_incrementInterlockedCount( &m_pChildImpl->m_refCount );
            // as we cover the job of the componenthelper we use the ...
            //  ((::cppu::OWeakObject*)m_pChildImpl)->release();
        }
    };
}
#endif // _CONNECTIVITY_OSUBCOMPONENT_HXX_

