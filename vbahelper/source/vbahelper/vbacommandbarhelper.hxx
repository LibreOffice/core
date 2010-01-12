/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
    static sal_Int32 findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, const rtl::OUString& sName ) throw (css::uno::RuntimeException);
    static rtl::OUString generateCustomURL();
};

#endif//VBA_COMMANDBARHELPER_HXX
