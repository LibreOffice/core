/*************************************************************************
 *
 *  $RCSfile: acccfg.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-20 10:14:18 $
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

#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#include <vcl/svapp.hxx>

#include <so3/svstor.hxx>

#ifndef GCC
#pragma hdrstop
#endif

#include "cfg.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "dialog.hrc"
#include "cfg.hrc"
#include "app.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "accmgr.hxx"
#include "sfxresid.hxx"
#include "macrconf.hxx"
#include "cfgmgr.hxx"
#include "sfxresid.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "request.hxx"
#include "docfac.hxx"

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XACCELERATORCONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XAcceleratorConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

#ifdef css
    #error "ambigous praeprozessor directive for css ..."
#else
    namespace css = ::com::sun::star;
#endif

#ifdef dcss
    #error "ambigous praeprozessor directive for dcss ..."
#else
    namespace dcss = ::drafts::com::sun::star;
#endif

//static const char __FAR_DATA pUnknownStr[]    = "???";
static USHORT __FAR_DATA aCodeArr[] =
{
    KEY_F1       ,
    KEY_F2       ,
    KEY_F3       ,
    KEY_F4       ,
    KEY_F5       ,
    KEY_F6       ,
    KEY_F7       ,
    KEY_F8       ,
    KEY_F9       ,
    KEY_F10      ,
    KEY_F11      ,
    KEY_F12      ,

    KEY_DOWN     ,
    KEY_UP       ,
    KEY_LEFT     ,
    KEY_RIGHT    ,
    KEY_HOME     ,
    KEY_END      ,
    KEY_PAGEUP   ,
    KEY_PAGEDOWN ,
    KEY_RETURN   ,
    KEY_ESCAPE   ,
    KEY_BACKSPACE,
    KEY_INSERT   ,
    KEY_DELETE   ,

    KEY_OPEN        ,
    KEY_CUT         ,
    KEY_COPY        ,
    KEY_PASTE       ,
    KEY_UNDO        ,
    KEY_REPEAT      ,
    KEY_FIND        ,
    KEY_PROPERTIES  ,
    KEY_FRONT       ,
    KEY_CONTEXTMENU ,
    KEY_MENU        ,
    KEY_HELP        ,

    KEY_F1        | KEY_SHIFT,
    KEY_F2        | KEY_SHIFT,
    KEY_F3        | KEY_SHIFT,
    KEY_F4        | KEY_SHIFT,
    KEY_F5        | KEY_SHIFT,
    KEY_F6        | KEY_SHIFT,
    KEY_F7        | KEY_SHIFT,
    KEY_F8        | KEY_SHIFT,
    KEY_F9        | KEY_SHIFT,
    KEY_F10       | KEY_SHIFT,
    KEY_F11       | KEY_SHIFT,
    KEY_F12       | KEY_SHIFT,

    KEY_DOWN      | KEY_SHIFT,
    KEY_UP        | KEY_SHIFT,
    KEY_LEFT      | KEY_SHIFT,
    KEY_RIGHT     | KEY_SHIFT,
    KEY_HOME      | KEY_SHIFT,
    KEY_END       | KEY_SHIFT,
    KEY_PAGEUP    | KEY_SHIFT,
    KEY_PAGEDOWN  | KEY_SHIFT,
    KEY_RETURN    | KEY_SHIFT,
    KEY_SPACE     | KEY_SHIFT,
    KEY_ESCAPE    | KEY_SHIFT,
    KEY_BACKSPACE | KEY_SHIFT,
    KEY_INSERT    | KEY_SHIFT,
    KEY_DELETE    | KEY_SHIFT,

    KEY_0         | KEY_MOD1 ,
    KEY_1         | KEY_MOD1 ,
    KEY_2         | KEY_MOD1 ,
    KEY_3         | KEY_MOD1 ,
    KEY_4         | KEY_MOD1 ,
    KEY_5         | KEY_MOD1 ,
    KEY_6         | KEY_MOD1 ,
    KEY_7         | KEY_MOD1 ,
    KEY_8         | KEY_MOD1 ,
    KEY_9         | KEY_MOD1 ,
    KEY_A         | KEY_MOD1 ,
    KEY_B         | KEY_MOD1 ,
    KEY_C         | KEY_MOD1 ,
    KEY_D         | KEY_MOD1 ,
    KEY_E         | KEY_MOD1 ,
    KEY_F         | KEY_MOD1 ,
    KEY_G         | KEY_MOD1 ,
    KEY_H         | KEY_MOD1 ,
    KEY_I         | KEY_MOD1 ,
    KEY_J         | KEY_MOD1 ,
    KEY_K         | KEY_MOD1 ,
    KEY_L         | KEY_MOD1 ,
    KEY_M         | KEY_MOD1 ,
    KEY_N         | KEY_MOD1 ,
    KEY_O         | KEY_MOD1 ,
    KEY_P         | KEY_MOD1 ,
    KEY_Q         | KEY_MOD1 ,
    KEY_R         | KEY_MOD1 ,
    KEY_S         | KEY_MOD1 ,
    KEY_T         | KEY_MOD1 ,
    KEY_U         | KEY_MOD1 ,
    KEY_V         | KEY_MOD1 ,
    KEY_W         | KEY_MOD1 ,
    KEY_X         | KEY_MOD1 ,
    KEY_Y         | KEY_MOD1 ,
    KEY_Z         | KEY_MOD1 ,

    KEY_F1        | KEY_MOD1 ,
    KEY_F2        | KEY_MOD1 ,
    KEY_F3        | KEY_MOD1 ,
    KEY_F4        | KEY_MOD1 ,
    KEY_F5        | KEY_MOD1 ,
    KEY_F6        | KEY_MOD1 ,
    KEY_F7        | KEY_MOD1 ,
    KEY_F8        | KEY_MOD1 ,
    KEY_F9        | KEY_MOD1 ,
    KEY_F10       | KEY_MOD1 ,
    KEY_F11       | KEY_MOD1 ,
    KEY_F12       | KEY_MOD1 ,

    KEY_DOWN      | KEY_MOD1 ,
    KEY_UP        | KEY_MOD1 ,
    KEY_LEFT      | KEY_MOD1 ,
    KEY_RIGHT     | KEY_MOD1 ,
    KEY_HOME      | KEY_MOD1 ,
    KEY_END       | KEY_MOD1 ,
    KEY_PAGEUP    | KEY_MOD1 ,
    KEY_PAGEDOWN  | KEY_MOD1 ,
    KEY_RETURN    | KEY_MOD1 ,
    KEY_SPACE     | KEY_MOD1 ,
    KEY_BACKSPACE | KEY_MOD1 ,
    KEY_INSERT    | KEY_MOD1 ,
    KEY_DELETE    | KEY_MOD1 ,

    KEY_ADD       | KEY_MOD1 ,
    KEY_SUBTRACT  | KEY_MOD1 ,
    KEY_MULTIPLY  | KEY_MOD1 ,
    KEY_DIVIDE    | KEY_MOD1 ,

    KEY_0         | KEY_SHIFT | KEY_MOD1,
    KEY_1         | KEY_SHIFT | KEY_MOD1,
    KEY_2         | KEY_SHIFT | KEY_MOD1,
    KEY_3         | KEY_SHIFT | KEY_MOD1,
    KEY_4         | KEY_SHIFT | KEY_MOD1,
    KEY_5         | KEY_SHIFT | KEY_MOD1,
    KEY_6         | KEY_SHIFT | KEY_MOD1,
    KEY_7         | KEY_SHIFT | KEY_MOD1,
    KEY_8         | KEY_SHIFT | KEY_MOD1,
    KEY_9         | KEY_SHIFT | KEY_MOD1,
    KEY_A         | KEY_SHIFT | KEY_MOD1,
    KEY_B         | KEY_SHIFT | KEY_MOD1,
    KEY_C         | KEY_SHIFT | KEY_MOD1,
    KEY_D         | KEY_SHIFT | KEY_MOD1,
    KEY_E         | KEY_SHIFT | KEY_MOD1,
    KEY_F         | KEY_SHIFT | KEY_MOD1,
    KEY_G         | KEY_SHIFT | KEY_MOD1,
    KEY_H         | KEY_SHIFT | KEY_MOD1,
    KEY_I         | KEY_SHIFT | KEY_MOD1,
    KEY_J         | KEY_SHIFT | KEY_MOD1,
    KEY_K         | KEY_SHIFT | KEY_MOD1,
    KEY_L         | KEY_SHIFT | KEY_MOD1,
    KEY_M         | KEY_SHIFT | KEY_MOD1,
    KEY_N         | KEY_SHIFT | KEY_MOD1,
    KEY_O         | KEY_SHIFT | KEY_MOD1,
    KEY_P         | KEY_SHIFT | KEY_MOD1,
    KEY_Q         | KEY_SHIFT | KEY_MOD1,
    KEY_R         | KEY_SHIFT | KEY_MOD1,
    KEY_S         | KEY_SHIFT | KEY_MOD1,
    KEY_T         | KEY_SHIFT | KEY_MOD1,
    KEY_U         | KEY_SHIFT | KEY_MOD1,
    KEY_V         | KEY_SHIFT | KEY_MOD1,
    KEY_W         | KEY_SHIFT | KEY_MOD1,
    KEY_X         | KEY_SHIFT | KEY_MOD1,
    KEY_Y         | KEY_SHIFT | KEY_MOD1,
    KEY_Z         | KEY_SHIFT | KEY_MOD1,

    KEY_F1        | KEY_SHIFT | KEY_MOD1,
    KEY_F2        | KEY_SHIFT | KEY_MOD1,
    KEY_F3        | KEY_SHIFT | KEY_MOD1,
    KEY_F4        | KEY_SHIFT | KEY_MOD1,
    KEY_F5        | KEY_SHIFT | KEY_MOD1,
    KEY_F6        | KEY_SHIFT | KEY_MOD1,
    KEY_F7        | KEY_SHIFT | KEY_MOD1,
    KEY_F8        | KEY_SHIFT | KEY_MOD1,
    KEY_F9        | KEY_SHIFT | KEY_MOD1,
    KEY_F10       | KEY_SHIFT | KEY_MOD1,
    KEY_F11       | KEY_SHIFT | KEY_MOD1,
    KEY_F12       | KEY_SHIFT | KEY_MOD1,

    KEY_DOWN      | KEY_SHIFT | KEY_MOD1,
    KEY_UP        | KEY_SHIFT | KEY_MOD1,
    KEY_LEFT      | KEY_SHIFT | KEY_MOD1,
    KEY_RIGHT     | KEY_SHIFT | KEY_MOD1,
    KEY_HOME      | KEY_SHIFT | KEY_MOD1,
    KEY_END       | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEUP    | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEDOWN  | KEY_SHIFT | KEY_MOD1,

    KEY_RETURN    | KEY_SHIFT | KEY_MOD1,
    KEY_SPACE     | KEY_SHIFT | KEY_MOD1,
    KEY_BACKSPACE | KEY_SHIFT | KEY_MOD1,
    KEY_INSERT    | KEY_SHIFT | KEY_MOD1,
    KEY_DELETE    | KEY_SHIFT | KEY_MOD1
};

static long nAccCfgTabs[] =
{
        2,                              // Number of Tabs
        0,
        120                             // Function
};

#define ACC_CODEARRSIZE   ( sizeof( aCodeArr ) / sizeof( aCodeArr[ 0 ] ) )

#ifdef MSC
#pragma warning (disable:4355)
#endif


class SfxAccCfgLBoxString_Impl : public SvLBoxString
{
public:
    SfxAccCfgLBoxString_Impl( SvLBoxEntry* pEntry, USHORT nFlags, const String& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}
    virtual ~SfxAccCfgLBoxString_Impl();

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags, SvLBoxEntry* pEntry );
};

void impl_initAccCfg(const SfxTabDialog*                                               pTabDlg   ,
                     const css::uno::Reference< css::lang::XMultiServiceFactory >&     xSMGR     ,
                           css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& xGlobalAcc,
                           css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& xModuleAcc)
                           //css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& xDocAcc   )
{
    ::rtl::OUString sShortName = ::rtl::OUString::createFromAscii(pTabDlg->GetViewFrame()->GetObjectShell()->GetFactory().GetShortName());

    try
    {
        // get global configuration
        xGlobalAcc = css::uno::Reference< dcss::ui::XAcceleratorConfiguration >(
                        xSMGR->createInstance(::rtl::OUString::createFromAscii("drafts.com.sun.star.ui.GlobalAcceleratorConfiguration")),
                        css::uno::UNO_QUERY_THROW);

        // get module configuration
        css::uno::Reference< css::frame::XFrame > xFrame(
            pTabDlg->GetViewFrame()->GetFrame()->GetFrameInterface(),
            css::uno::UNO_QUERY_THROW);

        css::uno::Reference< dcss::frame::XModuleManager > xModuleManager(
            xSMGR->createInstance(::rtl::OUString::createFromAscii("drafts.com.sun.star.frame.ModuleManager")),
            css::uno::UNO_QUERY_THROW);

        ::rtl::OUString sModule = xModuleManager->identify(xFrame);

        css::uno::Reference< dcss::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(
            xSMGR->createInstance(::rtl::OUString::createFromAscii("drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier")),
            css::uno::UNO_QUERY_THROW);

        css::uno::Reference< dcss::ui::XUIConfigurationManager > xUICfgManager = xModuleCfgSupplier->getUIConfigurationManager(sModule);
        xModuleAcc = css::uno::Reference< dcss::ui::XAcceleratorConfiguration >(xUICfgManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
/*TODO_ACC
        // get document configuration
        css::uno::Reference< css::frame::XModel >      xModel      ;
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();

        if (xModel.is())
        {
            css::uno::Reference< dcss::ui::XUIConfigurationManagerSupplier > xDocCfgSupplier(xModel, css::uno::UNO_QUERY_THROW);
            xUICfgManager = xDocCfgSupplier->getUIConfigurationManager();
            xDocAcc       = css::uno::Reference< dcss::ui::XAcceleratorConfiguration >(xUICfgManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
        }
*/
    }
    catch(const css::uno::RuntimeException& exRuntime)
        { throw exRuntime; }
    catch(const css::uno::Exception&)
        {}
}

