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


#include "searchopt.hxx"

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#include <rtl/logfile.hxx>


using namespace rtl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::i18n;

namespace binfilter
{

#define MAX_FLAGS_OFFSET	24

//////////////////////////////////////////////////////////////////////


class SvtSearchOptions_Impl : public ConfigItem
{
    INT32	nFlags;
    BOOL	bModified;

    // disallow copy-constructor and assignment-operator for now
    SvtSearchOptions_Impl( const SvtSearchOptions_Impl & );
    SvtSearchOptions_Impl & operator = ( const SvtSearchOptions_Impl & );

protected:
    BOOL			IsModified() const { return bModified; }
    using ConfigItem::SetModified;
    void			SetModified( BOOL bVal );
    BOOL			Load();
    BOOL			Save();

    Sequence< OUString >	GetPropertyNames() const;

public:
    SvtSearchOptions_Impl();
    virtual ~SvtSearchOptions_Impl();

    // ConfigItem
    virtual void	Commit();
    void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    BOOL			GetFlag( USHORT nOffset ) const;
    void			SetFlag( USHORT nOffset, BOOL bVal );
};



SvtSearchOptions_Impl::SvtSearchOptions_Impl() :
    ConfigItem( OUString::createFromAscii( "Office.Common/SearchOptions" ) )
{
    RTL_LOGFILE_CONTEXT(aLog, "svtools SvtSearchOptions_Impl::SvtSearchOptions_Impl()");
    nFlags = 0x0003FFFF;	// set all options values to 'true'
    Load();
    SetModified( FALSE );
}


SvtSearchOptions_Impl::~SvtSearchOptions_Impl()
{
    Commit();
}

    
void SvtSearchOptions_Impl::Commit()
{
    if (IsModified())
        Save();
}

    void SvtSearchOptions_Impl::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

BOOL SvtSearchOptions_Impl::GetFlag( USHORT nOffset ) const
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    return ((nFlags >> nOffset) & 0x01) ? TRUE : FALSE;
}


void SvtSearchOptions_Impl::SetFlag( USHORT nOffset, BOOL bVal )
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    INT32 nOldFlags = nFlags;
    INT32 nMask = ((INT32) 1)  << nOffset;
    if (bVal)
        nFlags |= nMask;
    else
        nFlags &= ~nMask;
    if (nFlags != nOldFlags)
        SetModified( TRUE );
}

    
void SvtSearchOptions_Impl::SetModified( BOOL bVal )
{
    bModified = bVal;
    if (bModified)
    {
        ConfigItem::SetModified();
    }
}


Sequence< OUString > SvtSearchOptions_Impl::GetPropertyNames() const
{
    static const char* aPropNames[ MAX_FLAGS_OFFSET + 1 ] =
    {
        "IsWholeWordsOnly",						//  0
        "IsBackwards",							//  1
        "IsUseRegularExpression",				//  2
        //"IsCurrentSelectionOnly",				// interactively set or not...
        "IsSearchForStyles",					//  3
        "IsSimilaritySearch",					//  4
        "IsUseAsianOptions",					//  5
        "IsMatchCase",							//  6
        "Japanese/IsMatchFullHalfWidthForms",	//  7
        "Japanese/IsMatchHiraganaKatakana",		//  8
        "Japanese/IsMatchContractions",			//  9
        "Japanese/IsMatchMinusDashCho-on",		// 10
        "Japanese/IsMatchRepeatCharMarks",		// 11
        "Japanese/IsMatchVariantFormKanji",		// 12
        "Japanese/IsMatchOldKanaForms",			// 13
        "Japanese/IsMatch_DiZi_DuZu",			// 14
        "Japanese/IsMatch_BaVa_HaFa",			// 15
        "Japanese/IsMatch_TsiThiChi_DhiZi",		// 16
        "Japanese/IsMatch_HyuIyu_ByuVyu",		// 17
        "Japanese/IsMatch_SeShe_ZeJe",			// 18
        "Japanese/IsMatch_IaIya",				// 19
        "Japanese/IsMatch_KiKu",				// 20
        "Japanese/IsIgnorePunctuation",			// 21
        "Japanese/IsIgnoreWhitespace",			// 22
        "Japanese/IsIgnoreProlongedSoundMark",	// 23
        "Japanese/IsIgnoreMiddleDot"			// 24
    };

    const int nCount = sizeof( aPropNames ) / sizeof( aPropNames[0] );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for (INT32 i = 0;  i < nCount;  ++i)
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}


