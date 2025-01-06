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

#pragma once

#include <editdoc.hxx>
#include <editeng/editund2.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editobj.hxx>
#include <vector>
#include <memory>

class EditEngine;
enum class SetAttribsMode;
enum class TransliterationFlags;

// EditUndoDelContent

class EditUndoDelContent : public EditUndo
{
private:
    sal_Int32 nNode;
    std::unique_ptr<ContentNode> mpContentNode;   // Points to the valid, undestroyed object!
public:
    EditUndoDelContent(EditEngine* pEE, std::unique_ptr<ContentNode> pNode, sal_Int32 nPortion);
    virtual ~EditUndoDelContent() override;

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
                         SfxItemSet aLeftParaAttribs, SfxItemSet aRightParaAttribs,
                         const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, bool bBackward);
    virtual ~EditUndoConnectParas() override;

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
    virtual ~EditUndoSplitPara() override;

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
    EditUndoInsertChars(EditEngine* pEE, const EPaM& rEPaM, OUString aStr);

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
    EditUndoRemoveChars(EditEngine* pEE, const EPaM& rEPaM, OUString aStr);

    virtual void    Undo() override;
    virtual void    Redo() override;
};


// EditUndoInsertFeature

class EditUndoInsertFeature : public EditUndo
{
private:
    EPaM                           aEPaM;
    std::unique_ptr<SfxPoolItem>   pFeature;

public:
    EditUndoInsertFeature(EditEngine* pEE, const EPaM& rEPaM, const SfxPoolItem& rFeature);
    virtual ~EditUndoInsertFeature() override;

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
    virtual ~EditUndoMoveParagraphs() override;

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
        OUString aPrevName, SfxStyleFamily ePrevFamily,
        OUString aNewName, SfxStyleFamily eNewFamily,
        SfxItemSet aPrevParaAttribs);
    virtual ~EditUndoSetStyleSheet() override;

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
    EditUndoSetParaAttribs(EditEngine* pEE, sal_Int32 nPara, SfxItemSet aPrevItems, SfxItemSet aNewItems);
    virtual ~EditUndoSetParaAttribs() override;

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

    SetAttribsMode      nSpecial;
    /// Once the attributes are set / unset, set the selection to the end of the formatted range?
    bool m_bSetSelection;
    bool                bSetIsRemove;
    bool                bRemoveParaAttribs;
    sal_uInt16          nRemoveWhich;

    void                ImpSetSelection();


public:
    EditUndoSetAttribs(EditEngine* pEE, const ESelection& rESel, SfxItemSet aNewItems);
    virtual ~EditUndoSetAttribs() override;

    SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }

    void                SetSpecial( SetAttribsMode n )  { nSpecial = n; }
    void                SetUpdateSelection( bool bSetSelection )  { m_bSetSelection = bSetSelection; }
    void                SetRemoveAttribs( bool b )      { bSetIsRemove = b; }
    void                SetRemoveParaAttribs( bool b )  { bRemoveParaAttribs = b; }
    void                SetRemoveWhich( sal_uInt16 n )  { nRemoveWhich = n; }

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

    TransliterationFlags
                        nMode;
    std::unique_ptr<EditTextObject>
                        pTxtObj;
    OUString            aText;

public:
    EditUndoTransliteration(EditEngine* pEE, const ESelection& rESel, TransliterationFlags nMode);
    virtual ~EditUndoTransliteration() override;

    void                SetText( const OUString& rText ) { aText = rText; }
    void                SetText( std::unique_ptr<EditTextObject> pObj ) { pTxtObj = std::move( pObj ); }
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
    virtual ~EditUndoMarkSelection() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