SfxAccCfgLBoxString_Impl::~SfxAccCfgLBoxString_Impl()
{
}

// -----------------------------------------------------------------------

void SfxAccCfgLBoxString_Impl::Paint( const Point& rPos, SvLBox& rDev, USHORT, SvLBoxEntry* pEntry )
{
    Font aOldFont( rDev.GetFont() );
    Font aFont( aOldFont );
    DBG_ASSERT( pEntry && pEntry->GetUserData(), "Entry or UserData invalid" );

    rDev.SetFont( aFont );

    BOOL bCanBeConfigured = ((SfxMenuConfigEntry*)pEntry->GetUserData())->IsConfigurable();
    if ( !bCanBeConfigured )
        rDev.DrawCtrlText( rPos, GetText(), 0, STRING_LEN, TEXT_DRAW_DISABLE );
    else
        rDev.DrawText( rPos, GetText() );
    rDev.SetFont( aOldFont );
}

void SfxAccCfgTabListBox_Impl::InitEntry( SvLBoxEntry* pEntry, const XubString& rTxt, const Image& rImg1, const Image& rImg2 )
{
    SvTabListBox::InitEntry( pEntry, rTxt, rImg1, rImg2 );
}

/* SfxAcceleratorConfigListBox::KeyInput() *******************************************

Springt den Eintrag an, der der gedrueckten Tastenkombination entspricht.
Ausgenommen davon sind die fuer die Dialogsteuerung ueblichen
Tastenkombinationen.

****************************************************************************/
void SfxAccCfgTabListBox_Impl::KeyInput( const KeyEvent &rKEvt )
{
    KeyCode aCode1 = rKEvt.GetKeyCode();
    USHORT  nCode  = aCode1.GetCode();

    if ( nCode != KEY_DOWN   && nCode != KEY_UP       &&
         nCode != KEY_LEFT   && nCode != KEY_RIGHT    &&
         nCode != KEY_PAGEUP && nCode != KEY_PAGEDOWN )
    {
        for ( USHORT i = 0; i < m_pAccelConfigPage->aConfigCodeArr.Count(); i++ )
        {
            KeyCode aCode2( m_pAccelConfigPage->aConfigCodeArr[ i ] );

            if ( aCode1.GetCode    () == aCode2.GetCode    () &&
                 aCode1.GetModifier() == aCode2.GetModifier() )
            {
                SvLBoxEntry *pEntry = GetEntry( 0, i );
                Select( pEntry );
                MakeVisible( pEntry );
                return;
            }
        }
    }

    SfxMenuCfgTabListBox_Impl::KeyInput( rKEvt );
}

SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
}

// SfxAcceleratorConfigPage::Ctor() ************************************************

SfxAcceleratorConfigPage::SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SfxResId( TP_CONFIG_ACCEL ), rSet ),

    aEntriesBox         ( this, this, ResId( BOX_ACC_ENTRIES   ) ),
    aKeyboardGroup      ( this, ResId( GRP_ACC_KEYBOARD        ) ),
    aChangeButton       ( this, ResId( BTN_ACC_CHANGE          ) ),
    aRemoveButton       ( this, ResId( BTN_ACC_REMOVE          ) ),
    aGroupText          ( this, ResId( TXT_ACC_GROUP           ) ),
    aGroupLBox          ( this, ResId( BOX_ACC_GROUP ), SFX_SLOT_ACCELCONFIG ),
    aFunctionText       ( this, ResId( TXT_ACC_FUNCTION        ) ),
    aFunctionBox        ( this, ResId( BOX_ACC_FUNCTION        ) ),
    aKeyText            ( this, ResId( TXT_ACC_KEY             ) ),
    aKeyBox             ( this, ResId( BOX_ACC_KEY             ) ),
    aFunctionsGroup     ( this, ResId( GRP_ACC_FUNCTIONS       ) ),
    aLoadButton         ( this, ResId( BTN_LOAD ) ),
    aSaveButton         ( this, ResId( BTN_SAVE ) ),
    aResetButton        ( this, ResId( BTN_RESET   ) ),
    aOfficeButton       ( this, ResId( RB_OFFICE   ) ),
    aModuleButton       ( this, ResId( RB_MODULE   ) ),
    pAct( 0 ),
    pModule( 0 ),
    pGlobal( 0 ),
    m_pMacroInfoItem( 0 ),
    m_bStylesInfoInitialized(sal_False)
{
    FreeResource();

    // Handler installieren
    aChangeButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      ChangeHdl ) );
    aRemoveButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      RemoveHdl ) );
    aEntriesBox  .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aGroupLBox    .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aFunctionBox .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aKeyBox      .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aLoadButton  .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Load      ) );
    aSaveButton  .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Save      ) );
    aResetButton .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Default      ) );

    Link aLink = LINK( this, SfxAcceleratorConfigPage, RadioHdl );
    aOfficeButton.SetClickHdl( aLink );
    aModuleButton.SetClickHdl( aLink );

    // initializing aAccelArr and aAccelConfigArr
    for ( USHORT i = 0; i < ACC_CODEARRSIZE; i++ )
    {
        aAccelArr.Append( 0 );
        KeyCode aKeyCode = PosToKeyCode_All( i );
        if ( aKeyCode.GetName().Len() > 0 )
        {
            aConfigAccelArr.Append( 0 );
            aConfigCodeArr.Append( aKeyCode.GetFullCode() );
        }
    }

    // Entriesbox initialisieren
//(mba)/task    SfxWaitCursor aWait;
    aEntriesBox.SetWindowBits( WB_HSCROLL|WB_CLIPCHILDREN );
    aEntriesBox.SetSelectionMode( SINGLE_SELECTION );
    aEntriesBox.SetTabs( &nAccCfgTabs[0], MAP_APPFONT );
    aEntriesBox.Resize(); // OS: Hack fuer richtige Selektion
//  aEntriesBox.SetFont( SFX_APP()->GetAppFont() );
    aEntriesBox.SetSpaceBetweenEntries( 0 );
    aEntriesBox.SetDragDropMode(0);
    aGroupLBox.SetFunctionListBox( &aFunctionBox );

}

