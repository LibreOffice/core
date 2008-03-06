#include "wrapper.hxx"

#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <cppuhelper/implbase1.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/event.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <list>

#include "layoutcore.hxx"

using namespace ::com::sun::star;

namespace layout
{

// Window/Control/Button
//class TOOLKIT_DLLPUBLIC ButtonImpl : public ControlImpl,
class ButtonImpl : public ControlImpl,
                   public ::cppu::WeakImplHelper1< awt::XActionListener >,
                   public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maClickHdl;
  protected:
    // we add toggle hooks here to cut on code
    Link maToggleHdl;
  public:
    uno::Reference< awt::XButton > mxButton;
    ButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
        , mxButton( xPeer, uno::UNO_QUERY )
    {
        mxButton->addActionListener( this );
    }

    ~ButtonImpl()
    {
    }

    virtual void Click() { /* make me pure virtual? */ };

    void SetClickHdl( const Link& rLink )
    {
        maClickHdl = rLink;
        if ( !rLink )
            mxButton->removeActionListener( this );
        else
            mxButton->addActionListener( this );
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    { mxButton.clear(); }

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        if ( !maClickHdl )
            Click();
        else
            maClickHdl.Call( static_cast<Window *>( mpWindow ) );
    }

    virtual  // HACK: doesn't need to be virtual... remove in future...
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        maToggleHdl.Call( static_cast<Window *>( mpWindow ) );
    }

    BOOL SetModeImage( css::uno::Reference< css::graphic::XGraphic > xGraph )
    {
        setProperty( "Graphic", uno::Any( xGraph ) );
        return true;
    }
};

void Button::SetText( const String& rStr )
{
    if ( !getImpl().mxButton.is() )
        return;
    getImpl().mxButton->setLabel( rStr );
}

void Button::SetClickHdl( const Link& rLink )
{
    getImpl().SetClickHdl( rLink );
}

BOOL Button::SetModeImage( const Image& rImage )
{
    return getImpl().SetModeImage( rImage.getImpl().mxGraphic );
}

void Button::SetImageAlign( ImageAlign eAlign )
{
    getImpl().setProperty( "ImageAlign", uno::Any( (sal_Int16) eAlign ) );
}

void Button::Click()
{
}

DECL_GET_IMPL_IMPL( Button )
DECL_CONSTRUCTOR_IMPLS( Button, Control, "button" );

// Window/Control/Button/PushButton
class PushButtonImpl : public ButtonImpl
{
  public:
    PushButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow ) {}

    void SetToggleHdl( const Link& rLink )
    {
        // XButton doesn't have an explicit event for Toggle. Anyway, it is a
        // superset of the clicks: all clicks, and explicit toggles
        maToggleHdl = rLink;
        if ( !rLink )
            mxButton->removeActionListener( this );
        else
            mxButton->addActionListener( this );
    }

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& rEvent )
        throw (css::uno::RuntimeException)
    {
        ButtonImpl::actionPerformed( rEvent );
        fireToggle();
    }

    inline void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }
};

void PushButton::Check( BOOL bCheck )
{
    getImpl().setProperty( "State", uno::Any( (sal_Int16) !!bCheck ) );
    // XButton doesn't have explicit toggle event
    getImpl().fireToggle();
}

BOOL PushButton::IsChecked() const
{
    return !!( getImpl().getProperty( "State" ).get< sal_Int16 >() );
}

void PushButton::Toggle()
{
    Check( true );
}

void PushButton::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( PushButton )
DECL_CONSTRUCTOR_IMPLS( PushButton, Button, "pushbutton" );

 // HACK: put every radio button into a group :/
static std::list< RadioButtonImpl*> mpRadioGroup;

// Window/Control/Button/RadioButton
class RadioButtonImpl : public ButtonImpl
{
public:
    uno::Reference< awt::XRadioButton > mxRadioButton;
    RadioButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow )
        , mxRadioButton( xPeer, uno::UNO_QUERY )
    {
        // TEMP:
        mpRadioGroup.push_back( this );
        mxRadioButton->addItemListener( this );
    }

    ~RadioButtonImpl()
    {
        //mxRadioButton->removeItemListener( this );
        mpRadioGroup.pop_front();
    }

    void Check( BOOL bCheck )
    {
        if ( !mxRadioButton.is() )
            return;
        mxRadioButton->setState( !!bCheck );
        fireToggle();

        if ( bCheck )
            unsetOthersGroup( this );
    }

    BOOL IsChecked()
    {
        if ( !mxRadioButton.is() )
            return FALSE;
        return mxRadioButton->getState();
    }

    static void unsetOthersGroup( RadioButtonImpl* current )
    {
        // set all others to false
        for( std::list< RadioButtonImpl*>::iterator it = mpRadioGroup.begin();
                 it != mpRadioGroup.end(); it++ )
        {
#if 0 // BREAKS in OOo
            if ( *it != current )
                (*it)->Check( false );
#else
            if ( *it != current && (*it)->IsChecked() )
            {
                (*it)->Check( false );
                return;
            }
#endif
        }
    }

    void SetToggleHdl( const Link& rLink )
    {
        maToggleHdl = rLink;
#if 0
        if ( !rLink )
            mxRadioButton->removeItemListener( this );
        else
            mxRadioButton->addItemListener( this );
#endif
    }

    inline void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }

