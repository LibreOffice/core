/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoanim.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:27:06 $
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

#ifndef _SD_UNDO_ANIM_HXX
#define _SD_UNDO_ANIM_HXX

#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif

#include "sdundo.hxx"

class SdPage;

namespace sd
{

struct UndoAnimationImpl;

class UndoAnimation : public SdrUndoAction
{
public:
    UndoAnimation( SdDrawDocument* pDoc, SdPage* pThePage );
    virtual ~UndoAnimation();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    UndoAnimationImpl*  mpImpl;
};

struct UndoTransitionImpl;

class UndoTransition : public SdUndoAction
{
public:
    UndoTransition( SdDrawDocument* pDoc, SdPage* pThePage );
    virtual ~UndoTransition();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    UndoTransitionImpl* mpImpl;
};

}

#endif      // _SD_UNDO_ANIM_HXX