void SfxAcceleratorConfigPage::CreateCustomItems( SvLBoxEntry* pEntry, const String& aCol1, const String& aCol2 )
{
    // Initialize text columns with own class to enable custom painting
    // This is needed as we have to paint disabled entries by ourself. No support for that in the
    // original SvTabListBox!
    SfxAccCfgLBoxString_Impl* pStrItem = new SfxAccCfgLBoxString_Impl( pEntry, 0, aCol1 );
    pEntry->ReplaceItem( pStrItem, 1 );

    pStrItem = new SfxAccCfgLBoxString_Impl( pEntry, 0, aCol2 );
    pEntry->ReplaceItem( pStrItem, 2 );
}

USHORT SfxAcceleratorConfigPage::impl_Command2ID(const ::rtl::OUString& sCommand, SfxSlotPool* pSlotPool)
{
    static ::rtl::OUString URL_SCHEMA_UNO   = ::rtl::OUString::createFromAscii(".uno:" );
    static ::rtl::OUString URL_SCHEMA_SLOT  = ::rtl::OUString::createFromAscii("slot:" );
    static ::rtl::OUString URL_SCHEMA_MACRO = ::rtl::OUString::createFromAscii("macro:");

    USHORT nId = USHRT_MAX;

    // Styles?
    // are handled outside!!!

    // macro?! TODO
    if (sCommand.indexOf(URL_SCHEMA_MACRO)>-1)
    {
    }
    else
    // uno
    if (sCommand.indexOf(URL_SCHEMA_UNO)>-1)
    {
        const SfxSlot* pSlot = pSlotPool->GetUnoSlot(sCommand);
        DBG_ASSERT(pSlot, "ACC TAB PAGE: Found no slot for given command ... ");
        if (pSlot)
             nId = pSlot->GetSlotId();
    }
    else
    // slot
    if (sCommand.indexOf(URL_SCHEMA_SLOT)>-1)
    {
        nId = (USHORT)(sCommand.copy(5).toInt32());
    }

    return nId;
}

::rtl::OUString SfxAcceleratorConfigPage::impl_ID2Command(USHORT nId, SfxSlotPool* pSlotPool)
{
    ::rtl::OUString sCommand;

    // Styles?
    if (nId >= SfxStylesInfo_Impl::START_ID_STYLES)
    {
        ::rtl::OUString sCommand = m_aStylesInfo.getStyle(nId);
        return sCommand;
    }

    // Macros?
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo( nId );
        if (pInfo)
        {
            sCommand = pInfo->GetURL();
            return sCommand;
        }
    }

    // slot, uno?
    const SfxSlot* pSlot = pSlotPool->GetSlot(nId);
    if (pSlot && pSlot->pUnoName)
    {
        sCommand  = ::rtl::OUString::createFromAscii(".uno:");
        sCommand += ::rtl::OUString::createFromAscii(pSlot->GetUnoName());

        return sCommand;
    }

    // ???
    DBG_ASSERT(sCommand.getLength(), "Found no command for ID.");
    return ::rtl::OUString();
}

void SfxAcceleratorConfigPage::Init(const css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& pAccMgr)
{
    if (!m_bStylesInfoInitialized)
    {
        m_aStylesInfo.setModel(GetTabDialog()->GetViewFrame()->GetObjectShell()->GetModel());
        aFunctionBox.SetStylesInfo(&m_aStylesInfo);
        aGroupLBox.SetStylesInfo(&m_aStylesInfo);
        m_bStylesInfoInitialized = sal_True;
    }

    // Insert all editable accelerators into list box. It is possible
    // that some accelerators are not mapped on the current system/keyboard
    // but we don't want to lose these mappings.
    for ( USHORT i=0; i< aConfigAccelArr.Count(); i++ )
    {
        String aEntry = PosToKeyCode_Config( i ).GetName();
        SfxMenuConfigEntry *pEntry = new SfxMenuConfigEntry( 0, aEntry, String(), FALSE );
        SvLBoxEntry *pLBEntry = aEntriesBox.InsertEntry( aEntry, 0L, LIST_APPEND, 0xFFFF );
        pLBEntry->SetUserData( pEntry );
        aEntriesBox.EntryInserted( pLBEntry );
    }

    SfxSlotPool* pSlotPool = &(SFX_APP()->GetSlotPool( GetTabDialog()->GetViewFrame() ));
//TODO global slot pool doesnt know ALL global commands! Dont use it yet ...    pSlotPool = &(SFX_APP()->GetAppSlotPool_Impl());

    css::uno::Sequence< css::awt::KeyEvent > lKeys = pAccMgr->getAllKeyEvents();
    sal_Int32 c = lKeys.getLength();
    sal_Int32 y = 0;

    for (y=0; y<c; ++y)
    {
        const css::awt::KeyEvent& aAWTKey  = lKeys[y];
              ::rtl::OUString     sCommand = pAccMgr->getCommandByKeyEvent(aAWTKey);
              KeyCode             aKeyCode = ::svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);

        USHORT            nId = USHRT_MAX;
        SfxStyleInfo_Impl aStyle;
        sal_Bool          bStyle = m_aStylesInfo.parseStyleCommand(sCommand, aStyle);
        if (bStyle)
            nId = aStyle.nId;
        else
            nId = impl_Command2ID(sCommand, pSlotPool);

        // init full accelerator array
        USHORT nPos = KeyCodeToPos_All( aKeyCode );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            aAccelArr[ nPos ] = nId;

        // init configurable accelerator array
        nPos = KeyCodeToPos_Config( aKeyCode );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nCol = aEntriesBox.TabCount() - 1;
            String aText ('[');
            if (bStyle)
                aText += String(aStyle.sLabel);
            else
                aText += pSlotPool->GetSlotName_Impl( nId );
            aText += ']';
            aEntriesBox.SetEntryText( aText, nPos, nCol );
            SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
            pEntry->SetId( nId );
            aConfigAccelArr[ nPos ] = nId;

            SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry( NULL, nPos );
            CreateCustomItems( pLBEntry, aEntriesBox.GetEntryText( pLBEntry, 0 ), aText );
        }
        else
        {
            // Preserve not editable accelerators
            m_aAccelBackup.push_back( AccelBackup( aKeyCode, nId ));
        }
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    ULONG nCount = Application::GetReservedKeyCodeCount();
    for ( ULONG n = 0; n < nCount; n++ )
    {
        const KeyCode* pKeyCode = Application::GetReservedKeyCode( n );
        USHORT nPos = KeyCodeToPos_Config( *pKeyCode );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nCol = aEntriesBox.TabCount() - 1;
            String aText = Application::GetReservedKeyCodeDescription( n );
            if( aText.Len() )
            {
                aText.Insert( (sal_Unicode)'[', 0 );
                aText.Append( ']' );
                aEntriesBox.SetEntryText( aText, nPos, nCol );
            }

            // Hardcoded function mapped so no ID possible and mark entry as not changeable
            SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
            pEntry->SetConfigurable( FALSE );
            pEntry->SetId( 0 );
            aConfigAccelArr[ nPos ] = 0;

            SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry( NULL, nPos );
            CreateCustomItems( pLBEntry, aEntriesBox.GetEntryText( pLBEntry, 0 ), aText );
        }
    }
}

