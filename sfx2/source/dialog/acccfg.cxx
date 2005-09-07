/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acccfg.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:10:47 $
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

//-----------------------------------------------
// include own files

#include "acccfg.hxx"
#include "cfg.hxx"
#include "sfxresid.hxx"
#include "msg.hxx"
#include "macrconf.hxx"
#include "app.hxx"
#include "filedlghelper.hxx"

#include "dialog.hrc"
#include "cfg.hrc"

//-----------------------------------------------
// include interface declarations

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
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

#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

//-----------------------------------------------
// include other projects

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

//-----------------------------------------------
// namespaces

#ifdef css
    #error "ambigous praeprozessor directive for css ..."
#else
    namespace css = ::com::sun::star;
#endif

//-----------------------------------------------
// definitions

//-----------------------------------------------
static ::rtl::OUString SERVICE_STORAGEFACTORY           = ::rtl::OUString::createFromAscii("com.sun.star.embed.StorageFactory"                          );
static ::rtl::OUString SERVICE_UICONFIGMGR              = ::rtl::OUString::createFromAscii("com.sun.star.ui.UIConfigurationManager"              );
static ::rtl::OUString SERVICE_DESKTOP                  = ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"                                 );
static ::rtl::OUString SERVICE_MODULEMANAGER            = ::rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager"                    );
static ::rtl::OUString SERVICE_GLOBALACCCFG             = ::rtl::OUString::createFromAscii("com.sun.star.ui.GlobalAcceleratorConfiguration"      );
static ::rtl::OUString SERVICE_MODULEUICONFIGSUPPLIER   = ::rtl::OUString::createFromAscii("com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
static ::rtl::OUString SERVICE_UICMDDESCRIPTION         = ::rtl::OUString::createFromAscii("com.sun.star.frame.UICommandDescription"             );

static ::rtl::OUString MODULEPROP_SHORTNAME             = ::rtl::OUString::createFromAscii("ooSetupFactoryShortName"                                    );
static ::rtl::OUString MODULEPROP_UINAME                = ::rtl::OUString::createFromAscii("ooSetupFactoryUIName"                                       );
static ::rtl::OUString CMDPROP_UINAME                   = ::rtl::OUString::createFromAscii("Name"                                                       );

static ::rtl::OUString FOLDERNAME_UICONFIG              = ::rtl::OUString::createFromAscii("Configurations2"                                            );

static ::rtl::OUString MEDIATYPE_PROPNAME               = ::rtl::OUString::createFromAscii("MediaType"                                                  );
static ::rtl::OUString MEDIATYPE_UICONFIG               = ::rtl::OUString::createFromAscii("application/vnd.sun.xml.ui.configuration"                   );

//-----------------------------------------------
static USHORT __FAR_DATA KEYCODE_ARRAY[] =
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
    KEY_DELETE    | KEY_SHIFT | KEY_MOD1,
};

static USHORT KEYCODE_ARRAY_SIZE = (sizeof(KEYCODE_ARRAY) / sizeof(KEYCODE_ARRAY[0]));

//-----------------------------------------------
// seems to be needed to layout the list box, which shows all
// assignable shortcuts
static long AccCfgTabs[] =
{
    2,  // Number of Tabs
    0,
    120 // Function
};

//-----------------------------------------------
class SfxAccCfgLBoxString_Impl : public SvLBoxString
{
    public:
    SfxAccCfgLBoxString_Impl(      SvLBoxEntry* pEntry,
                                   USHORT       nFlags,
                             const String&      sText );

    virtual ~SfxAccCfgLBoxString_Impl();

    virtual void Paint(const Point&       aPos   ,
                             SvLBox&      rDevice,
                             USHORT       nFlags ,
                             SvLBoxEntry* pEntry );
};

//-----------------------------------------------
SfxAccCfgLBoxString_Impl::SfxAccCfgLBoxString_Impl(      SvLBoxEntry* pEntry,
                                                         USHORT       nFlags,
                                                   const String&      sText )
        : SvLBoxString(pEntry, nFlags, sText)
{
}

//-----------------------------------------------
SfxAccCfgLBoxString_Impl::~SfxAccCfgLBoxString_Impl()
{
}

