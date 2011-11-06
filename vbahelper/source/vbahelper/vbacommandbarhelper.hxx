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


#ifndef VBA_COMMANDBARHELPER_HXX
#define VBA_COMMANDBARHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <boost/shared_ptr.hpp>


static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_ISVISIBLE[]   = "IsVisible";
static const char ITEM_DESCRIPTOR_RESOURCEURL[] = "ResourceURL";
static const char ITEM_DESCRIPTOR_UINAME[]      = "UIName";
static const char ITEM_DESCRIPTOR_ENABLED[]     = "Enabled";

static const char ITEM_MENUBAR_URL[] = "private:resource/menubar/menubar";
static const char ITEM_TOOLBAR_URL[] = "private:resource/toolbar/";

static const char CUSTOM_TOOLBAR_STR[] = "custom_toolbar_";
static const char CUSTOM_MENU_STR[] = "vnd.openoffice.org:CustomMenu";

class VbaCommandBarHelper;
typedef ::boost::shared_ptr< VbaCommandBarHelper > VbaCommandBarHelperRef;

class VbaCommandBarHelper
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xDocCfgMgr; // current document
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xAppCfgMgr;
    css::uno::Reference< css::container::XNameAccess > m_xWindowState;
    rtl::OUString maModuleId;

    void Init() throw (css::uno::RuntimeException);
    sal_Bool hasToolbar( const rtl::OUString& sResourceUrl, const rtl::OUString& sName )  throw (css::uno::RuntimeException);
public:
    VbaCommandBarHelper( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );

    css::uno::Reference< css::frame::XModel > getModel() { return mxModel; }

    css::uno::Reference< css::ui::XUIConfigurationManager > getDocCfgManager() throw (css::uno::RuntimeException)
    {
        return m_xDocCfgMgr;
    }
    css::uno::Reference< css::ui::XUIConfigurationManager > getAppCfgManager() throw (css::uno::RuntimeException)
    {
        return m_xAppCfgMgr;
    }
    css::uno::Reference< css::container::XNameAccess > getPersistentWindowState() throw (css::uno::RuntimeException)
    {
        return m_xWindowState;
    }
    sal_Bool persistChanges() throw (css::uno::RuntimeException);
    css::uno::Reference< css::container::XIndexAccess > getSettings( const rtl::OUString& sResourceUrl ) throw (css::uno::RuntimeException);
    void removeSettings( const rtl::OUString& sResourceUrl ) throw (css::uno::RuntimeException);
    void ApplyChange( const rtl::OUString& sResourceUrl, const css::uno::Reference< css::container::XIndexAccess >& xSettings, sal_Bool bTemporary = sal_True ) throw (css::uno::RuntimeException);

    css::uno::Reference< css::frame::XLayoutManager > getLayoutManager() throw (css::uno::RuntimeException);

    const rtl::OUString getModuleId(){ return maModuleId; }
    rtl::OUString findToolbarByName( const css::uno::Reference< css::container::XNameAccess >& xNameAccess, const rtl::OUString& sName ) throw (css::uno::RuntimeException);
    static sal_Int32 findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, const rtl::OUString& sName, bool bMenu = false ) throw (css::uno::RuntimeException);
    static rtl::OUString generateCustomURL();
};

#endif //VBA_COMMANDBARHELPER_HXX
