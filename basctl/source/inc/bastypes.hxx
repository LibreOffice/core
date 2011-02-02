/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _BASTYPES_HXX
#define _BASTYPES_HXX

#include <svheader.hxx>

#include <iderid.hxx>

#include "scriptdocument.hxx"

class ModulWindow;
class SfxRequest;
class SvTreeListBox;
class SvLBoxEntry;
class ScrollBar;
class SbModule;
class BasicManager;
class SfxItemSet;
#include <vcl/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <basic/sbstar.hxx>

#include <iderid.hxx>

#include <tools/table.hxx>

#include <svtools/tabbar.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>

#include <hash_map>

#define LINE_SEP_CR     0x0D
#define LINE_SEP        0x0A

// Implementation: baside2b.cxx
sal_Int32 searchEOL( const ::rtl::OUString& rStr, sal_Int32 fromIndex );


// Bedeutung von bToBeKilled:
// Wenn in Reschedule-Schleife, darf ich das Fenster nicht zerstoeren.
// Es muss erst aus der Reschedule-Schleife Fallen, um sich dann
// selbst zu zerstoeren.
// Geht so leider nicht: Destroying Window with living Child!


struct BasicStatus
{
//  BOOL    bCompiled       : 1;
    BOOL    bIsRunning      : 1;
    BOOL    bError          : 1;
    BOOL    bIsInReschedule : 1;
    USHORT  nBasicFlags;

    BasicStatus()   {
            bIsRunning = FALSE; bError = FALSE;
            nBasicFlags = 0; bIsInReschedule = FALSE; }
};

struct BreakPoint
{
    BOOL    bEnabled;
    BOOL    bTemp;
    ULONG   nLine;
    ULONG   nStopAfter;
    ULONG   nHitCount;

    BreakPoint( ULONG nL )  { nLine = nL; nStopAfter = 0; nHitCount = 0; bEnabled = TRUE; bTemp = FALSE; }

};

class BasicDockingWindow : public DockingWindow
{
    Rectangle       aFloatingPosAndSize;

protected:
    virtual BOOL    Docking( const Point& rPos, Rectangle& rRect );
    virtual void    EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    virtual void    ToggleFloatingMode();
    virtual BOOL    PrepareToggleFloatingMode();
    virtual void    StartDocking();

public:
    BasicDockingWindow( Window* pParent );
};

DECLARE_LIST( BreakPL, BreakPoint* )
class BreakPointList : public BreakPL
{
private:
    void operator =(BreakPointList); // not implemented

public:
    BreakPointList();

    BreakPointList(BreakPointList const & rList);

    ~BreakPointList();

    void reset();

    void transfer(BreakPointList & rList);

    void        InsertSorted( BreakPoint* pBrk );
    BreakPoint* FindBreakPoint( ULONG nLine );
    void        AdjustBreakPoints( ULONG nLine, BOOL bInserted );
    void        SetBreakPointsInBasic( SbModule* pModule );
    void        ResetHitCount();
};

// helper class for sorting TabBar
class TabBarSortHelper
{
public:
    USHORT          nPageId;
    String          aPageText;

    bool operator<(const TabBarSortHelper& rComp) const { return (aPageText.CompareIgnoreCaseToAscii( rComp.aPageText ) == COMPARE_LESS); }
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

#define BASWIN_OK               0x00
#define BASWIN_RUNNINGBASIC     0x01
#define BASWIN_TOBEKILLED       0x02
#define BASWIN_SUSPENDED        0x04
#define BASWIN_INRESCHEDULE     0x08

class Printer;
class BasicEntryDescriptor;

namespace svl
{
    class IUndoManager;
}

class IDEBaseWindow : public Window
{
private:
    ScrollBar*      pShellHScrollBar;
    ScrollBar*      pShellVScrollBar;

    DECL_LINK( ScrollHdl, ScrollBar * );
    BYTE            nStatus;

    ScriptDocument      m_aDocument;
    String              m_aLibName;
    String              m_aName;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();
                    IDEBaseWindow( Window* pParent, const ScriptDocument& rDocument, String aLibName, String aName );
    virtual         ~IDEBaseWindow();

    void            Init();
    virtual void    DoInit();
    virtual void    Deactivating();
    void            GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );

    ScrollBar*      GetHScrollBar() const { return pShellHScrollBar; }
    ScrollBar*      GetVScrollBar() const { return pShellVScrollBar; }

