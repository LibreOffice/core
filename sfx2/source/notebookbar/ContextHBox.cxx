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

#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>
#include <sfx2/notebookbar/NotebookbarMainControl.hxx>

/*
 * ContextHBox is a VclHBox which shows own childs depending on current context.
 * This control can be used as a main container in the notebookbar .ui files
 * (instead of the Notebook).
 */

class SFX2_DLLPUBLIC ContextHBox : public VclHBox,
                                   public NotebookbarMainControl
{
public:
    explicit ContextHBox( vcl::Window *pParent )
        : VclHBox( pParent )
    {
    }

    virtual ~ContextHBox() override
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        VclHBox::dispose();
    }

    void SetContext( vcl::EnumContext::Context eContext ) override
    {
        for (int nChild = 0; nChild < GetChildCount(); ++nChild)
        {
            if ( GetChild( nChild )->GetType() == WINDOW_CONTAINER )
            {
                VclContainer* pChild = static_cast<VclContainer*>( GetChild( nChild ) );

                if ( pChild->HasContext( eContext ) || pChild->HasContext( vcl::EnumContext::Context::Context_Any ) )
                    pChild->Show();
                else
                    pChild->Hide();
            }
        }
    }

    void SetIconClickHdl( Link<NotebookBar*, void> ) override
    {
        // Menu not supported
    }
};

VCL_BUILDER_FACTORY(ContextHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
