/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmodpg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:26:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    // #67720#
    SfxUndoManager* mpManager;

    SdPage*         mpPage;
    String          maOldName;
    String          maNewName;
    AutoLayout      meOldAutoLayout;
    AutoLayout      meNewAutoLayout;
    BOOL            mbOldBckgrndVisible;
    BOOL            mbNewBckgrndVisible;
    BOOL            mbOldBckgrndObjsVisible;
    BOOL            mbNewBckgrndObjsVisible;

    String          maComment;

public:
    TYPEINFO();
    ModifyPageUndoAction(
        SfxUndoManager*         pManager, // #67720#
        SdDrawDocument*         pTheDoc,
        SdPage*                 pThePage,
        String                  aTheNewName,
        AutoLayout              eTheNewAutoLayout,
        BOOL                    bTheNewBckgrndVisible,
        BOOL                    bTheNewBckgrndObjsVisible);

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

