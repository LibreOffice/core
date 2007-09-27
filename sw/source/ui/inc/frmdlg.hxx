/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:59:58 $
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

#ifndef _FRMDLG_HXX
#define _FRMDLG_HXX

#include "globals.hrc"

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung:    Rahmendialog
 --------------------------------------------------------------------*/

class SwFrmDlg : public SfxTabDialog
{
    BOOL                m_bFormat;
    BOOL                m_bNew;
    BOOL                m_bHTMLMode;
    const SfxItemSet&   m_rSet;
    USHORT              m_nDlgType;
    SwWrtShell*         m_pWrtShell;


    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );

public:
    SwFrmDlg(   SfxViewFrame *pFrame, Window *pParent,
                const SfxItemSet& rCoreSet,
                BOOL            bNewFrm  = TRUE,
                USHORT          nResType = DLG_FRM_STD,
                BOOL            bFmt     = FALSE,
                UINT16          nDefPage = 0,
                const String*   pFmtStr  = 0);

    ~SwFrmDlg();

    inline SwWrtShell*  GetWrtShell()   { return m_pWrtShell; }
};


#endif // _FRMDLG_HXX