    virtual void    ExecuteCommand( SfxRequest& rReq );
    virtual void    GetState( SfxItemSet& );
    virtual long    Notify( NotifyEvent& rNEvt );

    virtual void    StoreData();
    virtual void    UpdateData();
    virtual BOOL    CanClose();

    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) = 0;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) = 0;

    virtual String  GetTitle();
    String          CreateQualifiedName();
    virtual BasicEntryDescriptor CreateEntryDescriptor() = 0;

    virtual BOOL    IsModified();
    virtual BOOL    IsPasteAllowed();

    virtual BOOL    AllowUndo();

    virtual void    SetReadOnly( BOOL bReadOnly );
    virtual BOOL    IsReadOnly();

    BYTE            GetStatus()             { return nStatus; }
    void            SetStatus( BYTE n )     { nStatus = n; }
    void            AddStatus( BYTE n )     { nStatus = nStatus | n; }
    void            ClearStatus( BYTE n )   { nStatus = nStatus & ~n; }

    virtual Window* GetLayoutWindow();

    virtual ::svl::IUndoManager*
                    GetUndoManager();

    virtual USHORT  GetSearchOptions();

    virtual void    BasicStarted();
    virtual void    BasicStopped();

    BOOL            IsSuspended() const
                        { return ( nStatus & BASWIN_SUSPENDED ) ? TRUE : FALSE; }

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }
    bool            IsDocument( const ScriptDocument& rDocument ) const { return rDocument == m_aDocument; }
    const String&   GetLibName() const { return m_aLibName; }
    void            SetLibName( const String& aLibName ) { m_aLibName = aLibName; }
    const String&   GetName() const { return m_aName; }
    void            SetName( const String& aName ) { m_aName = aName; }
};

class LibInfoKey
{
private:
    ScriptDocument      m_aDocument;
    String              m_aLibName;

public:
    LibInfoKey( const ScriptDocument& rDocument, const String& rLibName );
    ~LibInfoKey();

    LibInfoKey( const LibInfoKey& rKey );
    LibInfoKey& operator=( const LibInfoKey& rKey );

    bool operator==( const LibInfoKey& rKey ) const;

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    const String&   GetLibName() const { return m_aLibName; }
};

class LibInfoItem
{
private:
    ScriptDocument      m_aDocument;
    String              m_aLibName;
    String              m_aCurrentName;
    USHORT              m_nCurrentType;

public:
    LibInfoItem( const ScriptDocument& rDocument, const String& rLibName, const String& rCurrentName, USHORT nCurrentType );
    ~LibInfoItem();

    LibInfoItem( const LibInfoItem& rItem );
    LibInfoItem& operator=( const LibInfoItem& rItem );

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    const String&   GetLibName() const { return m_aLibName; }
    const String&   GetCurrentName() const { return m_aCurrentName; }
    USHORT          GetCurrentType() const { return m_nCurrentType; }
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

    typedef ::std::hash_map< LibInfoKey, LibInfoItem*, LibInfoKeyHash, ::std::equal_to< LibInfoKey > > LibInfoMap;
    LibInfoMap  m_aLibInfoMap;

public:
                    LibInfos();
                    ~LibInfos();

    void            InsertInfo( LibInfoItem* pItem );
    void            RemoveInfoFor( const ScriptDocument& _rDocument );

    LibInfoItem*    GetInfo( const LibInfoKey& rKey );
};

void            CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, BOOL bEraseTrailingEmptyLines = FALSE );
String          CreateMgrAndLibStr( const String& rMgrName, const String& rLibName );
ULONG           CalcLineCount( SvStream& rStream );

BOOL            QueryReplaceMacro( const String& rName, Window* pParent = 0 );
BOOL            QueryDelMacro( const String& rName, Window* pParent = 0 );
BOOL            QueryDelDialog( const String& rName, Window* pParent = 0 );
BOOL            QueryDelModule( const String& rName, Window* pParent = 0 );
BOOL            QueryDelLib( const String& rName, BOOL bRef = FALSE, Window* pParent = 0 );
BOOL            QueryPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xLibContainer, const String& rLibName, String& rPassword, BOOL bRepeat = FALSE, BOOL bNewTitle = FALSE );

class ModuleInfoHelper
{
ModuleInfoHelper();
ModuleInfoHelper(const ModuleInfoHelper&);
ModuleInfoHelper& operator = (const ModuleInfoHelper&);
public:
    static void getObjectName( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const String& rModName, String& rObjName );
    static sal_Int32 getModuleType(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const String& rModName );
};
#endif  // _BASTYPES_HXX
