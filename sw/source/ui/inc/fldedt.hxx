/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldedt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:41:13 $
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
#ifndef _FLDEDT_HXX
#define _FLDEDT_HXX
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

class SwView;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldEditDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;
    ImageButton aPrevBT;
    ImageButton aNextBT;
    PushButton  aAddressBT;

    DECL_LINK( AddressHdl, PushButton *pBt = 0 );
    DECL_LINK( NextPrevHdl, Button *pBt = 0 );

    void            Init();
    SfxTabPage*     CreatePage(USHORT nGroup);

public:

     SwFldEditDlg(SwView& rVw);
    ~SwFldEditDlg();

    DECL_LINK( OKHdl, Button * );

    virtual short   Execute();

    void            EnableInsert(BOOL bEnable);
    void            InsertHdl();
};


#endif
