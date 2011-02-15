/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXVIEWSH_HXX
#define _SFXVIEWSH_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <svl/lstner.hxx>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include "shell.hxx"
#include <tools/gen.hxx>
#include <tools/errcode.hxx>
#include <vcl/jobset.hxx>
class SfxBaseController;
class Size;
class Fraction;
class Window;
class KeyEvent;
class WorkWindow;
class SvBorder;
class SdrView;
class SfxFrame;
class SfxMedium;
class SfxModule;
class SfxViewFrame;
class SfxItemPool;
class SfxTabPage;
class SfxPrintMonitor;
class SfxFrameSetDescriptor;
class Printer;
class SfxPrinter;
class SfxProgress;
class SvStringsDtor;
class SfxFrameItem;
class Dialog;
class Menu;
class NotifyEvent;

#define SFX_PRINTER_PRINTER          1  // ohne JOBSETUP => temporaer
#define SFX_PRINTER_JOBSETUP         2
#define SFX_PRINTER_OPTIONS          4
#define SFX_PRINTER_CHG_ORIENTATION  8
#define SFX_PRINTER_CHG_SIZE        16
#define SFX_PRINTER_ALL             31

#define SFX_PRINTER_CHG_ORIENTATION_FLAG  3
#define SFX_PRINTER_CHG_SIZE_FLAG         4

#define SFX_PRINTERROR_NONE          0
#define SFX_PRINTERROR_BUSY          1

enum SfxScrollingMode
{
    SCROLLING_NO,
    SCROLLING_YES,
    SCROLLING_AUTO,
    SCROLLING_DEFAULT
};

// "Geeicht" mit www.apple.com und Netscape 3.01
#define DEFAULT_MARGIN_WIDTH 8
#define DEFAULT_MARGIN_HEIGHT 12

//========================================================================

//  @[SfxViewShell-Flags]

#define SFX_VIEW_HAS_PRINTOPTIONS    0x0010 /*  Options-Button und Options-
                                                Dialog im PrintDialog */
#define SFX_VIEW_CAN_PRINT           0x0020 /*  enabled Printing ohne Printer
                                                erzeugen zu m"ussen */
#define SFX_VIEW_NO_SHOW             0x0040 /*  Window der ViewShell darf nicht
                                                automatisch geshowed werden */
#define SFX_VIEW_NO_NEWWINDOW       0x0100      /* keine weitere View erlauben */

/*  [Beschreibung]

    Die SfxViewShell-Flags steuern das Verhalten der SfxViewShell f"ur die
    Dauer ihrer Lebenszeit. Sie werden im Konstruktor der <SfxViewShell>
    angegeben.
*/

//=========================================================================

#define SFX_DECL_VIEWFACTORY(Class) \
private: \
    static SfxViewFactory *pFactory; \
public: \
    static SfxViewShell  *CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldView); \
    static void           RegisterFactory( sal_uInt16 nPrio = USHRT_MAX ); \
    static SfxViewFactory&Factory() { return *pFactory; } \
    static void           InitFactory()

#define SFX_IMPL_NAMED_VIEWFACTORY(Class, AsciiViewName) \
    SfxViewFactory* Class::pFactory; \
    SfxViewShell* __EXPORT Class::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldView) \
    { return new Class(pFrame, pOldView); } \
    void Class::RegisterFactory( sal_uInt16 nPrio ) \
    { \
        pFactory = new SfxViewFactory(&CreateInstance,&InitFactory,nPrio,AsciiViewName);\
        InitFactory(); \
    } \
    void Class::InitFactory()

#define SFX_VIEW_REGISTRATION(DocClass) \
            DocClass::Factory().RegisterViewFactory( Factory() )

#define SFX_REGISTER_VIEW(ViewShellClass) \
            Factory().RegisterViewFactory( ViewShellClass::Factory() )

class SfxInPlaceClient;
DECLARE_LIST( SfxInPlaceClientList, SfxInPlaceClient* )

// -----------------------------------------------------------------------
class SFX2_DLLPUBLIC SfxViewShell: public SfxShell, public SfxListener
{
#ifdef _SFXVIEWSH_HXX
friend class SfxViewFrame;
friend class SfxPlugInFrame;
friend class SfxBaseController;
friend class SfxPrinterController;
#endif

    struct SfxViewShell_Impl*   pImp;
    SfxInPlaceClientList*       pIPClientList;
    SfxViewFrame*               pFrame;
    SfxShell*                   pSubShell;
    Window*                     pWindow;
    sal_Bool                        bNoNewWindow;

protected:
    virtual void                Activate(sal_Bool IsMDIActivate);
    virtual void                Deactivate(sal_Bool IsMDIActivate);

    virtual Size                GetOptimalSizePixel() const;

