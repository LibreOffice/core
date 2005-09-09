/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmodpg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:58:26 $
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

    SdPage*         pPage;
    String          aOldName;
    String          aNewName;
    AutoLayout      eOldAutoLayout;
    AutoLayout      eNewAutoLayout;
    BOOL            bOldBckgrndVisible;
    BOOL            bNewBckgrndVisible;
    BOOL            bOldBckgrndObjsVisible;
    BOOL            bNewBckgrndObjsVisible;

    String          aComment;

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
    virtual void Repeat();

    virtual String GetComment() const;

    // erst mal kein Repeat, da kein Kontext erreichbar
    virtual BOOL CanRepeat(SfxRepeatTarget&) const { return FALSE; }
};

#endif      // _SD_UNMODPG_HXX

