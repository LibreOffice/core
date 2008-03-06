#ifndef LAYOUT_AWT_VCLXBUTTON_HXX
#define LAYOUT_AWT_VCLXBUTTON_HXX

#include "vclxproxy.hxx"

/* Replacements for broken toolkit/ impls. of ok, cancel, help button, etc. */

class IconButton : public VCLXProxy
{
protected:
    IconButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton,
                rtl::OUString aDefaultLabel, const char *pGraphName );
};

class VCLXOKButton : public IconButton
{
public:
    VCLXOKButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXCancelButton : public IconButton
{
public:
    VCLXCancelButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXYesButton : public IconButton
{
public:
    VCLXYesButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXNoButton : public IconButton
{
public:
    VCLXNoButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXRetryButton : public IconButton
{
public:
    VCLXRetryButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXIgnoreButton : public IconButton
{
public:
    VCLXIgnoreButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXResetButton : public IconButton
{
public:
    VCLXResetButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXApplyButton : public IconButton
{
public:
    VCLXApplyButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

class VCLXHelpButton : public IconButton
{
public:
    VCLXHelpButton( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xButton );
};

#endif // LAYOUT_AWT_VCLXBUTTON_HXX
