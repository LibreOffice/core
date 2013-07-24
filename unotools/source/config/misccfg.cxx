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
#include "rtl/instance.hxx"
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include "itemholder1.hxx"

using namespace com::sun::star::uno;


namespace utl
{

static SfxMiscCfg* pOptions = NULL;
static sal_Int32 nRefCount = 0;

class SfxMiscCfg : public utl::ConfigItem
{
    sal_Bool            bPaperSize;     // printer warnings
    sal_Bool            bPaperOrientation;
    sal_Bool            bNotFound;
    sal_Int32       nYear2000;      // two digit year representation

    const com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    void                    Load();

public:
    SfxMiscCfg( );
    ~SfxMiscCfg( );

    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames);
    virtual void            Commit();

    sal_Bool        IsNotFoundWarning()     const {return bNotFound;}
    void        SetNotFoundWarning( sal_Bool bSet);

    sal_Bool        IsPaperSizeWarning()    const {return bPaperSize;}
    void        SetPaperSizeWarning(sal_Bool bSet);

    sal_Bool        IsPaperOrientationWarning()     const {return bPaperOrientation;}
    void        SetPaperOrientationWarning( sal_Bool bSet);

                // 0 ... 99
    sal_Int32   GetYear2000()           const { return nYear2000; }
    void        SetYear2000( sal_Int32 nSet );

};

SfxMiscCfg::SfxMiscCfg() :
    ConfigItem(OUString("Office.Common") ),
    bPaperSize(sal_False),
    bPaperOrientation (sal_False),
    bNotFound (sal_False),
    nYear2000( 1930 )
{
    Load();
}

SfxMiscCfg::~SfxMiscCfg()
{
}

void SfxMiscCfg::SetNotFoundWarning( sal_Bool bSet)
{
    if(bNotFound != bSet)
        SetModified();
    bNotFound = bSet;
}

void SfxMiscCfg::SetPaperSizeWarning( sal_Bool bSet)
{
    if(bPaperSize != bSet)
        SetModified();
    bPaperSize = bSet;
}

void SfxMiscCfg::SetPaperOrientationWarning( sal_Bool bSet)
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
                    case  0: bPaperSize        = *(sal_Bool*)pValues[nProp].getValue(); break;      //"Print/Warning/PaperSize",
                    case  1: bPaperOrientation = *(sal_Bool*)pValues[nProp].getValue();  break;     //"Print/Warning/PaperOrientation",
                    case  2: bNotFound         = *(sal_Bool*)pValues[nProp].getValue()  ;  break;   //"Print/Warning/NotFound",
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

void SfxMiscCfg::Commit()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues(rNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
// -----------------------------------------------------------------------
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

sal_Bool MiscCfg::IsNotFoundWarning()   const
{
    return pImpl->IsNotFoundWarning();
}

void MiscCfg::SetNotFoundWarning(   sal_Bool bSet)
{
    pImpl->SetNotFoundWarning( bSet );
}

sal_Bool MiscCfg::IsPaperSizeWarning()  const
{
    return pImpl->IsPaperSizeWarning();
}

void MiscCfg::SetPaperSizeWarning(sal_Bool bSet)
{
    pImpl->SetPaperSizeWarning( bSet );
}

sal_Bool MiscCfg::IsPaperOrientationWarning()   const
{
    return pImpl->IsPaperOrientationWarning();
}

void MiscCfg::SetPaperOrientationWarning(   sal_Bool bSet)
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
