/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabpages.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:26:05 $
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

#ifndef SC_TABPAGES_HXX
#define SC_TABPAGES_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

//========================================================================

class ScTabPageProtection : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rAttrSet );
    static  USHORT*     GetRanges       ();
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreAttrs );
    virtual void        Reset           ( const SfxItemSet& );

protected:
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTabPageProtection( Window*            pParent,
                                     const SfxItemSet&  rCoreAttrs );
                ~ScTabPageProtection();

private:
    FixedLine   aFlProtect;
    TriStateBox aBtnHideCell;
    TriStateBox aBtnProtect;
    TriStateBox aBtnHideFormula;
    FixedInfo   aTxtHint;

    FixedLine   aFlPrint;
    TriStateBox aBtnHidePrint;
    FixedInfo   aTxtHint2;

                                    // aktueller Status:
    BOOL        bTriEnabled;        //  wenn vorher Dont-Care
    BOOL        bDontCare;          //  alles auf TriState
    BOOL        bProtect;           //  einzelne Einstellungen ueber TriState sichern
    BOOL        bHideForm;
    BOOL        bHideCell;
    BOOL        bHidePrint;

    // Handler:
    DECL_LINK( ButtonClickHdl, TriStateBox* pBox );
    void        UpdateButtons();
};



#endif // SC_TABPAGES_HXX
