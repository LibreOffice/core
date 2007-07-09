/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:29 $
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
#include "precompiled_reportdesign.hxx"

#include <svtools/intitem.hxx> //add CHINA001
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_TAB_AREA_HXX
#include <svx/tabarea.hxx>
#endif

#ifndef RPTUI_DLGPAGE_HXX
#include "dlgpage.hxx"
#endif
#include "ModuleHelper.hxx"
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxenum.hxx>

namespace rptui
{
/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

ORptPageDialog::ORptPageDialog( Window* pParent, const SfxItemSet* pAttr,USHORT _nPageId) :
SfxTabDialog ( pParent, ModuleRes( _nPageId ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    switch( _nPageId )
    {
        case RID_PAGEDIALOG_BACKGROUND:
            AddTabPage( RID_SVXPAGE_BACKGROUND,String(ModuleRes(1)));
            break;
        case RID_PAGEDIALOG_PAGE:
            //AddTabPage( RID_SVXPAGE_PAGE,String(ModuleRes(1)));
   //         AddTabPage( RID_SVXPAGE_BACKGROUND,String(ModuleRes(1)));
            AddTabPage(RID_SVXPAGE_PAGE, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), 0 );
            AddTabPage(RID_SVXPAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            break;
        case RID_PAGEDIALOG_CHAR:
            AddTabPage(RID_PAGE_CHAR, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
            AddTabPage(RID_PAGE_EFFECTS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
            AddTabPage(RID_PAGE_POSITION, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
            AddTabPage(RID_PAGE_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
            AddTabPage(RID_PAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            AddTabPage(RID_PAGE_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), 0 );

            break;
        case RID_PAGEDIALOG_LINE:
            AddTabPage( RID_SVXPAGE_LINE,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE ), 0 );
            //AddTabPage( RID_SVXPAGE_LINE_DEF,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE_DEF ), 0 );
            //AddTabPage( RID_SVXPAGE_LINEEND_DEF,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINEEND_DEF ), 0 );
            break;
        default:
            OSL_ENSURE(0,"Unknown page id");
    }

    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage(RID_PAGE_TWOLN);

    FreeResource();
}
// =============================================================================
} // namespace rptui
// =============================================================================
