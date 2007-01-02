/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmenu.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 16:54:04 $
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
 *    Initial Contributer was Fabalabs Software GmbH, Jakob Lechner
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

// SMARTTAGS

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _STMENU_HXX
#include <stmenu.hxx>
#endif
#ifndef _STMENU_HRC
#include <stmenu.hrc>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include "com/sun/star/frame/XModel.hpp"
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif

#define C2U(cChar) OUString::createFromAscii(cChar)

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

SwSmartTagPopup::SwSmartTagPopup( SwView* _pSwView, std::vector <ActionReference> _aActionRefs, Reference<XTextRange> _xTextRange ) :
   PopupMenu(SW_RES(MN_SMARTTAG_POPUP)),
   pSwView ( _pSwView ),
   aActionRefs(_aActionRefs), xTextRange (_xTextRange)
{
    CreateAutoMnemonics();

    PopupMenu *pMenu = GetPopupMenu(MN_SMARTTAG);
   if (!pMenu) {
       return;
   }


   pMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
   sal_Bool bEnable = sal_True;

   Reference <XController> xController = pSwView->GetController();

   nMaxVerbCount=0;
   for (int i=0; i<aActionRefs.size(); i++) {
       Reference<XSmartTagAction> aSmartTagAction = aActionRefs[i].aSmartTagAction;
       sal_Int32 nSmartTagIndex = aActionRefs[i].nSmartTagIndex;
       if (aSmartTagAction->getActionCount(nSmartTagIndex, xController)>nMaxVerbCount)
           nMaxVerbCount = aSmartTagAction->getActionCount(nSmartTagIndex, xController);
   }

   // add menuitem for every smarttag
   for (sal_uInt16 i=0; i<aActionRefs.size(); i++) {
       Reference<XSmartTagAction> aSmartTagAction = aActionRefs[i].aSmartTagAction;
       sal_Int32 nSmartTagIndex = aActionRefs[i].nSmartTagIndex;

       InsertItem(i+1, aSmartTagAction->getSmartTagCaption(nSmartTagIndex, xController));
       PopupMenu* pSubMenu = new PopupMenu;
       SetPopupMenu(i+1, pSubMenu);


       // add subitem for every action of current smarttag
       for (int j=0; j<aSmartTagAction->getActionCount(nSmartTagIndex, xController); j++) {
           // compute the unique id that comprises the information in which
           // smarttag library the action can be found and which action is actually meant
           // This id is needed later in the execute method to invoke the correct
           // action in the smarttag object
           sal_uInt16 nId = (i*nMaxVerbCount + j) + MN_ST_INSERT_START;
           pSubMenu->InsertItem(nId, aSmartTagAction->getActionCaption(nSmartTagIndex, j, xController));
       }
   }
   EnableItem( MN_SMARTTAG, bEnable );

   RemoveDisabledEntries( TRUE, TRUE );
}

/** Function: Execute

   executes actions by calling the invoke function of the appropriate
   smarttag library.

*/
sal_uInt16  SwSmartTagPopup::Execute( Window* pWin, const Rectangle& rWordPos )
{
  SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
  sal_uInt16 nRet = PopupMenu::Execute(pWin, pWin->LogicToPixel(rWordPos));

  if (nRet < MN_ST_INSERT_START) return nRet;

  // compute smarttag lib index and action index
  nRet -= MN_ST_INSERT_START;
  sal_Int32 nRefIndex  = nRet / nMaxVerbCount;
  sal_Int32 nActionIndex =  nRet % nMaxVerbCount;

  Reference<XSmartTagAction> aSmartTagAction = aActionRefs[nRefIndex].aSmartTagAction;
  sal_Int32 nSmartTagIndex = aActionRefs[nRefIndex].nSmartTagIndex;

  Reference <XController> xController = pSwView->GetController();

  // execute action
  aSmartTagAction->invokeAction(nSmartTagIndex, nActionIndex, xTextRange, xController);

  return nRet;
}

