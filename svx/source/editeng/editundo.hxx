/*************************************************************************
 *
 *  $RCSfile: editundo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EDITUNDO_HXX
#define _EDITUNDO_HXX

#include <editdoc.hxx>
#include <editund2.hxx>
#include <editdata.hxx>

#define UNDO_NOACTION           0
#define UNDO_NEWUNDO            1
#define UNDO_UNDOSDELETED       2
#define UNDO_EMPTYGROUPDELETED  3
#define UNDO_INVALIDEND         4

class ImpEditEngine;
class EditView;

// -----------------------------------------------------------------------
// EditUndoDelContent
// ------------------------------------------------------------------------
class EditUndoDelContent : public EditUndo
{
private:
    BOOL            bDelObject;
    USHORT          nNode;
    ContentNode*    pContentNode;   // Zeigt auf das gueltige,
                                    // nicht zerstoerte Objekt!

public:
                    TYPEINFO();
                    EditUndoDelContent( ImpEditEngine* pImpEE, ContentNode* pNode, USHORT nPortio );
                    ~EditUndoDelContent();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoConnectParas
// ------------------------------------------------------------------------
class EditUndoConnectParas : public EditUndo
{
private:
    USHORT          nNode;
    USHORT          nSepPos;
    SfxItemSet      aLeftParaAttribs;
    SfxItemSet      aRightParaAttribs;

    // 2 Pointer waeren schoener, aber dann muesste es ein SfxListener sein.
    String          aLeftStyleName;
    String          aRightStyleName;
    SfxStyleFamily  eLeftStyleFamily;
    SfxStyleFamily  eRightStyleFamily;

    BOOL            bBackward;

public:
                    TYPEINFO();
                    EditUndoConnectParas( ImpEditEngine* pImpEE, USHORT nNode, USHORT nSepPos,
                                            const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
                                            const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, BOOL bBackward );
                    ~EditUndoConnectParas();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSplitPara
// ------------------------------------------------------------------------
class EditUndoSplitPara : public EditUndo
{
private:
    USHORT          nNode;
    USHORT          nSepPos;

public:
                    TYPEINFO();
                    EditUndoSplitPara( ImpEditEngine* pImpEE, USHORT nNode, USHORT nSepPos );
                    ~EditUndoSplitPara();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoInsertChars
// ------------------------------------------------------------------------
class EditUndoInsertChars : public EditUndo
{
private:
    EPaM            aEPaM;
    String          aText;

public:
                    TYPEINFO();
                    EditUndoInsertChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const String& rStr );

    const EPaM&     GetEPaM() { return aEPaM; }
    String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();

    virtual BOOL    Merge( SfxUndoAction *pNextAction );
};

// -----------------------------------------------------------------------
// EditUndoRemoveChars
// ------------------------------------------------------------------------
class EditUndoRemoveChars : public EditUndo
{
private:
    EPaM            aEPaM;
    String          aText;

public:
                    TYPEINFO();
                    EditUndoRemoveChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const String& rStr );

    const EPaM&     GetEPaM() { return aEPaM; }
    String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoInsertFeature
// ------------------------------------------------------------------------
class EditUndoInsertFeature : public EditUndo
{
private:
    EPaM            aEPaM;
    SfxPoolItem*    pFeature;

public:
                    TYPEINFO();
                    EditUndoInsertFeature( ImpEditEngine* pImpEE, const EPaM& rEPaM,
                                            const SfxPoolItem& rFeature);
                    ~EditUndoInsertFeature();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoMoveParagraphs
// ------------------------------------------------------------------------
class EditUndoMoveParagraphs: public EditUndo
{
private:
    Range           nParagraphs;
    USHORT          nDest;

public:
                    TYPEINFO();
                    EditUndoMoveParagraphs( ImpEditEngine* pImpEE, const Range& rParas, USHORT nDest );
                    ~EditUndoMoveParagraphs();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSetStyleSheet
// ------------------------------------------------------------------------
class EditUndoSetStyleSheet: public EditUndo
{
private:
    USHORT          nPara;
    XubString       aPrevName;
    XubString       aNewName;
    SfxStyleFamily  ePrevFamily;
    SfxStyleFamily  eNewFamily;
    SfxItemSet      aPrevParaAttribs;

public:
                    TYPEINFO();

                    EditUndoSetStyleSheet( ImpEditEngine* pImpEE, USHORT nPara,
                        const XubString& rPrevName, SfxStyleFamily ePrevFamily,
                        const XubString& rNewName, SfxStyleFamily eNewFamily,
                        const SfxItemSet& rPrevParaAttribs );
                    ~EditUndoSetStyleSheet();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSetParaAttribs
// ------------------------------------------------------------------------
class EditUndoSetParaAttribs: public EditUndo
{
private:
    USHORT          nPara;
    SfxItemSet      aPrevItems;
    SfxItemSet      aNewItems;

public:
                    TYPEINFO();
                    EditUndoSetParaAttribs( ImpEditEngine* pImpEE, USHORT nPara, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems );
                    ~EditUndoSetParaAttribs();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSetAttribs
// ------------------------------------------------------------------------
class EditUndoSetAttribs: public EditUndo
{
private:
    ESelection          aESel;
    SfxItemSet          aNewAttribs;
    ContentInfoArray    aPrevAttribs;

    BYTE                nSpecial;
    BOOL                bSetIsRemove;
    BOOL                bRemoveParaAttribs;
    USHORT              nRemoveWhich;

    void                ImpSetSelection( EditView* pView );


public:
                        TYPEINFO();
                        EditUndoSetAttribs( ImpEditEngine* pImpEE, const ESelection& rESel, const SfxItemSet& rNewItems );
                        ~EditUndoSetAttribs();

    ContentInfoArray&   GetContentInfos()   { return aPrevAttribs; }
    SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }

    void                SetSpecial( BYTE n )            { nSpecial = n; }
    void                SetRemoveAttribs( BOOL b )      { bSetIsRemove = b; }
    void                SetRemoveParaAttribs( BOOL b )  { bRemoveParaAttribs = b; }
    void                SetRemoveWhich( USHORT n )      { nRemoveWhich = n; }

    virtual void        Undo();
    virtual void        Redo();
    virtual void        Repeat();
};

// -----------------------------------------------------------------------
// EditUndoMarkSelection
// ------------------------------------------------------------------------
class EditUndoMarkSelection: public EditUndo
{
private:
    ESelection      aSelection;

public:
                    TYPEINFO();
                    EditUndoMarkSelection( ImpEditEngine* pImpEE, const ESelection& rSel );
                    ~EditUndoMarkSelection();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};


#endif // _EDITUNDO_HXX
