/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _LAYOUT_HXX
#define _LAYOUT_HXX

#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/Color.hpp>
#include <i18npool/lang.h>
#include <toolkit/dllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <vcl/bitmap.hxx>
#include <tools/fldunit.hxx>
#include <vcl/lstbox.h>
#include <tools/wintypes.hxx>

class Button;
class Color;
class Control;
class Dialog;
class Edit;
class Font;
class Image;
class ListBox;
class MapMode;
class Pointer;
class PushButton;
class ResId;
struct SfxChildWinInfo;
class TabControl;
class TabPage;
class VCLXWindow;
class Window;

namespace layoutimpl
{
class VCLXTabControl;
}

class VCLXTabControl;

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }

namespace layout
{

namespace css = ::com::sun::star;

typedef css::uno::Reference< css::uno::XInterface > PeerHandle;
class ContextImpl;
class TOOLKIT_DLLPUBLIC Context
{
    ContextImpl *pImpl;
public:
    Context( char const* pPath );
    virtual ~Context();
    PeerHandle GetPeerHandle( char const* id, sal_uInt32 nId = 0 ) const;
    PeerHandle getRoot();
};

// make declaring wrappers easier ...
#define DECL_CONSTRUCTORS(t,par,defaultWinBit) \
    protected:                                 \
        explicit t( WindowImpl *pImpl ) : \
            par( pImpl ) {} \
        char const* GetUnoName() const; \
    public: \
        t( Context *context, char const* id, sal_uInt32 nId = 0 ); \
        t( Window *parent, WinBits nStyle = defaultWinBit ); \
        t (Window *parent, ResId const &res)
#define DECL_GET_IMPL(t) \
        inline t##Impl &getImpl() const

#define DECL_GET_WINDOW( cls ) ::cls* Get##cls() const
#define IMPL_GET_WINDOW( cls ) ::cls* cls::Get##cls() const { return dynamic_cast< ::cls*>( GetWindow() ); }

#define DECL_GET_VCLXWINDOW( cls ) ::VCLX##cls* GetVCLX##cls() const
#define IMPL_GET_VCLXWINDOW( cls ) ::VCLX##cls* cls::Get##VCLX##cls() const { return dynamic_cast< ::VCLX##cls*>( GetVCLXWindow() ); }

#define DECL_GET_LAYOUT_VCLXWINDOW( cls ) ::layoutimpl::VCLX##cls* GetVCLX##cls() const
#define IMPL_GET_LAYOUT_VCLXWINDOW( cls ) ::layoutimpl::VCLX##cls* cls::Get##VCLX##cls() const { return dynamic_cast< ::layoutimpl::VCLX##cls*>( GetVCLXWindow() ); }

// follows the VCL inheritance hierarchy ...

class WindowImpl;
class TOOLKIT_DLLPUBLIC Window
{
protected:
    friend class WindowImpl;
    WindowImpl *mpImpl;
    static PeerHandle CreatePeer( Window *parent, WinBits nStyle,
                                  char const* pName);

    virtual void setRes (ResId const& res);

public:
    DECL_GET_IMPL( Window );
    explicit Window( WindowImpl *pImpl );
    virtual ~Window();

    PeerHandle GetPeer() const;
    Context *getContext();

    void Show( bool bVisible = true );
    inline void Hide() { Show( false ); }
    void GrabFocus();
    void FreeResource() {}

    void SetParent( ::Window *parent );
    virtual void ParentSet (Window *window);
    void SetParent( Window *parent );

    css::uno::Reference<css::awt::XWindow> GetRef() const;
    VCLXWindow* GetVCLXWindow() const;
    ::Window* GetWindow() const;
    ::Window* GetParent() const;

    void SetPointer( Pointer const& pointer );
    Pointer const& GetPointer() const;
    WinBits GetStyle();
    void SetText( rtl::OUString const& str );
    String GetText() const;
    void SetStyle( WinBits style );
    void SetUpdateMode( bool mode );
    void SetHelpId( const rtl::OString& id );
    const rtl::OString& GetHelpId() const;
    void EnterWait ();
    void LeaveWait ();
    bool IsWait () const;
    bool IsEnabled () const;
    void EnableInput (bool enable=true, bool child=true);
    bool IsInputEnabled () const;

    bool IsVisible () const;
    bool HasChildPathFocus (bool systemWindow=false) const;
    void SetPosPixel (Point const&);
    Point GetPosPixel () const;
    void SetSizePixel (Size const&);
    Size GetSizePixel () const;
    void SetPosSizePixel (Point const&, Size const&);
    sal_Int32 GetCtrlTextWidth (rtl::OUString const& str) const;
    sal_Int32 GetTextHeight () const;

    Size LogicToPixel( Size const& size ) const;
    Size LogicToPixel( Size const& size, MapMode const& mapMode ) const;

