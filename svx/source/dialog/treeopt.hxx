/*************************************************************************
 *
 *  $RCSfile: treeopt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 16:26:24 $
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

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_FIXBRD_HXX //autogen
#include <vcl/fixbrd.hxx>
#endif

#define NUMBER_OF_OPTION_PAGES  12
class SfxModule;
class SfxShell;

class OfaOptionsTreeListBox : public SvTreeListBox
{
private:
    BOOL            bInCollapse;

public:
    OfaOptionsTreeListBox(Window* pParent, const ResId& rResId) :
        SvTreeListBox( pParent, rResId ), bInCollapse(FALSE) {}

    virtual BOOL    Collapse( SvLBoxEntry* pParent );
    BOOL            IsInCollapse()const {return bInCollapse;}
};

BOOL            EnableSSO();
void*           GetSSOCreator( void );

/* -----------------11.02.99 07:51-------------------
 *
 * --------------------------------------------------*/
class OfaTreeOptionsDialog : public SfxModalDialog
{
private:
    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    PushButton      aBackPB;

    FixedBorder     aHiddenGB;
    FixedText       aPageTitleFT;
    FixedLine       aLine1FL;
    FixedText       aHelpFT;
    FixedImage      aHelpImg;

    ImageList       aPageImages;
    ImageList       aPageImagesHC;

    ResStringArray  aHelpTextsArr;

    OfaOptionsTreeListBox   aTreeLB;

    String          sTitle;
    String          sNotLoadedError;

    SvLBoxEntry*            pCurrentPageEntry;

    // for the ColorTabPage
    SfxItemSet*     pColorPageItemSet;
    XColorTable*    pColorTab;
    USHORT          nChangeType;
    USHORT          nUnknownType;
    USHORT          nUnknownPos;
    BOOL            bIsAreaTP;

    BOOL            bForgetSelection;
    BOOL            bExternBrowserActive;
    BOOL            bImageResized;
    bool            bInSelectHdl_Impl;

    Timer           aSelectTimer;

    static USHORT   nLastDialogPageId;

    SfxItemSet*     CreateItemSet( USHORT nId );
    void            ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    void            Initialize();
    void            ResizeTreeLB( void );   // resizes dialog so that treelistbox has no horizontal scroll bar

protected:
        DECL_LINK(ExpandedHdl_Impl, SvTreeListBox* );
        DECL_LINK(ShowPageHdl_Impl, SvTreeListBox* );
        DECL_LINK(BackHdl_Impl, PushButton* );
        DECL_LINK( OKHdl_Impl, Button * );
        DECL_LINK( HintHdl_Impl, Timer * );
        DECL_LINK( SelectHdl_Impl, Timer * );

        virtual long    Notify( NotifyEvent& rNEvt );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
        virtual short   Execute();

public:
        OfaTreeOptionsDialog(Window* pParent);
        ~OfaTreeOptionsDialog();

        void    AddTabPage( USHORT nId, const String& rPageName, USHORT nGroup);
        USHORT  AddGroup(const String& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, USHORT nDialogId);
        void    ActivateLastSelection();
        void    ActivatePage(USHORT nResId);
        void    ApplyItemSets();


    USHORT                  GetColorChanged() const { return nChangeType; }
    XColorTable*            GetColorTable() { return pColorTab; }
};
/* -----------------11.02.99 15:49-------------------
 *
 * --------------------------------------------------*/
class OfaPageResource : public Resource
{
    ResStringArray      aGeneralDlgAry;
    ResStringArray      aInetDlgAry;
    ResStringArray      aLangDlgAry;
    ResStringArray      aTextDlgAry;
    ResStringArray      aHTMLDlgAry;
    ResStringArray      aCalcDlgAry;
    ResStringArray      aStarMathDlgAry;
    ResStringArray      aImpressDlgAry;
    ResStringArray      aDrawDlgAry;
    ResStringArray      aChartDlgAry;
    ResStringArray      aFilterDlgAry;
    ResStringArray      aDatasourcesDlgAry;

public:
    OfaPageResource();

    ResStringArray& GetGeneralArray()       {return aGeneralDlgAry;}
    ResStringArray& GetInetArray()          {return aInetDlgAry;}
    ResStringArray& GetLangArray()          {return aLangDlgAry;}
    ResStringArray& GetTextArray()          {return aTextDlgAry;}
    ResStringArray& GetHTMLArray()          {return aHTMLDlgAry;}
    ResStringArray& GetCalcArray()          {return aCalcDlgAry;}
    ResStringArray& GetStarMathArray()      {return aStarMathDlgAry;}
    ResStringArray& GetImpressArray()       {return aImpressDlgAry;}
    ResStringArray& GetDrawArray()          {return aDrawDlgAry;}
    ResStringArray& GetChartArray()         {return aChartDlgAry;}
    ResStringArray& GetFilterArray()        {return aFilterDlgAry;}
    ResStringArray& GetDatasourcesArray()   {return aDatasourcesDlgAry;}
};

