/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prltempl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:47:10 $
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


#ifndef SD_PRLTEMPL_HXX
#define SD_PRLTEMPL_HXX

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include <svx/tabarea.hxx>
#endif

#include "prlayout.hxx" // fuer enum PresentationObjects


class XColorTable;
class XGradientList;
class XHatchList;
class XBitmapList;
class XDashList;
class XLineEndList;
class SfxObjectShell;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;

/*************************************************************************
|*
|* Vorlagen-Tab-Dialog
|*
\************************************************************************/
class SdPresLayoutTemplateDlg : public SfxTabDialog
{
private:
    const SfxObjectShell*   mpDocShell;

    XColorTable*        pColorTab;
    XGradientList*      pGradientList;
    XHatchList*         pHatchingList;
    XBitmapList*        pBitmapList;
    XDashList*          pDashList;
    XLineEndList*       pLineEndList;

    USHORT              nPageType;
    USHORT              nDlgType;
    USHORT              nPos;

    ChangeType          nColorTableState;
    ChangeType          nBitmapListState;
    ChangeType          nGradientListState;
    ChangeType          nHatchingListState;
    ChangeType          nLineEndListState;
    ChangeType          nDashListState;

    PresentationObjects ePO;

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

    // fuers Maping mit dem neuen SvxNumBulletItem
    SfxItemSet aInputSet;
    SfxItemSet* pOutSet;
    const SfxItemSet* pOrgSet;

    USHORT GetOutlineLevel() const;

    using SfxTabDialog::GetOutputItemSet;

public:
    SdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
    ~SdPresLayoutTemplateDlg();

    const SfxItemSet* GetOutputItemSet() const;
};


#endif // SD_PRLTEMPL_HXX

