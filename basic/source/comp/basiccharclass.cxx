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

#include "basiccharclass.hxx"

#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>


bool isLetterUnicode( sal_Unicode c )
{
  static CharClass* pCharClass = NULL;
  if( pCharClass == NULL )
    pCharClass = new CharClass( Application::GetSettings().GetLanguageTag() );
  // can we get pCharClass to accept a sal_Unicode instead of this waste?
  return pCharClass->isLetter( OUString(c), 0 );
}

bool basic::isAlpha( sal_Unicode c, bool bCompatible )
{
  if (rtl::isAscii(c))
    return rtl::isAsciiAlpha(c);
  if (bCompatible)
  {
    if (c < 256)
      //Latin-1 supplement alpha
      return c > 0xC0;
    else
      return isLetterUnicode(c);
  }
  return false;
}

bool basic::isAlphaNumeric( sal_Unicode c, bool bCompatible )
{
  return rtl::isAsciiDigit( c ) || basic::isAlpha( c, bCompatible );
}

bool basic::isWhitespace( sal_Unicode c )
{
  return (c == ' ') || (c == '\t') || (c == '\f');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
