/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "basiccharclass.hxx"

#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

BasicCharClass::BasicCharClass()
{
  for( sal_uInt16 i = 0; i < 256; ++i )
    IsLetterTab[i] = false;

  IsLetterTab[0xC0] = true;   
  IsLetterTab[0xC1] = true;   
  IsLetterTab[0xC2] = true;   
  IsLetterTab[0xC3] = true;   
  IsLetterTab[0xC4] = true;   
  IsLetterTab[0xC5] = true;   
  IsLetterTab[0xC6] = true;   
  IsLetterTab[0xC7] = true;   
  IsLetterTab[0xC8] = true;   
  IsLetterTab[0xC9] = true;   
  IsLetterTab[0xCA] = true;   
  IsLetterTab[0xCB] = true;   
  IsLetterTab[0xCC] = true;   
  IsLetterTab[0xCD] = true;   
  IsLetterTab[0xCE] = true;   
  IsLetterTab[0xCF] = true;   
  IsLetterTab[0xD0] = true;   
  IsLetterTab[0xD1] = true;   
  IsLetterTab[0xD2] = true;   
  IsLetterTab[0xD3] = true;   
  IsLetterTab[0xD4] = true;   
  IsLetterTab[0xD5] = true;   
  IsLetterTab[0xD6] = true;   
  IsLetterTab[0xD8] = true;   
  IsLetterTab[0xD9] = true;   
  IsLetterTab[0xDA] = true;   
  IsLetterTab[0xDB] = true;   
  IsLetterTab[0xDC] = true;   
  IsLetterTab[0xDD] = true;   
  IsLetterTab[0xDE] = true;   
  IsLetterTab[0xDF] = true;   
  IsLetterTab[0xE0] = true;   
  IsLetterTab[0xE1] = true;   
  IsLetterTab[0xE2] = true;   
  IsLetterTab[0xE3] = true;   
  IsLetterTab[0xE4] = true;   
  IsLetterTab[0xE5] = true;   
  IsLetterTab[0xE6] = true;   
  IsLetterTab[0xE7] = true;   
  IsLetterTab[0xE8] = true;   
  IsLetterTab[0xE9] = true;   
  IsLetterTab[0xEA] = true;   
  IsLetterTab[0xEB] = true;   
  IsLetterTab[0xEC] = true;   
  IsLetterTab[0xED] = true;   
  IsLetterTab[0xEE] = true;   
  IsLetterTab[0xEF] = true;   
  IsLetterTab[0xF0] = true;   
  IsLetterTab[0xF1] = true;   
  IsLetterTab[0xF2] = true;   
  IsLetterTab[0xF3] = true;   
  IsLetterTab[0xF4] = true;   
  IsLetterTab[0xF5] = true;   
  IsLetterTab[0xF6] = true;   
  IsLetterTab[0xF8] = true;   
  IsLetterTab[0xF9] = true;   
  IsLetterTab[0xFA] = true;   
  IsLetterTab[0xFB] = true;   
  IsLetterTab[0xFC] = true;   
  IsLetterTab[0xFD] = true;   
  IsLetterTab[0xFE] = true;   
  IsLetterTab[0xFF] = true;   
}

bool BasicCharClass::isLetter( sal_Unicode c )
{
  return (c < 256 ? IsLetterTab[c] : isLetterUnicode( c ));
}

bool BasicCharClass::isLetterUnicode( sal_Unicode c )
{
  static CharClass* pCharClass = NULL;
  if( pCharClass == NULL )
    pCharClass = new CharClass( Application::GetSettings().GetLanguageTag() );
  
  return pCharClass->isLetter( OUString(c), 0 );
}

bool BasicCharClass::isAlpha( sal_Unicode c, bool bCompatible )
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
    || (bCompatible && isLetter( c ));
}

bool BasicCharClass::isDigit( sal_Unicode c )
{
  return (c >= '0' && c <= '9');
}

bool BasicCharClass::isAlphaNumeric( sal_Unicode c, bool bCompatible )
{
  return isDigit( c ) || isAlpha( c, bCompatible );
}

bool BasicCharClass::isWhitespace( sal_Unicode c )
{
  return (c == ' ') || (c == '\t') || (c == '\f');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
