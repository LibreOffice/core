/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ActiveMSPList.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:20:14 $
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
#ifndef _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_

#include <hash_map>
#include <map>

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>


namespace func_provider
{
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================


typedef ::std::map < css::uno::Reference< css::frame::XModel >,
                     css::uno::Reference< css::script::provider::XScriptProvider > > Model_map;

typedef ::std::hash_map< ::rtl::OUString,
    css::uno::Reference< css::script::provider::XScriptProvider >,
    ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > Msp_hash;

class ActiveMSPList : public ::cppu::WeakImplHelper1< css::lang::XEventListener  >
{

public:

    ActiveMSPList(  const css::uno::Reference<
        css::uno::XComponentContext > & xContext  );
    ~ActiveMSPList();

    css::uno::Reference< css::script::provider::XScriptProvider >
        createMSP( const ::rtl::OUString& context )
            throw ( css::uno::RuntimeException );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createMSP( const css::uno::Any& context )
            throw ( css::uno::RuntimeException );
    css::uno::Sequence< css::uno::Reference< css::script::provider::XScriptProvider > >
        getActiveProviders();
    //XEventListener
    //======================================================================

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw ( css::uno::RuntimeException );

private:
    void addActiveMSP( const css::uno::Reference< css::frame::XModel >& xModel,
                       const css::uno::Reference< css::script::provider::XScriptProvider >& msp );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createNewMSP( const ::rtl::OUString& )
            throw ( css::uno::RuntimeException );
    void createNonDocMSPs();
    Msp_hash m_hMsps;
    Model_map m_mModels;
    osl::Mutex m_mutex;
    ::rtl::OUString userDirString;
    ::rtl::OUString shareDirString;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};
} // func_provider
#endif
