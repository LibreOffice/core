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

#include <vcl/svapp.hxx>
#include <vcl/font.hxx>

#include <factory.hxx>
#include <svdata.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::awt;

namespace vcl
{

namespace {

class FontIdentificator : public ::cppu::WeakImplHelper< XMaterialHolder, XInitialization, XServiceInfo >
{
    Font        m_aFont;
public:
    FontIdentificator() {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& ) override;

    // XMaterialHolder
    virtual Any SAL_CALL getMaterial() override;

};

}

void SAL_CALL FontIdentificator::initialize( const Sequence<Any>& i_rArgs )
{
    if( !ImplGetSVData() )
        return; // VCL not initialized

    Sequence< sal_Int8 > aFontBuf;
    for( const auto& rArg : i_rArgs )
    {
        if( rArg >>= aFontBuf )
        {
            m_aFont = Font::identifyFont( aFontBuf.getConstArray(), aFontBuf.getLength() );
            break;
        }
    }
}

Any SAL_CALL FontIdentificator::getMaterial()
{
    if( !ImplGetSVData() )
        return Any(); // VCL not initialized

    FontDescriptor aFD;
    aFD.Name                = m_aFont.GetFamilyName();
    aFD.Height              = 0;
    aFD.Width               = 0;
    aFD.StyleName           = m_aFont.GetStyleName();
    aFD.CharSet             = 0;
    aFD.CharacterWidth      = 0;
    aFD.Underline           = 0;
    aFD.Strikeout           = 0;
    aFD.Orientation         = 0;
    aFD.Kerning             = false;
    aFD.WordLineMode        = false;
    aFD.Type                = 0;
    switch( m_aFont.GetFamilyType() )
    {
    case FAMILY_DECORATIVE: aFD.Family = css::awt::FontFamily::DECORATIVE;break;
    case FAMILY_MODERN: aFD.Family = css::awt::FontFamily::MODERN;break;
    case FAMILY_ROMAN: aFD.Family = css::awt::FontFamily::ROMAN;break;
    case FAMILY_SCRIPT: aFD.Family = css::awt::FontFamily::SCRIPT;break;
    case FAMILY_SWISS: aFD.Family = css::awt::FontFamily::SWISS;break;
    case FAMILY_SYSTEM: aFD.Family = css::awt::FontFamily::SYSTEM;break;
    default:
        aFD.Family = css::awt::FontFamily::DONTKNOW;
        break;
    }
    switch( m_aFont.GetPitch() )
    {
    case PITCH_VARIABLE: aFD.Pitch = css::awt::FontPitch::VARIABLE;break;
    case PITCH_FIXED: aFD.Pitch = css::awt::FontPitch::FIXED;break;
    default:
        aFD.Pitch = css::awt::FontPitch::DONTKNOW;
        break;
    }
    switch( m_aFont.GetWeight() )
    {
    case WEIGHT_THIN: aFD.Weight = css::awt::FontWeight::THIN;break;
    case WEIGHT_ULTRALIGHT: aFD.Weight = css::awt::FontWeight::ULTRALIGHT;break;
    case WEIGHT_LIGHT: aFD.Weight = css::awt::FontWeight::LIGHT;break;
    case WEIGHT_SEMILIGHT: aFD.Weight = css::awt::FontWeight::SEMILIGHT;break;
    case WEIGHT_MEDIUM:
    case WEIGHT_NORMAL: aFD.Weight = css::awt::FontWeight::NORMAL;break;
    case WEIGHT_SEMIBOLD: aFD.Weight = css::awt::FontWeight::SEMIBOLD;break;
    case WEIGHT_BOLD: aFD.Weight = css::awt::FontWeight::BOLD;break;
    case WEIGHT_ULTRABOLD: aFD.Weight = css::awt::FontWeight::ULTRABOLD;break;
    case WEIGHT_BLACK: aFD.Weight = css::awt::FontWeight::BLACK;break;
    default:
        aFD.Weight = css::awt::FontWeight::DONTKNOW;
        break;
    }
    switch( m_aFont.GetItalic() )
    {
    case ITALIC_OBLIQUE: aFD.Slant = css::awt::FontSlant_OBLIQUE;break;
    case ITALIC_NORMAL: aFD.Slant = css::awt::FontSlant_ITALIC;break;
    default:
        aFD.Slant = css::awt::FontSlant_DONTKNOW;
        break;
    }
    return Any( aFD );
}

// XServiceInfo
OUString SAL_CALL FontIdentificator::getImplementationName()
{
    return u"vcl::FontIdentificator"_ustr;
}

sal_Bool SAL_CALL FontIdentificator::supportsService( const OUString& i_rServiceName )
{
    return cppu::supportsService(this, i_rServiceName);
}

Sequence< OUString > SAL_CALL FontIdentificator::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.FontIdentificator"_ustr };
}

} // namespace vcl

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
vcl_FontIdentificator_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new vcl::FontIdentificator());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
