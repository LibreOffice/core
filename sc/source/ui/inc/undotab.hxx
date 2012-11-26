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



#ifndef SC_UNDOTAB_HXX
#define SC_UNDOTAB_HXX

#include "undobase.hxx"
#include "markdata.hxx"
#include "formula/grammar.hxx"
#include <tools/color.hxx>
#include "tabbgcolor.hxx"

#ifndef _SVSTDARR_SHORTS

#define _SVSTDARR_SHORTS
#include <svl/svstdarr.hxx>

#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif

#include <com/sun/star/uno/Sequence.hxx>

#include <memory>

class ScDocShell;
class ScDocument;
class SdrUndoAction;
class ScPrintRangeSaver;
class SdrObject;
class ScDocProtection;
class ScTableProtection;

//----------------------------------------------------------------------------

class ScUndoInsertTab : public ScSimpleUndo
{
public:
                    ScUndoInsertTab(
                            ScDocShell* pNewDocShell,
                            SCTAB nTabNum,
                            sal_Bool bApp,
                            const String& rNewName);
    virtual         ~ScUndoInsertTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          sNewName;
    SdrUndoAction*  pDrawUndo;
    sal_uLong           nEndChangeAction;
    SCTAB           nTab;
    sal_Bool            bAppend;

    void            SetChangeTrack();
};

class ScUndoInsertTables : public ScSimpleUndo
{
public:
                    ScUndoInsertTables(
                            ScDocShell* pNewDocShell,
                            SCTAB nTabNum,
                            sal_Bool bApp,
                            SvStrings *pNewNameList);
    virtual         ~ScUndoInsertTables();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:

    SdrUndoAction*  pDrawUndo;
    SvStrings*      pNameList;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    SCTAB           nTab;
    sal_Bool            bAppend;

    void            SetChangeTrack();
};


class ScUndoDeleteTab: public ScMoveUndo    // Draw vom Move fuer geloeschte Tabelle
{
public:
                    ScUndoDeleteTab(
                            ScDocShell* pNewDocShell,
                            const SvShorts &theTabs,        //SCTAB nNewTab,
                            ScDocument* pUndoDocument,
                            ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SvShorts    theTabs;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoRenameTab: public ScSimpleUndo
{
public:
                    ScUndoRenameTab(
                            ScDocShell* pNewDocShell,
                            SCTAB nT,
                            const String& rOldName,
                            const String& rNewName);
    virtual         ~ScUndoRenameTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB   nTab;
    String  sOldName;
    String  sNewName;

    void DoChange( SCTAB nTab, const String& rName ) const;
};


class ScUndoMoveTab: public ScSimpleUndo
{
public:
                    ScUndoMoveTab( ScDocShell* pNewDocShell,
                                  const SvShorts &aOldTab,
                                  const SvShorts &aNewTab);
    virtual         ~ScUndoMoveTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SvShorts    theOldTabs;
    SvShorts    theNewTabs;

    void DoChange( sal_Bool bUndo ) const;
};


class ScUndoCopyTab: public ScSimpleUndo
{
public:
                    ScUndoCopyTab(ScDocShell* pNewDocShell,
                                  const SvShorts &aOldTab,
                                  const SvShorts &aNewTab);

    virtual         ~ScUndoCopyTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SdrUndoAction*  pDrawUndo;
    SvShorts    theOldTabs;
    SvShorts    theNewTabs;

    void DoChange() const;
};

class ScUndoTabColor: public ScSimpleUndo
{
public:
                    ScUndoTabColor(
                            ScDocShell* pNewDocShell,
                            SCTAB nT,
                            const Color& aOTabBgColor,
                            const Color& aNTabBgColor);
                    ScUndoTabColor(
                            ScDocShell* pNewDocShell,
                            const ScUndoTabColorInfo::List& rUndoTabColorList);
    virtual         ~ScUndoTabColor();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

virtual String  GetComment() const;

private:
    ScUndoTabColorInfo::List aTabColorList;
    bool    bIsMultipleUndo;

    void DoChange(bool bUndoType) const;
};

class ScUndoMakeScenario: public ScSimpleUndo
{
public:
                    ScUndoMakeScenario(
                            ScDocShell* pNewDocShell,
                            SCTAB nSrc, SCTAB nDest,
                            const String& rN, const String& rC,
                            const Color& rCol, sal_uInt16 nF,
                            const ScMarkData& rMark );
    virtual         ~ScUndoMakeScenario();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB       nSrcTab;
    SCTAB       nDestTab;
    String      aName;
    String      aComment;
    Color       aColor;
    sal_uInt16      nFlags;
    ScMarkData  aMarkData;
    SdrUndoAction* pDrawUndo;
};


class ScUndoImportTab : public ScSimpleUndo
{
public:
                    ScUndoImportTab(
                            ScDocShell* pShell,
                            SCTAB nNewTab, SCTAB nNewCount,
                            sal_Bool bNewLink );
    virtual         ~ScUndoImportTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB       nTab;
    SCTAB       nCount;
    sal_Bool        bLink;
    ScDocument* pRedoDoc;
    SdrUndoAction*  pDrawUndo;

    void DoChange() const;
};


class ScUndoRemoveLink : public ScSimpleUndo
{
public:
                    ScUndoRemoveLink(               // vor dem Loeschen aufrufen!
                            ScDocShell* pShell,
                            const String& rDoc );
    virtual         ~ScUndoRemoveLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String  aDocName;
    String  aFltName;
    String  aOptions;
    sal_uLong   nRefreshDelay;
    sal_uInt16  nCount;
    SCTAB*  pTabs;
    sal_uInt8*  pModes;
    String* pTabNames;

    void DoChange( sal_Bool bLink ) const;
};


class ScUndoShowHideTab : public ScSimpleUndo
{
public:
                    ScUndoShowHideTab(
                            ScDocShell* pShell,
                            SCTAB nNewTab, sal_Bool bNewShow );
    virtual         ~ScUndoShowHideTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB   nTab;
    sal_Bool    bShow;

    void DoChange( sal_Bool bShow ) const;
};

// ============================================================================

/** This class implements undo & redo of document protect & unprotect
    operations. */
class ScUndoDocProtect : public ScSimpleUndo
{
public:
                    ScUndoDocProtect(ScDocShell* pShell, ::std::auto_ptr<ScDocProtection> pProtectSettings);
    virtual         ~ScUndoDocProtect();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ::std::auto_ptr<ScDocProtection> mpProtectSettings;

    void    DoProtect(bool bProtect);
};

// ============================================================================

/** This class implements undo & redo of both protect and unprotect of
    sheet. */
class ScUndoTabProtect : public ScSimpleUndo
{
public:
                    ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab,
                                     ::std::auto_ptr<ScTableProtection> pProtectSettings);
    virtual         ~ScUndoTabProtect();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB   mnTab;
    ::std::auto_ptr<ScTableProtection> mpProtectSettings;

    void    DoProtect(bool bProtect);
};


class ScUndoPrintRange : public ScSimpleUndo
{
public:
                    ScUndoPrintRange( ScDocShell* pShell, SCTAB nNewTab,
                                        ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew );
    virtual         ~ScUndoPrintRange();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB               nTab;
    ScPrintRangeSaver*  pOldRanges;
    ScPrintRangeSaver*  pNewRanges;

