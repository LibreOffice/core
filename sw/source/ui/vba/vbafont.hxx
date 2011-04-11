/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef VBA_FONT_HXX
#define VBA_FONT_HXX

#include <vbahelper/vbafontbase.hxx>
#include <ooo/vba/word/XFont.hpp>

typedef cppu::ImplInheritanceHelper1< VbaFontBase, ov::word::XFont > SwVbaFont_BASE;

class SwVbaFont : public SwVbaFont_BASE
{
public:
    SwVbaFont( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPalette, css::uno::Reference< css::beans::XPropertySet > xPropertySet ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaFont(){}

    // Attributes
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSubscript() throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getSuperscript() throw ( css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
