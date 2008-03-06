#ifndef _LAYOUT_HXX
#define _LAYOUT_HXX

#if ENABLE_LAYOUT

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <tools/link.hxx>
#include <tools/string.hxx>

// FIXME: eventually wrap FieldUnit
#include <vcl/fldunit.hxx>
// FIXME: eventually wrap this too ...
#include <vcl/wintypes.hxx>

#include <toolkit/dllapi.h>

class Window;

namespace layout
{

namespace css = ::com::sun::star;

typedef css::uno::Reference< css::uno::XInterface > PeerHandle;
class ContextImpl;
class TOOLKIT_DLLPUBLIC Context
{
    ContextImpl *pImpl;
public:
    Context( const char *pPath );
    ~Context();
    PeerHandle GetPeerHandle( const char *pId, sal_uInt32 nId = 0 ) const;
    void setToplevel( PeerHandle xToplevel );
    PeerHandle getToplevel();
    PeerHandle getRoot();
};

class ImageImpl;
class TOOLKIT_DLLPUBLIC Image
{
    ImageImpl *pImpl;
public:
    Image( const char *pName );
    ~Image();
    ImageImpl &getImpl() const { return *pImpl; }
};

// make declaring wrappers easier ...
#define DECL_CONSTRUCTORS(t,par,defaultWinBit) \
    protected:                                 \
        explicit t( WindowImpl *pImpl ) : \
            par( pImpl ) {} \
        const char *GetUnoName() const; \
    public: \
        t( Context *pCtx, const char *pId, sal_uInt32 nId = 0 ); \
        t( Window *pParent, WinBits nStyle = defaultWinBit )
#define DECL_GET_IMPL(t) \
        inline t##Impl &getImpl() const

// follows the VCL inheritance hierarchy ...

class WindowImpl;
class TOOLKIT_DLLPUBLIC TOOLKIT_DLLPUBLIC Window
{
protected:
    WindowImpl *mpImpl;
    static PeerHandle CreatePeer( Window *pParent, WinBits nStyle,
                                  const char *pName);
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
// void SetText( const String& rStr ); - can't do this here ...
};

class FixedLineImpl;
class TOOLKIT_DLLPUBLIC FixedLine : public Control
{
    friend class FixedLineImpl;
    DECL_GET_IMPL( FixedLine );
    DECL_CONSTRUCTORS( FixedLine, Control, WB_HORZ );
};

class FixedTextImpl;
class TOOLKIT_DLLPUBLIC FixedText : public Control
{
    friend class FixedTextImpl;
    DECL_GET_IMPL( FixedText );
    DECL_CONSTRUCTORS( FixedText, Control, 0 );
public:
    void SetText( const String& rStr );
};

class FixedInfoImpl;
class TOOLKIT_DLLPUBLIC FixedInfo : public FixedText
{
    friend class FixedInfoImpl;
    DECL_GET_IMPL( FixedInfo );
    DECL_CONSTRUCTORS( FixedInfo, FixedText, 0 );
};

class ButtonImpl;
class TOOLKIT_DLLPUBLIC Button : public Control
{
    friend class ButtonImpl;
    DECL_GET_IMPL( Button );
    DECL_CONSTRUCTORS( Button, Control, 0 );
public:
    void SetText( const String& rStr );
    BOOL SetModeImage( const Image& rImage );
    void SetImageAlign( ImageAlign eAlign );

    void SetClickHdl( const Link& rLink );
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
    void SetToggleHdl( const Link& rLink );
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

class RadioButtonImpl;
class TOOLKIT_DLLPUBLIC RadioButton : public Button
{
    DECL_GET_IMPL( RadioButton );
    DECL_CONSTRUCTORS( RadioButton, Button, 0 );
public:
    void Check( BOOL bCheck = TRUE );
    BOOL IsChecked() const;

    void Toggle();
    void SetToggleHdl( const Link& rLink );
};

class CheckBoxImpl;
class TOOLKIT_DLLPUBLIC TOOLKIT_DLLPUBLIC CheckBox : public Button
{
    DECL_GET_IMPL( CheckBox );
    DECL_CONSTRUCTORS( CheckBox, Button, 0 );
public:
    void Check( BOOL bCheck = TRUE );
    BOOL IsChecked() const;

