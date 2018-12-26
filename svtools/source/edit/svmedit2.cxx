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


#include <svtools/svmedit2.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>

ExtMultiLineEdit::ExtMultiLineEdit( vcl::Window* pParent, WinBits nWinStyle ) :
    MultiLineEdit( pParent, nWinStyle )
{
}

void ExtMultiLineEdit::InsertText( const OUString& rNew )
{
    GetTextView()->InsertText( rNew );
}

void ExtMultiLineEdit::SetAutoScroll( bool bAutoScroll )
{
    GetTextView()->SetAutoScroll( bAutoScroll );
}

void ExtMultiLineEdit::SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd )
{
    GetTextEngine()->SetAttrib( rAttr, nPara, nStart, nEnd );
}

void ExtMultiLineEdit::SetLeftMargin( sal_uInt16 nLeftMargin )
{
    GetTextEngine()->SetLeftMargin( nLeftMargin );
}

sal_uInt32 ExtMultiLineEdit::GetParagraphCount() const
{
    return GetTextEngine()->GetParagraphCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
