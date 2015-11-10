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

#include <editeng/forbiddencharacterstable.hxx>

#include "forbiuno.hxx"
#include "docsh.hxx"

using namespace ::com::sun::star;

static rtl::Reference<SvxForbiddenCharactersTable> lcl_GetForbidden( ScDocShell* pDocSh )
{
    rtl::Reference<SvxForbiddenCharactersTable> xRet;
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        xRet = rDoc.GetForbiddenCharacters();
        if ( !xRet.is() )
        {
            //  create an empty SvxForbiddenCharactersTable for SvxUnoForbiddenCharsTable,
            //  so changes can be stored.

            xRet = new SvxForbiddenCharactersTable( comphelper::getProcessComponentContext() );
            rDoc.SetForbiddenCharacters( xRet );
        }
    }
    return xRet;
}

ScForbiddenCharsObj::ScForbiddenCharsObj( ScDocShell* pDocSh ) :
    SvxUnoForbiddenCharsTable( lcl_GetForbidden( pDocSh ) ),
    pDocShell( pDocSh )
{
    if (pDocShell)
        pDocShell->GetDocument().AddUnoObject(*this);
}

ScForbiddenCharsObj::~ScForbiddenCharsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScForbiddenCharsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // document gone
    }
}

void ScForbiddenCharsObj::onChange()
{
    if (pDocShell)
    {
        pDocShell->GetDocument().SetForbiddenCharacters( mxForbiddenChars );
        pDocShell->PostPaintGridAll();
        pDocShell->SetDocumentModified();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
