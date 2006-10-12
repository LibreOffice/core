/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editund2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:40:22 $
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

#ifndef _EDITUND2_HXX
#define _EDITUND2_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

class ImpEditEngine;

class SVX_DLLPUBLIC EditUndoManager : public SfxUndoManager
{
    using SfxUndoManager::Undo;
    using SfxUndoManager::Redo;

private:
    ImpEditEngine*  pImpEE;
public:
                    EditUndoManager( ImpEditEngine* pImpEE );

    virtual BOOL    Undo( USHORT nCount=1 );
    virtual BOOL    Redo( USHORT nCount=1 );
};

// -----------------------------------------------------------------------
// EditUndo
// ------------------------------------------------------------------------
class SVX_DLLPUBLIC EditUndo : public SfxUndoAction
{
private:
    USHORT          nId;
    ImpEditEngine*  pImpEE;

public:
                    TYPEINFO();
                    EditUndo( USHORT nI, ImpEditEngine* pImpEE );
    virtual         ~EditUndo();

    ImpEditEngine*  GetImpEditEngine() const    { return pImpEE; }

    virtual void    Undo()      = 0;
    virtual void    Redo()      = 0;

    virtual BOOL    CanRepeat(SfxRepeatTarget&) const;
    virtual String  GetComment() const;
    virtual USHORT  GetId() const;
};

#endif  // _EDITUND2_HXX


