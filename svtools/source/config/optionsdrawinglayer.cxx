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

// primitives
#define DEFAULT_ANTIALIASING                        true

// primitives
#define PROPERTYNAME_ANTIALIASING OUString("AntiAliasing")

// primitives
#define PROPERTYHANDLE_ANTIALIASING                 0

#define PROPERTYCOUNT                               1

class SvtOptionsDrawinglayer_Impl : public ConfigItem
{
public:
    SvtOptionsDrawinglayer_Impl();
    ~SvtOptionsDrawinglayer_Impl() override;

    virtual void Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

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

        // primitives
        bool        m_bAntiAliasing;

        // local values
        bool        m_bAllowAA : 1;
        bool        m_bAllowAAChecked : 1;
};

SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl() :
    ConfigItem( ROOTNODE_START  ),

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

Color SvtOptionsDrawinglayer::getHilightColor()
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
