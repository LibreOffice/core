/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_VIEWSH_HXX
#define INCLUDED_SFX2_VIEWSH_HXX

#include <sal/config.h>
#include <memory>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <svl/lstner.hxx>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <tools/gen.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <vcl/errcode.hxx>
#include <vcl/jobset.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/tabdlg.hxx>
#include <LibreOfficeKit/LibreOfficeKitTypes.h>
#include <editeng/outliner.hxx>
#include <functional>

class SfxBaseController;
class Size;
class Fraction;
namespace vcl { class Window; }
namespace weld { class Window; }
class KeyEvent;
class WorkWindow;
class SvBorder;
class SdrView;
class SfxFrame;
class SfxMedium;
class SfxModule;
class SfxViewFrame;
class SfxItemPool;
class SfxFrameSetDescriptor;
class Printer;
class SfxPrinter;
class SfxProgress;
class SfxFrameItem;
class Menu;
class NotifyEvent;
class SfxInPlaceClient;
namespace vcl { class PrinterController; }


enum class SfxPrinterChangeFlags
{
    NONE             = 0,
    PRINTER          = 1,  // without JOB SETUP => Temporary
    JOBSETUP         = 2,
    OPTIONS          = 4,
    CHG_ORIENTATION  = 8,
    CHG_SIZE         = 16
};
namespace o3tl
{
    template<> struct typed_flags<SfxPrinterChangeFlags> : is_typed_flags<SfxPrinterChangeFlags, 31> {};
}
#define SFX_PRINTER_ALL             (SfxPrinterChangeFlags::PRINTER | SfxPrinterChangeFlags::JOBSETUP | SfxPrinterChangeFlags::OPTIONS | SfxPrinterChangeFlags::CHG_ORIENTATION | SfxPrinterChangeFlags::CHG_SIZE)

#define SFX_PRINTERROR_BUSY          1

// "Verified" using www.apple.com and Netscape 3.01
#define DEFAULT_MARGIN_WIDTH 8
#define DEFAULT_MARGIN_HEIGHT 12


//  @[SfxViewShell-Flags]

enum class SfxViewShellFlags
{
    NONE              = 0x0000,
    HAS_PRINTOPTIONS  = 0x0010, /* Options-Button and Options-Dialog in PrintDialog */
    NO_NEWWINDOW      = 0x0100, /* Allow N View */
};
namespace o3tl
{
    template<> struct typed_flags<SfxViewShellFlags> : is_typed_flags<SfxViewShellFlags, 0x0110> {};
}

/*  [Description]

    The SfxViewShell flags control the behavior of SfxViewShell for the
    duration of its lifetime. They are defined in the constructor of
    <SfxViewShell>.
*/


class SfxViewFactory;
#define SFX_DECL_VIEWFACTORY(Class) \
private: \
    static SfxViewFactory *pFactory; \
public: \
    static SfxViewShell  *CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldView); \
    static void           RegisterFactory( SfxInterfaceId nPrio ); \
    static SfxViewFactory*Factory() { return pFactory; } \
    static void           InitFactory()

#define SFX_IMPL_NAMED_VIEWFACTORY(Class, AsciiViewName) \
    SfxViewFactory* Class::pFactory; \
    SfxViewShell* Class::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldView) \
    { return new Class(pFrame, pOldView); } \
    void Class::RegisterFactory( SfxInterfaceId nPrio ) \
    { \
        pFactory = new SfxViewFactory(&CreateInstance,nPrio,AsciiViewName);\
        InitFactory(); \
    } \
    void Class::InitFactory()

#define SFX_VIEW_REGISTRATION(DocClass) \
            DocClass::Factory().RegisterViewFactory( *Factory() )

template<class T> bool checkSfxViewShell(const SfxViewShell* pShell)
{
    return dynamic_cast<const T*>(pShell) != nullptr;
}

class SFX2_DLLPUBLIC SfxViewShell: public SfxShell, public SfxListener, public OutlinerViewShell, public vcl::ILibreOfficeKitNotifier
{
friend class SfxViewFrame;
friend class SfxBaseController;
friend class SfxPrinterController;

    std::unique_ptr<struct SfxViewShell_Impl>   pImpl;
    SfxViewFrame*               pFrame;
    VclPtr<vcl::Window>         pWindow;
    bool                        bNoNewWindow;
    bool                        mbPrinterSettingsModified;
    LanguageTag                 maLOKLanguageTag;

protected:
    virtual void                Activate(bool IsMDIActivate) override;
    virtual void                Deactivate(bool IsMDIActivate) override;

    virtual void                InnerResizePixel( const Point &rOfs, const Size &rSize, bool inplaceEditModeChange );
    virtual void                OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void                SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void                Move();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

public:
    // Iteration
    static size_t               GetActiveShells( bool bOnlyVisible = true );
    static SfxViewShell*        GetFirst( bool bOnlyVisible = true, const std::function<bool ( const SfxViewShell* )>& isViewShell = nullptr );
    static SfxViewShell*        GetNext( const SfxViewShell& rPrev,
                                         bool bOnlyVisible = true,
                                         const std::function<bool ( const SfxViewShell* )>& isViewShell = nullptr );
    static SfxViewShell*        Current();

