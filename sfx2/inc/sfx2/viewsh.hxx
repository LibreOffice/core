/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsh.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:46:35 $
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
#ifndef _SFXVIEWSH_HXX
#define _SFXVIEWSH_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XCONTEXTMENUINTERCEPTOR_HPP_
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#include "shell.hxx"
#include <tools/gen.hxx>
#include <tools/errcode.hxx>
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
class PrintDialog;
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

#define SFX_VIEW_MAXIMIZE_FIRST      0x0001 /*  die erste View wird maximiert
                                                dargestellt */
#define SFX_VIEW_OPTIMIZE_EACH       0x0002 /*  jede View wird in optimaler
                                                Gr"o\se dargestellt */
#define SFX_VIEW_DISABLE_ACCELS      0x0004 /*  die Acceleratoren werden
                                                disabled, solange diese
                                                View den Focus hat */
#define SFX_VIEW_OBJECTSIZE_EMBEDDED 0x0008 /*  Views von embedded Objekten
                                                werden in optimaler Gr"o\se
                                                dargestellt */
#define SFX_VIEW_HAS_PRINTOPTIONS    0x0010 /*  Options-Button und Options-
                                                Dialog im PrintDialog */
#define SFX_VIEW_CAN_PRINT           0x0020 /*  enabled Printing ohne Printer
                                                erzeugen zu m"ussen */
#define SFX_VIEW_NO_SHOW             0x0040 /*  Window der ViewShell darf nicht
                                                automatisch geshowed werden */
#define SFX_VIEW_IMPLEMENTED_AS_FRAMESET 0x0080 /*  Das Dokument ist als
                                                    Frameset implementiert*/
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
    static void           RegisterFactory( USHORT nPrio = USHRT_MAX ); \
    static SfxViewFactory&Factory() { return *pFactory; } \
    static void           InitFactory()

#define SFX_IMPL_VIEWFACTORY(Class, rResId) \
    SfxViewFactory* Class::pFactory; \
    SfxViewShell* __EXPORT Class::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldView) \
    { return new Class(pFrame, pOldView); } \
    void Class::RegisterFactory( USHORT nPrio ) \
    { \
        pFactory = new SfxViewFactory(&CreateInstance,&InitFactory,nPrio,rResId);\
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
friend class SfxTopViewFrame;
friend class SfxPlugInFrame;
friend class SfxInternalFrame;
friend class SfxExternalTopViewFrame_Impl;
friend class SfxOfficeDocController;
friend class SfxBaseController;
#endif

    struct SfxViewShell_Impl*   pImp;
    SfxInPlaceClientList*       pIPClientList;
    SfxViewFrame*               pFrame;
    SfxShell*                   pSubShell;
    Window*                     pWindow;
    BOOL                        bMaximizeFirst;
    BOOL                        bOptimizeEach;
    BOOL                        bNoNewWindow;

protected:
    virtual void                Activate(BOOL IsMDIActivate);
    virtual void                Deactivate(BOOL IsMDIActivate);

    virtual Size                GetOptimalSizePixel() const;

    virtual void                InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void                OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void                SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void                Move();

    virtual void                SFX_NOTIFY( SfxBroadcaster& rBC,
                                        const TypeId& rBCType,
                                        const SfxHint& rHint,
                                        const TypeId& rHintType );
public:
    // Iteration
    static SfxViewShell*        GetFirst( const TypeId* pType = 0, BOOL bOnlyVisible = TRUE );
    static SfxViewShell*        GetNext( const SfxViewShell& rPrev,
                                         const TypeId* pType = 0, BOOL bOnlyVisible = TRUE );
    static SfxViewShell*        Current();

    // Ctoren/Dtoren Initialisierung
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWSH)

                                SfxViewShell( SfxViewFrame *pFrame, USHORT nFlags = 0 );
    virtual                     ~SfxViewShell();

    // In-Place
    // should be superfluous
    //virtual SfxInPlaceClient* CreateIPClient( WorkWindow * pTop, WorkWindow * pDoc, Window * pDraw );

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
    BOOL                        IsMaximizeFirst() const { return bMaximizeFirst; }
    BOOL                        IsOptimizeEach() const { return bOptimizeEach; }
    HACK(inline) BOOL           UseObjectSize() const;
    SfxScrollingMode            GetScrollingMode() const;
    void                        SetScrollingMode( SfxScrollingMode eMode );

    // Misc
    virtual USHORT              PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );
    virtual String              GetSelectionText( BOOL bCompleteWords = FALSE );
    virtual BOOL                HasSelection( BOOL bText = TRUE ) const;
    virtual SdrView*            GetDrawView() const;
    void                        SetSubShell( SfxShell *pShell );
    SfxShell*                   GetSubShell() const { return pSubShell; }
    void                        AddSubShell( SfxShell& rShell );
    void                        RemoveSubShell( SfxShell *pShell=NULL );
    SfxShell*                   GetSubShell( USHORT );

    // Focus, KeyInput, Cursor
    void                        GotFocus() const;
    inline void                 LostFocus() const;
    virtual void                ShowCursor( FASTBOOL bOn = TRUE );
    virtual FASTBOOL            KeyInput( const KeyEvent &rKeyEvent );
    BOOL                        Escape();

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
    virtual void                PreparePrint( PrintDialog *pPrintDialog = 0 );
    virtual ErrCode             DoPrint( SfxPrinter *pPrinter, PrintDialog *pPrintDialog, BOOL bSilent, BOOL bIsAPI );
    virtual USHORT              Print( SfxProgress &rProgress, BOOL bIsAPI, PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*         GetPrinter( BOOL bCreate = FALSE );
    virtual USHORT              SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=FALSE );
    virtual SfxTabPage*         CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );
    virtual PrintDialog*        CreatePrintDialog( Window *pParent );
    void                        LockPrinter( BOOL bLock = TRUE );
    BOOL                        IsPrinterLocked() const;

    // Workingset
    virtual void                WriteUserData( String&, BOOL bBrowse = FALSE );
    virtual void                ReadUserData( const String&, BOOL bBrowse = FALSE );
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

    virtual void                MarginChanged();
    const Size&                 GetMargin() const;
    void                        SetMargin( const Size& );
    void                        DisconnectAllClients();
    virtual SfxFrame*           GetSmartSelf( SfxFrame* pSelf, SfxMedium& rMedium );
    BOOL                        NewWindowAllowed() const            { return !bNoNewWindow; }
    void                        SetNewWindowAllowed( BOOL bSet )    { bNoNewWindow = !bSet; }

    void                        SetController( SfxBaseController* pController );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    ::cppu::OInterfaceContainerHelper& GetContextMenuInterceptors() const;
    BOOL                        TryContextMenuInterception( Menu& rIn, Menu*& rpOut, ::com::sun::star::ui::ContextMenuExecuteEvent aEvent );

    void                        SetAdditionalPrintOptions( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );

