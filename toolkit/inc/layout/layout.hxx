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
#include <vcl/combobox.h>
#include <tools/fldunit.hxx>
#include <vcl/lstbox.h>
#include <tools/wintypes.hxx>

class Button;
class ComboBox;
class Color;
class Control;
class Dialog;
class Edit;
class Font;
class Image;
class ListBox;
class MapMode;
class MultiListBox;
class NotifyEvent;
class Pointer;
class PushButton;
class RadioButton;
class ResId;
struct SfxChildWinInfo;
class TabControl;
class TabPage;
class VCLXRadioButton;
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
    void setToplevel( PeerHandle xToplevel );
    PeerHandle getToplevel();
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

    void Enable( bool bEnable = true );
    inline void Disable() { Enable( false ); }
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
    Link& GetGetFocusHdl ();
    void SetLoseFocusHdl (Link const& link);
    Link& GetLoseFocusHdl ();
};

class FixedLineImpl;
class TOOLKIT_DLLPUBLIC FixedLine : public Control
{
    DECL_GET_IMPL( FixedLine );
    DECL_CONSTRUCTORS( FixedLine, Control, WB_HORZ );

public:
    bool IsEnabled() const;
};

class FixedTextImpl;
class TOOLKIT_DLLPUBLIC FixedText : public Control
{
    DECL_GET_IMPL( FixedText );
    DECL_CONSTRUCTORS( FixedText, Control, 0 );

public:
    ~FixedText ();
    void SetText( rtl::OUString const& rStr );
};

class FixedImageImpl;
class TOOLKIT_DLLPUBLIC FixedImage : public Control
{
    DECL_GET_IMPL( FixedImage );
    DECL_CONSTRUCTORS( FixedImage, Control, 0 );
public:
    void setImage( ::Image const& );
};

class FixedInfoImpl;
class TOOLKIT_DLLPUBLIC FixedInfo : public FixedText
{
    DECL_GET_IMPL( FixedInfo );
    DECL_CONSTRUCTORS( FixedInfo, FixedText, 0 );
};

class ImageImpl;
class TOOLKIT_DLLPUBLIC Image
{
    ImageImpl *pImpl;
public:
    Image( char const* pName );
    ~Image();
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
    bool SetModeImage (Image const& image);
    bool SetModeImage (::Image const& image);
    void SetImageAlign( ImageAlign eAlign );

    void SetClickHdl( Link const& rLink );
    virtual void Click() /* pure virtual? */;
    Link& GetClickHdl ();
};

class PushButtonImpl;
class TOOLKIT_DLLPUBLIC PushButton : public Button
{
    DECL_GET_IMPL( PushButton );
    DECL_CONSTRUCTORS( PushButton, Button, 0 );
    DECL_GET_WINDOW (PushButton);

public:
    ~PushButton ();
    void Check( bool bCheck=true );
    bool IsChecked() const;

    void Toggle();
    void SetToggleHdl( Link const& rLink );
};

