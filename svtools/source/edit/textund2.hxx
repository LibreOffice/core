/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textund2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:29:12 $
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
#ifndef _TEXTUND2_HXX
#define _TEXTUND2_HXX

#include <textundo.hxx>


class TextUndoDelPara : public TextUndo
{
private:
    BOOL            mbDelObject;
    ULONG           mnPara;
    TextNode*       mpNode; // Zeigt auf das gueltige, nicht zerstoerte Objekt!

public:
                    TYPEINFO();
                    TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, ULONG nPara );
                    ~TextUndoDelPara();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoConnectParas : public TextUndo
{
private:
    ULONG           mnPara;
    USHORT          mnSepPos;

public:
                    TYPEINFO();
                    TextUndoConnectParas( TextEngine* pTextEngine, ULONG nPara, USHORT nSepPos );
                    ~TextUndoConnectParas();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoSplitPara : public TextUndo
{
private:
    ULONG           mnPara;
    USHORT          mnSepPos;

public:
                    TYPEINFO();
                    TextUndoSplitPara( TextEngine* pTextEngine, ULONG nPara, USHORT nSepPos );
                    ~TextUndoSplitPara();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoInsertChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TYPEINFO();
                    TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

//  const TextPaM&  GetTextPaM() { return aTextPaM; }
//  String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();

    virtual BOOL    Merge( SfxUndoAction *pNextAction );
};


class TextUndoRemoveChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TYPEINFO();
                    TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

//  const TextPaM&      GetTextPaM() { return aTextPaM; }
//  String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoSetAttribs: public TextUndo
{
private:
    TextSelection       maSelection;
//  SfxItemSet          aNewAttribs;
//  TextInfoArray       aPrevAttribs;
//  BYTE                nSpecial;
//  BOOL                bSetIsRemove;
//  USHORT              nRemoveWhich;
//
//  void                ImpSetSelection( TextView* pView );


public:
                        TYPEINFO();
                        TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rESel );
                        ~TextUndoSetAttribs();

//  TextInfoArray&      GetTextInfos()  { return aPrevAttribs; }
//  SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }
//  void                SetSpecial( BYTE n )            { nSpecial = n; }
//  void                SetRemoveAttribs( BOOL b )      { bSetIsRemove = b; }
//  void                SetRemoveWhich( USHORT n )      { nRemoveWhich = n; }

    virtual void        Undo();
    virtual void        Redo();
};

#endif // _TEXTUND2_HXX
