/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDITUNDO_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDITUNDO_HXX

#include <editdoc.hxx>
#include <editeng/editund2.hxx>
#include <editeng/editdata.hxx>
#include <vector>
#include <memory>

class EditEngine;
class EditView;


// EditUndoDelContent

class EditUndoDelContent : public EditUndo
{
private:
    bool            bDelObject;
    sal_Int32       nNode;
    ContentNode*    pContentNode;   // Points to the valid,
                                    // undestroyed object!

public:
    EditUndoDelContent(EditEngine* pEE, ContentNode* pNode, sal_Int32 nPortion);
    virtual ~EditUndoDelContent();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoConnectParas

class EditUndoConnectParas : public EditUndo
{
private:
    sal_Int32       nNode;
    sal_uInt16      nSepPos;
    SfxItemSet      aLeftParaAttribs;
    SfxItemSet      aRightParaAttribs;

    // 2 Pointers would be nicer but then it would have to be a SfxListener.
    OUString        aLeftStyleName;
    OUString        aRightStyleName;
    SfxStyleFamily  eLeftStyleFamily;
    SfxStyleFamily  eRightStyleFamily;

    bool            bBackward;

public:
    EditUndoConnectParas(EditEngine* pEE, sal_Int32 nNode, sal_uInt16 nSepPos,
                         const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
                         const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, bool bBackward);
    virtual ~EditUndoConnectParas();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoSplitPara

class EditUndoSplitPara : public EditUndo
{
private:
    sal_Int32           nNode;
    sal_uInt16          nSepPos;

public:
    EditUndoSplitPara(EditEngine* pEE, sal_Int32 nNode, sal_uInt16 nSepPos);
    virtual ~EditUndoSplitPara();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoInsertChars

class EditUndoInsertChars : public EditUndo
{
private:
    EPaM            aEPaM;
    OUString        aText;

public:
    EditUndoInsertChars(EditEngine* pEE, const EPaM& rEPaM, const OUString& rStr);

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;
};


// EditUndoRemoveChars

class EditUndoRemoveChars : public EditUndo
{
private:
    EPaM            aEPaM;
    OUString        aText;

public:
    EditUndoRemoveChars(EditEngine* pEE, const EPaM& rEPaM, const OUString& rStr);

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoInsertFeature

class EditUndoInsertFeature : public EditUndo
{
private:
    EPaM            aEPaM;
    SfxPoolItem*    pFeature;

public:
    EditUndoInsertFeature(EditEngine* pEE, const EPaM& rEPaM, const SfxPoolItem& rFeature);
    virtual ~EditUndoInsertFeature();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoMoveParagraphs

class EditUndoMoveParagraphs: public EditUndo
{
private:
    Range           nParagraphs;
    sal_Int32       nDest;

public:
    EditUndoMoveParagraphs(EditEngine* pEE, const Range& rParas, sal_Int32 nDest);
    virtual ~EditUndoMoveParagraphs();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoSetStyleSheet

class EditUndoSetStyleSheet: public EditUndo
{
private:
    sal_Int32       nPara;
    OUString        aPrevName;
    OUString        aNewName;
    SfxStyleFamily  ePrevFamily;
    SfxStyleFamily  eNewFamily;
    SfxItemSet      aPrevParaAttribs;

public:
    EditUndoSetStyleSheet(EditEngine* pEE, sal_Int32 nPara,
        const OUString& rPrevName, SfxStyleFamily ePrevFamily,
        const OUString& rNewName, SfxStyleFamily eNewFamily,
        const SfxItemSet& rPrevParaAttribs);
    virtual ~EditUndoSetStyleSheet();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoSetParaAttribs

class EditUndoSetParaAttribs: public EditUndo
{
private:
    sal_Int32       nPara;
    SfxItemSet      aPrevItems;
    SfxItemSet      aNewItems;

public:
    EditUndoSetParaAttribs(EditEngine* pEE, sal_Int32 nPara, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems);
    virtual ~EditUndoSetParaAttribs();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoSetAttribs

class EditUndoSetAttribs: public EditUndo
{
private:
    typedef std::vector<std::unique_ptr<ContentAttribsInfo> > InfoArrayType;

    ESelection          aESel;
    SfxItemSet          aNewAttribs;
    InfoArrayType       aPrevAttribs;

    sal_uInt8               nSpecial;
    bool                bSetIsRemove;
    bool                bRemoveParaAttribs;
    sal_uInt16              nRemoveWhich;

    void                ImpSetSelection( EditView* pView );


public:
    EditUndoSetAttribs(EditEngine* pEE, const ESelection& rESel, const SfxItemSet& rNewItems);
    virtual ~EditUndoSetAttribs();

    SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }

    void                SetSpecial( sal_uInt8 n )           { nSpecial = n; }
    void                SetRemoveAttribs( bool b )      { bSetIsRemove = b; }
    void                SetRemoveParaAttribs( bool b )  { bRemoveParaAttribs = b; }
    void                SetRemoveWhich( sal_uInt16 n )      { nRemoveWhich = n; }

    virtual void        Undo() override;
    virtual void        Redo() override;

    void AppendContentInfo(ContentAttribsInfo* pNew);
};


// EditUndoTransliteration

class EditUndoTransliteration: public EditUndo
{
private:
    ESelection          aOldESel;
    ESelection          aNewESel;

    sal_Int32           nMode;
    EditTextObject*     pTxtObj;
    OUString            aText;

public:
    EditUndoTransliteration(EditEngine* pEE, const ESelection& rESel, sal_Int32 nMode);
    virtual ~EditUndoTransliteration();

    void                SetText( const OUString& rText ) { aText = rText; }
    void                SetText( EditTextObject* pObj ) { pTxtObj = pObj; }
    void                SetNewSelection( const ESelection& rSel ) { aNewESel = rSel; }

    virtual void        Undo() override;
    virtual void        Redo() override;
};


// EditUndoMarkSelection

class EditUndoMarkSelection: public EditUndo
{
private:
    ESelection      aSelection;

public:
    EditUndoMarkSelection(EditEngine* pEE, const ESelection& rSel);
    virtual ~EditUndoMarkSelection();

    virtual void    Undo() override;
    virtual void    Redo() override;
};


#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EDITUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
