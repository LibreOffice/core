/*************************************************************************
 *
 *  $RCSfile: fldpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-09-20 12:49:58 $
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
#ifndef _SWFLDPAGE_HXX
#define _SWFLDPAGE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif


/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

const int coLBCount = 3;

class SwFldPage : public SfxTabPage
{
    String              aLstStrArr[ coLBCount ];
    SwFldMgr            aMgr;
    SwField             *pCurFld;
    USHORT              nFldDlgAktGrpSel;
    USHORT              nPageId;
    USHORT              nTypeSel;
    USHORT              nSelectionSel;
    BOOL                bFldEdit;
    BOOL                bInsert;
    BOOL                bFldDlgHtmlMode;
    BOOL                bRefresh;
    BOOL                bFirstHTMLInit;

protected:

    USHORT              GetTypeSel() const          { return nTypeSel;}
    void                SetTypeSel(USHORT nSet)     { nTypeSel = nSet;}
    USHORT              GetSelectionSel() const     { return nSelectionSel;}
    void                SetSelectionSel(USHORT nSet){ nSelectionSel = nSet;}
    BOOL                IsFldDlgHtmlMode() const    { return bFldDlgHtmlMode;}
    BOOL                IsRefresh() const           { return bRefresh;}
    SwField*            GetCurField()               { return pCurFld;}

    DECL_LINK( InsertHdl, Button *pBtn = 0 );
    DECL_LINK( NumFormatHdl, ListBox *pBtn = 0 );

    void                Init();
    void                SavePos( const ListBox* pLst1,
                                 const ListBox* pLst2 = 0,
                                 const ListBox* pLst3 = 0);
    void                RestorePos( ListBox* pLst1, ListBox* pLst2 = 0,
                                    ListBox* pLst3 = 0 );
    void                EnableInsert(BOOL bEnable = TRUE);
    inline BOOL         IsFldEdit() const   { return bFldEdit; }

    // Feld einfuegen
    BOOL                InsertFld(  USHORT nTypeId,
                                    USHORT nSubType,
                                    const String& rPar1,
                                    const String& rPar2,
                                    ULONG nFormatId,
                                    sal_Unicode cDelim = ' ');

public:
                        SwFldPage(  Window *pParent,
                                    const ResId &rId,
                                    const SfxItemSet &rAttrSet );

                        virtual ~SwFldPage();

    virtual void        ActivatePage();

    inline SwFldMgr&    GetFldMgr()         { return aMgr; }
    void                EditNewField( BOOL bOnlyActivate = FALSE );
    virtual USHORT      GetGroup() = 0;
};


#endif