class TOOLKIT_DLLPUBLIC OKButton : public PushButton
{
    DECL_CONSTRUCTORS( OKButton, PushButton, WB_DEFBUTTON );
};
class TOOLKIT_DLLPUBLIC CancelButton : public PushButton
{
    DECL_CONSTRUCTORS( CancelButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC YesButton : public PushButton
{
    DECL_CONSTRUCTORS( YesButton, PushButton, WB_DEFBUTTON );
};
class TOOLKIT_DLLPUBLIC NoButton : public PushButton
{
    DECL_CONSTRUCTORS( NoButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC RetryButton : public PushButton
{
    DECL_CONSTRUCTORS( RetryButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC IgnoreButton : public PushButton
{
    DECL_CONSTRUCTORS( IgnoreButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC ResetButton : public PushButton
{
    DECL_CONSTRUCTORS( ResetButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC ApplyButton : public PushButton
{
    DECL_CONSTRUCTORS( ApplyButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC HelpButton : public PushButton
{
    DECL_CONSTRUCTORS( HelpButton, PushButton, 0 );
};
class TOOLKIT_DLLPUBLIC ImageButton : public PushButton
{
    DECL_CONSTRUCTORS( ImageButton, PushButton, 0 );
};

class AdvancedButtonImpl;
class TOOLKIT_DLLPUBLIC AdvancedButton : public PushButton
{
    DECL_CONSTRUCTORS( AdvancedButton, PushButton, 0 );
    DECL_GET_IMPL( AdvancedButton );

    void AddAdvanced( Window* w );
    void AddSimple( Window* w );
    void RemoveAdvanced( Window* w );
    void RemoveSimple( Window* w );

    void SetAdvancedText (rtl::OUString const& text);
    void SetSimpleText (rtl::OUString const& text);
    rtl::OUString GetAdvancedText () const;
    rtl::OUString GetSimpleText () const;
    void SetDelta (int);
};

class MoreButtonImpl;
class TOOLKIT_DLLPUBLIC MoreButton : public AdvancedButton
{
    DECL_CONSTRUCTORS( MoreButton, AdvancedButton, 0 );
    DECL_GET_IMPL( MoreButton );
    void AddWindow( Window* w );
    void RemoveWindow( Window* w );

    void SetMoreText (rtl::OUString const& text);
    void SetLessText (rtl::OUString const& text);
    rtl::OUString GetMoreText () const;
    rtl::OUString GetLessText () const;
};

class RadioButtonImpl;
class TOOLKIT_DLLPUBLIC RadioButton : public Button
{
    DECL_GET_IMPL( RadioButton );
    DECL_CONSTRUCTORS( RadioButton, Button, 0 );
    DECL_GET_WINDOW( RadioButton );
    DECL_GET_VCLXWINDOW( RadioButton );
public:
    ~RadioButton ();
    void Check( bool bCheck=true );
    bool IsChecked() const;

    void Toggle();
    void SetToggleHdl( Link const& rLink );
};

class CheckBoxImpl;
class TOOLKIT_DLLPUBLIC CheckBox : public Button
{
    DECL_GET_IMPL( CheckBox );
    DECL_CONSTRUCTORS( CheckBox, Button, 0 );

public:
    ~CheckBox ();
    void Check( bool bCheck=true );
    bool IsChecked() const;

    void Toggle();
    void SetToggleHdl( Link const& rLink );
};

class EditImpl;
class TOOLKIT_DLLPUBLIC Edit : public Control
{
    DECL_GET_IMPL( Edit );
    DECL_CONSTRUCTORS( Edit, Control, WB_BORDER );
    DECL_GET_WINDOW (Edit);

public:
    ~Edit ();
    void SetText( rtl::OUString const& rStr );
    String GetText() const;
    void SetModifyHdl( Link const& rLink );
    void SetSelection( Selection const& rSelection );
};

class MultiLineEditImpl;
class TOOLKIT_DLLPUBLIC MultiLineEdit : public Edit
{
    DECL_GET_IMPL( MultiLineEdit );
    DECL_CONSTRUCTORS( MultiLineEdit, Edit, WB_LEFT|WB_BORDER );
};

class SpinFieldImpl;
class TOOLKIT_DLLPUBLIC SpinField : public Edit
{
    DECL_GET_IMPL( SpinField );
    DECL_CONSTRUCTORS( SpinField, Edit, 0 );
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
public:
    void SetMin( sal_Int64 nNewMin );
    void SetMax( sal_Int64 nNewMax );
    void SetFirst( sal_Int64 nNewFirst );
    void SetLast( sal_Int64 nNewLast );
    void SetSpinSize( sal_Int64 nNewSize );

    void SetValue( sal_Int64 nNewValue );
    sal_Int64 GetValue() const;
};

class NumericFieldImpl;
class TOOLKIT_DLLPUBLIC NumericField : public SpinField, public NumericFormatter
{
    DECL_GET_IMPL( NumericField );
public:
    NumericField( Context *context, char const* id, sal_uInt32 nId=0 );
    NumericField( Window *parent, WinBits nStyle );
};

class MetricFormatterImpl;
// Different inheritance to save code
class TOOLKIT_DLLPUBLIC MetricFormatter : public FormatterBase
{
  protected:
    explicit MetricFormatter( FormatterBaseImpl *pImpl );
    MetricFormatterImpl &getFormatImpl() const;
public:
    void SetMin( sal_Int64 nNewMin, FieldUnit nUnit=FUNIT_NONE );
    void SetMax( sal_Int64 nNewMax, FieldUnit nUnit=FUNIT_NONE );
    void SetFirst( sal_Int64 nNewFirst, FieldUnit nUnit=FUNIT_NONE );
    void SetLast( sal_Int64 nNewLast, FieldUnit nUnit=FUNIT_NONE );
    void SetValue( sal_Int64 nNewValue, FieldUnit nUnit=FUNIT_NONE );
    sal_Int64 GetValue( FieldUnit nUnit=FUNIT_NONE ) const;

    void SetSpinSize( sal_Int64 nNewSize );
};

class MetricFieldImpl;
class TOOLKIT_DLLPUBLIC MetricField : public SpinField, public MetricFormatter
{
    DECL_GET_IMPL( MetricField );
public:
    MetricField( Context *context, char const* id, sal_uInt32 nId=0 );
    MetricField( Window *parent, WinBits nStyle );
};

class ComboBoxImpl;
class TOOLKIT_DLLPUBLIC ComboBox : public Edit
{
    DECL_GET_IMPL( ComboBox );
    DECL_GET_WINDOW (ComboBox );
    DECL_CONSTRUCTORS( ComboBox, Edit, 0 );

public:
    ~ComboBox ();
    sal_uInt16 InsertEntry( String const& rStr, sal_uInt16 nPos=COMBOBOX_APPEND );
    void RemoveEntry( String const& rStr );
    void RemoveEntry( sal_uInt16 nPos );
    void Clear();

    sal_uInt16 GetEntryPos( String const& rStr ) const;
    String GetEntry( sal_uInt16 nPos ) const;
    sal_uInt16 GetEntryCount() const;

    void SetClickHdl( Link const& rLink );
    void SetSelectHdl( Link const& rLink );
    void EnableAutocomplete (bool enable, bool matchCase=false );
};

class ListBoxImpl;
class TOOLKIT_DLLPUBLIC ListBox : public Control
{
    DECL_GET_IMPL( ListBox );
    DECL_CONSTRUCTORS( ListBox, Control, WB_BORDER );
    DECL_GET_WINDOW (ListBox);

public:
    ~ListBox ();
    sal_uInt16 InsertEntry( String const& rStr, sal_uInt16 nPos=LISTBOX_APPEND );

    void RemoveEntry( String const& rStr );
    void RemoveEntry( sal_uInt16 nPos );
    void Clear();

    sal_uInt16 GetEntryPos( String const& rStr ) const;
    String GetEntry( sal_uInt16 nPos ) const;
    sal_uInt16 GetEntryCount() const;

    void SelectEntry( String const& rStr, bool bSelect=true );
    void SelectEntryPos( sal_uInt16 nPos, bool bSelect=true );

    sal_uInt16 GetSelectEntryCount() const;
    String GetSelectEntry( sal_uInt16 nSelIndex=0 ) const;
    sal_uInt16 GetSelectEntryPos( sal_uInt16 nSelIndex=0 ) const;

    void SetSelectHdl (Link const& link);
    Link& GetSelectHdl ();

    void SetClickHdl (Link const& link);
    Link& GetClickHdl ();

    void SetDoubleClickHdl (Link const& link);
    Link& GetDoubleClickHdl ();

    void SetEntryData (sal_uInt16 pos, void* data);
    void* GetEntryData (sal_uInt16 pos) const;

    virtual void SetNoSelection ();
};

class MultiListBoxImpl;
class TOOLKIT_DLLPUBLIC MultiListBox : public ListBox
{
    DECL_GET_IMPL( MultiListBox );
    DECL_CONSTRUCTORS( MultiListBox, ListBox, 0 );
    DECL_GET_WINDOW( MultiListBox );
};

class DialogImpl;
class TOOLKIT_DLLPUBLIC Dialog : public Context, public Window
{
    DECL_GET_WINDOW (Dialog);

public:
    DECL_GET_IMPL (Dialog);
    Dialog( Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0 );
    Dialog( ::Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0 );
    ~Dialog ();
    virtual short Execute();
    void EndDialog( long nResult=0 );
    void SetText( rtl::OUString const& rStr );
    void SetTitle (rtl::OUString const& rStr );
    bool Close ();
    long Notify (NotifyEvent & event);

    // Sxf*Dialog
    bool bConstruct;
    void Initialize (SfxChildWinInfo*);
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
CLASS_MESSAGE_BOX (Query);
CLASS_MESSAGE_BOX (Warning);

#undef CLASS_MESSAGE_BOX

#undef TAB_APPEND

class TabControlImpl;
class TOOLKIT_DLLPUBLIC TabControl : public Control
{
//#ifndef TAB_APPEND
#define TAB_APPEND 0xFFFF
//#endif /* !TAB_APPEND */

    DECL_GET_IMPL (TabControl);
    DECL_CONSTRUCTORS (TabControl, Control, 0);
    DECL_GET_WINDOW (TabControl);
    DECL_GET_LAYOUT_VCLXWINDOW (TabControl);

public:
    ~TabControl ();
    void InsertPage (sal_uInt16 id, rtl::OUString const& title, sal_uInt16 pos=TAB_APPEND);
    void RemovePage (sal_uInt16 id);
    sal_uInt16 GetPageCount () const;
    sal_uInt16 GetPageId (sal_uInt16 pos) const;
    sal_uInt16 GetPagePos (sal_uInt16 id) const;
    void SetCurPageId (sal_uInt16 id);
    sal_uInt16 GetCurPageId () const;
    void SetTabPage (sal_uInt16 id, ::TabPage* page);
    ::TabPage* GetTabPage (sal_uInt16 id) const;
    void SetActivatePageHdl (Link const& link);
    Link& GetActivatePageHdl () const;
    void SetDeactivatePageHdl (Link const& link);
    Link& GetDeactivatePageHdl () const;
    void SetTabPageSizePixel (Size const& size);
    Size GetTabPageSizePixel () const;
};

class TabPageImpl;
class TOOLKIT_DLLPUBLIC TabPage : public Context, public Window
{
    DECL_GET_IMPL( TabPage );
public:
    static ::Window* global_parent;
    static TabControl* global_tabcontrol;

    TabPage( Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0 );
    TabPage( ::Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0 );
    ~TabPage();
    DECL_GET_WINDOW( TabPage );
    virtual void ActivatePage();
    virtual void DeactivatePage();
};

class ProgressBarImpl;
class TOOLKIT_DLLPUBLIC ProgressBar : public Control
{
    DECL_GET_IMPL( ProgressBar );
    DECL_CONSTRUCTORS( ProgressBar, Control, WB_BORDER );
public:
    void SetForegroundColor( css::util::Color color );
    void SetBackgroundColor( css::util::Color color );
    void SetValue( sal_Int32 i );
    void SetRange( sal_Int32 min, sal_Int32 max );
    sal_Int32 GetValue();
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

    void Add( Window *pWindow );
    void Add( Container *pContainer );

    void Remove( Window *pWindow );
    void Remove( Container *pContainer );
    void Clear();
    // we can't really do a GetChildren() as they don't have a common class,
    // besides we would need to keep track of children, uh

    void ShowAll( bool bVisible );
    void Show();
    void Hide();

    css::uno::Reference< css::awt::XLayoutContainer > getImpl()
    { return mxContainer; }
};

class TOOLKIT_DLLPUBLIC Table : public Container
{
protected:
    Table( sal_Int32 nBorder, sal_Int32 nColumns );
public:
    Table( Context const* context, char const* id );
    void Add( Window *pWindow, bool bXExpand, bool bYExpand,
              sal_Int32 nXSpan=1, sal_Int32 nYSpan=1 );
    void Add( Container *pContainer, bool bXExpand, bool bYExpand,
              sal_Int32 nXSpan=1, sal_Int32 nYSpan=1 );

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nXSpan, sal_Int32 nYSpan );
};

class TOOLKIT_DLLPUBLIC Box : public Container
{
protected:
    Box( rtl::OUString const& rName, sal_Int32 nBorder, bool bHomogeneous );
public:
    Box( Context const* context, char const* id );
    void Add( Window *pWindow, bool bExpand, bool bFill, sal_Int32 nPadding);
    void Add( Container *pContainer, bool bExpand, bool bFill, sal_Int32 nPadding);

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nPadding );
};

class TOOLKIT_DLLPUBLIC HBox : public Box
{
public:
    HBox( Context const* context, char const* id );
    HBox( sal_Int32 nBorder, bool bHomogeneous );
};
class TOOLKIT_DLLPUBLIC VBox : public Box
{
public:
    VBox( Context const* context, char const* id );
    VBox( sal_Int32 nBorder, bool bHomogeneous );
};

class PluginImpl;
class TOOLKIT_DLLPUBLIC Plugin : public Control
{
    DECL_GET_IMPL( Plugin );
public:
    ::Control *mpPlugin;
    Plugin( Context *context, char const* id, ::Control *plugin );
};

class LocalizedStringImpl;
class TOOLKIT_DLLPUBLIC LocalizedString : public Window
{
    DECL_GET_IMPL( LocalizedString );

public:
    LocalizedString( Context *context, char const* id );

    rtl::OUString operator= (rtl::OUString const&);
    rtl::OUString operator+= (sal_Unicode );
    rtl::OUString operator+= (rtl::OUString const&);

    operator rtl::OUString ();
    operator rtl::OUString const& ();
    operator String();
    String getString ();
    rtl::OUString getOUString ();
    String GetToken (sal_uInt16 i, sal_Char c);
};

class InPlugImpl;
class TOOLKIT_DLLPUBLIC InPlug : public Context, public Window
{
    DECL_GET_IMPL (InPlug);

public:
    InPlug ( Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0);
    InPlug ( ::Window *parent, char const* xml_file, char const* id, sal_uInt32 nId=0);

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
