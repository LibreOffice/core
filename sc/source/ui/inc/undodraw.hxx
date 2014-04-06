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

#ifndef SC_UNDODRAW_HXX
#define SC_UNDODRAW_HXX

#include <svl/undo.hxx>

class ScDocShell;

class ScUndoDraw: public SfxUndoAction
{
    SfxUndoAction*  pDrawUndo;
    ScDocShell*     pDocShell;

    void            UpdateSubShell();

public:
                            TYPEINFO_OVERRIDE();
                            ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh );
    virtual                 ~ScUndoDraw();

    SfxUndoAction*          GetDrawUndo()       { return pDrawUndo; }
    void                    ForgetDrawUndo();

    virtual void SetLinkToSfxLinkUndoAction(SfxLinkUndoAction* pSfxLinkUndoAction) SAL_OVERRIDE;

    virtual void            Undo() SAL_OVERRIDE;
    virtual void            Redo() SAL_OVERRIDE;
    virtual void            Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool            CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;
    virtual bool            Merge( SfxUndoAction *pNextAction ) SAL_OVERRIDE;
    virtual OUString        GetComment() const SAL_OVERRIDE;
    virtual OUString        GetRepeatComment(SfxRepeatTarget&) const SAL_OVERRIDE;
    virtual sal_uInt16      GetId() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
