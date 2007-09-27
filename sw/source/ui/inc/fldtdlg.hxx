/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldtdlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:59:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SWFLDTDLG_HXX
#define _SWFLDTDLG_HXX
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

class SfxBindings;
class SfxTabPage;
class SwChildWinWrapper;
struct SfxChildWinInfo;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldDlg: public SfxTabDialog
{
    SwChildWinWrapper*  m_pChildWin;
    SfxBindings*        m_pBindings;
    BOOL                m_bHtmlMode;
    BOOL                m_bDataBaseMode;

    virtual BOOL        Close();
    virtual SfxItemSet* CreateInputItemSet( USHORT nId );
    virtual void        Activate();
    virtual void        PageCreated(USHORT nId, SfxTabPage& rPage);

    void                ReInitTabPage( USHORT nPageId,
                                        BOOL bOnlyActivate = FALSE );

public:
    SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent);
    virtual ~SwFldDlg();

    DECL_LINK( OKHdl, Button * );

    void                Initialize(SfxChildWinInfo *pInfo);
    void                ReInitDlg();
    void                EnableInsert(BOOL bEnable);
    void                InsertHdl();
    void                ActivateDatabasePage();
};

#endif
