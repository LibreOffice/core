/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageBridge.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:34:28 $
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


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include "StorageBridgeFactory.hxx"

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class StorageBridge : public ::cppu::WeakImplHelper1< dcsssf::storage::XScriptInfoAccess >
{
    friend class StorageBridgeFactory;
public:
    //XScriptInfoAccess
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
    virtual css::uno::Sequence< css::uno::Reference< dcsssf::storage::XScriptInfo > >
        SAL_CALL getImplementations(
            const ::rtl::OUString& queryURI )
        throw ( css::lang::IllegalArgumentException,
            css::uno::RuntimeException );
    //=========================================================================
    /**
     * Get the all logical names stored in this storage
     *
     * @return sequence < ::rtl::OUString >
     *      The logical names
     */
    virtual css::uno::Sequence< ::rtl::OUString >
        SAL_CALL getScriptLogicalNames()
        throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );
private:
    StorageBridge( const css::uno::Reference< css::uno::XComponentContext >& xContext,
        sal_Int32 sid );

    void initStorage() throw ( css::uno::RuntimeException );
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< dcsssf::storage::XScriptInfoAccess > m_xScriptInfoAccess;
    sal_Int32 m_sid;
};
}

#endif //_COM_SUN_STAR_SCRIPTING_STORAGEBRIDGE_HXX_