//HACK: temp
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent )
        throw (css::uno::RuntimeException)
    {
        if ( !!maToggleHdl )
        ButtonImpl::itemStateChanged( rEvent );

        if ( IsChecked() )
            unsetOthersGroup( this );
    }
};

void RadioButton::Check( BOOL bCheck )
{
    getImpl().Check( bCheck );
}

BOOL RadioButton::IsChecked() const
{
    return getImpl().IsChecked();
}

void RadioButton::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( RadioButton )
DECL_CONSTRUCTOR_IMPLS( RadioButton, Button, "radiobutton" );

// Window/Control/Button/CheckBox
class CheckBoxImpl : public ButtonImpl
{
  public:
    uno::Reference< awt::XCheckBox > mxCheckBox;
    CheckBoxImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow )
        , mxCheckBox( xPeer, uno::UNO_QUERY ) {}

    void SetToggleHdl( const Link& rLink )
    {
        maToggleHdl = rLink;
        if ( !rLink )
            mxCheckBox->removeItemListener( this );
        else
            mxCheckBox->addItemListener( this );
    }
};

void CheckBox::Check( BOOL bCheck )
{
    if ( !getImpl().mxCheckBox.is() )
        return;
    getImpl().mxCheckBox->setState( !!bCheck );
}

BOOL CheckBox::IsChecked() const
{
    if ( !getImpl().mxCheckBox.is() )
        return FALSE;
    return getImpl().mxCheckBox->getState();
}

void CheckBox::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( CheckBox )
DECL_CONSTRUCTOR_IMPLS( CheckBox, Button, "checkbox" );

// Window/Control/Button/PushButton/etc
#define BUTTON_IMPL(t, parent, response) \
    class t##Impl : public parent##Impl \
    { \
    public: \
        t##Impl( Context *pCtx, PeerHandle const& xPeer, Window *pWindow ) \
            : parent##Impl( pCtx, xPeer, pWindow ) \
        { \
        } \
        void Click() \
        { \
            Dialog *d = static_cast<Dialog *>( mpCtx ); \
            d->EndDialog( response ); \
        } \
    }

/* Common button types currently unavailable in OOo: */
/* mpReset */
/* mpApply */
/* mpAction */
#define RET_RESET 6
#define RET_APPLY 7
#define BUTTONID_RESET RET_RESET
#define BUTTONID_APPLY RET_APPLY

BUTTON_IMPL( OKButton, PushButton, BUTTONID_OK );
BUTTON_IMPL( CancelButton, PushButton, BUTTONID_CANCEL );
BUTTON_IMPL( YesButton, PushButton, BUTTONID_YES );
BUTTON_IMPL( NoButton, PushButton, BUTTONID_NO );
BUTTON_IMPL( RetryButton, PushButton, BUTTONID_RETRY );
BUTTON_IMPL( IgnoreButton, PushButton, BUTTONID_IGNORE );
BUTTON_IMPL( ResetButton, PushButton, BUTTONID_RESET );
BUTTON_IMPL( ApplyButton, PushButton, BUTTONID_APPLY ); /* Deprecated? */
BUTTON_IMPL( HelpButton, PushButton, BUTTONID_HELP );

DECL_CONSTRUCTOR_IMPLS( OKButton, PushButton, "okbutton" );
DECL_CONSTRUCTOR_IMPLS( CancelButton, PushButton, "cancelbutton" );
DECL_CONSTRUCTOR_IMPLS( YesButton, PushButton, "yesbutton" );
DECL_CONSTRUCTOR_IMPLS( NoButton, PushButton, "nobutton" );
DECL_CONSTRUCTOR_IMPLS( RetryButton, PushButton, "retrybutton" );
DECL_CONSTRUCTOR_IMPLS( IgnoreButton, PushButton, "ignorebutton" );
DECL_CONSTRUCTOR_IMPLS( ResetButton, PushButton, "resetbutton" );
DECL_CONSTRUCTOR_IMPLS( ApplyButton, PushButton, "applybutton" );  /* Deprecated? */
DECL_CONSTRUCTOR_IMPLS( HelpButton, PushButton, "helpbutton" );


ImageImpl::ImageImpl( const char *pName )
    : mxGraphic( layoutimpl::loadGraphic( pName ) )
{
    if ( !mxGraphic.is() )
    {
        DBG_ERROR1( "ERROR: failed to load image: `%s'", pName );
    }

}

Image::Image( const char *pName )
    : pImpl( new ImageImpl( pName ) )
{
}

Image::~Image()
{
    delete pImpl;
}

}; // namespace layout
