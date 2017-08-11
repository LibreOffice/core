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

#include <SpellAttrib.hxx>
#include <vcl/font.hxx>

using namespace svx;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;


SpellErrorAttrib::SpellErrorAttrib( const SpellErrorDescription& rDesc ) :
    TextAttrib(TEXTATTR_SPELL_ERROR),
    m_aSpellErrorDescription( rDesc )
{
}


void SpellErrorAttrib::SetFont( vcl::Font&  ) const
{
    //this attribute doesn't have a visual effect
}


TextAttrib*     SpellErrorAttrib::Clone() const
{
    return new SpellErrorAttrib(*this);
}


bool SpellErrorAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aSpellErrorDescription == static_cast<const SpellErrorAttrib&>(rAttr).m_aSpellErrorDescription;
}


SpellLanguageAttrib::SpellLanguageAttrib(LanguageType eLang) :
    TextAttrib(TEXTATTR_SPELL_LANGUAGE),
    m_eLanguage(eLang)
{
}


void SpellLanguageAttrib::SetFont( vcl::Font&  ) const
{
    //no visual effect
}


TextAttrib* SpellLanguageAttrib::Clone() const
{
    return new SpellLanguageAttrib(*this);
}


bool SpellLanguageAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_eLanguage == static_cast<const SpellLanguageAttrib&>(rAttr).m_eLanguage;
}


SpellBackgroundAttrib::SpellBackgroundAttrib(const Color& rCol) :
    TextAttrib(TEXTATTR_SPELL_BACKGROUND),
    m_aBackgroundColor(rCol)
{
}


void SpellBackgroundAttrib::SetFont( vcl::Font& rFont ) const
{
    rFont.SetFillColor(m_aBackgroundColor);
}


TextAttrib* SpellBackgroundAttrib::Clone() const
{
    return new SpellBackgroundAttrib(*this);
}


bool SpellBackgroundAttrib::operator==( const TextAttrib& rAttr ) const
{
    return Which() == rAttr.Which() &&
            m_aBackgroundColor == static_cast<const SpellBackgroundAttrib&>(rAttr).m_aBackgroundColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
