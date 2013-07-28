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

#include "TextDirectionListBox.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include <svl/languageoptions.hxx>
#include <vcl/window.hxx>

namespace chart
{

TextDirectionListBox::TextDirectionListBox( Window* pParent, const ResId& rResId, Window* pWindow1, Window* pWindow2 ) :
    svx::FrameDirectionListBox( pParent, rResId )
{
    InsertEntryValue( SCH_RESSTR( STR_TEXT_DIRECTION_LTR ), FRMDIR_HORI_LEFT_TOP );
    InsertEntryValue( SCH_RESSTR( STR_TEXT_DIRECTION_RTL ), FRMDIR_HORI_RIGHT_TOP );
    InsertEntryValue( SCH_RESSTR( STR_TEXT_DIRECTION_SUPER ), FRMDIR_ENVIRONMENT );

    if( !SvtLanguageOptions().IsCTLFontEnabled() )
    {
        Hide();
        if( pWindow1 ) pWindow1->Hide();
        if( pWindow2 ) pWindow2->Hide();
    }
}

TextDirectionListBox::~TextDirectionListBox()
{
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
