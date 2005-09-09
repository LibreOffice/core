/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabtempl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:55:04 $
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


#ifndef SD_TABTEMPL_HXX
#define SD_TABTEMPL_HXX

#ifndef _SVX_TAB_AREA_HXX //autogen
#include <svx/tabarea.hxx>
#endif
#ifndef _SFX_STYLEDLG_HXX //autogen
#include <sfx2/styledlg.hxx>
#endif
class SdrModel;
class SfxObjectShell;
class SdrView;
class XColorTable;
class XGradientList;
class XBitmapList;
class XDashList;
class XHatchList;
class XLineEndList;


/*************************************************************************
|*
|* Vorlagen-Tab-Dialog
|*
\************************************************************************/
class SdTabTemplateDlg : public SfxStyleDialog
{
private:
    const SfxObjectShell&   rDocShell;
    SdrView*                pSdrView;

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

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual const SfxItemSet*   GetRefreshedSet();

public:
                    SdTabTemplateDlg( Window* pParent,
                            const SfxObjectShell* pDocShell,
                            SfxStyleSheetBase& rStyleBase,
                            SdrModel* pModel,
                            SdrView* pView );
                    ~SdTabTemplateDlg();

};


#endif // SD_TABTEMPL_HXX

