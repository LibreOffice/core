/*************************************************************************
 *
 *  $RCSfile: ActiveMSPList.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: npower $ $Date: 2003-09-04 07:23:12 $
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

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_

#include <hash_map>
#include <map>

#include <osl/mutex.hxx>
#include <rtl/ustring>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <drafts/com/sun/star/script/framework/provider/XScriptProvider.hpp>
#include <drafts/com/sun/star/script/framework/browse/XBrowseNode.hpp>


namespace func_provider
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

//Typedefs
//=============================================================================

typedef ::std::hash_map< ::rtl::OUString,
    css::uno::Reference< dcsssf::provider::XScriptProvider >,
    ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > Msp_hash;

struct MspInst
{
    css::uno::Reference< dcsssf::provider::XScriptProvider > provider;
    ::rtl::OUString docName;
};
typedef ::std::map < css::uno::Reference< css::frame::XModel >,
                     MspInst > Model_map;

class ActiveMSPList : public ::cppu::WeakImplHelper2< css::lang::XEventListener , dcsssf::browse::XBrowseNode >
{

public:
    static ActiveMSPList& instance( const css::uno::Reference<
        css::uno::XComponentContext >& xContext );
    ~ActiveMSPList();
    void addActiveMSP( const css::uno::Reference< css::frame::XModel >& xModel,
                       const css::uno::Reference< dcsssf::provider::XScriptProvider >& msp );

    //XEventListener
    //======================================================================

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw ( css::uno::RuntimeException );

    //XBrowseNode implementation
    //======================================================================

    virtual ::rtl::OUString SAL_CALL getName()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< css::uno::Reference< dcsssf::browse::XBrowseNode > > SAL_CALL getChildNodes()
        throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasChildNodes()
        throw ( css::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL getType()
        throw ( css::uno::RuntimeException );

private:
    ActiveMSPList(  const css::uno::Reference<
        css::uno::XComponentContext > & xContext  );
    ::rtl::OUString
        getDocNameOrURLFromModel( const css::uno::Reference< css::frame::XModel >& xModel  );

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
