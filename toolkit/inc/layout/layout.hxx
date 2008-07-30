/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layout.hxx,v $
 *
 * $Revision: 1.3 $
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

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/util/Color.hpp>
#include <tools/link.hxx>
#include <tools/string.hxx>

// FIXME: eventually wrap FieldUnit
#include <vcl/fldunit.hxx>
// FIXME: eventually wrap this too ...
#include <vcl/wintypes.hxx>

#include <toolkit/dllapi.h>

class Window;
class Image;

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
    ~Context();
    PeerHandle GetPeerHandle( char const* pId, sal_uInt32 nId = 0 ) const;
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
        t( Context *pCtx, char const* pId, sal_uInt32 nId = 0 ); \
        t( Window *pParent, WinBits nStyle = defaultWinBit )
#define DECL_GET_IMPL(t) \
        inline t##Impl &getImpl() const

// follows the VCL inheritance hierarchy ...

class WindowImpl;
class TOOLKIT_DLLPUBLIC Window
{
protected:
    WindowImpl *mpImpl;
    static PeerHandle CreatePeer( Window *pParent, WinBits nStyle,
                                  char const* pName);
public:
    PeerHandle GetPeer();
    Context *getContext();
    DECL_GET_IMPL( Window );
    explicit Window( WindowImpl *pImpl );
    virtual ~Window();

    void Enable( bool bEnable = true );
    inline void Disable() { Enable( false ); }
    void Show( BOOL bVisible = TRUE );
    inline void Hide() { Show( FALSE ); }
    void GrabFocus();
    void FreeResource() {}

    WinBits GetStyle();
    void SetStyle( WinBits nStyle );
};

class ControlImpl;
class TOOLKIT_DLLPUBLIC Control : public Window
{
    DECL_GET_IMPL( Control );
    DECL_CONSTRUCTORS( Control, Window, 0 );
public:
// void SetText( String const& rStr ); - can't do this here ...
};

class FixedLineImpl;
class TOOLKIT_DLLPUBLIC FixedLine : public Control
{
    friend class FixedLineImpl;
    DECL_GET_IMPL( FixedLine );
    DECL_CONSTRUCTORS( FixedLine, Control, WB_HORZ );
public:
    bool IsEnabled();
};

class FixedTextImpl;
class TOOLKIT_DLLPUBLIC FixedText : public Control
{
    friend class FixedTextImpl;
    DECL_GET_IMPL( FixedText );
    DECL_CONSTRUCTORS( FixedText, Control, 0 );
public:
    void SetText( String const& rStr );
};

class FixedImageImpl;
class TOOLKIT_DLLPUBLIC FixedImage : public Control
{
    friend class FixedImageImpl;
    DECL_GET_IMPL( FixedImage );
    DECL_CONSTRUCTORS( FixedImage, Control, 0 );
public:
    void setImage( ::Image const& );
};

class FixedInfoImpl;
class TOOLKIT_DLLPUBLIC FixedInfo : public FixedText
{
    friend class FixedInfoImpl;
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
    friend class ButtonImpl;
    DECL_GET_IMPL( Button );
    DECL_CONSTRUCTORS( Button, Control, 0 );
public:
    void SetText( String const& rStr );
    BOOL SetModeImage( const Image& rImage );
    void SetImageAlign( ImageAlign eAlign );

    void SetClickHdl( Link const& rLink );
    virtual void Click() /* pure virtual? */;
};

class PushButtonImpl;
class TOOLKIT_DLLPUBLIC PushButton : public Button
{
    DECL_GET_IMPL( PushButton );
    DECL_CONSTRUCTORS( PushButton, Button, 0 );
public:
    void Check( BOOL bCheck = TRUE );
    BOOL IsChecked() const;

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

class AdvancedButtonImpl;
class TOOLKIT_DLLPUBLIC AdvancedButton : public PushButton
{
    DECL_CONSTRUCTORS( AdvancedButton, PushButton, 0 );
    DECL_GET_IMPL( AdvancedButton );

    void AddAdvanced( Window* w );
    void AddSimple( Window* w );
    void RemoveAdvanced( Window* w );
    void RemoveSimple( Window* w );
};

class MoreButtonImpl;
class TOOLKIT_DLLPUBLIC MoreButton : public AdvancedButton
{
    DECL_CONSTRUCTORS( MoreButton, AdvancedButton, 0 );
    DECL_GET_IMPL( MoreButton );
    void AddWindow( Window* w );
    void RemoveWindow( Window* w );
};

class RadioButtonImpl;
class TOOLKIT_DLLPUBLIC RadioButton : public Button
{
    DECL_GET_IMPL( RadioButton );
    DECL_CONSTRUCTORS( RadioButton, Button, 0 );
public:
    void Check( BOOL bCheck = TRUE );
    BOOL IsChecked() const;

    void Toggle();
    void SetToggleHdl( Link const& rLink );
};

class CheckBoxImpl;
class TOOLKIT_DLLPUBLIC CheckBox : public Button
{
    DECL_GET_IMPL( CheckBox );
    DECL_CONSTRUCTORS( CheckBox, Button, 0 );
public:
    void Check( BOOL bCheck = TRUE );
    BOOL IsChecked() const;