    void DoChange( sal_Bool bUndo );
};


class ScUndoScenarioFlags: public ScSimpleUndo
{
public:
                    ScUndoScenarioFlags(
                            ScDocShell* pNewDocShell, SCTAB nT,
                            const String& rON, const String& rNN,
                            const String& rOC, const String& rNC,
                            const Color& rOCol, const Color& rNCol,
                            sal_uInt16 nOF, sal_uInt16 nNF );

    virtual         ~ScUndoScenarioFlags();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB   nTab;
    String  aOldName;
    String  aNewName;
    String  aOldComment;
    String  aNewComment;
    Color   aOldColor;
    Color   aNewColor;
    sal_uInt16  nOldFlags;
    sal_uInt16  nNewFlags;
};


class ScUndoRenameObject: public ScSimpleUndo
{
public:
                    ScUndoRenameObject(
                            ScDocShell* pNewDocShell, const String& rPN,
                            const String& rON, const String& rNN );

    virtual         ~ScUndoRenameObject();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String  aPersistName;       // to find object (works only for OLE objects)
    String  aOldName;
    String  aNewName;

    SdrObject*  GetObject();
};


class ScUndoLayoutRTL : public ScSimpleUndo
{
public:
                    ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, sal_Bool bNewRTL );
    virtual         ~ScUndoLayoutRTL();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB   nTab;
    sal_Bool    bRTL;

    void DoChange( sal_Bool bNew );
};


//UNUSED2009-05 class ScUndoSetGrammar : public ScSimpleUndo
//UNUSED2009-05 {
//UNUSED2009-05 public:
//UNUSED2009-05                     ScUndoSetGrammar( ScDocShell* pShell,
//UNUSED2009-05                                       formula::FormulaGrammar::Grammar eGrammar );
//UNUSED2009-05     virtual         ~ScUndoSetGrammar();
//UNUSED2009-05
//UNUSED2009-05     virtual void    Undo();
//UNUSED2009-05     virtual void    Redo();
//UNUSED2009-05     virtual void    Repeat(SfxRepeatTarget& rTarget);
//UNUSED2009-05     virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;
//UNUSED2009-05
//UNUSED2009-05     virtual String  GetComment() const;
//UNUSED2009-05
//UNUSED2009-05 private:
//UNUSED2009-05     formula::FormulaGrammar::Grammar meNewGrammar, meOldGrammar;
//UNUSED2009-05
//UNUSED2009-05     void DoChange( formula::FormulaGrammar::Grammar eGrammar );
//UNUSED2009-05 };

#endif

