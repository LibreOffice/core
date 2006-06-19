/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textundo.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 21:02:45 $
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
#ifndef _TEXTUNDO_HXX
#define _TEXTUNDO_HXX

#ifndef _UNDO_HXX
#include <undo.hxx>
#endif

class TextEngine;

class TextUndoManager : public SfxUndoManager
{
    TextEngine*     mpTextEngine;

protected:

    void            UndoRedoStart();
    void            UndoRedoEnd();

    TextView*       GetView() const { return mpTextEngine->GetActiveView(); }

public:
                    TextUndoManager( TextEngine* pTextEngine );
                    ~TextUndoManager();

    using SfxUndoManager::Undo;
    virtual BOOL    Undo( USHORT nCount=1 );
    using SfxUndoManager::Redo;
    virtual BOOL    Redo( USHORT nCount=1 );

};

class TextUndo : public SfxUndoAction
{
private:
    USHORT              mnId;
    TextEngine*         mpTextEngine;

protected:

    TextView*           GetView() const { return mpTextEngine->GetActiveView(); }
    void                SetSelection( const TextSelection& rSel );

    TextDoc*            GetDoc() const { return mpTextEngine->mpDoc; }
    TEParaPortions*     GetTEParaPortions() const { return mpTextEngine->mpTEParaPortions; }

public:
                        TYPEINFO();
                        TextUndo( USHORT nId, TextEngine* pTextEngine );
    virtual             ~TextUndo();

    TextEngine*         GetTextEngine() const   { return mpTextEngine; }

    virtual void        Undo()      = 0;
    virtual void        Redo()      = 0;

    virtual XubString   GetComment() const;
    virtual USHORT      GetId() const;
};

#endif // _TEXTUNDO_HXX
