#include "vclxbutton.hxx"

#include "layoutcore.hxx"
#include <com/sun/star/awt/ImagePosition.hpp>
#include <vcl/button.hxx>

namespace css = com::sun::star;

IconButton::IconButton( css::uno::Reference< css::uno::XInterface > xButton,
                        rtl::OUString aDefaultLabel, const char *pGraphName )
    : VCLXProxy( xButton )
{
    setLabel( aDefaultLabel );
    setProperty( rtl::OUString::createFromAscii( "Graphic" ),
                 css::uno::Any( layoutimpl::loadGraphic( pGraphName ) ) );
    setProperty( rtl::OUString::createFromAscii( "ImagePosition" ),
                 css::uno::Any( css::awt::ImagePosition::LeftCenter ) );
    setProperty( rtl::OUString::createFromAscii( "Align" ),
                 css::uno::Any( (sal_Int16) 1 /* magic - center */ ) );
}

// FIXME: l10n/i18n of Reset & Apply

VCLXOKButton::VCLXOKButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_OK ),
                  "res/commandimagelist/sc_ok.png" )
{
}

VCLXCancelButton::VCLXCancelButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_CANCEL ),
//    : IconButton( xButton, rtl::OUString::createFromAscii( "~Cancel " ),
                  "res/commandimagelist/sc_cancel.png" )
{
}

VCLXYesButton::VCLXYesButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_YES ),
                  "res/commandimagelist/sc_yes.png" )
{
}

VCLXNoButton::VCLXNoButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_NO ),
                  "res/commandimagelist/sc_no.png" )
{
}

VCLXRetryButton::VCLXRetryButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_RETRY ),
                  "res/commandimagelist/sc_retry.png" )
{
}

VCLXIgnoreButton::VCLXIgnoreButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_IGNORE ),
                  "res/commandimagelist/sc_ignore.png" )
{
}

VCLXResetButton::VCLXResetButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, rtl::OUString::createFromAscii( "~Reset " ),
                  "res/commandimagelist/sc_reset.png" )
{
}

VCLXApplyButton::VCLXApplyButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, rtl::OUString::createFromAscii( "Apply" ),
                  "res/commandimagelist/sc_apply.png" )
{
}

VCLXHelpButton::VCLXHelpButton( css::uno::Reference< css::uno::XInterface > xButton )
    : IconButton( xButton, Button::GetStandardText( BUTTON_HELP ),
                  "res/commandimagelist/sc_help.png" )
{
}

namespace layoutimpl
{
css::uno::Reference< css::awt::XLayoutConstrains >
createInternalWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                      css::uno::Reference< css::uno::XInterface > xParent,
                      const rtl::OUString &rName, long nProps )
{
    css::uno::Reference< css::awt::XLayoutConstrains > xRef, xWrapped;
    bool bOK = false;
    bool bCancel = false;
    bool bYes = false;
    bool bNo = false;
    bool bRetry = false;
    bool bIgnore = false;
    bool bReset = false;
    bool bApply = false;
    bool bHelp = false;
    if ( ( bOK = rName.equalsAscii( "okbutton" ) )
         || ( bCancel = rName.equalsAscii( "cancelbutton" ) )
         || ( bYes = rName.equalsAscii( "yesbutton" ) )
         || ( bNo = rName.equalsAscii( "nobutton" ) )
         || ( bRetry = rName.equalsAscii( "retrybutton" ) )
         || ( bIgnore = rName.equalsAscii( "ignorebutton" ) )
         || ( bReset = rName.equalsAscii( "resetbutton" ) )
         || ( bApply = rName.equalsAscii( "applybutton" ) )
         || ( bHelp = rName.equalsAscii( "helpbutton" ) ) )
    {
        xWrapped = createWidget( xToolkit, xParent,
                                 rtl::OUString::createFromAscii( "pushbutton" ),
                                 nProps );
        if ( bOK )
            xRef = new VCLXOKButton( xWrapped );
        if ( bCancel )
            xRef = new VCLXCancelButton( xWrapped );
        if ( bYes )
            xRef = new VCLXYesButton( xWrapped );
        if ( bNo )
            xRef = new VCLXNoButton( xWrapped );
        if ( bRetry )
            xRef = new VCLXRetryButton( xWrapped );
        if ( bIgnore )
            xRef = new VCLXIgnoreButton( xWrapped );
        if ( bReset )
            xRef = new VCLXResetButton( xWrapped );
        if ( bApply )
            xRef = new VCLXApplyButton( xWrapped );
        if ( bHelp )
            xRef = new VCLXHelpButton( xWrapped );
    }
    return xRef;
}
};