    bool HasFocus () const;
    Font& GetFont () const;
    void SetFont (Font const& font);
    virtual void Invalidate (sal_uInt8 flags=0);
};

class ControlImpl;
class TOOLKIT_DLLPUBLIC Control : public Window
{
    DECL_GET_IMPL( Control );
    DECL_CONSTRUCTORS( Control, Window, 0 );

public:
    ~Control ();
    void SetGetFocusHdl (Link const& link);
    void SetLoseFocusHdl (Link const& link);
};

class FixedTextImpl;
class TOOLKIT_DLLPUBLIC FixedText : public Control
{
    DECL_GET_IMPL( FixedText );

public:
    FixedText( Context *context, char const* id, sal_uInt32 nId = 0 );
    ~FixedText ();
    void SetText( rtl::OUString const& rStr );
};

class FixedImageImpl;
class TOOLKIT_DLLPUBLIC FixedImage : public Control
{
    DECL_GET_IMPL( FixedImage );

public:
    FixedImage( Context *context, char const* id, sal_uInt32 nId = 0 );
};

class ImageImpl;
class TOOLKIT_DLLPUBLIC Image
{
    ImageImpl *pImpl;
public:
    ImageImpl &getImpl() const { return *pImpl; }
};

class ButtonImpl;
class TOOLKIT_DLLPUBLIC Button : public Control
{
    DECL_GET_IMPL( Button );
    DECL_CONSTRUCTORS( Button, Control, 0 );
    DECL_GET_WINDOW (Button);

public:
    ~Button ();
    static String GetStandardText (sal_uInt16 button_type);
    void SetText( rtl::OUString const& rStr );

    virtual void Click() /* pure virtual? */;
};

class PushButtonImpl;
class TOOLKIT_DLLPUBLIC PushButton : public Button
{
    DECL_GET_IMPL( PushButton );
    DECL_GET_WINDOW (PushButton);

protected:
    explicit PushButton( WindowImpl *pImpl ) : Button( pImpl ) {}

public:
    ~PushButton ();
    void Check( bool bCheck=true );

    void SetToggleHdl( Link const& rLink );
};

class TOOLKIT_DLLPUBLIC CancelButton : public PushButton
{
public:
    CancelButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};
class TOOLKIT_DLLPUBLIC YesButton : public PushButton
{
public:
    YesButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};
class TOOLKIT_DLLPUBLIC NoButton : public PushButton
{
public:
    NoButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};
class TOOLKIT_DLLPUBLIC RetryButton : public PushButton
{
public:
    RetryButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};
class TOOLKIT_DLLPUBLIC IgnoreButton : public PushButton
{
public:
    IgnoreButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};
class TOOLKIT_DLLPUBLIC HelpButton : public PushButton
{
public:
    HelpButton( Context *context, char const* id, sal_uInt32 nId = 0 );
};

class EditImpl;
class TOOLKIT_DLLPUBLIC Edit : public Control
{
    DECL_GET_IMPL( Edit );
    DECL_CONSTRUCTORS( Edit, Control, WB_BORDER );
    DECL_GET_WINDOW (Edit);

public:
    ~Edit ();
    void SetModifyHdl( Link const& rLink );
};

class FormatterBaseImpl;
class TOOLKIT_DLLPUBLIC FormatterBase
{
protected:
    FormatterBaseImpl *mpFormatImpl;
    FormatterBase( FormatterBaseImpl *pFormatImpl );
};

class NumericFormatterImpl;
class TOOLKIT_DLLPUBLIC NumericFormatter : public FormatterBase
{
protected:
    explicit NumericFormatter( FormatterBaseImpl *pImpl );
    NumericFormatterImpl &getFormatImpl() const;
};

class MetricFormatterImpl;
// Different inheritance to save code
class TOOLKIT_DLLPUBLIC MetricFormatter : public FormatterBase
{
  protected:
    explicit MetricFormatter( FormatterBaseImpl *pImpl );
    MetricFormatterImpl &getFormatImpl() const;
};

class ListBoxImpl;
class TOOLKIT_DLLPUBLIC ListBox : public Control
{
    DECL_GET_IMPL( ListBox );
    DECL_CONSTRUCTORS( ListBox, Control, WB_BORDER );
    DECL_GET_WINDOW (ListBox);

public:
    ~ListBox ();

    sal_uInt16 GetEntryPos( String const& rStr ) const;
    String GetEntry( sal_uInt16 nPos ) const;

    void SelectEntryPos( sal_uInt16 nPos, bool bSelect=true );

    sal_uInt16 GetSelectEntryPos( sal_uInt16 nSelIndex=0 ) const;

    virtual void SetNoSelection ();
};

class DialogImpl;
class TOOLKIT_DLLPUBLIC Dialog : public Context, public Window
{
    DECL_GET_WINDOW (Dialog);

public:
    DECL_GET_IMPL (Dialog);
    Dialog( ::Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0 );
    ~Dialog ();
    virtual short Execute();
    void EndDialog( long nResult=0 );
    void SetTitle (rtl::OUString const& rStr );

