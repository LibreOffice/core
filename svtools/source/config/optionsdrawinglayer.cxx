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

#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>

//  namespaces

using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_START                  "Office.Common/Drawinglayer"
#define DEFAULT_STRIPE_COLOR_A          Color(0)
#define DEFAULT_STRIPE_COLOR_B          Color(16581375)
#define DEFAULT_STRIPE_LENGTH           4

// #i4219#
#define DEFAULT_MAXIMUMPAPERWIDTH           600
#define DEFAULT_MAXIMUMPAPERHEIGHT          600
#define DEFAULT_MAXIMUMPAPERLEFTMARGIN      9999
#define DEFAULT_MAXIMUMPAPERRIGHTMARGIN     9999
#define DEFAULT_MAXIMUMPAPERTOPMARGIN       9999
#define DEFAULT_MAXIMUMPAPERBOTTOMMARGIN    9999

// primitives
#define DEFAULT_ANTIALIASING                        true

#define PROPERTYNAME_STRIPE_COLOR_A     OUString("StripeColorA"     )
#define PROPERTYNAME_STRIPE_COLOR_B     OUString("StripeColorB"     )
#define PROPERTYNAME_STRIPE_LENGTH      OUString("StripeLength"     )

// #i4219#
#define PROPERTYNAME_MAXIMUMPAPERWIDTH OUString("MaximumPaperWidth")
#define PROPERTYNAME_MAXIMUMPAPERHEIGHT OUString("MaximumPaperHeight")
#define PROPERTYNAME_MAXIMUMPAPERLEFTMARGIN OUString("MaximumPaperLeftMargin")
#define PROPERTYNAME_MAXIMUMPAPERRIGHTMARGIN OUString("MaximumPaperRightMargin")
#define PROPERTYNAME_MAXIMUMPAPERTOPMARGIN OUString("MaximumPaperTopMargin")
#define PROPERTYNAME_MAXIMUMPAPERBOTTOMMARGIN OUString("MaximumPaperBottomMargin")

// primitives
#define PROPERTYNAME_ANTIALIASING OUString("AntiAliasing")

#define PROPERTYHANDLE_STRIPE_COLOR_A               0
#define PROPERTYHANDLE_STRIPE_COLOR_B               1
#define PROPERTYHANDLE_STRIPE_LENGTH                2

// #i4219#
#define PROPERTYHANDLE_MAXIMUMPAPERWIDTH            3
#define PROPERTYHANDLE_MAXIMUMPAPERHEIGHT           4
#define PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN       5
#define PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN      6
#define PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN        7
#define PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN     8

// primitives
#define PROPERTYHANDLE_ANTIALIASING                 9

#define PROPERTYCOUNT                               10

class SvtOptionsDrawinglayer_Impl : public ConfigItem
{
public:
    SvtOptionsDrawinglayer_Impl();
    ~SvtOptionsDrawinglayer_Impl() override;

    virtual void Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    const Color& GetStripeColorA() const { return m_bStripeColorA;}
    const Color& GetStripeColorB() const { return m_bStripeColorB;}
    sal_uInt16  GetStripeLength() const { return m_nStripeLength;}

    // #i4219#
    sal_uInt32  GetMaximumPaperWidth() const { return m_nMaximumPaperWidth;}
    sal_uInt32  GetMaximumPaperHeight() const { return m_nMaximumPaperHeight;}
    sal_uInt32  GetMaximumPaperLeftMargin() const { return m_nMaximumPaperLeftMargin;}
    sal_uInt32  GetMaximumPaperRightMargin() const { return m_nMaximumPaperRightMargin;}
    sal_uInt32  GetMaximumPaperTopMargin() const { return m_nMaximumPaperTopMargin;}
    sal_uInt32  GetMaximumPaperBottomMargin() const { return m_nMaximumPaperBottomMargin;}

    // helper
    bool        IsAAPossibleOnThisSystem() const;

    // primitives
    bool        IsAntiAliasing() const { return m_bAntiAliasing;}

    void        SetAntiAliasing( bool bState );

//  private methods

private:
    virtual void ImplCommit() final override;

    static Sequence< OUString > impl_GetPropertyNames();

//  private member

private:

        Color       m_bStripeColorA;
        Color       m_bStripeColorB;
        sal_uInt16  m_nStripeLength;

        // #i4219#
        sal_uInt32  m_nMaximumPaperWidth;
        sal_uInt32  m_nMaximumPaperHeight;
        sal_uInt32  m_nMaximumPaperLeftMargin;
        sal_uInt32  m_nMaximumPaperRightMargin;
        sal_uInt32  m_nMaximumPaperTopMargin;
        sal_uInt32  m_nMaximumPaperBottomMargin;

        // primitives
        bool        m_bAntiAliasing;

        // local values
        bool        m_bAllowAA : 1;
        bool        m_bAllowAAChecked : 1;
};

SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl() :
    ConfigItem( ROOTNODE_START  ),
    m_bStripeColorA(DEFAULT_STRIPE_COLOR_A),
    m_bStripeColorB(DEFAULT_STRIPE_COLOR_B),
    m_nStripeLength(DEFAULT_STRIPE_LENGTH),

    // #i4219#
    m_nMaximumPaperWidth(DEFAULT_MAXIMUMPAPERWIDTH),
    m_nMaximumPaperHeight(DEFAULT_MAXIMUMPAPERHEIGHT),
    m_nMaximumPaperLeftMargin(DEFAULT_MAXIMUMPAPERLEFTMARGIN),
    m_nMaximumPaperRightMargin(DEFAULT_MAXIMUMPAPERRIGHTMARGIN),
    m_nMaximumPaperTopMargin(DEFAULT_MAXIMUMPAPERTOPMARGIN),
    m_nMaximumPaperBottomMargin(DEFAULT_MAXIMUMPAPERBOTTOMMARGIN),

    // primitives
    m_bAntiAliasing(DEFAULT_ANTIALIASING),

    // local values
    m_bAllowAA(true),
    m_bAllowAAChecked(false)
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for(sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;

        switch( nProperty )
        {
            case PROPERTYHANDLE_STRIPE_COLOR_A:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorA\"?" );
                sal_Int32 nValue = 0;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorA = Color(nValue);
            }
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorB\"?" );
                sal_Int32 nValue = 0;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorB = Color(nValue);
            }
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeLength\"?" );
                seqValues[nProperty] >>= m_nStripeLength;
            }
            break;

            // #i4219#
            case PROPERTYHANDLE_MAXIMUMPAPERWIDTH:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperWidth\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperWidth;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERHEIGHT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperHeight\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperHeight;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperLeftMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperLeftMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperRightMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperRightMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperTopMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperTopMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperBottomMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperBottomMargin;
            }
            break;

            // primitives
            case PROPERTYHANDLE_ANTIALIASING:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\AntiAliasing\"?" );
                seqValues[nProperty] >>= m_bAntiAliasing;
            }
            break;

        }
    }
}

SvtOptionsDrawinglayer_Impl::~SvtOptionsDrawinglayer_Impl()
{
        if (IsModified())
            Commit();
}

//  Commit

void SvtOptionsDrawinglayer_Impl::ImplCommit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_STRIPE_COLOR_A:
                aSeqValues[nProperty] <<= m_bStripeColorA;
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
                aSeqValues[nProperty] <<= m_bStripeColorB;
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
                aSeqValues[nProperty] <<= m_nStripeLength;
            break;

            // #i4219#
            case PROPERTYHANDLE_MAXIMUMPAPERWIDTH:
                aSeqValues[nProperty] <<= m_nMaximumPaperWidth;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERHEIGHT:
                aSeqValues[nProperty] <<= m_nMaximumPaperHeight;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperLeftMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperRightMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperTopMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperBottomMargin;
            break;

            // primitives
            case PROPERTYHANDLE_ANTIALIASING:
                aSeqValues[nProperty] <<= m_bAntiAliasing;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

void SvtOptionsDrawinglayer_Impl::Notify( const css::uno::Sequence<OUString>& )
{
}
// #i73602#

// #i74769#, #i75172#

// #i4219#