#if _SOLAR__PRIVATE
    SAL_DLLPRIVATE SfxInPlaceClient* GetUIActiveIPClient_Impl() const;
    SAL_DLLPRIVATE void AddContextMenuInterceptor_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE void RemoveContextMenuInterceptor_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE FASTBOOL GlobalKeyInput_Impl( const KeyEvent &rKeyEvent );
    SAL_DLLPRIVATE BOOL IsImplementedAsFrameset_Impl() const;

    SAL_DLLPRIVATE void NewIPClient_Impl( SfxInPlaceClient *pIPClient )
                                { GetIPClientList_Impl(TRUE)->Insert(pIPClient); }
    SAL_DLLPRIVATE void IPClientGone_Impl( SfxInPlaceClient *pIPClient )
                                { GetIPClientList_Impl(TRUE)->Remove(pIPClient); }
    SAL_DLLPRIVATE SfxInPlaceClientList* GetIPClientList_Impl( BOOL bCreate = TRUE ) const;
    SAL_DLLPRIVATE void ResetAllClients_Impl( SfxInPlaceClient *pIP );
    SAL_DLLPRIVATE void DiscardClients_Impl();
    SAL_DLLPRIVATE BOOL PlugInsActive() const;

    SAL_DLLPRIVATE SfxPrinter* SetPrinter_Impl( SfxPrinter *pNewPrinter );
    SAL_DLLPRIVATE BOOL IsShowView_Impl() const;

    SAL_DLLPRIVATE long HandleNotifyEvent_Impl( NotifyEvent& rEvent );
    SAL_DLLPRIVATE BOOL HasKeyListeners_Impl();
    SAL_DLLPRIVATE BOOL HasMouseClickListeners_Impl();

    // Shell Interface
    SAL_DLLPRIVATE void ExecPrint_Impl(SfxRequest &);
    SAL_DLLPRIVATE void ExecMisc_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet&);
    SAL_DLLPRIVATE SfxFrameSetDescriptor* GetFrameSet_Impl() const;
    SAL_DLLPRIVATE void SetFrameSet_Impl(SfxFrameSetDescriptor*);
    SAL_DLLPRIVATE void CheckIPClient_Impl( SfxInPlaceClient*, const Rectangle& );
    SAL_DLLPRIVATE void PushSubShells_Impl( BOOL bPush=TRUE );
    SAL_DLLPRIVATE void TakeOwnerShip_Impl();
    SAL_DLLPRIVATE void CheckOwnerShip_Impl();
    SAL_DLLPRIVATE void TakeFrameOwnerShip_Impl();
    SAL_DLLPRIVATE BOOL ExecKey_Impl(const KeyEvent& aKey);

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


