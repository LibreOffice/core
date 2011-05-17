/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <SpellAttrib.hxx>
#include <vcl/font.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
using namespace svx;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;


//-----------------------------------------------------------------------
SpellErrorAttrib::SpellErrorAttrib( const SpellErrorDescription& rDesc ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_aSpellErrorDescription( rDesc )
{
}


//-----------------------------------------------------------------------
SpellErrorAttrib::SpellErrorAttrib( const SpellErrorAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_aSpellErrorDescription( rAttr.m_aSpellErrorDescription )
{
}

//-----------------------------------------------------------------------
SpellErrorAttrib::~SpellErrorAttrib()
{
}

//-----------------------------------------------------------------------
void SpellErrorAttrib::SetFont( Font&  ) const
{
    //this attribute doesn't have a visual effect
}

//-----------------------------------------------------------------------
TextAttrib*     SpellErrorAttrib::Clone() const
{
    return new SpellErrorAttrib(*this);
}

//-----------------------------------------------------------------------
int SpellErrorAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aSpellErrorDescription == static_cast<const SpellErrorAttrib&>(rAttr).m_aSpellErrorDescription;
}

//-----------------------------------------------------------------------
SpellLanguageAttrib::SpellLanguageAttrib(LanguageType eLang) :
    TextAttrib(TEXTATTR_SPELL_LANGUAGE),
    m_eLanguage(eLang)
{
}

//-----------------------------------------------------------------------
SpellLanguageAttrib::SpellLanguageAttrib( const SpellLanguageAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_LANGUAGE),
    m_eLanguage(rAttr.m_eLanguage)
{
}

//-----------------------------------------------------------------------
SpellLanguageAttrib::~SpellLanguageAttrib()
{
}

//-----------------------------------------------------------------------
void SpellLanguageAttrib::SetFont( Font&  ) const
{
    //no visual effect
}

//-----------------------------------------------------------------------
TextAttrib* SpellLanguageAttrib::Clone() const
{
    return new SpellLanguageAttrib(*this);
}

//-----------------------------------------------------------------------
int SpellLanguageAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_eLanguage == static_cast<const SpellLanguageAttrib&>(rAttr).m_eLanguage;
}

//-----------------------------------------------------------------------
SpellBackgroundAttrib::SpellBackgroundAttrib(const Color& rCol) :
    TextAttrib(TEXTATTR_SPELL_BACKGROUND),
    m_aBackgroundColor(rCol)
{
}

//-----------------------------------------------------------------------
SpellBackgroundAttrib::SpellBackgroundAttrib( const SpellBackgroundAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_BACKGROUND),
    m_aBackgroundColor(rAttr.m_aBackgroundColor)
{
}

//-----------------------------------------------------------------------
SpellBackgroundAttrib::~SpellBackgroundAttrib()
{
}

//-----------------------------------------------------------------------
void SpellBackgroundAttrib::SetFont( Font& rFont ) const
{
    rFont.SetFillColor(m_aBackgroundColor);
}

//-----------------------------------------------------------------------
TextAttrib* SpellBackgroundAttrib::Clone() const
{
    return new SpellBackgroundAttrib(*this);
}

//-----------------------------------------------------------------------
int  SpellBackgroundAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aBackgroundColor == static_cast<const SpellBackgroundAttrib&>(rAttr).m_aBackgroundColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