void SfxAcceleratorConfigPage::ResetConfig()
{
    USHORT i;

    aEntriesBox.Clear();
    for ( i = 0; i < ACC_CODEARRSIZE; i++ )
        aAccelArr[i] = 0;

    for ( i = 0; i < aConfigAccelArr.Count(); i++ )
        aConfigAccelArr[i] = 0;
}

void SfxAcceleratorConfigPage::impl_appendItem(const css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& pAccMgr  ,
                                                     USHORT                                                      nId      ,
                                               const KeyCode&                                                    aVCLKey  ,
                                                     SfxSlotPool*                                                pSlotPool)
{
    css::awt::KeyEvent aAWTKey  = ::svt::AcceleratorExecute::st_VCLKey2AWTKey(aVCLKey);
    ::rtl::OUString    sCommand ;
    if (nId>0 && nId!=USHRT_MAX)
        sCommand = impl_ID2Command(nId, pSlotPool);

    try
    {
        if (sCommand.getLength())
            pAccMgr->setKeyEvent(aAWTKey, sCommand);
        else
            pAccMgr->removeKeyEvent(aAWTKey);
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}
}

void SfxAcceleratorConfigPage::Apply(const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration >& pAccMgr, BOOL bIsDefault )
{
    /* TODO_ACC
    if ( bIsDefault )
    {
        pAccMgr->UseDefault();
        pAccMgr->SetDefault(TRUE);
        return;
    }
    */

    SvUShorts aListOfIds;

    // zaehlen
    USHORT nCount = 0;
    USHORT i;
    for ( i = ACC_CODEARRSIZE; i > 0; --i )
    {
        if ( aAccelArr[i-1] )
            ++nCount;
    }

    SfxSlotPool* pSlotPool = &(SFX_APP()->GetSlotPool( GetTabDialog()->GetViewFrame() ));

    css::uno::Sequence< css::awt::KeyEvent > lKeys = pAccMgr->getAllKeyEvents();
    sal_Int32 c = lKeys.getLength();
    sal_Int32 y = 0;
    for (y=0; y<c; ++y)
    {
        const css::awt::KeyEvent& aAWTKey  = lKeys[y];
              ::rtl::OUString     sCommand = pAccMgr->getCommandByKeyEvent(aAWTKey);
              SfxStyleInfo_Impl   aStyle   ;
              USHORT              nId      ;
              if (m_aStylesInfo.parseStyleCommand(sCommand, aStyle))
                  nId = aStyle.nId;
              else
                  nId  = impl_Command2ID(sCommand, pSlotPool);

        // Macro-Eintraege referenzieren, da sie sonst beim Clear eventuell
        // entfernt werden koennten !
        if ( SfxMacroConfig::IsMacroSlot( nId ) )
        {
            aListOfIds.Insert( nId, aListOfIds.Count() );
             SFX_APP()->GetMacroConfig()->RegisterSlotId(nId);
        }
    }

//TODO_ACC  pAccMgr->Reset(nCount);

    // Liste von hinten durchgehen, damit logische Acceleratoren Vorrang
    // vor physikalischen haben.
    for ( i = ACC_CODEARRSIZE; i > 0; --i )
    {
        if ( aAccelArr[i-1] )
        {
            if ( SfxMacroConfig::IsMacroSlot( aAccelArr[i-1] ) )
            {
                USHORT nPos;
                for (nPos=0; nPos<aListOfIds.Count(); nPos++)
                    if (aListOfIds[nPos] == aAccelArr[i-1])
                        break;
                if (nPos < aListOfIds.Count())
                    aListOfIds.Remove(nPos);
                else
                    SFX_APP()->GetMacroConfig()->RegisterSlotId(aAccelArr[i-1]);
            }

            impl_appendItem(pAccMgr, aAccelArr[i-1], PosToKeyCode_All( i-1 ), pSlotPool);
        }
        else
            // HACK: remove item explicitly ... not implicitly!
            impl_appendItem(pAccMgr, 0, PosToKeyCode_All( i-1 ), pSlotPool);
    }

    // Apply the not editable accelerators to the accelerator manager,
    // otherwise we would lose them.
    std::vector< AccelBackup >::const_iterator pBackup;
    for ( pBackup = m_aAccelBackup.begin(); pBackup != m_aAccelBackup.end(); pBackup++ )
        impl_appendItem(pAccMgr, pBackup->nId, pBackup->aKeyCode, pSlotPool );

    for (i=0; i<aListOfIds.Count(); i++)
    {
        // Check if macro is still present to prevent an assertion if the macro was removed by the user before!
        if ( SFX_APP()->GetMacroConfig()->GetMacroInfo( aListOfIds[i] ))
            SFX_APP()->GetMacroConfig()->ReleaseSlotId(aListOfIds[i]);
    }

// TODO_ACC pAccMgr->SetDefault(FALSE);
}