    static SfxViewShell*        Get( const css::uno::Reference< css::frame::XController>& i_rController );

    // Initialize Constructors/Destructors
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWSH)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:

                                SfxViewShell( SfxViewFrame *pFrame, SfxViewShellFlags nFlags );
    virtual                     ~SfxViewShell() override;

    SfxInPlaceClient*           GetIPClient() const;
    SfxInPlaceClient*           GetUIActiveClient() const;
    SfxInPlaceClient*           FindIPClient( const css::uno::Reference < css::embed::XEmbeddedObject >&  xObj, vcl::Window *pObjParentWin ) const;

    virtual ErrCode             DoVerb(long nVerb);

    void                        OutplaceActivated( bool bActive );
    virtual void                UIActivating( SfxInPlaceClient* pClient );
    virtual void                UIDeactivated( SfxInPlaceClient* pClient );

    void                        JumpToMark( const OUString& rMark );
    void                        VisAreaChanged();

    // Misc

    /**
     * Initialize is called after the frame has been loaded and the controller
     * has been set.  By the time this is called the document has been fully
     * imported.
     */
    virtual bool                PrepareClose( bool bUI = true );
    virtual OUString            GetSelectionText( bool bCompleteWords = false );
    virtual bool                HasSelection( bool bText = true ) const;
    virtual SdrView*            GetDrawView() const;

    void                        AddSubShell( SfxShell& rShell );
    void                        RemoveSubShell( SfxShell *pShell=nullptr );
    SfxShell*                   GetSubShell( sal_uInt16 );

    virtual       SfxShell*     GetFormShell()       { return nullptr; };
    virtual const SfxShell*     GetFormShell() const { return nullptr; };

    // ILibreOfficeKitNotifier
    virtual void                notifyWindow(vcl::LOKWindowId nLOKWindowId, const OUString& rAction, const std::vector<vcl::LOKPayloadItem>& rPayload = std::vector<vcl::LOKPayloadItem>()) const override;

    // Focus, KeyInput, Cursor
    virtual void                ShowCursor( bool bOn = true );
    virtual bool                KeyInput( const KeyEvent &rKeyEvent );
    bool                        Escape();

    // Viewing Interface
    vcl::Window*                GetWindow() const { return pWindow; }
    weld::Window*               GetFrameWeld() const;
    void                        SetWindow( vcl::Window *pViewPort );
    const SvBorder&             GetBorderPixel() const;
    void                        SetBorderPixel( const SvBorder &rBorder );
    void                        InvalidateBorder();
    inline SfxViewFrame*        GetViewFrame() const;

    // Printing Interface
    virtual SfxPrinter*         GetPrinter( bool bCreate = false );
    virtual sal_uInt16          SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL );
    virtual bool                HasPrintOptionsPage() const;
    virtual VclPtr<SfxTabPage>  CreatePrintOptionsPage(TabPageParent pParent, const SfxItemSet &rOptions);
    Printer*                    GetActivePrinter() const;

    // Working set
    virtual void                WriteUserData( OUString&, bool bBrowse = false );
    virtual void                ReadUserData( const OUString&, bool bBrowse = false );
    virtual void                WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& );
    virtual void                ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& );
    virtual void                QueryObjAreaPixel( tools::Rectangle& rRect ) const;

    virtual SfxObjectShell*     GetObjectShell() override;

    /** retrieves the document which shall be considered the "current document" when the frame is active

        The default implementation simply returns the XModel of the associated SfxObjectShell. You will rarely
        need to overwrite this behavior.
    */
    virtual css::uno::Reference< css::frame::XModel >
                                GetCurrentDocument() const;

    /** forwards the current document, as returned by ->GetCurrentDocument, to SfxObjectShell::SetWorkingDocument
    */
    void                        SetCurrentDocument() const;

    /** get an XRenderable instance that can render this document
    */
    virtual css::uno::Reference< css::view::XRenderable > GetRenderable();


    virtual void                MarginChanged();
    const Size&                 GetMargin() const;
    void                        SetMargin( const Size& );
    void                        DisconnectAllClients();
    bool                        NewWindowAllowed() const            { return !bNoNewWindow; }
    void                        SetNewWindowAllowed( bool bSet )    { bNoNewWindow = !bSet; }

    void                        SetController( SfxBaseController* pController );
    css::uno::Reference< css::frame::XController >
                                GetController();

    bool                        TryContextMenuInterception( Menu& rIn, const OUString& rMenuIdentifier, VclPtr<Menu>& rpOut, css::ui::ContextMenuExecuteEvent aEvent );
    bool                        TryContextMenuInterception( Menu& rMenu, const OUString& rMenuIdentifier, css::ui::ContextMenuExecuteEvent aEvent );

    void                        ExecPrint( const css::uno::Sequence < css::beans::PropertyValue >&, bool, bool );
    // Like ExecPrint(), but only sets up for printing. Use Printer::ExecutePrintJob() and Printer::FinishPrintJob() afterwards.
    void                        StartPrint( const css::uno::Sequence < css::beans::PropertyValue >&, bool, bool );
    const std::shared_ptr< vcl::PrinterController >& GetPrinterController() const;

    void                        AddRemoveClipboardListener( const css::uno::Reference < css::datatransfer::clipboard::XClipboardListener>&, bool );
    css::uno::Reference< css::datatransfer::clipboard::XClipboardNotifier > GetClipboardNotifier();

    SAL_DLLPRIVATE SfxInPlaceClient* GetUIActiveIPClient_Impl() const;
    SAL_DLLPRIVATE void AddContextMenuInterceptor_Impl( const css::uno::Reference < css::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE void RemoveContextMenuInterceptor_Impl( const css::uno::Reference < css::ui::XContextMenuInterceptor >& xInterceptor );
    SAL_DLLPRIVATE bool GlobalKeyInput_Impl( const KeyEvent &rKeyEvent );

    SAL_DLLPRIVATE void NewIPClient_Impl( SfxInPlaceClient *pIPClient );
    SAL_DLLPRIVATE void IPClientGone_Impl( SfxInPlaceClient const *pIPClient );
    SAL_DLLPRIVATE void ResetAllClients_Impl( SfxInPlaceClient const *pIP );

    SAL_DLLPRIVATE void SetPrinter_Impl( VclPtr<SfxPrinter>& pNewPrinter );

    SAL_DLLPRIVATE bool HandleNotifyEvent_Impl( NotifyEvent const & rEvent );
    SAL_DLLPRIVATE bool HasKeyListeners_Impl();
    SAL_DLLPRIVATE bool HasMouseClickListeners_Impl();

    SAL_DLLPRIVATE SfxBaseController*   GetBaseController_Impl() const;

    // Shell Interface
    SAL_DLLPRIVATE void ExecPrint_Impl(SfxRequest &);
    SAL_DLLPRIVATE void ExecMisc_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet&);
    SAL_DLLPRIVATE void CheckIPClient_Impl(SfxInPlaceClient const *, const tools::Rectangle&);
    SAL_DLLPRIVATE void PushSubShells_Impl( bool bPush=true );
    SAL_DLLPRIVATE void PopSubShells_Impl() { PushSubShells_Impl( false ); }
    SAL_DLLPRIVATE bool ExecKey_Impl(const KeyEvent& aKey);

    /// The actual per-view implementation of lok::Document::registerCallback().
    void registerLibreOfficeKitViewCallback(LibreOfficeKitCallback pCallback, void* pLibreOfficeKitData);
    /// Invokes the registered callback, if there are any.
    void libreOfficeKitViewCallback(int nType, const char* pPayload) const override;
    /// Set if we are doing tiled searching.
    void setTiledSearching(bool bTiledSearching);
    /// See lok::Document::getPart().
    virtual int getPart() const;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
    /// See OutlinerViewShell::GetViewShellId().
    ViewShellId GetViewShellId() const override;
    /// See OutlinerViewShell::NotifyOtherViews().
    void NotifyOtherViews(int nType, const OString& rKey, const OString& rPayload) override;
    /// See OutlinerViewShell::NotifyOtherView().
    void NotifyOtherView(OutlinerViewShell* pOtherShell, int nType, const OString& rKey, const OString& rPayload) override;
    /// Ask this view to send its cursor position to pViewShell.
    virtual void NotifyCursor(SfxViewShell* /*pViewShell*/) const;
    /// Where a new view can perform some update/initialization soon after the callback has been registered.
    virtual void afterCallbackRegistered();
    /// See OutlinerViewShell::GetEditWindowForActiveOLEObj().
    virtual vcl::Window* GetEditWindowForActiveOLEObj() const override;

    /// Set the LibreOfficeKit language of this view.
    void SetLOKLanguageTag(const OUString& rBcp47LanguageTag);
    /// Get the LibreOfficeKit language of this view.
    const LanguageTag& GetLOKLanguageTag() const { return maLOKLanguageTag; }
};


inline SfxViewFrame* SfxViewShell::GetViewFrame() const

/*  [Description]

    This method returns a pointer to the <SfxViewFrame> Instance in which
    this SfxViewShell is displayed. This is the instance that was passed
    on in the constructor. It is guaranteed that the returned pointer
    points on the valid SfxViewFrame instance.

    [Cross-reference]

    <SfxShell::GetFrame()const>
*/

{
    return pFrame;
}

#endif // INCLUDED_SFX2_VIEWSH_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
