/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Surendran Mahendran <surenspost@gmail.com>]
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
