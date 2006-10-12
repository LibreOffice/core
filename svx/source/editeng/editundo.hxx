/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editundo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:36:56 $
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
};

// -----------------------------------------------------------------------
// EditUndoTransliteration
// ------------------------------------------------------------------------
class EditUndoTransliteration: public EditUndo
{
private:
    ESelection          aOldESel;
    ESelection          aNewESel;

    sal_Int32           nMode;
    EditTextObject*     pTxtObj;
    String              aText;

public:
                        TYPEINFO();
                        EditUndoTransliteration( ImpEditEngine* pImpEE, const ESelection& rESel, sal_Int32 nMode );
                        ~EditUndoTransliteration();

    void                SetText( const String& rText ) { aText = rText; }
    void                SetText( EditTextObject* pObj ) { pTxtObj = pObj; }
    void                SetNewSelection( const ESelection& rSel ) { aNewESel = rSel; }

    virtual void        Undo();
    virtual void        Redo();
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
};


#endif // _EDITUNDO_HXX