//-----------------------------------------------
void SfxAccCfgLBoxString_Impl::Paint(const Point&       aPos   ,
                                           SvLBox&      rDevice,
                                           USHORT       nFlags ,
                                           SvLBoxEntry* pEntry )
{
    /*/ ??? realy needed !!!
    Font aOldFont(rDevice.GetFont());
    Font aNewFont(aOldFont         );
    rDevice.SetFont( aFont );
    */

    if (!pEntry)
        return;

    TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
    if (!pUserData)
        return;

    if (pUserData->m_bIsConfigurable)
        rDevice.DrawText(aPos, GetText());
    else
        rDevice.DrawCtrlText(aPos, GetText(), 0, STRING_LEN, TEXT_DRAW_DISABLE);

    //rDev.SetFont( aOldFont );
}

//-----------------------------------------------
void SfxAccCfgTabListBox_Impl::InitEntry(      SvLBoxEntry* pEntry ,
                                         const XubString&   sText  ,
                                         const Image&       aImage1,
                                         const Image&       aImage2)
{
    SvTabListBox::InitEntry(pEntry, sText, aImage1, aImage2);
}

//-----------------------------------------------
/** select the entry, which match the current key input ... excepting
    keys, which are used for the dialog itself.
  */
void SfxAccCfgTabListBox_Impl::KeyInput(const KeyEvent& aKey)
{
    KeyCode aCode1 = aKey.GetKeyCode();
    USHORT  nCode1 = aCode1.GetCode();
    USHORT  nMod1  = aCode1.GetModifier();

    // is it related to our list box ?
    if (
        (nCode1 != KEY_DOWN    ) &&
        (nCode1 != KEY_UP      ) &&
        (nCode1 != KEY_LEFT    ) &&
        (nCode1 != KEY_RIGHT   ) &&
        (nCode1 != KEY_PAGEUP  ) &&
        (nCode1 != KEY_PAGEDOWN)
       )
    {
        SvLBoxEntry* pEntry = First();
        while (pEntry)
        {
            TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
            if (pUserData)
            {
                USHORT nCode2 = pUserData->m_aKey.GetCode();
                USHORT nMod2  = pUserData->m_aKey.GetModifier();
                if (
                    (nCode1 == nCode2) &&
                    (nMod1  == nMod2 )
                   )
                {
                    Select     (pEntry);
                    MakeVisible(pEntry);
                    return;
                }
            }
            pEntry = Next(pEntry);
        }
    }

    // no - handle it as normal dialog input
    SvTabListBox::KeyInput(aKey);
}

//-----------------------------------------------
SfxAcceleratorConfigPage::SfxAcceleratorConfigPage(      Window*     pParent,
                                                   const SfxItemSet& aSet   )

    : SfxTabPage              (pParent, SfxResId(TP_CONFIG_ACCEL), aSet)
    , aEntriesBox             (this   , this, ResId(BOX_ACC_ENTRIES   ))
    , aKeyboardGroup          (this   , ResId(GRP_ACC_KEYBOARD        ))
    , aChangeButton           (this   , ResId(BTN_ACC_CHANGE          ))
    , aRemoveButton           (this   , ResId(BTN_ACC_REMOVE          ))
    , aGroupText              (this   , ResId(TXT_ACC_GROUP           ))
    , aGroupLBox              (this   , ResId(BOX_ACC_GROUP), SFX_SLOT_ACCELCONFIG)
    , aFunctionText           (this   , ResId(TXT_ACC_FUNCTION        ))
    , aFunctionBox            (this   , ResId(BOX_ACC_FUNCTION        ))
    , aKeyText                (this   , ResId(TXT_ACC_KEY             ))
    , aKeyBox                 (this   , ResId(BOX_ACC_KEY             ))
    , aFunctionsGroup         (this   , ResId(GRP_ACC_FUNCTIONS       ))
    , aLoadButton             (this   , ResId(BTN_LOAD                ))
    , aSaveButton             (this   , ResId(BTN_SAVE                ))
    , aResetButton            (this   , ResId(BTN_RESET               ))
    , aOfficeButton           (this   , ResId(RB_OFFICE               ))
    , aModuleButton           (this   , ResId(RB_MODULE               ))
    , m_xAct                  ()
    , m_xModule               ()
    , m_xGlobal               ()
    , m_pMacroInfoItem        ()
    , m_bStylesInfoInitialized(sal_False)
{
    FreeResource();

    // install handler functions
    aChangeButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, ChangeHdl ));
    aRemoveButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RemoveHdl ));
    aEntriesBox.SetSelectHdl ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aGroupLBox.SetSelectHdl  ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aFunctionBox.SetSelectHdl( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aKeyBox.SetSelectHdl     ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aLoadButton.SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Load      ));
    aSaveButton.SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Save      ));
    aResetButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage, Default   ));
    aOfficeButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));
    aModuleButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));

    // initialize Entriesbox
    aEntriesBox.SetWindowBits(WB_HSCROLL|WB_CLIPCHILDREN);
    aEntriesBox.SetSelectionMode(SINGLE_SELECTION);
    aEntriesBox.SetTabs(&AccCfgTabs[0], MAP_APPFONT);
    aEntriesBox.Resize(); // OS: Hack for right selection
    aEntriesBox.SetSpaceBetweenEntries(0);
    aEntriesBox.SetDragDropMode(0);

    // initialize GroupBox
    aGroupLBox.SetFunctionListBox(&aFunctionBox);

    // initialize KeyBox
    aKeyBox.SetWindowBits(WB_CLIPCHILDREN|WB_HSCROLL|WB_SORT);
}

