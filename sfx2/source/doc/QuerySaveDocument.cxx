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

#include "sfx2/QuerySaveDocument.hxx"

#include <sfx2/sfx.hrc>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>
#include "doc.hrc"
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
// -----------------------------------------------------------------------------
short ExecuteQuerySaveDocument(Window* _pParent,const String& _rTitle)
{
    if (Application::IsHeadlessModeEnabled())
    {   // don't block Desktop::terminate() if there's no user to ask
        return RET_NO;
    }
    String aText( SfxResId(STR_QUERY_SAVE_DOCUMENT).toString() );
    aText.SearchAndReplace( DEFINE_CONST_UNICODE( "$(DOC)" ),
                            _rTitle );
    QueryBox aQBox( _pParent, WB_YES_NO_CANCEL | WB_DEF_YES, aText );
    aQBox.SetText(SfxResId(STR_QUERY_SAVE_DOCUMENT_TITLE).toString()); // Window title
    aQBox.SetButtonText( BUTTONID_NO, SfxResId(STR_NOSAVEANDCLOSE).toString() );
    aQBox.SetButtonText( BUTTONID_YES, SfxResId(STR_SAVEDOC).toString() );
    return aQBox.Execute();
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
