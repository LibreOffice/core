/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldpage.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:47:47 $
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
#ifndef _SWFLDPAGE_HXX
#define _SWFLDPAGE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif

class ListBox;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

const int coLBCount = 3;

class SwFldPage : public SfxTabPage
{
    String              m_aLstStrArr[ coLBCount ];
    SwFldMgr            m_aMgr;
    SwField             *m_pCurFld;
    SwWrtShell*         m_pWrtShell;
    USHORT              m_nPageId;
    USHORT              m_nTypeSel;
    USHORT              m_nSelectionSel;
    BOOL                m_bFldEdit;
    BOOL                m_bInsert;
    BOOL                m_bFldDlgHtmlMode;
    BOOL                m_bRefresh;
    BOOL                m_bFirstHTMLInit;

protected:

    USHORT              GetTypeSel() const          { return m_nTypeSel;}
    void                SetTypeSel(USHORT nSet)     { m_nTypeSel = nSet;}
    USHORT              GetSelectionSel() const     { return m_nSelectionSel;}
    void                SetSelectionSel(USHORT nSet){ m_nSelectionSel = nSet;}
    BOOL                IsFldDlgHtmlMode() const    { return m_bFldDlgHtmlMode;}
    BOOL                IsRefresh() const           { return m_bRefresh;}
    SwField*            GetCurField()               { return m_pCurFld;}
    SwWrtShell*         GetWrtShell() { return m_pWrtShell;}

    DECL_LINK( InsertHdl, Button *pBtn = 0 );
    DECL_LINK( NumFormatHdl, ListBox *pBtn = 0 );

    void                Init();
    void                SavePos( const ListBox* pLst1,
                                 const ListBox* pLst2 = 0,
                                 const ListBox* pLst3 = 0);
    void                RestorePos( ListBox* pLst1, ListBox* pLst2 = 0,
                                    ListBox* pLst3 = 0 );
    void                EnableInsert(BOOL bEnable = TRUE);
    inline BOOL         IsFldEdit() const   { return m_bFldEdit; }

    // Feld einfuegen
    BOOL                InsertFld(  USHORT nTypeId,
                                    USHORT nSubType,
                                    const String& rPar1,
                                    const String& rPar2,
                                    ULONG nFormatId,
                                    sal_Unicode cDelim = ' ',
                                    BOOL bIsAutomaticLanguage = TRUE);

public:
                        SwFldPage(  Window *pParent,
                                    const ResId &rId,
                                    const SfxItemSet &rAttrSet );

                        virtual ~SwFldPage();

    using SfxTabPage::ActivatePage;
    virtual void        ActivatePage();

    inline SwFldMgr&    GetFldMgr()         { return m_aMgr; }
    void                SetWrtShell( SwWrtShell* m_pWrtShell );
    void                EditNewField( BOOL bOnlyActivate = FALSE );
    virtual USHORT      GetGroup() = 0;
};


#endif