//-----------------------------------------------
SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
    // free memory - remove all dynamic user data
    SvLBoxEntry* pEntry = aEntriesBox.First();
    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
            delete pUserData;
        pEntry = aEntriesBox.Next(pEntry);
    }

    pEntry = aKeyBox.First();
    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
            delete pUserData;
        pEntry = aKeyBox.Next(pEntry);
    }

    aEntriesBox.Clear();
    aKeyBox.Clear();
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::InitAccCfg()
{
    // already initialized ?
    if (m_xSMGR.is())
        return; // yes -> do nothing

    try
    {
        // no - initialize this instance
        m_xSMGR = ::utl::getProcessServiceFactory();

        m_xUICmdDescription = css::uno::Reference< css::container::XNameAccess >(m_xSMGR->createInstance(SERVICE_UICMDDESCRIPTION), css::uno::UNO_QUERY_THROW);

        // get the current active frame, which should be our "parent"
        // for this session
        css::uno::Reference< css::frame::XFramesSupplier > xDesktop(m_xSMGR->createInstance(SERVICE_DESKTOP), css::uno::UNO_QUERY_THROW);
        m_xFrame = xDesktop->getActiveFrame();

        // identify module
        css::uno::Reference< css::frame::XModuleManager > xModuleManager    (m_xSMGR->createInstance(SERVICE_MODULEMANAGER), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xModuleManagerCont(xModuleManager                                , css::uno::UNO_QUERY_THROW);
        m_sModuleLongName = xModuleManager->identify(m_xFrame);
        ::comphelper::SequenceAsHashMap lModuleProps(xModuleManagerCont->getByName(m_sModuleLongName));
        m_sModuleShortName = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_SHORTNAME, ::rtl::OUString());
        m_sModuleUIName    = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_UINAME   , ::rtl::OUString());

        // get global accelerator configuration
        m_xGlobal = css::uno::Reference< css::ui::XAcceleratorConfiguration >(m_xSMGR->createInstance(SERVICE_GLOBALACCCFG), css::uno::UNO_QUERY_THROW);

        // get module accelerator configuration
        css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(m_xSMGR->createInstance(SERVICE_MODULEUICONFIGSUPPLIER), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::ui::XUIConfigurationManager > xUICfgManager = xModuleCfgSupplier->getUIConfigurationManager(m_sModuleLongName);
        m_xModule = css::uno::Reference< css::ui::XAcceleratorConfiguration >(xUICfgManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        { m_xSMGR.clear(); }
}

//-----------------------------------------------
/** Initialize text columns with own class to enable custom painting
    This is needed as we have to paint disabled entries by ourself. No support for that in the
    original SvTabListBox!
  */
void SfxAcceleratorConfigPage::CreateCustomItems(      SvLBoxEntry* pEntry,
                                                 const String&      sCol1 ,
                                                 const String&      sCol2 )
{
    SfxAccCfgLBoxString_Impl* pStringItem = new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol1);
    pEntry->ReplaceItem(pStringItem, 1);

    pStringItem = new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol2);
    pEntry->ReplaceItem(pStringItem, 2);
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    if (!m_bStylesInfoInitialized)
    {
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel > xModel;
        if (m_xFrame.is())
            xController = m_xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();

        m_aStylesInfo.setModel(xModel);
        aFunctionBox.SetStylesInfo(&m_aStylesInfo);
        aGroupLBox.SetStylesInfo(&m_aStylesInfo);
        m_bStylesInfoInitialized = sal_True;
    }

    // Insert all editable accelerators into list box. It is possible
    // that some accelerators are not mapped on the current system/keyboard
    // but we don't want to lose these mappings.
    USHORT c1       = KEYCODE_ARRAY_SIZE;
    USHORT i1       = 0;
    USHORT nListPos = 0;
    for (i1=0; i1<c1; ++i1)
    {
        KeyCode aKey = KEYCODE_ARRAY[i1];
        String  sKey = aKey.GetName();
        if (!sKey.Len())
            continue;
        TAccInfo*    pEntry   = new TAccInfo(i1, nListPos, aKey);
        SvLBoxEntry* pLBEntry = aEntriesBox.InsertEntry(sKey, 0L, LIST_APPEND, 0xFFFF);
        pLBEntry->SetUserData(pEntry);
    }

    // Assign all commands to its shortcuts - reading the accelerator config.
    css::uno::Sequence< css::awt::KeyEvent > lKeys = xAccMgr->getAllKeyEvents();
    sal_Int32                                c2    = lKeys.getLength();
    sal_Int32                                i2    = 0;
    USHORT                                   nCol  = aEntriesBox.TabCount()-1;

    for (i2=0; i2<c2; ++i2)
    {
        const css::awt::KeyEvent& aAWTKey  = lKeys[i2];
              ::rtl::OUString     sCommand = xAccMgr->getCommandByKeyEvent(aAWTKey);
              String              sLabel   = GetLabel4Command(sCommand);
              KeyCode             aKeyCode = ::svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);
              USHORT              nPos     = MapKeyCodeToPos(aKeyCode);

        if (nPos == LISTBOX_ENTRY_NOTFOUND)
            continue;

        aEntriesBox.SetEntryText(sLabel, nPos, nCol);

        SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry(0, nPos);
        TAccInfo*    pEntry   = (TAccInfo*)pLBEntry->GetUserData();

        pEntry->m_bIsConfigurable = sal_True;
        pEntry->m_sCommand        = sCommand;
        CreateCustomItems(pLBEntry, aEntriesBox.GetEntryText(pLBEntry, 0), sLabel);
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    ULONG c3 = Application::GetReservedKeyCodeCount();
    ULONG i3 = 0;
    for (i3=0; i3<c3; ++i3)
    {
        const KeyCode* pKeyCode = Application::GetReservedKeyCode(i3);
              USHORT   nPos     = MapKeyCodeToPos(*pKeyCode);

        if (nPos == LISTBOX_ENTRY_NOTFOUND)
            continue;

        // Hardcoded function mapped so no ID possible and mark entry as not changeable
        SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry(0, nPos);
        TAccInfo*    pEntry   = (TAccInfo*)pLBEntry->GetUserData();

        pEntry->m_bIsConfigurable = sal_False;
        CreateCustomItems(pLBEntry, aEntriesBox.GetEntryText(pLBEntry, 0), String());
    }
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    // Go through the list from the bottom to the top ...
    // because logical accelerator must be preferred instead of
    // physical ones!
    SvLBoxEntry* pEntry = aEntriesBox.First();
    while (pEntry)
    {
        TAccInfo*          pUserData = (TAccInfo*)pEntry->GetUserData();
        ::rtl::OUString    sCommand  ;
        css::awt::KeyEvent aAWTKey   ;

        if (pUserData)
        {
            sCommand = pUserData->m_sCommand;
            aAWTKey  = ::svt::AcceleratorExecute::st_VCLKey2AWTKey(pUserData->m_aKey);
        }

        try
        {
            if (sCommand.getLength())
                xAccMgr->setKeyEvent(aAWTKey, sCommand);
            else
                xAccMgr->removeKeyEvent(aAWTKey);
        }
        catch(const css::uno::RuntimeException& exRun)
            { throw exRun; }
        catch(const css::uno::Exception&)
            {}

        pEntry = aEntriesBox.Next(pEntry);
    }
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::ResetConfig()
{
    aEntriesBox.Clear();
}

String FileDialog_Impl( Window *pParent, WinBits nBits, const String& rTitle )
{
    BOOL bSave = ( ( nBits & WB_SAVEAS ) == WB_SAVEAS );
    short nDialogType = bSave? ::sfx2::FILESAVE_SIMPLE : ::sfx2::FILEOPEN_SIMPLE;

    sfx2::FileDialogHelper aFileDlg( nDialogType, 0 );
    aFileDlg.SetTitle( rTitle );
//  aFileDlg.SetDialogHelpId( bSave? HID_CONFIG_SAVE : HID_CONFIG_LOAD );
    aFileDlg.AddFilter( String(SfxResId(STR_FILTERNAME_ALL) ), DEFINE_CONST_UNICODE(FILEDIALOG_FILTER_ALL) );
    aFileDlg.AddFilter( String(SfxResId(STR_FILTERNAME_CFG)),DEFINE_CONST_UNICODE("*.cfg") );
    if ( ERRCODE_NONE == aFileDlg.Execute() )
        return aFileDlg.GetPath();
    else
        return String();
}

//-----------------------------------------------
IMPL_LINK(SfxAcceleratorConfigPage, Load, Button*, pButton)
{
    // ask for filename, where we should load the new config data from
    ::rtl::OUString sCfgName = FileDialog_Impl(this, WB_OPEN | WB_STDMODAL | WB_3DLOOK, String(SfxResId(STR_LOADACCELCONFIG)));
    if (!sCfgName.getLength())
        return 0;

    GetTabDialog()->EnterWait();

    css::uno::Reference< css::frame::XModel >                xDoc        ;
    css::uno::Reference< css::ui::XUIConfigurationManager > xCfgMgr     ;
    css::uno::Reference< css::embed::XStorage >              xRootStorage; // we must hold the root storage alive, if xCfgMgr is used!

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // Get ui config manager. There should always be one at the model.
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xCfgSupplier(xDoc, css::uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            // dont forget to release the storage afterwards!
            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory(m_xSMGR->createInstance(SERVICE_STORAGEFACTORY), css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any >                     lArgs(2);
            lArgs[0] <<= sCfgName;
            lArgs[1] <<= css::embed::ElementModes::READ;

            xRootStorage = css::uno::Reference< css::embed::XStorage >(xStorageFactory->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::embed::XStorage > xUIConfig = xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, css::embed::ElementModes::READ);
            if (xUIConfig.is())
            {
                xCfgMgr = css::uno::Reference< css::ui::XUIConfigurationManager >(m_xSMGR->createInstance(SERVICE_UICONFIGMGR), css::uno::UNO_QUERY_THROW);
                css::uno::Reference< css::ui::XUIConfigurationStorage > xCfgMgrStore(xCfgMgr, css::uno::UNO_QUERY_THROW);
                xCfgMgrStore->setStorage(xUIConfig);
            }
        }

        if (xCfgMgr.is())
        {
            // open the configuration and update our UI
            css::uno::Reference< css::ui::XAcceleratorConfiguration > xTempAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);

            aEntriesBox.SetUpdateMode(FALSE);
            ResetConfig();
            Init(xTempAccMgr);
            aEntriesBox.SetUpdateMode(TRUE);
            aEntriesBox.Invalidate();
            aEntriesBox.Select(aEntriesBox.GetEntry(0, 0));

        }

        // dont forget to close the new opened storage!
        // We are the owner of it.
        if (xRootStorage.is())
        {
            css::uno::Reference< css::lang::XComponent > xComponent;
            xComponent = css::uno::Reference< css::lang::XComponent >(xCfgMgr, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xComponent = css::uno::Reference< css::lang::XComponent >(xRootStorage, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}

    GetTabDialog()->LeaveWait();

    return 0;
}

//-----------------------------------------------
IMPL_LINK(SfxAcceleratorConfigPage, Save, Button*, pButton)
{
    ::rtl::OUString sCfgName = FileDialog_Impl(this, WB_SAVEAS | WB_STDMODAL | WB_3DLOOK, String(SfxResId(STR_SAVEACCELCONFIG)));
    if (!sCfgName.getLength())
        return 0;

    GetTabDialog()->EnterWait();

    css::uno::Reference< css::frame::XModel >                xDoc        ;
    css::uno::Reference< css::ui::XUIConfigurationManager > xCfgMgr     ;
    css::uno::Reference< css::embed::XStorage >              xRootStorage;

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // get config manager, force creation if there was none before
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xCfgSupplier(xDoc, css::uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory(m_xSMGR->createInstance(SERVICE_STORAGEFACTORY), css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any >                     lArgs(2);
            lArgs[0] <<= sCfgName;
            lArgs[1] <<= css::embed::ElementModes::WRITE;

            xRootStorage = css::uno::Reference< css::embed::XStorage >(
                                xStorageFactory->createInstanceWithArguments(lArgs),
                                css::uno::UNO_QUERY_THROW);

            css::uno::Reference< css::embed::XStorage > xUIConfig(
                                xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, css::embed::ElementModes::WRITE),
                                css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::beans::XPropertySet > xUIConfigProps(
                                xUIConfig,
                                css::uno::UNO_QUERY_THROW);

            // set the correct media type if the storage was new created
            ::rtl::OUString sMediaType;
            xUIConfigProps->getPropertyValue(MEDIATYPE_PROPNAME) >>= sMediaType;
            if (!sMediaType.getLength())
                xUIConfigProps->setPropertyValue(MEDIATYPE_PROPNAME, css::uno::makeAny(MEDIATYPE_UICONFIG));

            xCfgMgr = css::uno::Reference< css::ui::XUIConfigurationManager >(m_xSMGR->createInstance(SERVICE_UICONFIGMGR), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::ui::XUIConfigurationStorage > xUICfgStore(xCfgMgr, css::uno::UNO_QUERY_THROW);
            xUICfgStore->setStorage(xUIConfig);
        }

        if (xCfgMgr.is())
        {
            // get target accelerator manager ...
            // and source accelerator manager.
            css::uno::Reference< css::ui::XAcceleratorConfiguration > xTargetAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::ui::XAcceleratorConfiguration > xSourceAccMgr(m_xAct                       , css::uno::UNO_QUERY_THROW);

            // copy the whole configuration set from source to target!
            CopySource2Target(xSourceAccMgr, xTargetAccMgr);

            // commit (order is important!)
            css::uno::Reference< css::ui::XUIConfigurationPersistence > xCommit1(xTargetAccMgr, css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::ui::XUIConfigurationPersistence > xCommit2(xCfgMgr      , css::uno::UNO_QUERY_THROW);
            xCommit1->store();
            xCommit2->store();

            if (xRootStorage.is())
            {
                // Commit root storage
                css::uno::Reference< css::embed::XTransactedObject > xCommit3(xRootStorage, css::uno::UNO_QUERY_THROW);
                xCommit3->commit();
            }
        }

        if (xRootStorage.is())
        {
            css::uno::Reference< css::lang::XComponent > xComponent;
            xComponent = css::uno::Reference< css::lang::XComponent >(xCfgMgr, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xComponent = css::uno::Reference< css::lang::XComponent >(xRootStorage, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}

    GetTabDialog()->LeaveWait();

    return 0;
}

//-----------------------------------------------
IMPL_LINK(SfxAcceleratorConfigPage, Default, PushButton*, pPushButton)
{
    css::uno::Reference< css::form::XReset > xReset(m_xAct, css::uno::UNO_QUERY);
    if (xReset.is())
        xReset->reset();

    aEntriesBox.SetUpdateMode(FALSE);
    ResetConfig();
    Init(m_xAct);
    aEntriesBox.SetUpdateMode(TRUE);
    aEntriesBox.Invalidate();
    aEntriesBox.Select(aEntriesBox.GetEntry(0, 0));

    return 0;
}

//-----------------------------------------------
IMPL_LINK(SfxAcceleratorConfigPage, ChangeHdl, Button*, pButton)
{
    USHORT    nPos        = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    TAccInfo* pEntry      = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
    String    sNewCommand = aFunctionBox.GetCurCommand();
    String    sLabel      = aFunctionBox.GetCurLabel();
    if (!sLabel.Len())
        sLabel = GetLabel4Command(sNewCommand);

    pEntry->m_sCommand = sNewCommand;
    USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText(sLabel, nPos, nCol);

    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

//-----------------------------------------------
IMPL_LINK( SfxAcceleratorConfigPage, RemoveHdl, Button *, pButton )
{
    // get selected entry
    USHORT    nPos   = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    TAccInfo* pEntry = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();

    // remove function name from selected entry
    USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( String(), nPos, nCol );
    pEntry->m_sCommand = ::rtl::OUString();

    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

//-----------------------------------------------
IMPL_LINK( SfxAcceleratorConfigPage, SelectHdl, Control*, pListBox )
{
    // disable help
    Help::ShowBalloon( this, Point(), String() );
    if ( pListBox == &aEntriesBox )
    {
        USHORT          nPos                = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        TAccInfo*       pEntry              = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
        ::rtl::OUString sPossibleNewCommand = aFunctionBox.GetCurCommand();

        aRemoveButton.Enable( FALSE );
        aChangeButton.Enable( FALSE );

        if (pEntry->m_bIsConfigurable)
        {
            if (pEntry->isConfigured())
                aRemoveButton.Enable( TRUE );
            aChangeButton.Enable( pEntry->m_sCommand != sPossibleNewCommand );
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
        aRemoveButton.Enable( FALSE );
        aChangeButton.Enable( FALSE );

        // #i36994 First selected can return zero!
        SvLBoxEntry*    pLBEntry = aEntriesBox.FirstSelected();
        if ( pLBEntry != 0 )
        {
            USHORT          nPos                = (USHORT) aEntriesBox.GetModel()->GetRelPos( pLBEntry );
            TAccInfo*       pEntry              = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
            ::rtl::OUString sPossibleNewCommand = aFunctionBox.GetCurCommand();

            if (pEntry->m_bIsConfigurable)
            {
                if (pEntry->isConfigured())
                    aRemoveButton.Enable( TRUE );
                aChangeButton.Enable( pEntry->m_sCommand != sPossibleNewCommand );
            }

            // update key box
            aKeyBox.Clear();
            SvLBoxEntry* pIt = aEntriesBox.First();
            while (pIt)
            {
                TAccInfo* pUserData = (TAccInfo*)pIt->GetUserData();
                if (
                    (pUserData                                   ) &&
                    (pUserData->m_sCommand == sPossibleNewCommand)
                )
                {
                    TAccInfo*    pU1 = new TAccInfo(-1, -1, pUserData->m_aKey);
                    SvLBoxEntry* pE1 = aKeyBox.InsertEntry( pUserData->m_aKey.GetName(), 0, LIST_APPEND, 0xFFFF);
                    pE1->SetUserData(pU1);
                    pE1->EnableChildsOnDemand( FALSE );
                }
                pIt = aEntriesBox.Next(pIt);
            }
        }
    }
    else
    {
        // goto selected "key" entry of the key box
        SvLBoxEntry* pE2 = 0;
        TAccInfo*    pU2 = 0;
        USHORT       nP2 = LISTBOX_ENTRY_NOTFOUND;
        SvLBoxEntry* pE3 = 0;

        pE2 = aKeyBox.FirstSelected();
        if (pE2)
            pU2 = (TAccInfo*)pE2->GetUserData();
        if (pU2)
            nP2 = MapKeyCodeToPos(pU2->m_aKey);
        if (nP2 != LISTBOX_ENTRY_NOTFOUND)
            pE3 = aEntriesBox.GetEntry( 0, nP2 );
        if (pE3)
        {
            aEntriesBox.Select( pE3 );
            aEntriesBox.MakeVisible( pE3 );
        }
    }

    return 0;
}

//-----------------------------------------------
IMPL_LINK( SfxAcceleratorConfigPage, RadioHdl, RadioButton *, pBtn )
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xOld = m_xAct;

    if (aOfficeButton.IsChecked())
        m_xAct = m_xGlobal;
    else
    if (aModuleButton.IsChecked())
        m_xAct = m_xModule;

    // nothing changed? => do nothing!
    if (
        (m_xAct.is()   ) &&
        (xOld == m_xAct)
       )
        return 0;

    aEntriesBox.SetUpdateMode( FALSE );
    ResetConfig();
    Init(m_xAct);
    aEntriesBox.SetUpdateMode( TRUE );
    aEntriesBox.Invalidate();

     aGroupLBox.Init(m_xSMGR, m_xFrame, m_sModuleLongName);
    aEntriesBox.Select(aEntriesBox.GetEntry(0, 0));
    aGroupLBox.Select (aGroupLBox.GetEntry (0, 0));

    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 1L;
}

//-----------------------------------------------
String SfxAcceleratorConfigPage::GetFunctionName(KeyFuncType eType) const
{
    ::rtl::OUStringBuffer sName(256);
    sName.appendAscii("\"");
    switch(eType)
    {
        case KEYFUNC_NEW :
                sName.append( String( SfxResId( STR_NEW )));
                break;

        case KEYFUNC_OPEN :
                sName.append( String( SfxResId( STR_OPEN )));
                break;

        case KEYFUNC_SAVE :
                sName.append( String( SfxResId( STR_SAVE )));
                break;

        case KEYFUNC_SAVEAS :
                sName.append( String( SfxResId( STR_SAVEAS )));
                break;

        case KEYFUNC_PRINT :
                sName.append( String( SfxResId( STR_PRINT )));
                break;

        case KEYFUNC_CLOSE :
                sName.append( String( SfxResId( STR_CLOSE)));
                break;

        case KEYFUNC_QUIT :
                sName.append( String( SfxResId( STR_QUIT )));
                break;

        case KEYFUNC_CUT :
                sName.append( String( SfxResId( STR_CUT )));
                break;

        case KEYFUNC_COPY :
                sName.append( String( SfxResId( STR_COPY )));
                break;

        case KEYFUNC_PASTE :
                sName.append( String( SfxResId( STR_PASTE )));
                break;

        case KEYFUNC_UNDO :
                sName.append( String( SfxResId( STR_UNDO )));
                break;

        case KEYFUNC_REDO :
                sName.append( String( SfxResId( STR_REDO )));
                break;

        case KEYFUNC_DELETE :
                sName.append( String( SfxResId( STR_DELETE )));
                break;

        case KEYFUNC_REPEAT :
                sName.append( String( SfxResId( STR_REPEAT )));
                break;

        case KEYFUNC_FIND :
                sName.append( String( SfxResId( STR_FIND )));
                break;

        case KEYFUNC_FINDBACKWARD :
                sName.append( String( SfxResId( STR_FINDBACKWARD )));
                break;

        case KEYFUNC_PROPERTIES :
                sName.append( String( SfxResId( STR_PROPERTIES )));
                break;

        case KEYFUNC_FRONT :
                sName.append( String( SfxResId( STR_FRONT )));
                break;
    }
    sName.appendAscii("\"");
    return String(sName.makeStringAndClear());
}

//-----------------------------------------------
BOOL SfxAcceleratorConfigPage::FillItemSet( SfxItemSet& )
{
    Apply(m_xAct);
    try
    {
        m_xAct->store();
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        { return FALSE; }

    return TRUE;
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Reset( const SfxItemSet& )
{
    // open accelerator configs
    // Note: It initialize some other members too, which are needed here ...
    // e.g. m_sModuleUIName!
    InitAccCfg();

    // change te description of the radio button, which switch to the module
    // dependend accelerator configuration
    String sButtonText = aModuleButton.GetText();
    sButtonText.SearchAndReplace(String::CreateFromAscii("$(MODULE)"), m_sModuleUIName);
    aModuleButton.SetText(sButtonText);

    if (m_xModule.is())
        aModuleButton.Check();
    else
    {
        aModuleButton.Hide();
        aOfficeButton.Check();
    }

    RadioHdl(0);

    /* TODO ???
    if ( m_pMacroInfoItem )
        aGroupLBox.SelectMacro( m_pMacroInfoItem );
    */
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::SelectMacro(const SfxMacroInfoItem *pItem)
{
    m_pMacroInfoItem = pItem;
    aGroupLBox.SelectMacro( pItem );
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::CopySource2Target(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xSourceAccMgr,
                                                 const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xTargetAccMgr)
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

//-----------------------------------------------
KeyCode SfxAcceleratorConfigPage::MapPosToKeyCode(USHORT nPos) const
{
    TAccInfo* pEntry = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
    KeyCode aCode(KEYCODE_ARRAY[pEntry->m_nKeyPos] & 0xFFF                 ,
                  KEYCODE_ARRAY[pEntry->m_nKeyPos] & (KEY_SHIFT | KEY_MOD1));
    return aCode;
}

//-----------------------------------------------
USHORT SfxAcceleratorConfigPage::MapKeyCodeToPos(const KeyCode& aKey) const
{
    USHORT       nCode1 = aKey.GetCode()+aKey.GetModifier();
    SvLBoxEntry* pEntry = aEntriesBox.First();
    USHORT       i      = 0;

    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
        {
            USHORT nCode2 = pUserData->m_aKey.GetCode()+pUserData->m_aKey.GetModifier();
            if (nCode1 == nCode2)
                return i;
        }
        pEntry = aEntriesBox.Next(pEntry);
        ++i;
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

//-----------------------------------------------
String SfxAcceleratorConfigPage::GetLabel4Command(const String& sCommand)
{
    try
    {
        // check global command configuration first
        css::uno::Reference< css::container::XNameAccess > xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            String sLabel = String(lProps.getUnpackedValueOrDefault(CMDPROP_UINAME, ::rtl::OUString()));
            if (sLabel.Len())
                return sLabel;
        }
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}

    // may be it's a style URL .. they must be handled special
    SfxStyleInfo_Impl aStyle;
    aStyle.sCommand = sCommand;
    if (m_aStylesInfo.parseStyleCommand(aStyle))
    {
        m_aStylesInfo.getLabel4Style(aStyle);
        return aStyle.sLabel;
    }

    return sCommand;
}

//-----------------------------------------------
css::uno::Reference< css::frame::XModel > SfxAcceleratorConfigPage::SearchForAlreadyLoadedDoc(const String& sName)
{
    return css::uno::Reference< css::frame::XModel >();
}

