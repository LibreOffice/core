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


#include "querytemplate.hxx"
#include <sfx2/sfxresid.hxx>
#include "doc.hrc"
#include "helpid.hrc"
#include <vcl/svapp.hxx>

namespace sfx2
{

QueryTemplateBox::QueryTemplateBox( vcl::Window* pParent, const OUString& rMessage ) :
    MessBox ( pParent, 0, Application::GetDisplayName(), rMessage )
{
    SetImage( QueryBox::GetStandardImage() );
    SetHelpId( HID_QUERY_LOAD_TEMPLATE );

    AddButton( SfxResId( STR_QRYTEMPL_UPDATE_BTN ).toString(), RET_YES,
            ButtonDialogFlags::Default | ButtonDialogFlags::OK | ButtonDialogFlags::Focus );
    AddButton( SfxResId(STR_QRYTEMPL_KEEP_BTN).toString(), RET_NO, ButtonDialogFlags::Cancel );
}

} // end of namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
