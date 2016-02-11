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
#ifndef INCLUDED_BASCTL_SOURCE_INC_BASTYPES_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASTYPES_HXX

#include "scriptdocument.hxx"

#include <sbxitem.hxx>
#include <iderid.hxx>
#include <svtools/tabbar.hxx>
#include <vcl/toolbox.hxx>

#include <unordered_map>

class SbModule;
class SfxItemSet;
class SfxRequest;
class SvxSearchItem;
class Printer;
enum class SearchOptionFlags;

namespace svl
{
    class IUndoManager;
}

namespace basctl
{

class Layout;
class ModulWindow;
class DialogWindow;

#define LINE_SEP_CR     0x0D
#define LINE_SEP        0x0A

// Implementation: baside2b.cxx
sal_Int32 searchEOL( const OUString& rStr, sal_Int32 fromIndex );

// Meaning of bToBeKilled:
// While being in a reschedule-loop, I may not destroy the window.
// It must first break from the reschedule-loop to self-destroy then.
// Does unfortunately not work that way: Destroying Window with living Child!

struct BasicStatus
{
    bool bIsRunning : 1;
    bool bError : 1;
    bool bIsInReschedule : 1;
    sal_uInt16 nBasicFlags;

    BasicStatus():
        bIsRunning(false),
        bError(false),
        bIsInReschedule(false),
        nBasicFlags(0) { }
};


// basctl::DockingWindow -- special docking window for the Basic IDE
// Not to be confused with ::DockingWindow from vcl.

class DockingWindow : public ::DockingWindow
{
public:
    DockingWindow (vcl::Window* pParent);
    DockingWindow (Layout* pParent);
    virtual ~DockingWindow();
    virtual void dispose() override;
    void ResizeIfDocking (Point const&, Size const&);
    void ResizeIfDocking (Size const&);
    Size GetDockingSize () const { return aDockingRect.GetSize(); }
    void SetLayoutWindow (Layout*);
public:
    void Show (bool = true);
    void Hide ();

protected:
    virtual bool Docking( const Point& rPos, Rectangle& rRect ) override;
    virtual void     EndDocking( const Rectangle& rRect, bool bFloatMode ) override;
    virtual void     ToggleFloatingMode() override;
    virtual bool PrepareToggleFloatingMode() override;
    virtual void     StartDocking() override;

private:
    // the position and the size of the floating window
    Rectangle aFloatingRect;
    // the position and the size of the docking window
    Rectangle aDockingRect;
    // the parent layout window (only when docking)
    VclPtr<Layout> pLayout;
    // > 0: shown, <= 0: hidden, ++ by Show() and -- by Hide()
    int nShowCount;

    static WinBits const StyleBits;

private:
    void DockThis ();
};


// basctl::TabBar
// Not to be confused with ::TabBar from svtools.

class TabBar : public ::TabBar
{
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;

    virtual TabBarAllowRenamingReturnCode  AllowRenaming() override;
    virtual void    EndRenaming() override;

public:
    TabBar (vcl::Window* pParent);

    void            Sort();
};

enum BasicWindowStatus
{
    BASWIN_OK           = 0x00,
    BASWIN_RUNNINGBASIC = 0x01,
    BASWIN_TOBEKILLED   = 0x02,
    BASWIN_SUSPENDED    = 0x04,
    BASWIN_INRESCHEDULE = 0x08
};

class EntryDescriptor;


// BaseWindow -- the base of both ModulWindow and DialogWindow.

class BaseWindow : public vcl::Window
{
private:
    VclPtr<ScrollBar>      pShellHScrollBar;
    VclPtr<ScrollBar>      pShellVScrollBar;

    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    int nStatus;

    ScriptDocument      m_aDocument;
    OUString            m_aLibName;
    OUString            m_aName;

    friend class ModulWindow;
    friend class DialogWindow;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
    BaseWindow( vcl::Window* pParent, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName );
    virtual         ~BaseWindow();
    virtual void    dispose() override;

    void            Init();
    virtual void    DoInit();
    virtual void    Activating () = 0;
    virtual void    Deactivating () = 0;
    void            GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );

    ScrollBar*      GetHScrollBar() const { return pShellHScrollBar; }
    ScrollBar*      GetVScrollBar() const { return pShellVScrollBar; }

    virtual void    ExecuteCommand (SfxRequest&);
    virtual void    ExecuteGlobal (SfxRequest&);
    virtual void    GetState (SfxItemSet&) = 0;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;

