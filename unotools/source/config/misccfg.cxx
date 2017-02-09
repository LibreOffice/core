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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <unotools/misccfg.hxx>
#include <rtl/instance.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include "itemholder1.hxx"

using namespace com::sun::star::uno;

namespace utl
{
class SfxMiscCfg;

static std::weak_ptr<SfxMiscCfg> g_pOptions;

class SfxMiscCfg : public utl::ConfigItem
{
private:
    bool            bPaperSize;     // printer warnings
    bool            bPaperOrientation;
    bool            bNotFound;
    sal_Int32       nYear2000;      // two digit year representation

    static const css::uno::Sequence<OUString> GetPropertyNames();
    void                    Load();

    virtual void            ImplCommit() final override;

public:
    SfxMiscCfg( );
    virtual ~SfxMiscCfg( ) override;

    virtual void            Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    bool        IsNotFoundWarning()     const {return bNotFound;}
    void        SetNotFoundWarning( bool bSet);

    bool        IsPaperSizeWarning()    const {return bPaperSize;}
    void        SetPaperSizeWarning(bool bSet);

    bool        IsPaperOrientationWarning()     const {return bPaperOrientation;}
    void        SetPaperOrientationWarning( bool bSet);

                // 0 ... 99
    sal_Int32   GetYear2000()           const { return nYear2000; }
    void        SetYear2000( sal_Int32 nSet );

};

SfxMiscCfg::SfxMiscCfg() :
    ConfigItem( "Office.Common" ),
    bPaperSize(false),
    bPaperOrientation (false),
    bNotFound (false),
    nYear2000( 1930 )
{
    Load();
}

SfxMiscCfg::~SfxMiscCfg()
{
    if ( IsModified() )
        Commit();
}

void SfxMiscCfg::SetNotFoundWarning( bool bSet)
{
    if(bNotFound != bSet)
        SetModified();
    bNotFound = bSet;
}

void SfxMiscCfg::SetPaperSizeWarning( bool bSet)
{
    if(bPaperSize != bSet)
        SetModified();
    bPaperSize = bSet;
}

void SfxMiscCfg::SetPaperOrientationWarning( bool bSet)
{
    if(bPaperOrientation != bSet)
        SetModified();
    bPaperOrientation = bSet;
}

void SfxMiscCfg::SetYear2000( sal_Int32 nSet )
{
    if(nYear2000 != nSet)
        SetModified();
    nYear2000 = nSet;
}

const Sequence<OUString> SfxMiscCfg::GetPropertyNames()
{
    const OUString pProperties[] =
    {
        OUString("Print/Warning/PaperSize"),
        OUString("Print/Warning/PaperOrientation"),
        OUString("Print/Warning/NotFound"),
        OUString("DateFormat/TwoDigitYear")
    };
    const Sequence< OUString > seqPropertyNames( pProperties, 4 );
    return seqPropertyNames;
}

void SfxMiscCfg::Load()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    EnableNotification(rNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == rNames.getLength())
    {
        for(int nProp = 0; nProp < rNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: bPaperSize        = *o3tl::doAccess<bool>(pValues[nProp]); break;      //"Print/Warning/PaperSize",
                    case  1: bPaperOrientation = *o3tl::doAccess<bool>(pValues[nProp]);  break;     //"Print/Warning/PaperOrientation",
                    case  2: bNotFound         = *o3tl::doAccess<bool>(pValues[nProp]);  break;   //"Print/Warning/NotFound",
                    case  3: pValues[nProp] >>= nYear2000;break;                                    //"DateFormat/TwoDigitYear",
                }
            }
        }
    }
}

void SfxMiscCfg::Notify( const css::uno::Sequence<OUString>& )
{
    Load();
}

void SfxMiscCfg::ImplCommit()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues(rNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < rNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= bPaperSize;break;  //"Print/Warning/PaperSize",
            case  1: pValues[nProp] <<= bPaperOrientation;break;     //"Print/Warning/PaperOrientation",
            case  2: pValues[nProp] <<= bNotFound;break;   //"Print/Warning/NotFound",
            case  3: pValues[nProp] <<= nYear2000;break;                 //"DateFormat/TwoDigitYear",
        }
    }
    PutProperties(rNames, aValues);
}

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

MiscCfg::MiscCfg( )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    m_pImpl = g_pOptions.lock();
    if ( !m_pImpl )
    {
        m_pImpl = std::make_shared<SfxMiscCfg>();
        g_pOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::MiscConfig);
    }

    m_pImpl->AddListener(this);
}

MiscCfg::~MiscCfg( )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    m_pImpl->RemoveListener(this);
    m_pImpl.reset();
}

bool MiscCfg::IsNotFoundWarning()   const
{
    return m_pImpl->IsNotFoundWarning();
}

void MiscCfg::SetNotFoundWarning(   bool bSet)
{
    m_pImpl->SetNotFoundWarning( bSet );
}

bool MiscCfg::IsPaperSizeWarning()  const
{
    return m_pImpl->IsPaperSizeWarning();
}

void MiscCfg::SetPaperSizeWarning(bool bSet)
{
    m_pImpl->SetPaperSizeWarning( bSet );
}

bool MiscCfg::IsPaperOrientationWarning()   const
{
    return m_pImpl->IsPaperOrientationWarning();
}

void MiscCfg::SetPaperOrientationWarning(   bool bSet)
{
    m_pImpl->SetPaperOrientationWarning( bSet );
}

sal_Int32 MiscCfg::GetYear2000() const
{
    return m_pImpl->GetYear2000();
}

void MiscCfg::SetYear2000( sal_Int32 nSet )
{
    m_pImpl->SetYear2000( nSet );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
