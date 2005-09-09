/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageBridgeFactory.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:34:55 $
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


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_

#include <rtl/ustring>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class StorageBridgeFactory
{

public:
    explicit StorageBridgeFactory(
        const css::uno::Reference < css::uno::XComponentContext > & xContext );
    css::uno::Reference < dcsssf::storage::XScriptInfoAccess >
        getStorageInstance( sal_Int32 sid );
private:
    StorageBridgeFactory(); // No definition for default ctor
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
};
} // scripting_runtimemgr
#endif //_FRAMEWORK_SCRIPT_NAMERESOLVER_HXX_