    virtual void                InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void                OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void                SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void                Move();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
    // Iteration
    static SfxViewShell*        GetFirst( const TypeId* pType = 0, sal_Bool bOnlyVisible = sal_True );
    static SfxViewShell*        GetNext( const SfxViewShell& rPrev,
                                         const TypeId* pType = 0, sal_Bool bOnlyVisible = sal_True );
    static SfxViewShell*        Current();

    static SfxViewShell*        Get( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>& i_rController );

    // Ctoren/Dtoren Initialisierung
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWSH)

                                SfxViewShell( SfxViewFrame *pFrame, sal_uInt16 nFlags = 0 );
    virtual                     ~SfxViewShell();

    SfxInPlaceClient*           GetIPClient() const;
    SfxInPlaceClient*           GetUIActiveClient() const;
    SfxInPlaceClient*           FindIPClient( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&  xObj, Window *pObjParentWin ) const;

    virtual ErrCode             DoVerb(long nVerb);

    virtual void                OutplaceActivated( sal_Bool bActive, SfxInPlaceClient* pClient );
    virtual void                InplaceActivating( SfxInPlaceClient* pClient );
    virtual void                InplaceDeactivated( SfxInPlaceClient* pClient );
    virtual void                UIActivating( SfxInPlaceClient* pClient );
    virtual void                UIDeactivated( SfxInPlaceClient* pClient );

    virtual void                JumpToMark( const String& rMark );
    void                        VisAreaChanged(const Rectangle& rRect);

    // Verhaltens-Flags
    SfxScrollingMode            GetScrollingMode() const;
    void                        SetScrollingMode( SfxScrollingMode eMode );

    // Misc
    virtual sal_uInt16              PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );
    virtual String              GetSelectionText( sal_Bool bCompleteWords = sal_False );
    virtual sal_Bool                HasSelection( sal_Bool bText = sal_True ) const;
    virtual SdrView*            GetDrawView() const;

    void                        SetSubShell( SfxShell *pShell );
    SfxShell*                   GetSubShell() const { return pSubShell; }
    void                        AddSubShell( SfxShell& rShell );
    void                        RemoveSubShell( SfxShell *pShell=NULL );
    SfxShell*                   GetSubShell( sal_uInt16 );

    // Focus, KeyInput, Cursor
    void                        GotFocus() const;
    inline void                 LostFocus() const;
    virtual void                ShowCursor( FASTBOOL bOn = sal_True );
    virtual FASTBOOL            KeyInput( const KeyEvent &rKeyEvent );
    sal_Bool                        Escape();

    // Viewing Interface
    Window*                     GetWindow() const { return pWindow; }
    void                        SetWindow( Window *pViewPort );
    virtual void                AdjustPosSizePixel( const Point &rOfs, const Size &rSize );
    const SvBorder&             GetBorderPixel() const;
    void                        SetBorderPixel( const SvBorder &rBorder );
    void                        InvalidateBorder();
    inline SfxViewFrame*        GetViewFrame() const;
    void                        AdjustVisArea(const Rectangle& rRect);

    // Printing Interface
    virtual SfxPrinter*         GetPrinter( sal_Bool bCreate = sal_False );
    virtual sal_uInt16              SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=sal_False );
    virtual SfxTabPage*         CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );
    void                        LockPrinter( sal_Bool bLock = sal_True );
    sal_Bool                    IsPrinterLocked() const;
    virtual JobSetup            GetJobSetup() const;
    Printer*                    GetActivePrinter() const;

    // Workingset
    virtual void                WriteUserData( String&, sal_Bool bBrowse = sal_False );
    virtual void                ReadUserData( const String&, sal_Bool bBrowse = sal_False );
    virtual void                WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void                ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void                QueryObjAreaPixel( Rectangle& rRect ) const;

    virtual SfxObjectShell*     GetObjectShell();
    /** retrieves the document which shall be considered the "current document" when the frame is active

        The default implementation simply returns the XModel of the associated SfxObjectShell. You will rarely
        need to overwrite this behavior.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                                GetCurrentDocument() const;
    /** forwards the current document, as returned by ->GetCurrentDocument, to SfxObjectShell::SetWorkingDocument
    */
    void                        SetCurrentDocument() const;

    /** get an XRenderable instance that can render this docuement
    */
    virtual com::sun::star::uno::Reference< com::sun::star::view::XRenderable > GetRenderable();


    virtual void                MarginChanged();
    const Size&                 GetMargin() const;
    void                        SetMargin( const Size& );
    void                        DisconnectAllClients();
    virtual SfxFrame*           GetSmartSelf( SfxFrame* pSelf, SfxMedium& rMedium );
    sal_Bool                        NewWindowAllowed() const            { return !bNoNewWindow; }
    void                        SetNewWindowAllowed( sal_Bool bSet )    { bNoNewWindow = !bSet; }

    void                        SetController( SfxBaseController* pController );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    ::cppu::OInterfaceContainerHelper& GetContextMenuInterceptors() const;
    sal_Bool                        TryContextMenuInterception( Menu& rIn, const ::rtl::OUString& rMenuIdentifier, Menu*& rpOut, ::com::sun::star::ui::ContextMenuExecuteEvent aEvent );

    void                        SetAdditionalPrintOptions( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );
    void                        ExecPrint( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >&, sal_Bool, sal_Bool );

    void                        AddRemoveClipboardListener( const com::sun::star::uno::Reference < com::sun::star::datatransfer::clipboard::XClipboardListener>&, sal_Bool );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardNotifier > GetClipboardNotifier();