IMPL_LINK( SfxAcceleratorConfigPage, Load, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_OPEN | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_LOADACCELCONFIG) ) );
    if ( aCfgName.Len() )
    {
        GetTabDialog()->EnterWait();
        BOOL bCreated = FALSE;
        SfxObjectShellRef xDoc;
        css::uno::Reference< dcss::ui::XUIConfigurationManager > xCfgMgr;
        css::uno::Reference< css::embed::XStorage > xRootStorage;

        // it was not the global configuration manager
        // first check if URL points to a document already loaded
        xDoc = SFX_APP()->DocAlreadyLoaded( aCfgName, TRUE, TRUE );
        if ( xDoc.Is() )
        {
            // Get ui config manager. There should always be one at the model.
            css::uno::Reference< css::frame::XModel > xModel = xDoc->GetModel();
            if ( xModel.is() )
            {
                css::uno::Reference< dcss::ui::XUIConfigurationManagerSupplier > xCfgSupplier( xModel, css::uno::UNO_QUERY_THROW );
                xCfgMgr = xCfgSupplier->getUIConfigurationManager();
            }
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            bCreated = TRUE;

            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory( pSMGR->createInstance(
                                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.StorageFactory" ))),
                                                                css::uno::UNO_QUERY_THROW );
            css::uno::Sequence< css::uno::Any > aArgs( 2 );
            aArgs[0] <<= ::rtl::OUString( aCfgName );
            aArgs[1] <<= css::embed::ElementModes::READ;

            ::rtl::OUString aUIConfigFolderName( RTL_CONSTASCII_USTRINGPARAM( "Configurations2" ));
            xRootStorage = css::uno::Reference< css::embed::XStorage >( xStorageFactory->createInstanceWithArguments( aArgs ), css::uno::UNO_QUERY_THROW );
            css::uno::Reference< css::embed::XStorage > xCfgStorage = xRootStorage->openStorageElement( aUIConfigFolderName, css::embed::ElementModes::READ );
            if ( xCfgStorage.is() )
            {
                xCfgMgr = css::uno::Reference< dcss::ui::XUIConfigurationManager >( pSMGR->createInstance(
                                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.ui.UIConfigurationManager" ))),
                                                                css::uno::UNO_QUERY_THROW );
                css::uno::Reference< dcss::ui::XUIConfigurationStorage > xCfgStore( xCfgMgr, css::uno::UNO_QUERY_THROW );
                xCfgStore->setStorage( xCfgStorage );
            }
        }

        if ( xCfgMgr.is() )
        {
            css::uno::Reference< dcss::ui::XAcceleratorConfiguration > pAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);

            aEntriesBox.SetUpdateMode( FALSE );
            ResetConfig();
            Init(pAccMgr);
            aEntriesBox.SetUpdateMode( TRUE );
            aEntriesBox.Invalidate();
            aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );

            if ( bCreated )
            {
                css::uno::Reference< css::lang::XComponent > xComponent;
                xComponent = css::uno::Reference< css::lang::XComponent >( xCfgMgr, css::uno::UNO_QUERY );
                if ( xComponent.is() )
                    xComponent->dispose();
                xComponent = css::uno::Reference< css::lang::XComponent >( xRootStorage, css::uno::UNO_QUERY );
                if ( xComponent.is() )
                    xComponent->dispose();
            }
        }

        GetTabDialog()->LeaveWait();
    }

    return 0;
}

void impl_copySource2Target(const css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& xSourceAccMgr,
                            const css::uno::Reference< dcss::ui::XAcceleratorConfiguration >& xTargetAccMgr)
{
    const css::uno::Sequence< css::awt::KeyEvent > lKeys = xSourceAccMgr->getAllKeyEvents();
          sal_Int32                                c     = lKeys.getLength();
          sal_Int32                                i     = 0;
    for (i=0; i<c; ++i)
    {
        const css::awt::KeyEvent& rKey     = lKeys[i];
              ::rtl::OUString     sCommand = xSourceAccMgr->getCommandByKeyEvent(rKey);
        xTargetAccMgr->setKeyEvent(rKey, sCommand);
    }
}

