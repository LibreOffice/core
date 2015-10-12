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

#ifndef INCLUDED_SC_SOURCE_UI_INC_UNDOTAB_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNDOTAB_HXX

#include "undobase.hxx"
#include <formula/grammar.hxx>
#include <tools/color.hxx>
#include "tabbgcolor.hxx"

#include <com/sun/star/uno/Sequence.hxx>

#include <memory>

#include <memory>
#include <vector>

class ScDocShell;
class ScDocument;
class SdrUndoAction;
class ScPrintRangeSaver;
class SdrObject;
class ScDocProtection;
class ScTableProtection;
class ScMarkData;

class ScUndoInsertTab : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoInsertTab(
                            ScDocShell* pNewDocShell,
                            SCTAB nTabNum,
                            bool bApp,
                            const OUString& rNewName);
    virtual         ~ScUndoInsertTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString        sNewName;
    SdrUndoAction*  pDrawUndo;
    sal_uLong       nEndChangeAction;
    SCTAB           nTab;
    bool            bAppend;

    void            SetChangeTrack();
};

class ScUndoInsertTables : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoInsertTables(
                            ScDocShell* pNewDocShell,
                            SCTAB nTabNum,
                            std::vector<OUString>& newNameList);
    virtual         ~ScUndoInsertTables();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:

    SdrUndoAction*  pDrawUndo;
    std::vector<OUString>      aNameList;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    SCTAB           nTab;

    void            SetChangeTrack();
};

class ScUndoDeleteTab: public ScMoveUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoDeleteTab(
                            ScDocShell* pNewDocShell,
                            const std::vector<SCTAB> &theTabs,      //SCTAB nNewTab,
                            ScDocument* pUndoDocument,
                            ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::vector<SCTAB> theTabs;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};

class ScUndoRenameTab: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoRenameTab(
                            ScDocShell* pNewDocShell,
                            SCTAB nT,
                            const OUString& rOldName,
                            const OUString& rNewName);
    virtual         ~ScUndoRenameTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB     nTab;
    OUString  sOldName;
    OUString  sNewName;

    void DoChange( SCTAB nTab, const OUString& rName ) const;
};

class ScUndoMoveTab: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoMoveTab(
                        ScDocShell* pNewDocShell,
                        ::std::vector<SCTAB>* pOldTabs,
                        ::std::vector<SCTAB>* pNewTabs,
                        ::std::vector< OUString>* pOldNames = NULL,
                        ::std::vector< OUString>* pNewNames = NULL );

    virtual         ~ScUndoMoveTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::shared_ptr< ::std::vector<SCTAB> > mpOldTabs;
    std::shared_ptr< ::std::vector<SCTAB> > mpNewTabs;
    std::shared_ptr< ::std::vector< OUString> > mpOldNames;
    std::shared_ptr< ::std::vector< OUString> > mpNewNames;

    void DoChange( bool bUndo ) const;
};

class ScUndoCopyTab: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoCopyTab(
                        ScDocShell* pNewDocShell,
                        ::std::vector<SCTAB>* pOldTabs,
                        ::std::vector<SCTAB>* pNewTabs,
                        ::std::vector< OUString>* pNewNames = NULL );

    virtual         ~ScUndoCopyTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::shared_ptr< ::std::vector<SCTAB> > mpOldTabs;
    std::shared_ptr< ::std::vector<SCTAB> > mpNewTabs;
    std::shared_ptr< ::std::vector< OUString> > mpNewNames;
    SdrUndoAction*  pDrawUndo;

    void DoChange() const;
};

class ScUndoTabColor: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoTabColor(
                            ScDocShell* pNewDocShell,
                            SCTAB nT,
                            const Color& aOTabBgColor,
                            const Color& aNTabBgColor);
                    ScUndoTabColor(
                            ScDocShell* pNewDocShell,
                            const ScUndoTabColorInfo::List& rUndoTabColorList);
    virtual         ~ScUndoTabColor();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScUndoTabColorInfo::List aTabColorList;

    void DoChange(bool bUndoType) const;
};