// helper
bool SvtOptionsDrawinglayer_Impl::IsAAPossibleOnThisSystem() const
{
    if(!m_bAllowAAChecked)
    {
        SvtOptionsDrawinglayer_Impl* pThat = const_cast< SvtOptionsDrawinglayer_Impl* >(this);
        pThat->m_bAllowAAChecked = true;

#ifdef _WIN32
        // WIN32 uses GDIPlus with VCL for the first incarnation; this will be enhanced
        // in the future to use canvases and the canvas renderer, thus an AA-abled
        // canvas needs to be checked here in the future.
        // Currently, just allow AA for WIN32
#endif

        // check XRenderExtension
        if(m_bAllowAA && !Application::GetDefaultDevice()->SupportsOperation( OutDevSupportType::TransparentRect ))
        {
            pThat->m_bAllowAA = false;
        }
    }

    return m_bAllowAA;
}

// primitives

void SvtOptionsDrawinglayer_Impl::SetAntiAliasing( bool bState )
{
    if(m_bAntiAliasing != bState)
    {
        m_bAntiAliasing = bState;
        SetModified();
    }
}

//  private method

Sequence< OUString > SvtOptionsDrawinglayer_Impl::impl_GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_STRIPE_COLOR_A     ,
        PROPERTYNAME_STRIPE_COLOR_B     ,
        PROPERTYNAME_STRIPE_LENGTH      ,

        // #i4219#
        PROPERTYNAME_MAXIMUMPAPERWIDTH,
        PROPERTYNAME_MAXIMUMPAPERHEIGHT,
        PROPERTYNAME_MAXIMUMPAPERLEFTMARGIN,
        PROPERTYNAME_MAXIMUMPAPERRIGHTMARGIN,
        PROPERTYNAME_MAXIMUMPAPERTOPMARGIN,
        PROPERTYNAME_MAXIMUMPAPERBOTTOMMARGIN,

        // primitives
        PROPERTYNAME_ANTIALIASING,
    };

    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

namespace {
    //global
    std::weak_ptr<SvtOptionsDrawinglayer_Impl> g_pOptionsDrawinglayer;
}

SvtOptionsDrawinglayer::SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl = g_pOptionsDrawinglayer.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtOptionsDrawinglayer_Impl>();
        g_pOptionsDrawinglayer = m_pImpl;
    }
}

SvtOptionsDrawinglayer::~SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

Color SvtOptionsDrawinglayer::GetStripeColorA() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeColorA();
}

//  public method

Color SvtOptionsDrawinglayer::GetStripeColorB() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeColorB();
}

//  public method

sal_uInt16 SvtOptionsDrawinglayer::GetStripeLength() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeLength();
}

// #i4219#
sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperWidth() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperWidth();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperHeight() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperHeight();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperLeftMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperLeftMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperRightMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperRightMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperTopMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperTopMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperBottomMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperBottomMargin();
}

// helper
bool SvtOptionsDrawinglayer::IsAAPossibleOnThisSystem() const
{
    return m_pImpl->IsAAPossibleOnThisSystem();
}

// primitives
bool SvtOptionsDrawinglayer::IsAntiAliasing() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsAntiAliasing() && IsAAPossibleOnThisSystem();
}

void SvtOptionsDrawinglayer::SetAntiAliasing( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetAntiAliasing( bState );
}

/**
   Specifies the maximum allowed luminance the system's selection
   color may have. When the colorfetched from the system is brighter
   (luminance is bigger), it will be scaled to a luminance of exactly
   this given value.
*/
constexpr double fSelectionMaximumLuminancePercent = 70 / 100.0;

Color SvtOptionsDrawinglayer::getHilightColor() const
{
    Color aRetval(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    const basegfx::BColor aSelection(aRetval.getBColor());
    const double fLuminance(aSelection.luminance());

    if(fLuminance > fSelectionMaximumLuminancePercent)
    {
        const double fFactor(fSelectionMaximumLuminancePercent / fLuminance);
        const basegfx::BColor aNewSelection(
            aSelection.getRed() * fFactor,
            aSelection.getGreen() * fFactor,
            aSelection.getBlue() * fFactor);

        aRetval = Color(aNewSelection);
    }

    return aRetval;
}

namespace
{
    class theOptionsDrawinglayerMutex : public rtl::Static<osl::Mutex, theOptionsDrawinglayerMutex>{};
}

//  private method

Mutex& SvtOptionsDrawinglayer::GetOwnStaticMutex()
{
    return theOptionsDrawinglayerMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