    // Sxf*Dialog
    bool bConstruct;
};

#define DECL_MESSAGE_BOX_CTORS(Name)\
    public:\
    Name##Box (::Window *parent, char const* message,\
               char const* yes=0, char const* no=0, const rtl::OString& help_id=rtl::OString(),\
               char const* xml_file="message-box.xml", char const* id="message-box");\
    Name##Box (::Window *parent, rtl::OUString const& message,\
               rtl::OUString yes=String (),\
               rtl::OUString no=String (),\
               const rtl::OString& help_id=rtl::OString(),\
               char const* xml_file="message-box.xml", char const* id="message-box");\
    Name##Box (::Window *parent, WinBits, char const* message,\
               char const* yes=0, char const* no=0, const rtl::OString& help_id=rtl::OString(),\
               char const* xml_file="message-box.xml", char const* id="message-box");\
    Name##Box (::Window *parent, WinBits, rtl::OUString const& message,\
               rtl::OUString yes=String (),\
               rtl::OUString no=String (),\
               const rtl::OString& help_id=rtl::OString(),\
               char const* xml_file="message-box.xml", char const* id="message-box")\

class TOOLKIT_DLLPUBLIC MessageBox : public Dialog
{
    DECL_MESSAGE_BOX_CTORS(Message);

protected:
    FixedImage imageError;
    FixedImage imageInfo;
    FixedImage imageQuery;
    FixedImage imageWarning;
    FixedText messageText;
    CancelButton cancelButton;
    HelpButton helpButton;
    IgnoreButton ignoreButton;
    NoButton noButton;
    RetryButton retryButton;
    YesButton yesButton;

    void bits_init (WinBits bits, rtl::OUString const& message,
                    rtl::OUString yes, rtl::OUString, const rtl::OString& help_id);
    void init (rtl::OUString const& message,
               rtl::OUString const& yes, rtl::OUString const& no, const rtl::OString& help_id);
    void init (char const* message, char const* yes, char const* no, const rtl::OString& help_id);
};

#define CLASS_MESSAGE_BOX(Name)\
    class TOOLKIT_DLLPUBLIC Name##Box : public MessageBox\
    {\
        DECL_MESSAGE_BOX_CTORS (Name);\
    }

//CLASS_MESSAGE_BOX (Mess);
typedef MessageBox MessBox;
CLASS_MESSAGE_BOX (Error);
CLASS_MESSAGE_BOX (Info);

#undef CLASS_MESSAGE_BOX

#undef TAB_APPEND

class TabControlImpl;
class TOOLKIT_DLLPUBLIC TabControl : public Control
{
//#ifndef TAB_APPEND
#define TAB_APPEND 0xFFFF
//#endif /* !TAB_APPEND */

    DECL_GET_IMPL (TabControl);
    DECL_GET_WINDOW (TabControl);

public:
    ~TabControl ();
    void SetCurPageId (sal_uInt16 id);
    void SetActivatePageHdl (Link const& link);
    void SetDeactivatePageHdl (Link const& link);
};

class TabPageImpl;
class TOOLKIT_DLLPUBLIC TabPage : public Context, public Window
{
    DECL_GET_IMPL( TabPage );
public:
    static ::Window* global_parent;
    static TabControl* global_tabcontrol;

    ~TabPage();
    DECL_GET_WINDOW( TabPage );
    virtual void ActivatePage();
    virtual void DeactivatePage();
};

// -----------------------------------------------------------------
//                 layout container / helper wrappers
// -----------------------------------------------------------------

class TOOLKIT_DLLPUBLIC Container
{
protected:
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
    Container( rtl::OUString const& rName, sal_Int32 nBorder );
public:
    Container( Context const* context, char const* id );

    // we can't really do a GetChildren() as they don't have a common class,
    // besides we would need to keep track of children, uh

    void ShowAll( bool bVisible );

    css::uno::Reference< css::awt::XLayoutContainer > getImpl()
    { return mxContainer; }
};

class TOOLKIT_DLLPUBLIC Table : public Container
{
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nXSpan, sal_Int32 nYSpan );
};

class TOOLKIT_DLLPUBLIC Box : public Container
{
protected:
    Box( rtl::OUString const& rName, sal_Int32 nBorder, bool bHomogeneous );
public:
    Box( Context const* context, char const* id );

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nPadding );
};

class InPlugImpl;
class TOOLKIT_DLLPUBLIC InPlug : public Context, public Window
{
    DECL_GET_IMPL (InPlug);

public:

    void ParentSet (Window *window);
};

} // end namespace layout

#if ENABLE_LAYOUT
#define LAYOUT_NS layout::
#define LAYOUT_DIALOG_PARENT\
    VCLXWindow::GetImplementation( uno::Reference <awt::XWindow> ( GetPeer(), uno::UNO_QUERY ) )->GetWindow()
#define LAYOUT_THIS_WINDOW( this ) this->GetWindow ()
#else /* !ENABLE_LAYOUT */
#define LAYOUT_NS
#define LAYOUT_DIALOG_PARENT this
#define LAYOUT_THIS_WINDOW( this ) this
#endif /* !ENABLE_LAYOUT */

#endif /* _LAYOUT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
