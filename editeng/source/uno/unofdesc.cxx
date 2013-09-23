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


#include <editeng/eeitem.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/memberids.hrc>
#include <svl/itempool.hxx>

#include <editeng/unofdesc.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;


void SvxUnoFontDescriptor::ConvertToFont( const awt::FontDescriptor& rDesc, Font& rFont )
{
    rFont.SetName( rDesc.Name );
    rFont.SetStyleName( rDesc.StyleName );
    rFont.SetSize( Size( rDesc.Width, rDesc.Height ) );
    rFont.SetFamily( (FontFamily)rDesc.Family );
    rFont.SetCharSet( (rtl_TextEncoding)rDesc.CharSet );
    rFont.SetPitch( (FontPitch)rDesc.Pitch );
    rFont.SetOrientation( (short)(rDesc.Orientation*10) );
    rFont.SetKerning( rDesc.Kerning );
    rFont.SetWeight( VCLUnoHelper::ConvertFontWeight(rDesc.Weight) );
    rFont.SetItalic( (FontItalic)rDesc.Slant );
    rFont.SetUnderline( (FontUnderline)rDesc.Underline );
    rFont.SetStrikeout( (FontStrikeout)rDesc.Strikeout );
    rFont.SetWordLineMode( rDesc.WordLineMode );
}

void SvxUnoFontDescriptor::ConvertFromFont( const Font& rFont, awt::FontDescriptor& rDesc )
{
    rDesc.Name = rFont.GetName();
    rDesc.StyleName = rFont.GetStyleName();
    rDesc.Width = sal::static_int_cast< sal_Int16 >(rFont.GetSize().Width());
    rDesc.Height = sal::static_int_cast< sal_Int16 >(rFont.GetSize().Height());
    rDesc.Family = sal::static_int_cast< sal_Int16 >(rFont.GetFamily());
    rDesc.CharSet = rFont.GetCharSet();
    rDesc.Pitch = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
    rDesc.Orientation = static_cast< float >(rFont.GetOrientation() / 10);
    rDesc.Kerning = rFont.IsKerning();
    rDesc.Weight = VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    rDesc.Slant = (awt::FontSlant)rFont.GetItalic();
    rDesc.Underline = sal::static_int_cast< sal_Int16 >(rFont.GetUnderline());
    rDesc.Strikeout = sal::static_int_cast< sal_Int16 >(rFont.GetStrikeout());
    rDesc.WordLineMode = rFont.IsWordLineMode();
}

void SvxUnoFontDescriptor::FillItemSet( const awt::FontDescriptor& rDesc, SfxItemSet& rSet )
{
    uno::Any aTemp;

    {
        SvxFontItem aFontItem( EE_CHAR_FONTINFO );
        aFontItem.SetFamilyName( rDesc.Name);
        aFontItem.SetStyleName( rDesc.StyleName);
        aFontItem.SetFamily( (FontFamily)rDesc.Family);
        aFontItem.SetCharSet( rDesc.CharSet );
        aFontItem.SetPitch( (FontPitch)rDesc.Pitch);
        rSet.Put(aFontItem);
    }

    {
        SvxFontHeightItem aFontHeightItem( 0, 100, EE_CHAR_FONTHEIGHT );
        aTemp <<= (float)rDesc.Height;
        ((SfxPoolItem*)&aFontHeightItem)->PutValue( aTemp, MID_FONTHEIGHT|CONVERT_TWIPS );
        rSet.Put(aFontHeightItem);
    }

    {
        SvxPostureItem aPostureItem( (FontItalic)0, EE_CHAR_ITALIC );
        aTemp <<= rDesc.Slant;
        ((SfxPoolItem*)&aPostureItem)->PutValue( aTemp, MID_POSTURE );
        rSet.Put(aPostureItem);
    }

    {
        SvxUnderlineItem aUnderlineItem( (FontUnderline)0, EE_CHAR_UNDERLINE );
        aTemp <<= (sal_Int16)rDesc.Underline;
        ((SfxPoolItem*)&aUnderlineItem)->PutValue( aTemp, MID_TL_STYLE );
        rSet.Put( aUnderlineItem );
    }

    {
        SvxWeightItem aWeightItem( (FontWeight)0, EE_CHAR_WEIGHT );
        aTemp <<= rDesc.Weight;
        ((SfxPoolItem*)&aWeightItem)->PutValue( aTemp, MID_WEIGHT );
        rSet.Put( aWeightItem );
    }

    {
        SvxCrossedOutItem aCrossedOutItem( (FontStrikeout)0, EE_CHAR_STRIKEOUT );
        aTemp <<= rDesc.Strikeout;
        ((SfxPoolItem*)&aCrossedOutItem)->PutValue( aTemp, MID_CROSS_OUT );
        rSet.Put( aCrossedOutItem );
    }

    {
        SvxWordLineModeItem aWLMItem( rDesc.WordLineMode, EE_CHAR_WLM );
        rSet.Put( aWLMItem );
    }
}