BOOL SvtSearchOptions_Impl::Load()
{
    BOOL bSucc = FALSE;
    
    Sequence< OUString > aNames = GetPropertyNames();
    INT32 nProps = aNames.getLength();
    
    const Sequence< Any > aValues = GetProperties( aNames );
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), 
            "GetProperties failed" );
    //EnableNotification( aNames );
    
    if (nProps  &&  aValues.getLength() == nProps)
    {
        bSucc = TRUE;
        
        const Any* pValues = aValues.getConstArray();
        for (USHORT i = 0;  i < nProps;  ++i)
        {
            const Any &rVal = pValues[i];
            DBG_ASSERT( rVal.hasValue(), "property value missing" );
            if (rVal.hasValue())
            {
                BOOL bVal = BOOL();
                if (rVal >>= bVal)
                {
                    if (i <= MAX_FLAGS_OFFSET)
                    {
                        // use index in sequence as flag index
                        SetFlag( i, bVal );
                    }
                    else
                    {
                        DBG_ERROR( "unexpected index" );
                    }
                }
                else
                {
                    DBG_ERROR( "unexpected type" );
                    bSucc = FALSE;
                }
            }
            else
            {
                DBG_ERROR( "value missing" );
                bSucc = FALSE;
            }
        }
    }
    DBG_ASSERT( bSucc, "LoadConfig failed" );
    
    return bSucc;
}


BOOL SvtSearchOptions_Impl::Save()
{
    BOOL bSucc = FALSE;
    
    const Sequence< OUString > aNames = GetPropertyNames();
    INT32 nProps = aNames.getLength();
    
    Sequence< Any > aValues( nProps );
    Any *pValue = aValues.getArray();

    DBG_ASSERT( nProps == MAX_FLAGS_OFFSET + 1,
            "unexpected size of index" );
    if (nProps  &&  nProps == MAX_FLAGS_OFFSET + 1)
    {
        for (USHORT i = 0;  i < nProps;  ++i)
            pValue[i] <<= (BOOL) GetFlag(i);
        bSucc |= PutProperties( aNames, aValues );
    }

    if (bSucc)
        SetModified( FALSE );

    return bSucc;
}


//////////////////////////////////////////////////////////////////////

SvtSearchOptions::SvtSearchOptions()
{
    pImpl = new SvtSearchOptions_Impl;
}


SvtSearchOptions::~SvtSearchOptions()
{
    delete pImpl;
}

BOOL SvtSearchOptions::IsWholeWordsOnly() const
{
    return pImpl->GetFlag( 0 );
}


BOOL SvtSearchOptions::IsBackwards() const
{
    return pImpl->GetFlag( 1 );
}

BOOL SvtSearchOptions::IsUseRegularExpression() const
{
    return pImpl->GetFlag( 2 );
}

BOOL SvtSearchOptions::IsSimilaritySearch() const
{
    return pImpl->GetFlag( 4 );
}

BOOL SvtSearchOptions::IsUseAsianOptions() const
{
    return pImpl->GetFlag( 5 );
}

BOOL SvtSearchOptions::IsMatchCase() const
{
    return pImpl->GetFlag( 6 );
}

BOOL SvtSearchOptions::IsMatchFullHalfWidthForms() const
{
    return pImpl->GetFlag( 7 );
}

BOOL SvtSearchOptions::IsMatchHiraganaKatakana() const
{
    return pImpl->GetFlag( 8 );
}

BOOL SvtSearchOptions::IsMatchContractions() const
{
    return pImpl->GetFlag( 9 );
}

BOOL SvtSearchOptions::IsMatchMinusDashChoon() const
{
    return pImpl->GetFlag( 10 );
}


BOOL SvtSearchOptions::IsMatchRepeatCharMarks() const
{
    return pImpl->GetFlag( 11 );
}

BOOL SvtSearchOptions::IsMatchVariantFormKanji() const
{
    return pImpl->GetFlag( 12 );
}

BOOL SvtSearchOptions::IsMatchOldKanaForms() const
{
    return pImpl->GetFlag( 13 );
}


BOOL SvtSearchOptions::IsMatchDiziDuzu() const
{
    return pImpl->GetFlag( 14 );
}

BOOL SvtSearchOptions::IsMatchBavaHafa() const
{
    return pImpl->GetFlag( 15 );
}

BOOL SvtSearchOptions::IsMatchTsithichiDhizi() const
{
    return pImpl->GetFlag( 16 );
}

BOOL SvtSearchOptions::IsMatchHyuiyuByuvyu() const
{
    return pImpl->GetFlag( 17 );
}

BOOL SvtSearchOptions::IsMatchSesheZeje() const
{
    return pImpl->GetFlag( 18 );
}

BOOL SvtSearchOptions::IsMatchIaiya() const
{
    return pImpl->GetFlag( 19 );
}

BOOL SvtSearchOptions::IsMatchKiku() const
{
    return pImpl->GetFlag( 20 );
}

BOOL SvtSearchOptions::IsIgnorePunctuation() const
{
    return pImpl->GetFlag( 21 );
}

BOOL SvtSearchOptions::IsIgnoreWhitespace() const
{
    return pImpl->GetFlag( 22 );
}

BOOL SvtSearchOptions::IsIgnoreProlongedSoundMark() const
{
    return pImpl->GetFlag( 23 );
}

BOOL SvtSearchOptions::IsIgnoreMiddleDot() const
{
    return pImpl->GetFlag( 24 );
}

}
