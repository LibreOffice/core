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


#include <svmedit2.hxx>
#include <vcl/xtextedt.hxx>

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, WinBits nWinStyle ) :

    MultiLineEdit( pParent, nWinStyle )

{
}

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, const ResId& rResId ) :

    MultiLineEdit( pParent, rResId )

{
}

ExtMultiLineEdit::~ExtMultiLineEdit()
{
}

void ExtMultiLineEdit::InsertText( const String& rNew, sal_Bool )
{
    GetTextView()->InsertText( rNew, sal_False );
}

void ExtMultiLineEdit::SetAutoScroll( sal_Bool bAutoScroll )
{
    GetTextView()->SetAutoScroll( bAutoScroll );
}

void ExtMultiLineEdit::SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    GetTextEngine()->SetAttrib( rAttr, nPara, nStart, nEnd );
}

void ExtMultiLineEdit::SetLeftMargin( sal_uInt16 nLeftMargin )
{
    GetTextEngine()->SetLeftMargin( nLeftMargin );
}

sal_uLong ExtMultiLineEdit::GetParagraphCount() const
{
    return GetTextEngine()->GetParagraphCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
