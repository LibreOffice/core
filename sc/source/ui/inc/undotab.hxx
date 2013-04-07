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

#ifndef SC_UNDOTAB_HXX
#define SC_UNDOTAB_HXX

#include "undobase.hxx"
#include "formula/grammar.hxx"
#include <tools/color.hxx>
#include "tabbgcolor.hxx"

#include <com/sun/star/uno/Sequence.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

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

//----------------------------------------------------------------------------

class ScUndoInsertTab : public ScSimpleUndo
{
public:
                    TYPEINFO();
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

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoInsertTables(
                            ScDocShell* pNewDocShell,
                            SCTAB nTabNum,
                            std::vector<OUString>& newNameList);
    virtual         ~ScUndoInsertTables();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoDeleteTab(
                            ScDocShell* pNewDocShell,
                            const std::vector<SCTAB> &theTabs,      //SCTAB nNewTab,
                            ScDocument* pUndoDocument,
                            ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    std::vector<SCTAB> theTabs;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoRenameTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
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

    virtual OUString GetComment() const;

private:
    SCTAB   nTab;
    String  sOldName;
    String  sNewName;

    void DoChange( SCTAB nTab, const String& rName ) const;
};


class ScUndoMoveTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoMoveTab(
                        ScDocShell* pNewDocShell,
                        ::std::vector<SCTAB>* pOldTabs,
                        ::std::vector<SCTAB>* pNewTabs,
                        ::std::vector< OUString>* pOldNames = NULL,
                        ::std::vector< OUString>* pNewNames = NULL );

    virtual         ~ScUndoMoveTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ::boost::shared_ptr< ::std::vector<SCTAB> > mpOldTabs;
    ::boost::shared_ptr< ::std::vector<SCTAB> > mpNewTabs;
    ::boost::shared_ptr< ::std::vector< OUString> > mpOldNames;
    ::boost::shared_ptr< ::std::vector< OUString> > mpNewNames;

    void DoChange( sal_Bool bUndo ) const;
};


class ScUndoCopyTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoCopyTab(
                        ScDocShell* pNewDocShell,
                        ::std::vector<SCTAB>* pOldTabs,
                        ::std::vector<SCTAB>* pNewTabs,
                        ::std::vector< OUString>* pNewNames = NULL );

    virtual         ~ScUndoCopyTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ::boost::shared_ptr< ::std::vector<SCTAB> > mpOldTabs;
    ::boost::shared_ptr< ::std::vector<SCTAB> > mpNewTabs;
    ::boost::shared_ptr< ::std::vector< OUString> > mpNewNames;
    SdrUndoAction*  pDrawUndo;

    void DoChange() const;
};

class ScUndoTabColor: public ScSimpleUndo
{
public:
                    TYPEINFO();
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

    virtual OUString GetComment() const;

private:
    ScUndoTabColorInfo::List aTabColorList;

    void DoChange(bool bUndoType) const;
};

class ScUndoMakeScenario: public ScSimpleUndo
{
public:
                    TYPEINFO();
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

    virtual OUString GetComment() const;

private:
    boost::scoped_ptr<ScMarkData> mpMarkData;
    SCTAB       nSrcTab;
    SCTAB       nDestTab;
    String      aName;
    String      aComment;
    Color       aColor;
    sal_uInt16      nFlags;
    SdrUndoAction* pDrawUndo;
};


class ScUndoImportTab : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoImportTab(
                            ScDocShell* pShell,
                            SCTAB nNewTab, SCTAB nNewCount );
    virtual         ~ScUndoImportTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoRemoveLink(               // Call before delete!
                            ScDocShell* pShell,
                            const String& rDoc );
    virtual         ~ScUndoRemoveLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoShowHideTab(
                            ScDocShell* pShell,
                            const std::vector<SCTAB>& newUndoTabs,
                            sal_Bool bNewShow );
    virtual         ~ScUndoShowHideTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    std::vector<SCTAB>  undoTabs;
    sal_Bool            bShow;

    void DoChange( sal_Bool bShow ) const;
};

// ============================================================================

/** This class implements undo & redo of document protect & unprotect
    operations. */
class ScUndoDocProtect : public ScSimpleUndo
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ScUndoDocProtect(ScDocShell* pShell, ::std::auto_ptr<ScDocProtection> pProtectSettings);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual         ~ScUndoDocProtect();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDocProtection> mpProtectSettings;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void    DoProtect(bool bProtect);
};

// ============================================================================

/** This class implements undo & redo of both protect and unprotect of
    sheet. */
class ScUndoTabProtect : public ScSimpleUndo
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab,
                                     ::std::auto_ptr<ScTableProtection> pProtectSettings);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual         ~ScUndoTabProtect();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    SCTAB   mnTab;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScTableProtection> mpProtectSettings;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void    DoProtect(bool bProtect);
};


class ScUndoPrintRange : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPrintRange( ScDocShell* pShell, SCTAB nNewTab,
                                        ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew );
    virtual         ~ScUndoPrintRange();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    SCTAB               nTab;
    ScPrintRangeSaver*  pOldRanges;
    ScPrintRangeSaver*  pNewRanges;

    void DoChange( sal_Bool bUndo );
};


class ScUndoScenarioFlags: public ScSimpleUndo
{
public:
                    TYPEINFO();
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

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoRenameObject(
                            ScDocShell* pNewDocShell, const String& rPN,
                            const String& rON, const String& rNN );

    virtual         ~ScUndoRenameObject();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    String  aPersistName;       // to find object (works only for OLE objects)
    String  aOldName;
    String  aNewName;

    SdrObject*  GetObject();
};


class ScUndoLayoutRTL : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, sal_Bool bNewRTL );
    virtual         ~ScUndoLayoutRTL();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    SCTAB   nTab;
    sal_Bool    bRTL;

    void DoChange( sal_Bool bNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
