/*************************************************************************
 *
 *  $RCSfile: VDescriptor.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-19 07:33:30 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include <connectivity/sdbcx/VDescriptor.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

namespace connectivity
{
    namespace sdbcx
    {
        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::lang;
        using namespace ::com::sun::star::beans;

        // =========================================================================
        // = ODescriptor
        // =========================================================================
        // -------------------------------------------------------------------------
        ODescriptor::ODescriptor(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bCase, sal_Bool _bNew)
            :ODescriptor_PBASE(_rBHelper)
            ,m_bNew(_bNew)
            ,m_aCase(_bCase)
        {
        }

        // -------------------------------------------------------------------------
        // com::sun::star::lang::XUnoTunnel
        sal_Int64 SAL_CALL ODescriptor::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
        {
            if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                return (sal_Int64)this;

            return 0;
        }

        // -----------------------------------------------------------------------------
        Sequence< sal_Int8 > ODescriptor::getUnoTunnelImplementationId()
        {
            static ::cppu::OImplementationId * pId = 0;
            if (! pId)
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if (! pId)
                {
                    static ::cppu::OImplementationId aId;
                    pId = &aId;
                }
            }
            return pId->getImplementationId();
        }

        // -----------------------------------------------------------------------------
        Any SAL_CALL ODescriptor::queryInterface( const Type & rType ) throw(RuntimeException)
        {
            Any aRet = ::cppu::queryInterface(rType,static_cast< XUnoTunnel*> (this));
            if(!aRet.hasValue())
                aRet = ODescriptor_PBASE::queryInterface(rType);
            return aRet;
        }

        // -----------------------------------------------------------------------------
        void ODescriptor::setNew(sal_Bool _bNew)
        {
            m_bNew = _bNew;
        }

        // -----------------------------------------------------------------------------
        Sequence< Type > SAL_CALL ODescriptor::getTypes(  ) throw(RuntimeException)
        {
            ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XUnoTunnel > *)0 ));
            return aTypes.getTypes();
        }

    }
}

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 19.03.01 08:21:59  fs
 ************************************************************************/

