/*************************************************************************
*
*  $RCSfile: StorageBridge.hxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: jmrice $ $Date: 2002-09-27 12:16:26 $
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


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptImplAccess.hpp>
#include "StorageBridgeFactory.hxx"

namespace scripting_runtimemgr
{
class StorageBridge :
    public ::cppu::WeakImplHelper1<
        ::drafts::com::sun::star::script::framework::storage::XScriptImplAccess >
{
    friend class StorageBridgeFactory;
public:
    //XScriptImplAccess
    //=========================================================================
    /**
     * Get the implementations for a given URI
     *
     * @param queryURI
     *      The URI to get the implementations for
     *
     * @return XScriptURI
     *      The URIs of the implementations
     */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::script::framework::scripturi::XScriptURI > > SAL_CALL
    getImplementations(
    const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::script::framework::scripturi::XScriptURI >& queryURI )
    throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException );
private:
    StorageBridge( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext, sal_uInt16 sid );

    void initStorage() throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::script::framework::storage::XScriptImplAccess >
            m_xScriptImplAccess;
    sal_uInt16 m_sid;
};
}

#endif //_COM_SUN_STAR_SCRIPTING_STORAGEBRIDGE_HXX_