class ScUndoMakeScenario: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoMakeScenario(
                            ScDocShell* pNewDocShell,
                            SCTAB nSrc, SCTAB nDest,
                            const OUString& rN, const OUString& rC,
                            const Color& rCol, sal_uInt16 nF,
                            const ScMarkData& rMark );
    virtual         ~ScUndoMakeScenario();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<ScMarkData> mpMarkData;
    SCTAB       nSrcTab;
    SCTAB       nDestTab;
    OUString    aName;
    OUString    aComment;
    Color       aColor;
    sal_uInt16      nFlags;
    SdrUndoAction* pDrawUndo;
};

class ScUndoImportTab : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoImportTab(
                            ScDocShell* pShell,
                            SCTAB nNewTab, SCTAB nNewCount );
    virtual         ~ScUndoImportTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB       nTab;
    SCTAB       nCount;
    ScDocument* pRedoDoc;
    SdrUndoAction*  pDrawUndo;

    void DoChange() const;
};

class ScUndoRemoveLink : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoRemoveLink(               // Call before delete!
                            ScDocShell* pShell,
                            const OUString& rDoc );
    virtual         ~ScUndoRemoveLink();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString    aDocName;
    OUString    aFltName;
    OUString    aOptions;
    sal_uLong   nRefreshDelay;
    sal_uInt16  nCount;
    SCTAB*      pTabs;
    ScLinkMode* pModes;
    OUString*   pTabNames;

    void DoChange( bool bLink ) const;
};

class ScUndoShowHideTab : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoShowHideTab(
                            ScDocShell* pShell,
                            const std::vector<SCTAB>& newUndoTabs,
                            bool bNewShow );
    virtual         ~ScUndoShowHideTab();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::vector<SCTAB>  undoTabs;
    bool                bShow;

    void DoChange( bool bShow ) const;
};

/** This class implements undo & redo of document protect & unprotect
    operations. */
class ScUndoDocProtect : public ScSimpleUndo
{
public:
                    ScUndoDocProtect(ScDocShell* pShell, ::std::unique_ptr<ScDocProtection> && pProtectSettings);
    virtual         ~ScUndoDocProtect();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ::std::unique_ptr<ScDocProtection> mpProtectSettings;

    void    DoProtect(bool bProtect);
};

/** This class implements undo & redo of both protect and unprotect of
    sheet. */
class ScUndoTabProtect : public ScSimpleUndo
{
public:
                    ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab,
                                     std::unique_ptr<ScTableProtection> && pProtectSettings);
    virtual         ~ScUndoTabProtect();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB   mnTab;
    ::std::unique_ptr<ScTableProtection> mpProtectSettings;

    void    DoProtect(bool bProtect);
};

class ScUndoPrintRange : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoPrintRange( ScDocShell* pShell, SCTAB nNewTab,
                                        ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew );
    virtual         ~ScUndoPrintRange();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB               nTab;
    ScPrintRangeSaver*  pOldRanges;
    ScPrintRangeSaver*  pNewRanges;

    void DoChange( bool bUndo );
};

class ScUndoScenarioFlags: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoScenarioFlags(
                            ScDocShell* pNewDocShell, SCTAB nT,
                            const OUString& rON, const OUString& rNN,
                            const OUString& rOC, const OUString& rNC,
                            const Color& rOCol, const Color& rNCol,
                            sal_uInt16 nOF, sal_uInt16 nNF );

    virtual         ~ScUndoScenarioFlags();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB       nTab;
    OUString    aOldName;
    OUString    aNewName;
    OUString    aOldComment;
    OUString    aNewComment;
    Color       aOldColor;
    Color       aNewColor;
    sal_uInt16  nOldFlags;
    sal_uInt16  nNewFlags;
};

class ScUndoRenameObject: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoRenameObject(
                            ScDocShell* pNewDocShell, const OUString& rPN,
                            const OUString& rON, const OUString& rNN );

    virtual         ~ScUndoRenameObject();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString  aPersistName;       // to find object (works only for OLE objects)
    OUString  aOldName;
    OUString  aNewName;

    SdrObject*  GetObject();
};

class ScUndoLayoutRTL : public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, bool bNewRTL );
    virtual         ~ScUndoLayoutRTL();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB   nTab;
    bool    bRTL;

    void DoChange( bool bNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