void SvxUnoFontDescriptor::FillFromItemSet( const SfxItemSet& rSet, awt::FontDescriptor& rDesc )
{
    const SfxPoolItem* pItem = NULL;
    {
        SvxFontItem* pFontItem = (SvxFontItem*)&rSet.Get( EE_CHAR_FONTINFO, sal_True );
        rDesc.Name      = pFontItem->GetFamilyName();
        rDesc.StyleName = pFontItem->GetStyleName();
        rDesc.Family    = sal::static_int_cast< sal_Int16 >(
            pFontItem->GetFamily());
        rDesc.CharSet   = pFontItem->GetCharSet();
        rDesc.Pitch     = sal::static_int_cast< sal_Int16 >(
            pFontItem->GetPitch());
    }
    {
        pItem = &rSet.Get( EE_CHAR_FONTHEIGHT, sal_True );
        uno::Any aHeight;
        if( pItem->QueryValue( aHeight, MID_FONTHEIGHT ) )
            aHeight >>= rDesc.Height;
    }
    {
        pItem = &rSet.Get( EE_CHAR_ITALIC, sal_True );
        uno::Any aFontSlant;
        if(pItem->QueryValue( aFontSlant, MID_POSTURE ))
            aFontSlant >>= rDesc.Slant;
    }
    {
        pItem = &rSet.Get( EE_CHAR_UNDERLINE, sal_True );
        uno::Any aUnderline;
        if(pItem->QueryValue( aUnderline, MID_TL_STYLE ))
            aUnderline >>= rDesc.Underline;
    }
    {
        pItem = &rSet.Get( EE_CHAR_WEIGHT, sal_True );
        uno::Any aWeight;
        if(pItem->QueryValue( aWeight, MID_WEIGHT ))
            aWeight >>= rDesc.Weight;
    }
    {
        pItem = &rSet.Get( EE_CHAR_STRIKEOUT, sal_True );
        uno::Any aStrikeOut;
        if(pItem->QueryValue( aStrikeOut, MID_CROSS_OUT ))
            aStrikeOut >>= rDesc.Strikeout;
    }
    {
        SvxWordLineModeItem* pWLMItem = (SvxWordLineModeItem*)&rSet.Get( EE_CHAR_WLM, sal_True );
        rDesc.WordLineMode = pWLMItem->GetValue();
    }
}

void SvxUnoFontDescriptor::setPropertyToDefault( SfxItemSet& rSet )
{
    rSet.InvalidateItem( EE_CHAR_FONTINFO );
    rSet.InvalidateItem( EE_CHAR_FONTHEIGHT );
    rSet.InvalidateItem( EE_CHAR_ITALIC );
    rSet.InvalidateItem( EE_CHAR_UNDERLINE );
    rSet.InvalidateItem( EE_CHAR_WEIGHT );
    rSet.InvalidateItem( EE_CHAR_STRIKEOUT );
    rSet.InvalidateItem( EE_CHAR_WLM );
}

uno::Any SvxUnoFontDescriptor::getPropertyDefault( SfxItemPool* pPool )
{
    SfxItemSet aSet( *pPool, EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
                             EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT,
                             EE_CHAR_ITALIC, EE_CHAR_ITALIC,
                             EE_CHAR_UNDERLINE, EE_CHAR_UNDERLINE,
                             EE_CHAR_WEIGHT, EE_CHAR_WEIGHT,
                             EE_CHAR_STRIKEOUT, EE_CHAR_STRIKEOUT,
                             EE_CHAR_WLM, EE_CHAR_WLM, 0 );

    uno::Any aAny;

    if(!pPool->IsWhich(EE_CHAR_FONTINFO)||
        !pPool->IsWhich(EE_CHAR_FONTHEIGHT)||
        !pPool->IsWhich(EE_CHAR_ITALIC)||
        !pPool->IsWhich(EE_CHAR_UNDERLINE)||
        !pPool->IsWhich(EE_CHAR_WEIGHT)||
        !pPool->IsWhich(EE_CHAR_STRIKEOUT)||
        !pPool->IsWhich(EE_CHAR_WLM))
        return aAny;

    aSet.Put(pPool->GetDefaultItem(EE_CHAR_FONTINFO));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_FONTHEIGHT));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_ITALIC));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_UNDERLINE));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_WEIGHT));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_STRIKEOUT));
    aSet.Put(pPool->GetDefaultItem(EE_CHAR_WLM));

    awt::FontDescriptor aDesc;

    FillFromItemSet( aSet, aDesc );

    aAny <<= aDesc;

    return aAny;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
