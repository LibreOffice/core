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

#include <sal/config.h>

#include <vcl/NotebookbarContextControl.hxx>
#include <ContextVBox.hxx>

ContextVBox::ContextVBox( vcl::Window *pParent )
    : VclVBox( pParent )
{
}

ContextVBox::~ContextVBox()
{
    disposeOnce();
}

void ContextVBox::SetContext( vcl::EnumContext::Context eContext )
{
    for (int nChild = 0; nChild < GetChildCount(); ++nChild)
    {
        if ( GetChild( nChild )->GetType() == WindowType::CONTAINER )
        {
            VclContainer* pChild = static_cast<VclContainer*>( GetChild( nChild ) );

            if ( pChild->HasContext( eContext ) || pChild->HasContext( vcl::EnumContext::Context::Any ) )
            {
                Size aSize( pChild->GetOptimalSize() );
                aSize.AdjustHeight(6 );
                pChild->Show();
                pChild->SetSizePixel( aSize );
            }
            else
            {
                pChild->Hide();
                pChild->SetSizePixel( Size( 0, 0 ) );
            }
        }
    }
    Size aSize( GetOptimalSize() );
    aSize.AdjustWidth(6 );
    SetSizePixel( aSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
