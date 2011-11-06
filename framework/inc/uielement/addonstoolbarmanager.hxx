/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UILEMENT_TOOLBARMANAGER_HXX_
#include <uielement/toolbarmanager.hxx>
#endif
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/toolbox.hxx>


namespace framework
{

class ToolBar;
class AddonsToolBarManager : public ToolBarManager
{
    public:
        AddonsToolBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                              const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                              const rtl::OUString& rResourceName,
                              ToolBar* pToolBar );
        virtual ~AddonsToolBarManager();

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        virtual void RefreshImages();
        using ToolBarManager::FillToolbar;
        void FillToolbar( const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >& rAddonToolbar );

    protected:
        DECL_LINK( Click, ToolBox * );
        DECL_LINK( DoubleClick, ToolBox * );
        DECL_LINK( Command, CommandEvent * );
        DECL_LINK( Select, ToolBox * );
        DECL_LINK( Highlight, ToolBox * );
        DECL_LINK( Activate, ToolBox * );
        DECL_LINK( Deactivate, ToolBox * );
        DECL_LINK( StateChanged, StateChangedType* );
        DECL_LINK( DataChanged, DataChangedEvent* );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;
};

}

#endif // __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
