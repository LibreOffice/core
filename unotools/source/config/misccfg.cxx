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

#include <unotools/misccfg.hxx>
#include <rtl/instance.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include "itemholder1.hxx"

using namespace com::sun::star::uno;

namespace utl
{

static SfxMiscCfg* pOptions = nullptr;
static sal_Int32 nRefCount = 0;

class SfxMiscCfg : public utl::ConfigItem
{
private:
    bool            bPaperSize;     // printer warnings
    bool            bPaperOrientation;
    bool            bNotFound;
    sal_Int32       nYear2000;      // two digit year representation

    static const com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    void                    Load();

    virtual void            ImplCommit() override;

public:
    SfxMiscCfg( );
    virtual ~SfxMiscCfg( );

    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames) override;

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
    ConfigItem(OUString("Office.Common") ),
    bPaperSize(false),
    bPaperOrientation (false),
    bNotFound (false),
    nYear2000( 1930 )
{
    Load();
}

SfxMiscCfg::~SfxMiscCfg()
{
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
                    case  0: bPaperSize        = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;      //"Print/Warning/PaperSize",
                    case  1: bPaperOrientation = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;     //"Print/Warning/PaperOrientation",
                    case  2: bNotFound         = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;   //"Print/Warning/NotFound",
                    case  3: pValues[nProp] >>= nYear2000;break;                                    //"DateFormat/TwoDigitYear",
                }
            }
        }
    }
}

void SfxMiscCfg::Notify( const com::sun::star::uno::Sequence<OUString>& )
{
    Load();
}

void SfxMiscCfg::ImplCommit()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues(rNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    for(int nProp = 0; nProp < rNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp].setValue(&bPaperSize, rType);break;  //"Print/Warning/PaperSize",
            case  1: pValues[nProp].setValue(&bPaperOrientation, rType);break;     //"Print/Warning/PaperOrientation",
            case  2: pValues[nProp].setValue(&bNotFound, rType);break;   //"Print/Warning/NotFound",
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
    if ( !pOptions )
    {
        pOptions = new SfxMiscCfg;

        ItemHolder1::holdConfigItem(E_MISCCFG);
    }

    ++nRefCount;
    pImpl = pOptions;
    pImpl->AddListener(this);
}

MiscCfg::~MiscCfg( )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    pImpl->RemoveListener(this);
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

bool MiscCfg::IsNotFoundWarning()   const
{
    return pImpl->IsNotFoundWarning();
}

void MiscCfg::SetNotFoundWarning(   bool bSet)
{
    pImpl->SetNotFoundWarning( bSet );
}

bool MiscCfg::IsPaperSizeWarning()  const
{
    return pImpl->IsPaperSizeWarning();
}

void MiscCfg::SetPaperSizeWarning(bool bSet)
{
    pImpl->SetPaperSizeWarning( bSet );
}

bool MiscCfg::IsPaperOrientationWarning()   const
{
    return pImpl->IsPaperOrientationWarning();
}

void MiscCfg::SetPaperOrientationWarning(   bool bSet)
{
    pImpl->SetPaperOrientationWarning( bSet );
}

sal_Int32 MiscCfg::GetYear2000() const
{
    return pImpl->GetYear2000();
}

void MiscCfg::SetYear2000( sal_Int32 nSet )
{
    pImpl->SetYear2000( nSet );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
