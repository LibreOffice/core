/*************************************************************************
 *
 *  $RCSfile: undotab.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-22 18:12:14 $
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

#ifndef SC_UNDOTAB_HXX
#define SC_UNDOTAB_HXX

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif
#ifndef SC_MARKDATA_HXX
#include "markdata.hxx"
#endif

#ifndef _SVSTDARR_USHORTS

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#endif

class ScDocShell;
class ScDocument;
class SdrUndoAction;
class ScPrintRangeSaver;

//----------------------------------------------------------------------------

class ScUndoInsertTab : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertTab(
                            ScDocShell* pNewDocShell,
                            USHORT nTabNum,
                            BOOL bApp,
                            const String& rNewName);
    virtual         ~ScUndoInsertTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          sNewName;
    SdrUndoAction*  pDrawUndo;
    ULONG           nEndChangeAction;
    USHORT          nTab;
    BOOL            bAppend;

    void            SetChangeTrack();
};

class ScUndoInsertTables : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertTables(
                            ScDocShell* pNewDocShell,
                            USHORT nTabNum,
                            BOOL bApp,
                            SvStrings *pNewNameList);
    virtual         ~ScUndoInsertTables();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:

    SdrUndoAction*  pDrawUndo;
    SvStrings*      pNameList;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    USHORT          nTab;
    BOOL            bAppend;

    void            SetChangeTrack();
};


class ScUndoDeleteTab: public ScMoveUndo    // Draw vom Move fuer geloeschte Tabelle
{
public:
                    TYPEINFO();
                    ScUndoDeleteTab(
                            ScDocShell* pNewDocShell,
                            const SvUShorts &theTabs,       //USHORT nNewTab,
                            ScDocument* pUndoDocument,
                            ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SvUShorts   theTabs;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoRenameTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRenameTab(
                            ScDocShell* pNewDocShell,
                            USHORT nT,
                            const String& rOldName,
                            const String& rNewName);
    virtual         ~ScUndoRenameTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT  nTab;
    String  sOldName;
    String  sNewName;

    void DoChange( USHORT nTab, const String& rName ) const;
};


class ScUndoMoveTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoMoveTab( ScDocShell* pNewDocShell,
                                  const SvUShorts &aOldTab,
                                  const SvUShorts &aNewTab);
    virtual         ~ScUndoMoveTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SvUShorts   theOldTabs;
    SvUShorts   theNewTabs;

    void DoChange( BOOL bUndo ) const;
};


class ScUndoCopyTab: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoCopyTab(ScDocShell* pNewDocShell,
                                  const SvUShorts &aOldTab,
                                  const SvUShorts &aNewTab);

    virtual         ~ScUndoCopyTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SdrUndoAction*  pDrawUndo;
    SvUShorts   theOldTabs;
    SvUShorts   theNewTabs;

    void DoChange() const;
};


class ScUndoMakeScenario: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoMakeScenario(
                            ScDocShell* pNewDocShell,
                            USHORT nSrc, USHORT nDest,
                            ScDocument* pUndo,
                            const String& rN, const String& rC,
                            const Color& rCol, USHORT nF,
                            const ScMarkData& rMark );
    virtual         ~ScUndoMakeScenario();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT      nSrcTab;
    USHORT      nDestTab;
    ScDocument* pUndoDoc;
    String      aName;
    String      aComment;
    Color       aColor;
    USHORT      nFlags;
    ScMarkData  aMarkData;
};


class ScUndoImportTab : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoImportTab(
                            ScDocShell* pShell,
                            USHORT nNewTab, USHORT nNewCount,
                            BOOL bNewLink );
    virtual         ~ScUndoImportTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT      nTab;
    USHORT      nCount;
    BOOL        bLink;
    ScDocument* pRedoDoc;
    SdrUndoAction*  pDrawUndo;

    void DoChange() const;
};


class ScUndoRemoveLink : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveLink(               // vor dem Loeschen aufrufen!
                            ScDocShell* pShell,
                            const String& rDoc );
    virtual         ~ScUndoRemoveLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String  aDocName;
    String  aFltName;
    String  aOptions;
    USHORT  nCount;
    USHORT* pTabs;
    BYTE*   pModes;
    String* pTabNames;

    void DoChange( BOOL bLink ) const;
};


class ScUndoShowHideTab : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoShowHideTab(
                            ScDocShell* pShell,
                            USHORT nNewTab, BOOL bNewShow );
    virtual         ~ScUndoShowHideTab();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT  nTab;
    BOOL    bShow;

    void DoChange( BOOL bShow ) const;
};


class ScUndoProtect : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoProtect( ScDocShell* pShell, USHORT nNewTab,
                                    BOOL bNewProtect, const com::sun::star::uno::Sequence<sal_uInt8>& rNewPassword );
    virtual         ~ScUndoProtect();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT  nTab;
    BOOL    bProtect;
    com::sun::star::uno::Sequence<sal_uInt8>    aPassword;

    void    DoProtect( BOOL bDo );
};


class ScUndoPrintRange : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPrintRange( ScDocShell* pShell, USHORT nNewTab,
                                        ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew );
    virtual         ~ScUndoPrintRange();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT              nTab;
    ScPrintRangeSaver*  pOldRanges;
    ScPrintRangeSaver*  pNewRanges;

    void DoChange( BOOL bUndo );
};


class ScUndoScenarioFlags: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoScenarioFlags(
                            ScDocShell* pNewDocShell, USHORT nT,
                            const String& rON, const String& rNN,
                            const String& rOC, const String& rNC,
                            const Color& rOCol, const Color& rNCol,
                            USHORT nOF, USHORT nNF );

    virtual         ~ScUndoScenarioFlags();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT  nTab;
    String  aOldName;
    String  aNewName;
    String  aOldComment;
    String  aNewComment;
    Color   aOldColor;
    Color   aNewColor;
    USHORT  nOldFlags;
    USHORT  nNewFlags;
};




#endif

