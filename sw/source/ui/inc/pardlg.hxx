/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pardlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:56:28 $
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
#ifndef _SW_PARDLG_HXX
#define _SW_PARDLG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

class SwView;
class SfxItemSet;

//DialogModes:
#define DLG_STD         0
#define DLG_ENVELOP     2

//CHINA001 class SwParaDlg: public SfxTabDialog
//CHINA001 {
//CHINA001 SwView& rView;
//CHINA001 USHORT nHtmlMode;
//CHINA001 BYTE nDlgMode;
//CHINA001 BOOL bDrawParaDlg;
//CHINA001
//CHINA001 void PageCreated(USHORT nID, SfxTabPage& rPage);
//CHINA001
//CHINA001 public:
//CHINA001 SwParaDlg(  Window *pParent,
//CHINA001 SwView& rVw,
//CHINA001 const SfxItemSet&,
//CHINA001 BYTE nDialogMode,
//CHINA001 const String *pCollName = 0,
//CHINA001 BOOL bDraw = FALSE,
//CHINA001 UINT16 nDefPage = 0);
//CHINA001 ~SwParaDlg();
//CHINA001 };


#endif
