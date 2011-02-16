/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _EDITUNDO_HXX
#define _EDITUNDO_HXX

#include <editdoc.hxx>
#include <editeng/editund2.hxx>
#include <editeng/editdata.hxx>

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
    ContentNode*    pContentNode;   // Points to the valid,
                                    // undestroyed object!

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

    // 2 Pointers would be nicer but then it would have to be a SfxListener.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
