/*************************************************************************
 *
 *  $RCSfile: bastypes.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:08:34 $
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
#ifndef _BASTYPES_HXX
#define _BASTYPES_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#include <iderid.hxx>

class ModulWindow;
class SfxRequest;
class SvTreeListBox;
class SvLBoxEntry;
class ScrollBar;
class SbModule;
class BasicManager;
class SfxItemSet;

#ifndef _SV_DOCKWIN_HXX //autogen
#include <vcl/dockwin.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#include <iderid.hxx>

#include <tools/table.hxx>

#include <svtools/tabbar.hxx>

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif

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
    friend class BasicToolBox;

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

class BasicToolBox : public ToolBox
{
public:
                    BasicToolBox( Window* pParent, IDEResId nRes );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
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

class SfxUndoManager;
class SfxObjectShell;
class BasicEntryDescriptor;

class IDEBaseWindow : public Window
{
private:
    ScrollBar*      pShellHScrollBar;
    ScrollBar*      pShellVScrollBar;

    DECL_LINK( ScrollHdl, ScrollBar * );
    BYTE            nStatus;

    SfxObjectShell* m_pShell;
    String          m_aLibName;
    String          m_aName;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();
                    IDEBaseWindow( Window* pParent, SfxObjectShell* pShell, String aLibName, String aName );
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
    virtual void    PrintData( Printer* pPrinter );
    virtual BOOL    CanClose();

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

    virtual SfxUndoManager* GetUndoManager();

    virtual USHORT  GetSearchOptions();

    virtual void    BasicStarted();
    virtual void    BasicStopped();

    BOOL            IsSuspended() const
                        { return ( nStatus & BASWIN_SUSPENDED ) ? TRUE : FALSE; }

    SfxObjectShell* GetShell() const { return m_pShell; }
    void            SetShell( SfxObjectShell* pShell ) { m_pShell = pShell; }
    const String&   GetLibName() const { return m_aLibName; }
    void            SetLibName( const String& aLibName ) { m_aLibName = aLibName; }
    const String&   GetName() const { return m_aName; }
    void            SetName( const String& aName ) { m_aName = aName; }
};

class LibInfoKey
{
private:
    SfxObjectShell* m_pShell;
    String          m_aLibName;

public:
    LibInfoKey();
    LibInfoKey( SfxObjectShell* pShell, const String& rLibName );
    ~LibInfoKey();

    LibInfoKey( const LibInfoKey& rKey );
    LibInfoKey& operator=( const LibInfoKey& rKey );

    bool operator==( const LibInfoKey& rKey ) const;

    SfxObjectShell* GetShell() const { return m_pShell; }
    const String&   GetLibName() const { return m_aLibName; }
};

class LibInfoItem
{
private:
    SfxObjectShell* m_pShell;
    String          m_aLibName;
    String          m_aCurrentName;
    USHORT          m_nCurrentType;

public:
    LibInfoItem();
    LibInfoItem( SfxObjectShell* pShell, const String& rLibName, const String& rCurrentName, USHORT nCurrentType );
    ~LibInfoItem();

    LibInfoItem( const LibInfoItem& rItem );
    LibInfoItem& operator=( const LibInfoItem& rItem );

    SfxObjectShell* GetShell() const { return m_pShell; }
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
            size_t nHash = (size_t) rKey.GetShell();
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
    void            RemoveInfo( const LibInfoKey& rKey );

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


#endif  // _BASTYPES_HXX
