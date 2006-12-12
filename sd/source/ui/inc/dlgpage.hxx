/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgpage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:42:32 $
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


#ifndef _SD_DLGPAGE_HXX
#define _SD_DLGPAGE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#include "dlgpage.hrc"

class SfxObjectShell;
class XColorTable;
class XGradientList;
class XHatchList;
class XBitmapList;

typedef USHORT ChangeType;

/*************************************************************************
|*
|* Seite einrichten-Tab-Dialog
|*
\************************************************************************/
class SdPageDlg : public SfxTabDialog
{
private:
    const SfxItemSet&   mrOutAttrs;

    const SfxObjectShell* mpDocShell;

    XColorTable*        mpColorTab;
    XGradientList*      mpGradientList;
    XHatchList*         mpHatchingList;
    XBitmapList*        mpBitmapList;
public:

    SdPageDlg( SfxObjectShell* pDocSh, Window* pParent, const SfxItemSet* pAttr, BOOL bAreaPage = TRUE );
    ~SdPageDlg() {};

    virtual void PageCreated(USHORT nId, SfxTabPage& rPage);
};

#endif // _SD_DLGPAGE_HXX

