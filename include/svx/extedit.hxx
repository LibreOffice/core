/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _EXTEDIT_HXX
#define _EXTEDIT_HXX

#include <svtools/grfmgr.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <vcl/graph.hxx>
#include <vcl/timer.hxx>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC ExternalToolEdit
{
public:
    GraphicObject* m_pGraphicObject;
    OUString m_aFileName;

    ExternalToolEdit();
    virtual ~ExternalToolEdit();

    virtual void Update( Graphic& aGraphic ) = 0;
    void Edit( GraphicObject *pGraphic );

    DECL_LINK( StartListeningEvent, void *pEvent );

    static void threadWorker( void *pThreadData );
    static void HandleCloseEvent( ExternalToolEdit* pData );
};


#endif
