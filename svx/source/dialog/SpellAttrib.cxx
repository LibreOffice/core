/*************************************************************************
 *
 *  $RCSfile: SpellAttrib.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:25:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _SVX_SPELL_ATTRIB
#include <SpellAttrib.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif
using namespace svx;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;
/*-- 10.09.2003 12:54:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::SpellErrorAttrib(Reference<XSpellAlternatives> xAlt) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_xAlternatives(xAlt)
{
}
/*-- 10.09.2003 12:54:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::SpellErrorAttrib( const SpellErrorAttrib& rAttr ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_xAlternatives( rAttr.m_xAlternatives )
{
}
/*-- 10.09.2003 12:54:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellErrorAttrib::~SpellErrorAttrib()
{
}
/*-- 10.09.2003 12:54:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellErrorAttrib::SetFont( Font& rFont ) const
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
             m_xAlternatives.get() == static_cast<const SpellErrorAttrib&>(rAttr).m_xAlternatives.get();
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
void SpellLanguageAttrib::SetFont( Font& rFont ) const
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

