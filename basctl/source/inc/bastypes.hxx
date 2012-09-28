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
#ifndef BASCTL_BASTYPES_HXX
#define BASCTL_BASTYPES_HXX

#include "scriptdocument.hxx"

#include <sbxitem.hxx>
#include <iderid.hxx>
#include <svtools/tabbar.hxx>
#include <vcl/toolbox.hxx>

#include <boost/unordered_map.hpp>

class SbModule;
class SfxItemSet;
class SfxRequest;
class SvxSearchItem;
class Printer;

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
sal_Int32 searchEOL( const ::rtl::OUString& rStr, sal_Int32 fromIndex );

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

//
// basctl::DockingWindow -- special docking window for the Basic IDE
// Not to be confused with ::DockingWindow from vcl.
//
class DockingWindow : public ::DockingWindow
{
public:
    DockingWindow (Window* pParent);
    DockingWindow (Layout* pParent);
public:
    void ResizeIfDocking (Point const&, Size const&);
    void ResizeIfDocking (Size const&);
    Size GetDockingSize () const { return aDockingRect.GetSize(); }
    void SetLayoutWindow (Layout*);
public:
    void Show (bool = true);
    void Hide ();

protected:
    virtual sal_Bool Docking( const Point& rPos, Rectangle& rRect );
    virtual void     EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );
    virtual void     ToggleFloatingMode();
    virtual sal_Bool PrepareToggleFloatingMode();
    virtual void     StartDocking();

private:
    // the position and the size of the floating window
    Rectangle aFloatingRect;
    // the position and the size of the docking window
    Rectangle aDockingRect;
    // the parent layout window (only when docking)
    Layout* pLayout;
    // > 0: shown, <= 0: hidden, ++ by Show() and -- by Hide()
    int nShowCount;

    static WinBits const StyleBits;

private:
    void DockThis ();
};

//
// basctl::TabBar
// Not to be confused with ::TabBar from svtools.
//
class TabBar : public ::TabBar
{
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    virtual long    AllowRenaming();
    virtual void    EndRenaming();

public:
    TabBar (Window* pParent);

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

//
// BaseWindow -- the base of both ModulWindow and DialogWindow.
//
class BaseWindow : public Window
{
private:
    ScrollBar*      pShellHScrollBar;
    ScrollBar*      pShellVScrollBar;

    DECL_LINK( ScrollHdl, ScrollBar * );
    int nStatus;

    ScriptDocument      m_aDocument;
    rtl::OUString     m_aLibName;
    rtl::OUString     m_aName;

    friend class ModulWindow;
    friend class DialogWindow;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();
    BaseWindow( Window* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName );
    virtual         ~BaseWindow();

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
    virtual long    Notify( NotifyEvent& rNEvt );

    virtual void    StoreData();
    virtual void    UpdateData();
    virtual bool    CanClose();

    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) = 0;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) = 0;

    virtual ::rtl::OUString  GetTitle();
    ::rtl::OUString          CreateQualifiedName();
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

    virtual sal_uInt16  GetSearchOptions();
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false);

    virtual void    BasicStarted();
    virtual void    BasicStopped();

    bool            IsSuspended() const { return nStatus & BASWIN_SUSPENDED; }

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }
    bool            IsDocument( const ScriptDocument& rDocument ) const { return rDocument == m_aDocument; }
    const ::rtl::OUString&   GetLibName() const { return m_aLibName; }
    void            SetLibName( const ::rtl::OUString& aLibName ) { m_aLibName = aLibName; }
    const ::rtl::OUString&   GetName() const { return m_aName; }
    void            SetName( const ::rtl::OUString& aName ) { m_aName = aName; }

    virtual void OnNewDocument ();
    virtual char const* GetHid () const = 0;
    virtual ItemType GetType () const = 0;
    void InsertLibInfo () const;
    bool Is (ScriptDocument const&, rtl::OUString const&, rtl::OUString const&, ItemType, bool bFindSuspended);
    virtual bool HasActiveEditor () const;
};

class LibInfos
{
public:
    class Item;
public:
    LibInfos ();
    ~LibInfos ();
public:
    void InsertInfo (ScriptDocument const&, rtl::OUString const& rLibName, rtl::OUString const& rCurrentName, ItemType eCurrentType);
    void RemoveInfoFor (ScriptDocument const&);
    Item const* GetInfo (ScriptDocument const&, rtl::OUString const& rLibName);

private:
    class Key
    {
    private:
        ScriptDocument  m_aDocument;
        ::rtl::OUString m_aLibName;

    public:
        Key (ScriptDocument const&, rtl::OUString const& rLibName);
        ~Key ();
    public:
        bool operator == (Key const&) const;
        struct Hash
        {
            size_t operator () (Key const&) const;
        };
    public:
        const ScriptDocument& GetDocument() const { return m_aDocument; }
        const ::rtl::OUString& GetLibName() const { return m_aLibName; }
    };
public:
    class Item
    {
    private:
        ScriptDocument  m_aDocument;
        ::rtl::OUString m_aLibName;
        ::rtl::OUString m_aCurrentName;
        ItemType        m_eCurrentType;

    public:
        Item (ScriptDocument const&, rtl::OUString const& rLibName, rtl::OUString const& rCurrentName, ItemType eCurrentType);
        ~Item ();
    public:
        const ScriptDocument&  GetDocument()    const { return m_aDocument; }
        const ::rtl::OUString& GetLibName()     const { return m_aLibName; }
        const ::rtl::OUString& GetCurrentName() const { return m_aCurrentName; }
        ItemType               GetCurrentType() const { return m_eCurrentType; }
    };
private:
    typedef boost::unordered_map<Key, Item, Key::Hash> Map;
    Map m_aMap;
};

void            CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, bool bEraseTrailingEmptyLines = false );
::rtl::OUString CreateMgrAndLibStr( const ::rtl::OUString& rMgrName, const ::rtl::OUString& rLibName );
sal_uLong           CalcLineCount( SvStream& rStream );

bool QueryReplaceMacro( const ::rtl::OUString& rName, Window* pParent = 0 );
bool QueryDelMacro( const ::rtl::OUString& rName, Window* pParent = 0 );
bool QueryDelDialog( const ::rtl::OUString& rName, Window* pParent = 0 );
bool QueryDelModule( const ::rtl::OUString& rName, Window* pParent = 0 );
bool QueryDelLib( const ::rtl::OUString& rName, bool bRef = false, Window* pParent = 0 );
bool QueryPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xLibContainer, const ::rtl::OUString& rLibName, ::rtl::OUString& rPassword, bool bRepeat = false, bool bNewTitle = false );

class ModuleInfoHelper
{
    // non-constructible class
    ModuleInfoHelper ();
    ModuleInfoHelper (const ModuleInfoHelper&);
    ModuleInfoHelper& operator = (const ModuleInfoHelper&);
public:
    static void getObjectName( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName, ::rtl::OUString& rObjName );
    static sal_Int32 getModuleType(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName );
};

} // namespace basctl

#endif // BASCTL_BASTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