    void Toggle();
    void SetToggleHdl( Link const& rLink );
};

class EditImpl;
class TOOLKIT_DLLPUBLIC Edit : public Control
{
    DECL_GET_IMPL( Edit );
    DECL_CONSTRUCTORS( Edit, Control, WB_BORDER );
public:
    void SetText( XubString const& rStr ) const;
    XubString GetText() const;
    void SetModifyHdl( Link const& rLink );
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
    FormatterBase( FormatterBaseImpl *pFormatImpl )
        : mpFormatImpl( pFormatImpl ) {}
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
    NumericField( Context *pCtx, char const* pId, sal_uInt32 nId = 0 );
    NumericField( Window *pParent, WinBits nStyle );
};

class MetricFormatterImpl;
// Different inheritance to save code
class TOOLKIT_DLLPUBLIC MetricFormatter : public FormatterBase
{
  protected:
    explicit MetricFormatter( FormatterBaseImpl *pImpl );
    MetricFormatterImpl &getFormatImpl() const;
public:
    void SetMin( sal_Int64 nNewMin, FieldUnit nUnit = FUNIT_NONE );
    void SetMax( sal_Int64 nNewMax, FieldUnit nUnit = FUNIT_NONE );
    void SetFirst( sal_Int64 nNewFirst, FieldUnit nUnit = FUNIT_NONE );
    void SetLast( sal_Int64 nNewLast, FieldUnit nUnit = FUNIT_NONE );
    void SetValue( sal_Int64 nNewValue, FieldUnit nUnit = FUNIT_NONE );
    sal_Int64 GetValue( FieldUnit nUnit = FUNIT_NONE ) const;

    void SetSpinSize( sal_Int64 nNewSize );
};

class MetricFieldImpl;
class TOOLKIT_DLLPUBLIC MetricField : public SpinField, public MetricFormatter
{
    DECL_GET_IMPL( MetricField );
public:
    MetricField( Context *pCtx, char const* pId, sal_uInt32 nId = 0 );
    MetricField( Window *pParent, WinBits nStyle );
};

#define COMBOBOX_APPEND            ((USHORT)0xFFFF)
#define COMBOBOX_ENTRY_NOTFOUND    ((USHORT)0xFFFF)
class ComboBoxImpl;
class TOOLKIT_DLLPUBLIC ComboBox : public Edit
{
    DECL_GET_IMPL( ComboBox );
    DECL_CONSTRUCTORS( ComboBox, Edit, 0 );

public:
    USHORT InsertEntry( XubString const& rStr, USHORT nPos = COMBOBOX_APPEND );
    void RemoveEntry( XubString const& rStr );
    void RemoveEntry( USHORT nPos );
    void Clear();

    USHORT GetEntryPos( XubString const& rStr ) const;
    XubString GetEntry( USHORT nPos ) const;
    USHORT GetEntryCount() const;

    void SetClickHdl( Link const& rLink );
    void SetSelectHdl( Link const& rLink );
};

#define LISTBOX_APPEND               ((USHORT)0xFFFF)
#define LISTBOX_ENTRY_NOTFOUND       ((USHORT)0xFFFF)
class ListBoxImpl;
class TOOLKIT_DLLPUBLIC ListBox : public Control
{
    DECL_GET_IMPL( ListBox );
    DECL_CONSTRUCTORS( ListBox, Control, WB_BORDER );
public:
    USHORT InsertEntry( XubString const& rStr, USHORT nPos = LISTBOX_APPEND );

    void RemoveEntry( XubString const& rStr );
    void RemoveEntry( USHORT nPos );
    void Clear();

    USHORT GetEntryPos( XubString const& rStr ) const;
    XubString GetEntry( USHORT nPos ) const;
    USHORT GetEntryCount() const;

    void SelectEntry( XubString const& rStr, BOOL bSelect = TRUE );
    void SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE );

    USHORT GetSelectEntryCount() const;
    XubString GetSelectEntry( USHORT nSelIndex = 0 ) const;
    USHORT GetSelectEntryPos( USHORT nSelIndex = 0 ) const;

    void SetSelectHdl( Link const& rLink );
    void SetClickHdl( Link const& rLink );
};

class DialogImpl;
class TOOLKIT_DLLPUBLIC Dialog : public Context, public Window
{
    DECL_GET_IMPL( Dialog );
    void SetParent( Window *pParent );
    void SetParent( ::Window *pParent );
public:
    Dialog( Window *pOptParent, char const* pXMLPath, char const* pId, sal_uInt32 nId = 0 );
    Dialog( ::Window *pOptParent, char const* pXMLPath, char const* pId, sal_uInt32 nId = 0 );
    short Execute();
    void EndDialog( long nResult = 0 );
    void SetText( String const& rStr );
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
    Container( Context const* pCtx, char const* pId );

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
    Table( Context const* pCtx, char const* pId );
    void Add( Window *pWindow, bool bXExpand, bool bYExpand,
              sal_Int32 nXSpan = 1, sal_Int32 nYSpan = 1 );
    void Add( Container *pContainer, bool bXExpand, bool bYExpand,
              sal_Int32 nXSpan = 1, sal_Int32 nYSpan = 1 );

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nXSpan, sal_Int32 nYSpan );
};

class TOOLKIT_DLLPUBLIC Box : public Container
{
protected:
    Box( rtl::OUString const& rName, sal_Int32 nBorder, bool bHomogeneous );
public:
    Box( Context const* pCtx, char const* pId );
    void Add( Window *pWindow, bool bExpand, bool bFill, sal_Int32 nPadding);
    void Add( Container *pContainer, bool bExpand, bool bFill, sal_Int32 nPadding);

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nPadding );
};

class TOOLKIT_DLLPUBLIC HBox : public Box
{
public:
    HBox( Context const* pCtx, char const* pId );
    HBox( sal_Int32 nBorder, bool bHomogeneous );
};
class TOOLKIT_DLLPUBLIC VBox : public Box
{
public:
    VBox( Context const* pCtx, char const* pId );
    VBox( sal_Int32 nBorder, bool bHomogeneous );
};

} // end namespace layout

#endif /* _LAYOUT_HXX */