IMPL_LINK( SfxAcceleratorConfigPage, Save, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_SAVEAS | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_SAVEACCELCONFIG) ) );
    if ( aCfgName.Len() )
    {
        GetTabDialog()->EnterWait();
        BOOL bCreated = FALSE;
        BOOL bLoadedDocument = FALSE;
        SfxObjectShellRef xDoc;
        css::uno::Reference< dcss::ui::XUIConfigurationManager > xCfgMgr;
        css::uno::Reference< css::embed::XStorage > xRootStorage;

        // it was not the global configuration manager
        // first check if URL points to a document already loaded
        xDoc = SFX_APP()->DocAlreadyLoaded( aCfgName, TRUE, TRUE );
        if ( xDoc.Is() )
        {
            // get config manager, force creation if there was none before
            bLoadedDocument = TRUE;
            css::uno::Reference< css::frame::XModel > xModel = xDoc->GetModel();
            if ( xModel.is() )
            {
                css::uno::Reference< dcss::ui::XUIConfigurationManagerSupplier > xCfgSupplier( xModel, css::uno::UNO_QUERY_THROW );
                xCfgMgr = xCfgSupplier->getUIConfigurationManager();
            }
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            bCreated = TRUE;

            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory( pSMGR->createInstance(
                                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.StorageFactory" ))),
                                                                css::uno::UNO_QUERY_THROW );
            css::uno::Sequence< css::uno::Any > aArgs( 2 );
            aArgs[0] <<= ::rtl::OUString( aCfgName );
            aArgs[1] <<= css::embed::ElementModes::WRITE;

            ::rtl::OUString aUIConfigFolderName( RTL_CONSTASCII_USTRINGPARAM( "Configurations2" ));
            xRootStorage = css::uno::Reference< css::embed::XStorage >( xStorageFactory->createInstanceWithArguments( aArgs ), css::uno::UNO_QUERY_THROW );
            css::uno::Reference< css::embed::XStorage > xCfgStorage = xRootStorage->openStorageElement( aUIConfigFolderName, css::embed::ElementModes::WRITE );
            if ( xCfgStorage.is() )
            {
                ::rtl::OUString aMediaTypeProp( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ));
                ::rtl::OUString aUIConfigMediaType( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.xml.ui.configuration" ));
                ::rtl::OUString aMediaType;

                css::uno::Reference< css::beans::XPropertySet > xPropSet( xCfgStorage, css::uno::UNO_QUERY_THROW );
                css::uno::Any a = xPropSet->getPropertyValue( aMediaTypeProp );
                if ( !( a >>= aMediaType ) || ( aMediaType.getLength() == 0 ))
                {
                    a <<= aUIConfigMediaType;
                    xPropSet->setPropertyValue( aMediaTypeProp, a );
                }

                xCfgMgr = css::uno::Reference< dcss::ui::XUIConfigurationManager >( pSMGR->createInstance(
                                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.ui.UIConfigurationManager" ))),
                                                                css::uno::UNO_QUERY_THROW );
                css::uno::Reference< dcss::ui::XUIConfigurationStorage > xCfgStore( xCfgMgr, css::uno::UNO_QUERY_THROW );
                xCfgStore->setStorage( xCfgStorage );
            }
        }

        if ( xCfgMgr.is() )
        {
            // get target accelerator manager ...
            // and source accelerator manager.
            css::uno::Reference< dcss::ui::XAcceleratorConfiguration > xTargetAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< dcss::ui::XAcceleratorConfiguration > xSourceAccMgr(pAct                         , css::uno::UNO_QUERY_THROW);

            // copy the whole configuration set from source to target!
            impl_copySource2Target(xSourceAccMgr, xTargetAccMgr);

            // commit (order is important!)
            css::uno::Reference< dcss::ui::XUIConfigurationPersistence > xCommit1(xTargetAccMgr, css::uno::UNO_QUERY_THROW);
            css::uno::Reference< dcss::ui::XUIConfigurationPersistence > xCommit2(xCfgMgr, css::uno::UNO_QUERY_THROW);
            xCommit1->store();
            xCommit2->store();

            if ( bLoadedDocument )
            {
                SfxRequest aReq( SID_SAVEDOC, SFX_CALLMODE_SYNCHRON, xDoc->GetPool() );
                xDoc->ExecuteSlot( aReq );
            }
            else if ( xRootStorage.is() )
            {
                // Commit root storage
                css::uno::Reference< css::embed::XTransactedObject > xCommit3(xRootStorage, css::uno::UNO_QUERY_THROW);
                xCommit3->commit();

                css::uno::Reference< css::lang::XComponent > xComponent;
                xComponent = css::uno::Reference< css::lang::XComponent >(xCfgMgr, css::uno::UNO_QUERY);
                if (xComponent.is())
                    xComponent->dispose();
                xComponent = css::uno::Reference< css::lang::XComponent >(xRootStorage, css::uno::UNO_QUERY);
                if (xComponent.is())
                    xComponent->dispose();
            }
        }
        GetTabDialog()->LeaveWait();
    }

    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, Default, PushButton *, pPushButton )
{
    css::uno::Reference< css::form::XReset > xReset(pAct, css::uno::UNO_QUERY_THROW);
    xReset->reset();

    aEntriesBox.SetUpdateMode(FALSE);
    ResetConfig();
    Init( pAct );
    aEntriesBox.SetUpdateMode(TRUE);
    aEntriesBox.Invalidate();
    aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );

    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, ChangeHdl, Button *, pButton )
{
    // Selektierter Eintrag und selektierte Funktion
    USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    USHORT nId  = aFunctionBox.GetCurId();
    String aStr;

    // Styles?
    if (nId >= SfxStylesInfo_Impl::START_ID_STYLES)
    {
        USHORT nStylePos = (USHORT) aFunctionBox.GetModel()->GetRelPos( aFunctionBox.FirstSelected() );
        SfxGroupInfo_Impl* pInfo  = (SfxGroupInfo_Impl*)aFunctionBox.GetEntry( 0, nStylePos )->GetUserData();
        SfxStyleInfo_Impl* pStyle = (SfxStyleInfo_Impl*)pInfo->pObject;
        if (pStyle)
            aStr = pStyle->sLabel;
    }
    else
    // Macros?
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        // Es ist ein Macro selektiert, f"ur das schon eine SlotId reserviert wurde
        aStr = SFX_APP()->GetMacroConfig()->GetMacroInfo(nId)->GetMacroName();
    }
    // slot, uno?
    else
    {
        // Eine normale Funktion ist selektiert
        SfxSlotPool* pSlotPool = &( pAct == pModule ? SFX_APP()->GetSlotPool( GetTabDialog()->GetViewFrame() ) : SFX_APP()->GetAppSlotPool_Impl() );
        aStr = pSlotPool->GetSlotName_Impl( nId );
    }

    // Hilfetext setzen
    SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();

    // Funktions/Macronamen im Eintrag updaten
    String aText ('[');
    aText += aStr;
    aText += ']';
    USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( aText, nPos, nCol );

    // change configurable and full array!
    aConfigAccelArr[ nPos ] = nId;
    KeyCode aKeyCode = PosToKeyCode_Config( nPos );
    nPos = KeyCodeToPos_All( aKeyCode );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        aAccelArr[ nPos ] = nId;

    pEntry->SetId( nId );
    pEntry->SetHelpText( String() );
    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, RemoveHdl, Button *, pButton )
{
    // Selektierter Eintrag
    USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
    pEntry->SetHelpText( String() );

    // Funktionsnamen aus dem Eintrag l"oschen
    //USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( pEntry->GetName(), nPos );  // Nur letzte Spalte auf "" setzen funzt nicht - Bug ??

    // change configurable and full array!
    aConfigAccelArr[ nPos ] = 0;
    KeyCode aKeyCode = PosToKeyCode_Config( nPos );
    nPos = KeyCodeToPos_All( aKeyCode );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        aAccelArr[ nPos ] = 0;

    pEntry->SetId( 0 );
    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, SelectHdl, Control*, pListBox )
{
    // Alle Hilfetexte abschalten
    Help::ShowBalloon( this, Point(), String() );

    if ( pListBox == &aEntriesBox )
    {
        // Eintrag ausgewaehlt: Buttons enablen/disablen
        USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        if ( aConfigAccelArr[ nPos ] == 0 )
        {
            // Entry without mapped function.
            // Can be hardcoded keyboard shortcut or non-mapped keyboard shortcut
            USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
            SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
            if ( pEntry->IsConfigurable() )
                aChangeButton.Enable( TRUE );
            else
                aChangeButton.Enable( FALSE );
            aRemoveButton.Enable( FALSE );
        }
        else
        {
            aChangeButton.Enable( aConfigAccelArr[ nPos ] != aFunctionBox.GetCurId() );
            aRemoveButton.Enable( aConfigAccelArr[ nPos ] > 0 );
        }
    }
    else if ( pListBox == &aGroupLBox )
    {
        aGroupLBox.GroupSelected();
        if ( !aFunctionBox.FirstSelected() )
            aChangeButton.Enable( FALSE );
    }
    else if ( pListBox == &aFunctionBox )
    {
        aFunctionBox.FunctionSelected();

        // Zuerst "uberpr"ufen, ob durch den Wechsel der Selektion der Zustand des ChangeButtons wechselt
        USHORT nEntryPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        USHORT nId = aFunctionBox.GetCurId();

        if ( aConfigAccelArr[ nEntryPos ] == 0 )
        {
            // Entry without mapped function.
            // Can be hardcoded keyboard shortcut or non-mapped keyboard shortcut
            SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nEntryPos )->GetUserData();
            if ( pEntry->IsConfigurable() )
                aChangeButton.Enable( aConfigAccelArr[ nEntryPos ] != nId );
            else
                aChangeButton.Enable( FALSE );
            aRemoveButton.Enable( FALSE );
        }
        else
        {
            aChangeButton.Enable( aConfigAccelArr[ nEntryPos ] != nId );
            aRemoveButton.Enable( aConfigAccelArr[ nEntryPos ] > 0 );
        }

        aKeyBox.Clear();
        aKeyArr.Clear();

        for ( USHORT i = 0; i < aConfigAccelArr.Count(); i++ )
        {
            if ( aConfigAccelArr[ i ] == nId )
            {
                KeyCode aCode = PosToKeyCode_Config( i );
                aKeyBox.InsertEntry( aCode.GetName() );
                aKeyArr.Append( i );
            }
        }
    }
    else
    {
        // Taste ausgewaehlt: Eintrag anspringen
        USHORT n = aKeyBox.GetSelectEntryPos();
        USHORT nPos = aKeyArr[ n ];
        SvLBoxEntry *pEntry = aEntriesBox.GetEntry( 0, nPos );
        aEntriesBox.Select( pEntry );
        aEntriesBox.MakeVisible( pEntry );
    }
    return 0;
}

