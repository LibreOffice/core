/*************************************************************************
 *
 *  $RCSfile: OSubComponent.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-05 07:03:01 $
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
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
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
            ::osl::MutexGuard aGuard( m_pDerivedImplementation->rBHelper.rMutex );
            m_xParent = NULL;
        }
        void relase_ChildImpl()
        {
            ::connectivity::release(m_pDerivedImplementation->m_refCount,
                                    m_pDerivedImplementation->rBHelper,
                                    m_xParent,
                                    m_pDerivedImplementation);

            m_pDerivedImplementation->WEAK::release();
        }
    };
}
#endif // _CONNECTIVITY_OSUBCOMPONENT_HXX_