    virtual void    StoreData();
    virtual void    UpdateData();
    virtual bool    CanClose();

    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) = 0;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) = 0;

    virtual OUString         GetTitle();
    OUString                 CreateQualifiedName();
    virtual EntryDescriptor  CreateEntryDescriptor() = 0;

    virtual bool    IsModified();
    virtual bool    IsPasteAllowed();

    virtual bool    AllowUndo();

    virtual void    SetReadOnly (bool bReadOnly);
    virtual bool    IsReadOnly();

    int GetStatus() { return nStatus; }
    void SetStatus(int n) { nStatus = n; }
    void AddStatus(int n) { nStatus |= n; }
    void ClearStatus(int n) { nStatus &= ~n; }

    virtual svl::IUndoManager* GetUndoManager ();

    virtual SearchOptionFlags  GetSearchOptions();
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false);

    virtual void    BasicStarted();
    virtual void    BasicStopped();

    bool            IsSuspended() const { return nStatus & BASWIN_SUSPENDED; }

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    bool            IsDocument( const ScriptDocument& rDocument ) const { return rDocument == m_aDocument; }
    const OUString& GetLibName() const { return m_aLibName; }

    const OUString& GetName() const { return m_aName; }
    void            SetName( const OUString& aName ) { m_aName = aName; }

    virtual void OnNewDocument ();
    virtual char const* GetHid () const = 0;
    virtual ItemType GetType () const = 0;
    void InsertLibInfo () const;
    bool Is (ScriptDocument const&, OUString const&, OUString const&, ItemType, bool bFindSuspended);
    virtual bool HasActiveEditor () const;
};

class LibInfo
{
public:
    class Item;
public:
    LibInfo ();
    ~LibInfo ();
public:
    void InsertInfo (ScriptDocument const&, OUString const& rLibName, OUString const& rCurrentName, ItemType eCurrentType);
    void RemoveInfoFor (ScriptDocument const&);
    Item const* GetInfo (ScriptDocument const&, OUString const& rLibName);

private:
    class Key
    {
    private:
        ScriptDocument  m_aDocument;
        OUString        m_aLibName;

    public:
        Key (ScriptDocument const&, OUString const& rLibName);
        ~Key ();
    public:
        bool operator == (Key const&) const;
        struct Hash
        {
            size_t operator () (Key const&) const;
        };
    public:
        const ScriptDocument& GetDocument() const { return m_aDocument; }
    };
public:
    class Item
    {
    private:
        ScriptDocument  m_aDocument;
        OUString        m_aCurrentName;
        ItemType        m_eCurrentType;

    public:
        Item (ScriptDocument const&, OUString const& rCurrentName, ItemType eCurrentType);
        ~Item ();
        const OUString& GetCurrentName()        const { return m_aCurrentName; }
        ItemType        GetCurrentType()        const { return m_eCurrentType; }
    };
private:
    typedef std::unordered_map<Key, Item, Key::Hash> Map;
    Map m_aMap;
};

void            CutLines( OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, bool bEraseTrailingEmptyLines = false );
OUString CreateMgrAndLibStr( const OUString& rMgrName, const OUString& rLibName );
sal_uLong           CalcLineCount( SvStream& rStream );

bool QueryReplaceMacro( const OUString& rName, vcl::Window* pParent = nullptr );
bool QueryDelMacro( const OUString& rName, vcl::Window* pParent = nullptr );
bool QueryDelDialog( const OUString& rName, vcl::Window* pParent = nullptr );
bool QueryDelModule( const OUString& rName, vcl::Window* pParent = nullptr );
bool QueryDelLib( const OUString& rName, bool bRef = false, vcl::Window* pParent = nullptr );
bool QueryPassword( const css::uno::Reference< css::script::XLibraryContainer >& xLibContainer, const OUString& rLibName, OUString& rPassword, bool bRepeat = false, bool bNewTitle = false );

class ModuleInfoHelper
{
    ModuleInfoHelper (const ModuleInfoHelper&) = delete;
    ModuleInfoHelper& operator = (const ModuleInfoHelper&) = delete;
public:
    static void getObjectName( const css::uno::Reference< css::container::XNameContainer >& rLib, const OUString& rModName, OUString& rObjName );
    static sal_Int32 getModuleType(  const css::uno::Reference< css::container::XNameContainer >& rLib, const OUString& rModName );
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_BASTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