KeyCode SfxAcceleratorConfigPage::PosToKeyCode_Config( USHORT nPos ) const
{
    DBG_ASSERT( nPos < aConfigCodeArr.Count(), "Invalid position!" );

    KeyCode aTmpCode( aConfigCodeArr[ nPos ] & 0xFFF,
                      aConfigCodeArr[ nPos ] & ( KEY_SHIFT | KEY_MOD1 ) );
    return aTmpCode;
}

USHORT SfxAcceleratorConfigPage::KeyCodeToPos_Config( const KeyCode &rCode ) const
{
    USHORT nCode = rCode.GetCode() + rCode.GetModifier();

    for ( USHORT i = 0; i < aConfigCodeArr.Count(); i++ )
    {
        if ( aConfigCodeArr[ i ] == nCode )
            return i;
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

KeyCode SfxAcceleratorConfigPage::PosToKeyCode_All( USHORT nPos ) const
{
    DBG_ASSERT( nPos < ACC_CODEARRSIZE, "Invalid position!" );

    KeyCode aTmpCode( aCodeArr[ nPos ] & 0xFFF,
                      aCodeArr[ nPos ] & ( KEY_SHIFT | KEY_MOD1 ) );
    return aTmpCode;
}

USHORT SfxAcceleratorConfigPage::KeyCodeToPos_All( const KeyCode &rCode ) const
{
    USHORT nCode = rCode.GetCode() + rCode.GetModifier();

    for ( USHORT i = 0; i < ACC_CODEARRSIZE; i++ )
    {
        if ( aCodeArr[ i ] == nCode )
            return i;
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

String SfxAcceleratorConfigPage::GetFunctionName( KeyFuncType eType ) const
{
    String aStr;

    switch ( eType )
    {
        case KEYFUNC_NEW         : aStr = String( SfxResId( STR_NEW          ) ); break;
        case KEYFUNC_OPEN        : aStr = String( SfxResId( STR_OPEN         ) ); break;
        case KEYFUNC_SAVE        : aStr = String( SfxResId( STR_SAVE         ) ); break;
        case KEYFUNC_SAVEAS      : aStr = String( SfxResId( STR_SAVEAS       ) ); break;
        case KEYFUNC_PRINT       : aStr = String( SfxResId( STR_PRINT        ) ); break;
        case KEYFUNC_CLOSE       : aStr = String( SfxResId( STR_CLOSE        ) ); break;
        case KEYFUNC_QUIT        : aStr = String( SfxResId( STR_QUIT         ) ); break;
        case KEYFUNC_CUT         : aStr = String( SfxResId( STR_CUT          ) ); break;
        case KEYFUNC_COPY        : aStr = String( SfxResId( STR_COPY         ) ); break;
        case KEYFUNC_PASTE       : aStr = String( SfxResId( STR_PASTE        ) ); break;
        case KEYFUNC_UNDO        : aStr = String( SfxResId( STR_UNDO         ) ); break;
        case KEYFUNC_REDO        : aStr = String( SfxResId( STR_REDO         ) ); break;
        case KEYFUNC_DELETE      : aStr = String( SfxResId( STR_DELETE       ) ); break;
        case KEYFUNC_REPEAT      : aStr = String( SfxResId( STR_REPEAT       ) ); break;
        case KEYFUNC_FIND        : aStr = String( SfxResId( STR_FIND         ) ); break;
        case KEYFUNC_FINDBACKWARD: aStr = String( SfxResId( STR_FINDBACKWARD ) ); break;
        case KEYFUNC_PROPERTIES  : aStr = String( SfxResId( STR_PROPERTIES   ) ); break;
        case KEYFUNC_FRONT       : aStr = String( SfxResId( STR_FRONT        ) ); break;

        default: DBG_ERROR( "Invalid KeyFuncType!" );
    }

    aStr = String('\"').Append(aStr).Append('\"');
    return aStr;
}

void SfxAcceleratorConfigPage::SelectMacro(const SfxMacroInfoItem *pItem)
{
    m_pMacroInfoItem = pItem;
    aGroupLBox.SelectMacro( pItem );
}

BOOL SfxAcceleratorConfigPage::FillItemSet( SfxItemSet& )
{
    BOOL bRet = FALSE;

    css::uno::Reference< dcss::ui::XAcceleratorConfiguration > pOther = pAct == pGlobal ? pModule : pGlobal;
//  if ( pAct->bModified )
    {
        Apply( pAct, /*pAct->bDefault*/FALSE );
        //pAct->bModified = FALSE;
        pAct->store();
        //DELETEZ( pAct->pChanged );
        bRet = TRUE;
    }
return bRet; // TODO
    if ( pOther.is() )
    {
        aEntriesBox.SetUpdateMode( FALSE );
        ResetConfig();
        Init( pOther );
        Apply( pOther, /*pOther->bDefault*/FALSE );
        //pOther->bModified = FALSE;
        pOther->store();
        //DELETEZ( pOther->pChanged );
        bRet = TRUE;
        ResetConfig();
        Init( pAct );
        aEntriesBox.SetUpdateMode( TRUE );
    }
    return bRet;
}

void SfxAcceleratorConfigPage::Reset( const SfxItemSet& )
{
    String aModName = GetTabDialog()->GetViewFrame()->GetObjectShell()->GetFactory().GetModuleName();
    String aText = aModuleButton.GetText();
    aText.SearchAndReplace(String::CreateFromAscii("$(MODULE)"), aModName );
    aModuleButton.SetText( aText );

    if (!pSMGR.is())
        pSMGR = ::comphelper::getProcessServiceFactory();
    pGlobal.clear();
    pModule.clear();
    pAct.clear();
    impl_initAccCfg(GetTabDialog(), pSMGR, pGlobal, pModule);

    if (pModule.is())
        aModuleButton.Check();
    else
    {
        aModuleButton.Hide();
        aOfficeButton.Check();
    }
    RadioHdl(0);

    if ( m_pMacroInfoItem )
        aGroupLBox.SelectMacro( m_pMacroInfoItem );
}

IMPL_LINK( SfxAcceleratorConfigPage, RadioHdl, RadioButton *, pBtn )
{
    css::uno::Reference< dcss::ui::XAcceleratorConfiguration > pOld = pAct;
    if ( aOfficeButton.IsChecked())
        pAct = pGlobal;
    else if ( aModuleButton.IsChecked())
        pAct = pModule;

    // nothing changed? => do nothing!
    if (pOld == pAct)
        return 0;

    aEntriesBox.SetUpdateMode( FALSE );
    ResetConfig();
    Init( pAct );
    aEntriesBox.SetUpdateMode( TRUE );
    aEntriesBox.Invalidate();

    SfxSlotPool* pPool = &( pAct == pModule ? SFX_APP()->GetSlotPool( GetTabDialog()->GetViewFrame() ) : SFX_APP()->GetAppSlotPool_Impl() );
     aGroupLBox.Init( NULL, pPool );
    aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );
    aGroupLBox.Select( aGroupLBox.GetEntry( 0, 0 ) );
    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 1L;
}

