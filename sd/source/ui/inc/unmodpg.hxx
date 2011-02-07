/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_UNMODPG_HXX
#define _SD_UNMODPG_HXX

#include "sdundo.hxx"
#include "pres.hxx"

class SdDrawDocument;
class SdPage;

class ModifyPageUndoAction : public SdUndoAction
{
    SdPage*         mpPage;
    String          maOldName;
    String          maNewName;
    AutoLayout      meOldAutoLayout;
    AutoLayout      meNewAutoLayout;
    sal_Bool            mbOldBckgrndVisible;
    sal_Bool            mbNewBckgrndVisible;
    sal_Bool            mbOldBckgrndObjsVisible;
    sal_Bool            mbNewBckgrndObjsVisible;

    String          maComment;

public:
    TYPEINFO();
    ModifyPageUndoAction(
        SdDrawDocument*         pTheDoc,
        SdPage*                 pThePage,
        String                  aTheNewName,
        AutoLayout              eTheNewAutoLayout,
        sal_Bool                    bTheNewBckgrndVisible,
        sal_Bool                    bTheNewBckgrndObjsVisible);

    virtual ~ModifyPageUndoAction();
    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

// --------------------------------------------------------------------

class RenameLayoutTemplateUndoAction : public SdUndoAction
{
public:
    RenameLayoutTemplateUndoAction( SdDrawDocument* pDocument, const String& rOldLayoutName, const String& rNewLayoutName );

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    String maOldName;
    String maNewName;
    const String maComment;
};

#endif      // _SD_UNMODPG_HXX

