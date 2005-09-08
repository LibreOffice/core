/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OSubComponent.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:37:56 $
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

