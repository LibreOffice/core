/*************************************************************************
 *
 *  $RCSfile: content.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
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

#ifndef SC_CONTENT_HXX
#define SC_CONTENT_HXX

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScNavigatorDlg;
class ScDocument;
class ScDocShell;
class ScAreaLink;

#define SC_CONTENT_ROOT         0
#define SC_CONTENT_TABLE        1
#define SC_CONTENT_RANGENAME    2
#define SC_CONTENT_DBAREA       3
#define SC_CONTENT_GRAPHIC      4
#define SC_CONTENT_OLEOBJECT    5
#define SC_CONTENT_NOTE         6
#define SC_CONTENT_AREALINK     7
#define SC_CONTENT_COUNT        8

//
//  TreeListBox fuer Inhalte
//

class ScContentTree : public SvTreeListBox
{
    ScNavigatorDlg*     pParentWindow;
    Bitmap              aExpBmp;
    Bitmap              aCollBmp;
    ImageList           aEntryImages;
    SvLBoxEntry*        pRootNodes[SC_CONTENT_COUNT];
    USHORT              nRootType;          // als Root eingestellt
    String              aManualDoc;         // im Navigator umgeschaltet (Title)
    BOOL                bHiddenDoc;         // verstecktes aktiv?
    String              aHiddenName;        // URL zum Laden
    String              aHiddenTitle;       // fuer Anzeige
    ScDocument*         pHiddenDocument;    // temporaer

    USHORT              pPosList[SC_CONTENT_COUNT];     // fuer die Reihenfolge

    static BOOL bIsInDrag;      // static, falls der Navigator im ExecuteDrag geloescht wird

    ScDocShell* GetManualOrCurrent();

    void    InitRoot(USHORT nType);
    void    ClearType(USHORT nType);
    void    ClearAll();
    void    InsertContent( USHORT nType, const String& rValue );
    void    GetDrawNames( USHORT nType, USHORT nId );

    void    GetTableNames();
    void    GetAreaNames();
    void    GetDbNames();
    void    GetLinkNames();
    void    GetGraphicNames();
    void    GetOleNames();
    void    GetNoteStrings();

    BOOL    DrawNamesChanged( USHORT nType, USHORT nId );
    BOOL    NoteStringsChanged();

    ScAddress GetNotePos( ULONG nIndex );
    const ScAreaLink* GetLink( ULONG nIndex );

    USHORT  GetCurrentContent( String& rValue );
    ULONG   GetCurrentIndex( SvLBoxEntry* pCurrent = NULL );
    void    DoDrag();
    void    AdjustTitle();

    ScDocument* GetSourceDocument();

    DECL_LINK( DoubleClickHdl, ScContentTree* );
    DECL_STATIC_LINK( ScContentTree, ExecDragHdl, void* );

protected:
    virtual BOOL    Drop( const DropEvent& rEvt );
    virtual BOOL    QueryDrop( DropEvent& rEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

public:
            ScContentTree( Window* pParent, const ResId& rResId );
            ~ScContentTree();

    virtual void    KeyInput(const KeyEvent& rKEvt);

    void    InitWindowBits( BOOL bButtons );

    void    Refresh( USHORT nType = 0 );

    void    ToggleRoot();
    void    SetRootType( USHORT nNew );
    USHORT  GetRootType() const             { return nRootType; }

    void    ActiveDocChanged();
    void    ResetManualDoc();
    void    SetManualDoc(const String& rName);
    BOOL    LoadFile(const String& rUrl);
    void    SelectDoc(const String& rName);

    const String& GetHiddenTitle() const    { return aHiddenTitle; }

    static BOOL IsInDrag()  { return bIsInDrag; }
};




#endif // SC_NAVIPI_HXX

