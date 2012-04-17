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

class EditEngine;
class EditView;

// -----------------------------------------------------------------------
// EditUndoDelContent
// ------------------------------------------------------------------------
class EditUndoDelContent : public EditUndo
{
private:
    bool            bDelObject;
    size_t          nNode;
    ContentNode*    pContentNode;   // Points to the valid,
                                    // undestroyed object!

public:
    EditUndoDelContent(EditEngine* pEE, ContentNode* pNode, size_t nPortion);
    virtual ~EditUndoDelContent();

    virtual void    Undo();
    virtual void    Redo();
};

// -----------------------------------------------------------------------
// EditUndoConnectParas
// ------------------------------------------------------------------------
class EditUndoConnectParas : public EditUndo
{
private:
    sal_uInt16          nNode;
    sal_uInt16          nSepPos;
    SfxItemSet      aLeftParaAttribs;
    SfxItemSet      aRightParaAttribs;

    // 2 Pointers would be nicer but then it would have to be a SfxListener.
    String          aLeftStyleName;
    String          aRightStyleName;
    SfxStyleFamily  eLeftStyleFamily;
    SfxStyleFamily  eRightStyleFamily;

    bool            bBackward;

public:
    EditUndoConnectParas(EditEngine* pEE, sal_uInt16 nNode, sal_uInt16 nSepPos,
                         const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
                         const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, bool bBackward);
    virtual ~EditUndoConnectParas();

    virtual void    Undo();
    virtual void    Redo();
};

// -----------------------------------------------------------------------
// EditUndoSplitPara
// ------------------------------------------------------------------------
class EditUndoSplitPara : public EditUndo
{
private:
    sal_uInt16          nNode;
    sal_uInt16          nSepPos;

public:
    EditUndoSplitPara(EditEngine* pEE, sal_uInt16 nNode, sal_uInt16 nSepPos);
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
    EditUndoInsertChars(EditEngine* pEE, const EPaM& rEPaM, const String& rStr);

    const EPaM&     GetEPaM() { return aEPaM; }
    String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();

    virtual sal_Bool    Merge( SfxUndoAction *pNextAction );
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
    EditUndoRemoveChars(EditEngine* pEE, const EPaM& rEPaM, const String& rStr);

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
    EditUndoInsertFeature(EditEngine* pEE, const EPaM& rEPaM, const SfxPoolItem& rFeature);
    virtual ~EditUndoInsertFeature();

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
    sal_uInt16          nDest;

public:
    EditUndoMoveParagraphs(EditEngine* pEE, const Range& rParas, sal_uInt16 nDest);
    virtual ~EditUndoMoveParagraphs();

    virtual void    Undo();
    virtual void    Redo();
};

// -----------------------------------------------------------------------
// EditUndoSetStyleSheet
// ------------------------------------------------------------------------
class EditUndoSetStyleSheet: public EditUndo
{
private:
    sal_uInt16          nPara;
    String       aPrevName;
    String       aNewName;
    SfxStyleFamily  ePrevFamily;
    SfxStyleFamily  eNewFamily;
    SfxItemSet      aPrevParaAttribs;

public:
    EditUndoSetStyleSheet(EditEngine* pEE, sal_uInt16 nPara,
        const String& rPrevName, SfxStyleFamily ePrevFamily,
        const String& rNewName, SfxStyleFamily eNewFamily,
        const SfxItemSet& rPrevParaAttribs);
    virtual ~EditUndoSetStyleSheet();

    virtual void    Undo();
    virtual void    Redo();
};

// -----------------------------------------------------------------------
// EditUndoSetParaAttribs
// ------------------------------------------------------------------------
class EditUndoSetParaAttribs: public EditUndo
{
private:
    sal_uInt16          nPara;
    SfxItemSet      aPrevItems;
    SfxItemSet      aNewItems;

public:
    EditUndoSetParaAttribs(EditEngine* pEE, sal_uInt16 nPara, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems);
    virtual ~EditUndoSetParaAttribs();

    virtual void    Undo();
    virtual void    Redo();
};

// -----------------------------------------------------------------------
// EditUndoSetAttribs
// ------------------------------------------------------------------------
class EditUndoSetAttribs: public EditUndo
{
private:
    typedef boost::ptr_vector<ContentAttribsInfo> InfoArrayType;

    ESelection          aESel;
    SfxItemSet          aNewAttribs;
    InfoArrayType       aPrevAttribs;

    sal_uInt8               nSpecial;
    sal_Bool                bSetIsRemove;
    sal_Bool                bRemoveParaAttribs;
    sal_uInt16              nRemoveWhich;

    void                ImpSetSelection( EditView* pView );


public:
    EditUndoSetAttribs(EditEngine* pEE, const ESelection& rESel, const SfxItemSet& rNewItems);
    virtual ~EditUndoSetAttribs();

    SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }

    void                SetSpecial( sal_uInt8 n )           { nSpecial = n; }
    void                SetRemoveAttribs( sal_Bool b )      { bSetIsRemove = b; }
    void                SetRemoveParaAttribs( sal_Bool b )  { bRemoveParaAttribs = b; }
    void                SetRemoveWhich( sal_uInt16 n )      { nRemoveWhich = n; }

    virtual void        Undo();
    virtual void        Redo();

    void AppendContentInfo(ContentAttribsInfo* pNew);
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
    EditUndoTransliteration(EditEngine* pEE, const ESelection& rESel, sal_Int32 nMode);
    virtual ~EditUndoTransliteration();

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
    EditUndoMarkSelection(EditEngine* pEE, const ESelection& rSel);
    virtual ~EditUndoMarkSelection();

    virtual void    Undo();
    virtual void    Redo();
};


#endif // _EDITUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
