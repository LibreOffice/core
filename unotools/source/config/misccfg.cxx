/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/misccfg.hxx>
#include "rtl/instance.hxx"
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

#define DEFAULT_TAB 2000

#define DEF_INCH    2540L
#define DEF_RELTWIP 1440L

using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

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

    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    void                    Load();

public:
    SfxMiscCfg( );
    ~SfxMiscCfg( );

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
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

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
SfxMiscCfg::SfxMiscCfg() :
    ConfigItem(C2U("Office.Common") ),
    bPaperSize(sal_False),
    bPaperOrientation (sal_False),
    bNotFound (sal_False),
    nYear2000( 1930 )
{
    RTL_LOGFILE_CONTEXT(aLog, "svl SfxMiscCfg::SfxMiscCfg()");

    Load();
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
SfxMiscCfg::~SfxMiscCfg()
{
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

void SfxMiscCfg::SetNotFoundWarning( sal_Bool bSet)
{
    if(bNotFound != bSet)
        SetModified();
    bNotFound = bSet;
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

void SfxMiscCfg::SetPaperSizeWarning( sal_Bool bSet)
{
    if(bPaperSize != bSet)
        SetModified();
    bPaperSize = bSet;
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
void SfxMiscCfg::SetPaperOrientationWarning( sal_Bool bSet)
{
    if(bPaperOrientation != bSet)
        SetModified();
    bPaperOrientation = bSet;
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

void SfxMiscCfg::SetYear2000( sal_Int32 nSet )
{
    if(nYear2000 != nSet)
        SetModified();
    nYear2000 = nSet;
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence<OUString>& SfxMiscCfg::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
               "Print/Warning/PaperSize",               //  0
               "Print/Warning/PaperOrientation",        //  1
               "Print/Warning/NotFound",                //  2
            "DateFormat/TwoDigitYear",              //  3
        };
        const int nCount = 4;
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
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
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& )
{
    Load();
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp].setValue(&bPaperSize, rType);break;  //"Print/Warning/PaperSize",
            case  1: pValues[nProp].setValue(&bPaperOrientation, rType);break;     //"Print/Warning/PaperOrientation",
            case  2: pValues[nProp].setValue(&bNotFound, rType);break;   //"Print/Warning/NotFound",
            case  3: pValues[nProp] <<= nYear2000;break;                 //"DateFormat/TwoDigitYear",
        }
    }
    PutProperties(aNames, aValues);
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
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) SfxMiscCfg::ctor()");
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