    void Toggle();
    void SetToggleHdl( const Link& rLink );
};

class EditImpl;
class TOOLKIT_DLLPUBLIC Edit : public Control
{
    DECL_GET_IMPL( Edit );
    DECL_CONSTRUCTORS( Edit, Control, WB_BORDER );
public:
    void SetText( const XubString& rStr ) const;
    XubString GetText() const;
    void SetModifyHdl( const Link& rLink );
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
    NumericField( Context *pCtx, const char *pId, sal_uInt32 nId = 0 );
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
    MetricField( Context *pCtx, const char *pId, sal_uInt32 nId = 0 );
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
    USHORT InsertEntry( const XubString& rStr, USHORT nPos = COMBOBOX_APPEND );
    void RemoveEntry( const XubString& rStr );
    void RemoveEntry( USHORT nPos );
    void Clear();

    USHORT GetEntryPos( const XubString& rStr ) const;
    XubString GetEntry( USHORT nPos ) const;
    USHORT GetEntryCount() const;

    void SetClickHdl( const Link& rLink );
    void SetSelectHdl( const Link& rLink );
};

#define LISTBOX_APPEND               ((USHORT)0xFFFF)
#define LISTBOX_ENTRY_NOTFOUND       ((USHORT)0xFFFF)
class ListBoxImpl;
class TOOLKIT_DLLPUBLIC ListBox : public Control
{
    DECL_GET_IMPL( ListBox );
    DECL_CONSTRUCTORS( ListBox, Control, WB_BORDER );
public:
    USHORT InsertEntry( const XubString& rStr, USHORT nPos = LISTBOX_APPEND );

    void RemoveEntry( const XubString& rStr );
    void RemoveEntry( USHORT nPos );
    void Clear();

    USHORT GetEntryPos( const XubString& rStr ) const;
    XubString GetEntry( USHORT nPos ) const;
    USHORT GetEntryCount() const;

    void SelectEntry( const XubString& rStr, BOOL bSelect = TRUE );
    void SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE );

    USHORT GetSelectEntryCount() const;
    XubString GetSelectEntry( USHORT nSelIndex = 0 ) const;
    USHORT GetSelectEntryPos( USHORT nSelIndex = 0 ) const;

    void SetSelectHdl( const Link& rLink );
    void SetClickHdl( const Link& rLink );
};

class DialogImpl;
class TOOLKIT_DLLPUBLIC Dialog : public Context, public Window
{
    DECL_GET_IMPL( Dialog );
    void SetParent( Window *pParent );
    void SetParent( ::Window *pParent );
public:
    Dialog( Window *pOptParent, const char *pXMLPath, const char *pId, sal_uInt32 nId = 0 );
    Dialog( ::Window *pOptParent, const char *pXMLPath, const char *pId, sal_uInt32 nId = 0 );
    short Execute();
    void EndDialog( long nResult = 0 );
    void SetText( const String& rStr );
};

// -----------------------------------------------------------------
//                 layout container / helper wrappers
// -----------------------------------------------------------------

class TOOLKIT_DLLPUBLIC Container
{
protected:
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
    Container( const rtl::OUString &rName, sal_Int32 nBorder );
public:
    Container( const Context *pCtx, const char *pId );

    void Add( Window *pWindow );
    void Add( Container *pContainer );

    void Remove( Window *pWindow );
    void Remove( Container *pContainer );
    void Clear();
    // we can't really do a GetChildren() as they don't have a common class,
    // besides we would need to keep track of children, uh

    void ShowAll( bool bVisible );

    css::uno::Reference< css::awt::XLayoutContainer > getImpl()
    { return mxContainer; }
};

class TOOLKIT_DLLPUBLIC Table : public Container
{
protected:
    Table( sal_Int32 nBorder, sal_Int32 nColumns );
public:
    Table( const Context *pCtx, const char *pId )
        : Container( pCtx, pId ) {}
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
    Box( const rtl::OUString &rName, sal_Int32 nBorder, bool bHomogeneous );
public:
    Box( const Context *pCtx, const char *pId )
        : Container( pCtx, pId ) {}
    void Add( Window *pWindow, bool bExpand, bool bFill, sal_Int32 nPadding);
    void Add( Container *pContainer, bool bExpand, bool bFill, sal_Int32 nPadding);

private:
    void setProps( css::uno::Reference< css::awt::XLayoutConstrains > xChild,
                   bool bXExpand, bool bYExpand, sal_Int32 nPadding );
};

class TOOLKIT_DLLPUBLIC HBox : public Box
{
public:
    HBox( sal_Int32 nBorder, bool bHomogeneous )
        : Box( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hbox" ) ), nBorder, bHomogeneous ) {}
};
class TOOLKIT_DLLPUBLIC VBox : public Box
{
public:
    VBox( sal_Int32 nBorder, bool bHomogeneous )
        : Box( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vbox" ) ), nBorder, bHomogeneous ) {}
};

} // end namespace layout

#endif /* ENABLE_LAYOUT */

#endif /* _LAYOUT_HXX */
