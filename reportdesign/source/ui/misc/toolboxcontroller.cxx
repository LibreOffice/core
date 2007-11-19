/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolboxcontroller.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 17:23:57 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_TOOLBOXCONTROLLER_HXX
#include "toolboxcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_COLOR_HPP_
#include <com/sun/star/util/Color.hpp>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_COLOR        1
#define ITEMID_BRUSH        2
#define ITEMID_FONT         3
#define ITEMID_FONTHEIGHT   4
#include <svx/fontitem.hxx>
#include <svx/fhgtitem.hxx>
#ifndef _SVX_TBCONTRL_HXX
#include <svx/tbcontrl.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include <svx/tbxcustomshapes.hxx>
#endif

#include <comphelper/sequence.hxx>

#include <memory>

namespace rptui
{
    using namespace svt;
    using namespace com::sun::star;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace frame;
    using namespace util;
    using namespace ui;

::rtl::OUString SAL_CALL OToolboxController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OToolboxController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.comp.ReportToolboxController"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OToolboxController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ReportToolboxController"));
    return aSupported;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OToolboxController::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OToolboxController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > OToolboxController::create(Reference< XComponentContext > const & xContext)
{
    return *(new OToolboxController(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY)));
}
// -----------------------------------------------------------------------------
DBG_NAME(rpt_OToolboxController)
OToolboxController::OToolboxController(const Reference< XMultiServiceFactory >& _rxORB)
    : m_pToolbarController(NULL)
    ,m_nToolBoxId(1)
    ,m_nSlotId(0)
{
    DBG_CTOR(rpt_OToolboxController,NULL);
    osl_incrementInterlockedCount(&m_refCount);
    m_xServiceManager = _rxORB;
    osl_decrementInterlockedCount(&m_refCount);

}
// -----------------------------------------------------------------------------
OToolboxController::~OToolboxController()
{
    DBG_DTOR(rpt_OToolboxController,NULL);
}
// -----------------------------------------------------------------------------
// XInterface
Any SAL_CALL OToolboxController::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = ToolboxController::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = TToolboxController_BASE::queryInterface(_rType);
    return aReturn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::release() throw ()
{
    ToolboxController::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
{
    ToolboxController::initialize(_rArguments);
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    ToolBox*    pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
    if ( pToolBox )
    {
        const USHORT nCount = pToolBox->GetItemCount();
        for (USHORT nPos = 0; nPos < nCount; ++nPos)
        {
            const USHORT nItemId = pToolBox->GetItemId(nPos);
            if ( pToolBox->GetItemCommand(nItemId) == String(m_aCommandURL) )
            {
                m_nToolBoxId = nItemId;
                break;
            }
        }
        if ( m_aCommandURL.equalsAscii(".uno:BasicShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:BasicShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_BASIC,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:SymbolShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:SymbolShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_SYMBOL,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:ArrowShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:ArrowShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_ARROW,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:FlowChartShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FlowChartShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_FLOWCHART,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:CalloutShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CalloutShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_CALLOUT,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:StarShapes") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:StarShapes")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxTbxCtlCustomShapes(m_nSlotId = SID_DRAWTBX_CS_STAR,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:CharFontName") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CharFontName")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxFontNameToolBoxControl/*SvxStyleToolBoxControl*/(m_nSlotId = SID_ATTR_CHAR_FONT,m_nToolBoxId,*pToolBox));
        }
        else if ( m_aCommandURL.equalsAscii(".uno:FontColor") || m_aCommandURL.equalsAscii(".uno:Color") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FontColor")),sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Color")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxFontColorExtToolBoxControl/*SvxFontColorToolBoxControl*/(m_nSlotId = SID_ATTR_CHAR_COLOR2,m_nToolBoxId,*pToolBox));
        }
        else
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:BackgroundColor")),sal_True));
            m_pToolbarController = TToolbarHelper::createFromQuery(new SvxColorToolBoxControl(m_nSlotId = SID_BACKGROUND_COLOR,m_nToolBoxId,*pToolBox));
        }

        TCommandState::iterator aIter = m_aStates.begin();
        for (; aIter != m_aStates.end(); ++aIter)
            addStatusListener(aIter->first);

        if ( m_pToolbarController.is() )
            m_pToolbarController->initialize(_rArguments);
        // check if paste special is allowed, when not don't add DROPDOWN
        pToolBox->SetItemBits(m_nToolBoxId,pToolBox->GetItemBits(m_nToolBoxId) | TIB_DROPDOWN);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
    if ( aFind != m_aStates.end() )
    {
        aFind->second = Event.IsEnabled;
        if ( m_pToolbarController.is() )
        {
            // All other status events will be processed here
            sal_Bool bSetCheckmark      = sal_False;
            sal_Bool bCheckmark         = sal_False;
            //m_pToolbarController->GetToolBox().Enable(Event.IsEnabled);
            ToolBox& rTb = m_pToolbarController->GetToolBox();
            for ( USHORT i = 0; i < rTb.GetItemCount(); i++ )
            {
                USHORT nId = rTb.GetItemId( i );
                if ( nId == 0 )
                    continue;

                rtl::OUString aCmd = rTb.GetItemCommand( nId );
                if ( aCmd == Event.FeatureURL.Complete )
                {
                    // Enable/disable item
                    rTb.EnableItem( nId, Event.IsEnabled );

                    // Checkmark
                    if ( Event.State >>= bCheckmark )
                        bSetCheckmark = sal_True;

                    if ( bSetCheckmark )
                        rTb.CheckItem( nId, bCheckmark );
                    else
                    {
                        rtl::OUString aItemText;

                        if ( Event.State >>= aItemText )
                            rTb.SetItemText( nId, aItemText );
                    }
                }
            }

            switch(m_nSlotId)
            {
                case SID_ATTR_CHAR_COLOR2:
                case SID_BACKGROUND_COLOR:
                    {
                        util::Color nColor(COL_TRANSPARENT);
                        Event.State >>= nColor;
                        ::Color aGcc3WorkaroundTemporary( nColor);
                        SvxColorItem aColorItem(aGcc3WorkaroundTemporary,1);
                        if ( SID_ATTR_CHAR_COLOR2 == m_nSlotId )
                            static_cast<SvxFontColorExtToolBoxControl*>(m_pToolbarController.get())->StateChanged(m_nSlotId,Event.IsEnabled ? SFX_ITEM_SET : SFX_ITEM_DISABLED,&aColorItem);
                        else
                            static_cast<SvxColorToolBoxControl*>(m_pToolbarController.get())->StateChanged(m_nSlotId,Event.IsEnabled ? SFX_ITEM_SET : SFX_ITEM_DISABLED,&aColorItem);
                    }
                    break;
                case SID_ATTR_CHAR_FONT:
                    {
                        SvxFontItem aItem(ITEMID_FONT);
                        aItem.PutValue(Event.State);
                        static_cast<SvxFontNameToolBoxControl*>(m_pToolbarController.get())->StateChanged(m_nSlotId,Event.IsEnabled ? SFX_ITEM_AVAILABLE : SFX_ITEM_DISABLED,&aItem);
                    }
                    break;
            }
        }
    }
}
// -----------------------------------------------------------------------------
Reference< awt::XWindow > SAL_CALL OToolboxController::createPopupWindow() throw (RuntimeException)
{
    // execute the menu
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< awt::XWindow > xRet;
    if ( m_pToolbarController.is() )
        xRet = m_pToolbarController.getRef()->createPopupWindow();

    return xRet;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OToolboxController::opensSubToolbar() throw (uno::RuntimeException)
{
    return m_nSlotId == SID_DRAWTBX_CS_BASIC;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OToolboxController::getSubToolbarName() throw (uno::RuntimeException)
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< frame::XSubToolbarController> xSub(m_pToolbarController.getRef(),uno::UNO_QUERY);
    if ( xSub.is() )
        return xSub->getSubToolbarName();
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::functionSelected( const ::rtl::OUString& rCommand ) throw (uno::RuntimeException)
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< frame::XSubToolbarController> xSub(m_pToolbarController.getRef(),uno::UNO_QUERY);
    if ( xSub.is() )
    {
        xSub->functionSelected(m_aCommandURL = rCommand);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OToolboxController::updateImage(  ) throw (uno::RuntimeException)
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< frame::XSubToolbarController> xSub(m_pToolbarController.getRef(),uno::UNO_QUERY);
    if ( xSub.is() )
        xSub->updateImage();
}
// -----------------------------------------------------------------------------
uno::Reference< awt::XWindow > SAL_CALL OToolboxController::createItemWindow( const uno::Reference< awt::XWindow >& _xParent)
throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;
    if ( m_pToolbarController.is() )
    {
        switch(m_nSlotId)
        {
            case SID_ATTR_CHAR_FONT:
                xWindow = VCLUnoHelper::GetInterface(static_cast<SvxFontNameToolBoxControl*>(m_pToolbarController.get())->CreateItemWindow(VCLUnoHelper::GetWindow(_xParent)));
                break;
            default:
                ;
        }
    }
    return xWindow;
}
// -----------------------------------------------------------------------------
//..........................................................................
} // rptui
//..........................................................................


