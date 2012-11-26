/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_Bool            bDelObject;
    sal_uInt16          nNode;
    ContentNode*    pContentNode;   // Zeigt auf das gueltige,
                                    // nicht zerstoerte Objekt!

public:
                    EditUndoDelContent( ImpEditEngine* pImpEE, ContentNode* pNode, sal_uInt16 nPortio );
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
    sal_uInt16          nNode;
    sal_uInt16          nSepPos;
    SfxItemSet      aLeftParaAttribs;
    SfxItemSet      aRightParaAttribs;

    // 2 Pointer waeren schoener, aber dann muesste es ein SfxListener sein.
    String          aLeftStyleName;
    String          aRightStyleName;
    SfxStyleFamily  eLeftStyleFamily;
    SfxStyleFamily  eRightStyleFamily;

    sal_Bool            bBackward;

public:
                    EditUndoConnectParas( ImpEditEngine* pImpEE, sal_uInt16 nNode, sal_uInt16 nSepPos,
                                            const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
                                            const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, sal_Bool bBackward );
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
    sal_uInt16          nNode;
    sal_uInt16          nSepPos;

public:
                    EditUndoSplitPara( ImpEditEngine* pImpEE, sal_uInt16 nNode, sal_uInt16 nSepPos );
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
                    EditUndoInsertChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const String& rStr );

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
    sal_uInt16          nDest;

public:
                    EditUndoMoveParagraphs( ImpEditEngine* pImpEE, const Range& rParas, sal_uInt16 nDest );
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
    sal_uInt16          nPara;
    XubString       aPrevName;
    XubString       aNewName;
    SfxStyleFamily  ePrevFamily;
    SfxStyleFamily  eNewFamily;
    SfxItemSet      aPrevParaAttribs;

public:
                    EditUndoSetStyleSheet( ImpEditEngine* pImpEE, sal_uInt16 nPara,
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
    sal_uInt16          nPara;
    SfxItemSet      aPrevItems;
    SfxItemSet      aNewItems;

public:
                    EditUndoSetParaAttribs( ImpEditEngine* pImpEE, sal_uInt16 nPara, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems );
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

    sal_uInt8               nSpecial;
    sal_Bool                bSetIsRemove;
    sal_Bool                bRemoveParaAttribs;
    sal_uInt16              nRemoveWhich;

    void                ImpSetSelection( EditView* pView );


public:
                        EditUndoSetAttribs( ImpEditEngine* pImpEE, const ESelection& rESel, const SfxItemSet& rNewItems );
                        ~EditUndoSetAttribs();

    ContentInfoArray&   GetContentInfos()   { return aPrevAttribs; }
    SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }

    void                SetSpecial( sal_uInt8 n )           { nSpecial = n; }
    void                SetRemoveAttribs( sal_Bool b )      { bSetIsRemove = b; }
    void                SetRemoveParaAttribs( sal_Bool b )  { bRemoveParaAttribs = b; }
    void                SetRemoveWhich( sal_uInt16 n )      { nRemoveWhich = n; }

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
                    EditUndoMarkSelection( ImpEditEngine* pImpEE, const ESelection& rSel );
                    ~EditUndoMarkSelection();

    virtual void    Undo();
    virtual void    Redo();
};


#endif // _EDITUNDO_HXX