#if _SOLAR__PRIVATE
    SAL_DLLPRIVATE SfxInPlaceClient* GetUIActiveIPClient_Impl() const;
    SAL_DLLPRIVATE void AddContextMenuInterceptor_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE void RemoveContextMenuInterceptor_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE bool GlobalKeyInput_Impl( const KeyEvent &rKeyEvent );

    SAL_DLLPRIVATE void NewIPClient_Impl( SfxInPlaceClient *pIPClient )
                                { GetIPClientList_Impl(sal_True)->Insert(pIPClient); }
    SAL_DLLPRIVATE void IPClientGone_Impl( SfxInPlaceClient *pIPClient )
                                { GetIPClientList_Impl(sal_True)->Remove(pIPClient); }
    SAL_DLLPRIVATE SfxInPlaceClientList* GetIPClientList_Impl( sal_Bool bCreate = sal_True ) const;
    SAL_DLLPRIVATE void ResetAllClients_Impl( SfxInPlaceClient *pIP );
    SAL_DLLPRIVATE void DiscardClients_Impl();
    SAL_DLLPRIVATE sal_Bool PlugInsActive() const;

    SAL_DLLPRIVATE SfxPrinter* SetPrinter_Impl( SfxPrinter *pNewPrinter );
    SAL_DLLPRIVATE sal_Bool IsShowView_Impl() const;

    SAL_DLLPRIVATE long HandleNotifyEvent_Impl( NotifyEvent& rEvent );
    SAL_DLLPRIVATE sal_Bool HasKeyListeners_Impl();
    SAL_DLLPRIVATE sal_Bool HasMouseClickListeners_Impl();

    SAL_DLLPRIVATE SfxBaseController*   GetBaseController_Impl() const;

    // Shell Interface
    SAL_DLLPRIVATE void ExecPrint_Impl(SfxRequest &);
    SAL_DLLPRIVATE void ExecMisc_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet&);
    SAL_DLLPRIVATE SfxFrameSetDescriptor* GetFrameSet_Impl() const;
    SAL_DLLPRIVATE void SetFrameSet_Impl(SfxFrameSetDescriptor*);
    SAL_DLLPRIVATE void CheckIPClient_Impl( SfxInPlaceClient*, const Rectangle& );
    SAL_DLLPRIVATE void PushSubShells_Impl( sal_Bool bPush=sal_True );
    SAL_DLLPRIVATE void PopSubShells_Impl() { PushSubShells_Impl( sal_False ); }
    SAL_DLLPRIVATE void TakeOwnerShip_Impl();
    SAL_DLLPRIVATE void CheckOwnerShip_Impl();
    SAL_DLLPRIVATE void TakeFrameOwnerShip_Impl();
    SAL_DLLPRIVATE sal_Bool ExecKey_Impl(const KeyEvent& aKey);
#endif
};

//========================================================================

inline void SfxViewShell::LostFocus() const

/*  [Beschreibung]

    Diese Methode mu\s vom Applikationsentwickler gerufen werden, wenn
    das Edit-Window den Focus verloren hat. Der SFx hat so z.B. die
    M"oglichkeit, den Accelerator auszuschalten, damit in bestimmten
    Floating-Windows die Cursor-Tasten, die Delete-Taste etc. funktionieren,
    obwohl sie "uber den Accelerator umdefiniert sind.


    [Anmerkung]

    <StarView> liefert leider keine M"oglichkeit, solche Events
    'von der Seite' einzuh"angen.
*/

{
}

//------------------------------------------------------------------------

inline SfxViewFrame* SfxViewShell::GetViewFrame() const

/*  [Bechreibung]

    Diese Methode liefert einen Pointer auf die <SfxViewFrame>-Instanz,
    in der diese SfxViewShell dargestellt wird. Dieses ist die Instanz,
    die imKonstruktor durchgereicht wurde. Es ist gew"ahrleistet, da\s
    der zur"uckgegebene Pointer auf eine g"ultige SfxViewFrame-Instanz
    zeigt.


    [Querverweise]

    <SfxShell::GetFrame()const>
*/

{
    return pFrame;
}

#endif // #ifndef _SFXVIEWSH_HXX

