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

namespace basctl
{
    class Layout;
    class ModulWindow;
    class DialogWindow;
}

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

class BasicDockingWindow : public DockingWindow
{
public:
    BasicDockingWindow (Window* pParent);
    BasicDockingWindow (basctl::Layout* pParent);
public:
    void ResizeIfDocking (Point const&, Size const&);
    void SetLayoutWindow (basctl::Layout*);
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
    basctl::Layout* pLayout;
    // > 0: shown, <= 0: hidden, ++ by Show() and -- by Hide()
    int nShowCount;

    static WinBits const StyleBits;

private:
    void DockThis ();
};

class BasicIDETabBar : public TabBar
{
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    virtual long    AllowRenaming();
    virtual void    EndRenaming();

public:
                    BasicIDETabBar( Window* pParent );

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

class Printer;
class BasicEntryDescriptor;

namespace svl
{
    class IUndoManager;
}

//
// IDEBaseWindow -- the base of both ModulWindow and DialogWindow.
//
class IDEBaseWindow : public Window
{
private:
    ScrollBar*      pShellHScrollBar;
    ScrollBar*      pShellVScrollBar;

    DECL_LINK( ScrollHdl, ScrollBar * );
    int nStatus;

    ScriptDocument      m_aDocument;
    ::rtl::OUString     m_aLibName;
    ::rtl::OUString     m_aName;

    friend class basctl::ModulWindow;
    friend class basctl::DialogWindow;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();
    IDEBaseWindow( Window* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName );
    virtual         ~IDEBaseWindow();

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
    virtual BasicEntryDescriptor CreateEntryDescriptor() = 0;

    virtual bool    IsModified();
    virtual bool    IsPasteAllowed();

    virtual bool    AllowUndo();

    virtual void    SetReadOnly (bool bReadOnly);
    virtual bool    IsReadOnly();

    int GetStatus() { return nStatus; }
    void SetStatus(int n) { nStatus = n; }
    void AddStatus(int n) { nStatus |= n; }
    void ClearStatus(int n) { nStatus &= ~n; }

    virtual ::svl::IUndoManager*
                    GetUndoManager();

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
    virtual BasicIDEType GetType () const = 0;
    void InsertLibInfo () const;
    bool Is (ScriptDocument const&, rtl::OUString const&, rtl::OUString const&, BasicIDEType, bool bFindSuspended);
    virtual bool HasActiveEditor () const;
};

class LibInfoKey
{
private:
    ScriptDocument  m_aDocument;
    ::rtl::OUString m_aLibName;

public:
    LibInfoKey( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName );
    ~LibInfoKey();

    bool operator==( const LibInfoKey& rKey ) const;

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    const ::rtl::OUString& GetLibName() const { return m_aLibName; }
};

class LibInfoItem
{
private:
    ScriptDocument  m_aDocument;
    ::rtl::OUString m_aLibName;
    ::rtl::OUString m_aCurrentName;
    sal_uInt16      m_nCurrentType;

public:
    LibInfoItem( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rCurrentName, sal_uInt16 nCurrentType );
    ~LibInfoItem();

    LibInfoItem( const LibInfoItem& rItem );
    LibInfoItem& operator=( const LibInfoItem& rItem );

    const ScriptDocument&  GetDocument()    const { return m_aDocument; }
    const ::rtl::OUString& GetLibName()     const { return m_aLibName; }
    const ::rtl::OUString& GetCurrentName() const { return m_aCurrentName; }
    sal_uInt16             GetCurrentType() const { return m_nCurrentType; }
};

class LibInfos
{
private:

    struct LibInfoKeyHash
    {
        size_t operator()( const LibInfoKey& rKey ) const
        {
            size_t nHash = (size_t) rKey.GetDocument().hashCode();
            nHash += (size_t) ::rtl::OUString( rKey.GetLibName() ).hashCode();
            return nHash;
        }
    };

    typedef ::boost::unordered_map< LibInfoKey, LibInfoItem*, LibInfoKeyHash, ::std::equal_to< LibInfoKey > > LibInfoMap;
    LibInfoMap  m_aLibInfoMap;

public:
                    LibInfos();
                    ~LibInfos();

    void            InsertInfo( LibInfoItem* pItem );
    void            RemoveInfoFor( const ScriptDocument& _rDocument );

    LibInfoItem*    GetInfo( const LibInfoKey& rKey );
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
ModuleInfoHelper();
ModuleInfoHelper(const ModuleInfoHelper&);
ModuleInfoHelper& operator = (const ModuleInfoHelper&);
public:
    static void getObjectName( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName, ::rtl::OUString& rObjName );
    static sal_Int32 getModuleType(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName );
};

#endif  // BASCTL_BASTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
