/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SpellAttrib.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_cui.hxx"

#include <SpellAttrib.hxx>
#include <vcl/font.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
using namespace svx;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;

/*-- 26.06.2008 10:41:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::SpellErrorAttrib( const SpellErrorDescription& rDesc ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_aSpellErrorDescription( rDesc )
{
}

/*-- 10.09.2003 12:54:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::SpellErrorAttrib( const SpellErrorAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_aSpellErrorDescription( rAttr.m_aSpellErrorDescription )
{
}
/*-- 10.09.2003 12:54:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::~SpellErrorAttrib()
{
}
/*-- 10.09.2003 12:54:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellErrorAttrib::SetFont( Font&  ) const
{
    //this attribute doesn't have a visual effect
}
/*-- 10.09.2003 12:54:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttrib*     SpellErrorAttrib::Clone() const
{
    return new SpellErrorAttrib(*this);
}
/*-- 10.09.2003 12:54:35---------------------------------------------------

  -----------------------------------------------------------------------*/
int SpellErrorAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aSpellErrorDescription == static_cast<const SpellErrorAttrib&>(rAttr).m_aSpellErrorDescription;
}
/*-- 10.09.2003 14:27:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellLanguageAttrib::SpellLanguageAttrib(LanguageType eLang) :
    TextAttrib(TEXTATTR_SPELL_LANGUAGE),
    m_eLanguage(eLang)
{
}
/*-- 10.09.2003 14:27:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellLanguageAttrib::SpellLanguageAttrib( const SpellLanguageAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_LANGUAGE),
    m_eLanguage(rAttr.m_eLanguage)
{
}
/*-- 10.09.2003 14:27:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellLanguageAttrib::~SpellLanguageAttrib()
{
}
/*-- 10.09.2003 14:27:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellLanguageAttrib::SetFont( Font&  ) const
{
    //no visual effect
}
/*-- 10.09.2003 14:27:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttrib* SpellLanguageAttrib::Clone() const
{
    return new SpellLanguageAttrib(*this);
}
/*-- 10.09.2003 14:27:44---------------------------------------------------

  -----------------------------------------------------------------------*/
int SpellLanguageAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_eLanguage == static_cast<const SpellLanguageAttrib&>(rAttr).m_eLanguage;
}
/*-- 31.10.2003 16:07:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellBackgroundAttrib::SpellBackgroundAttrib(const Color& rCol) :
    TextAttrib(TEXTATTR_SPELL_BACKGROUND),
    m_aBackgroundColor(rCol)
{
}
/*-- 31.10.2003 16:07:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellBackgroundAttrib::SpellBackgroundAttrib( const SpellBackgroundAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_BACKGROUND),
    m_aBackgroundColor(rAttr.m_aBackgroundColor)
{
}
/*-- 31.10.2003 16:07:46---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellBackgroundAttrib::~SpellBackgroundAttrib()
{
}
/*-- 31.10.2003 16:07:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellBackgroundAttrib::SetFont( Font& rFont ) const
{
    rFont.SetFillColor(m_aBackgroundColor);
}
/*-- 31.10.2003 16:07:46---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttrib* SpellBackgroundAttrib::Clone() const
{
    return new SpellBackgroundAttrib(*this);
}
/*-- 31.10.2003 16:07:47---------------------------------------------------

  -----------------------------------------------------------------------*/
int  SpellBackgroundAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aBackgroundColor == static_cast<const SpellBackgroundAttrib&>(rAttr).m_aBackgroundColor;
}

