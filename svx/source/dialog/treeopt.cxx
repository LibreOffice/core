/*************************************************************************
 *
 *  $RCSfile: treeopt.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 13:10:55 $
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
#pragma hdrstop

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _SFX_PRINTOPT_HXX
#include <sfx2/printopt.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#include <svtools/misccfg.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tplpitem.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/linguprops.hxx>

#ifndef _XDEF_HXX //autogen
#include "xdef.hxx"
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include "xflclit.hxx"
#endif
#ifndef _XPOOL_HXX //autogen
#include "xpool.hxx"
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include "cuitabarea.hxx"
#endif
#ifndef _OFF_OFAITEM_HXX //autogen
#include "ofaitem.hxx"
#endif
#ifndef _OFA_OPTHTML_HXX //autogen
#include "opthtml.hxx"
#endif
#ifndef _SVX_OPTCOLOR_HXX
#include "optcolor.hxx"
#endif
#ifndef _SVX_OPTCTL_HXX
#include "optctl.hxx"
#endif
#ifndef _SVX_OPTJAVA_HXX
#include "optjava.hxx"
#endif
#ifndef _SVX_OPTSAVE_HXX //autogen
#include "optsave.hxx"
#endif
#ifndef _SVX_OPTPATH_HXX //autogen
#include "optpath.hxx"
#endif
#ifndef _SVX_CUIOPTGENRL_HXX //autogen
#include "cuioptgenrl.hxx"
#endif
#ifndef _SVX_OPTLINGU_HXX //autogen
#include "optlingu.hxx"
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include "cuitabarea.hxx"
#endif
#ifndef _SVX_OPTINET_HXX //autogen
#include "optinet2.hxx"
#endif
#ifndef _SVX_OPTASIAN_HXX
#include "optasian.hxx"
#endif
#ifndef _SVX_OPTACCESSIBILITY_HXX
#include "optaccessibility.hxx"
#endif
#ifndef _SVX_OPTJSEARCH_HXX_
#include "optjsearch.hxx"
#endif
#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#include "connpooloptions.hxx"
#endif

#include "optgdlg.hxx"
#include "optmemory.hxx"
#include "optfltr.hxx"
#include "dialogs.hrc"
#include "helpid.hrc"
#include "dialmgr.hxx"
#include "treeopt.hxx"
#include "treeopt.hrc"
#include "fontsubs.hxx"
#include "unolingu.hxx"
#include "xtable.hxx"
#include "connpoolconfig.hxx"
#include "dbregister.hxx"
#include "dbregisterednamesconfig.hxx"

#ifndef _SVX_LANGITEM_HXX
#define ITEMID_LANGUAGE SID_ATTR_CHAR_LANGUAGE
#include <langitem.hxx>
#endif
#ifndef _SVX_OPTITEMS_HXX
#define ITEMID_SPELLCHECK   SID_ATTR_SPELL
#define ITEMID_HYPHENREGION SID_ATTR_HYPHENREGION
#include <optitems.hxx>
#endif
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#ifndef _SVX_DRAWITEM_HXX
#include <drawitem.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
//using namespace com::sun::star::task;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define HINT_TIMEOUT            200
#define SELECT_FIRST_TIMEOUT    0
#define SELECT_TIMEOUT          300
#define COLORPAGE_UNKNOWN       ((sal_uInt16)0xFFFF)

sal_uInt16  OfaTreeOptionsDialog::nLastDialogPageId = USHRT_MAX;

// some stuff for easier changes for SvtViewOptions
static const sal_Char*      pViewOptDataName = "page data";
#define VIEWOPT_DATANAME    OUString::createFromAscii( pViewOptDataName )

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const String& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, makeAny( OUString( rData ) ) );
}

static inline String GetViewOptUserItem( const SvtViewOptions& rOpt )
{
    Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
    OUString aUserData;
    aAny >>= aUserData;

    return String( aUserData );
}

//typedef SfxTabPage* (*FNCreateTabPage)( Window *pParent, const SfxItemSet &rAttrSet );
SfxTabPage* CreateGeneralTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    CreateTabPage fnCreate = 0;
    switch(nId)
    {
        case RID_SFXPAGE_SAVE:                      fnCreate = &SvxSaveTabPage::Create; break;
        case RID_SFXPAGE_PATH:                      fnCreate = &SvxPathTabPage::Create; break;
        case RID_SFXPAGE_GENERAL:                   fnCreate = &SvxGeneralTabPage::Create; break;
        case RID_SFXPAGE_PRINTOPTIONS:              fnCreate = &SfxCommonPrintOptionsTabPage::Create; break;
        case OFA_TP_LANGUAGES:                      fnCreate = &OfaLanguagesTabPage::Create; break;
        case RID_SFXPAGE_LINGU:                     fnCreate = &SvxLinguTabPage::Create; break;
        case RID_SVXPAGE_COLOR:                     fnCreate = &SvxColorTabPage::Create; break;
        case OFA_TP_VIEW:                           fnCreate = &OfaViewTabPage::Create; break;
        case OFA_TP_MISC:                           fnCreate = &OfaMiscTabPage::Create; break;
        case OFA_TP_MEMORY:                         fnCreate = &OfaMemoryOptionsPage::Create; break;
        case RID_SVXPAGE_ASIAN_LAYOUT:              fnCreate = &SvxAsianLayoutPage::Create; break;
        case RID_SVX_FONT_SUBSTITUTION:             fnCreate = &SvxFontSubstTabPage::Create; break;
        case RID_SVXPAGE_INET_PROXY:                fnCreate = &SvxProxyTabPage::Create; break;
        case RID_SVXPAGE_INET_SEARCH:               fnCreate = &SvxSearchTabPage::Create; break;
        case RID_SVXPAGE_INET_SCRIPTING:            fnCreate = &SvxScriptingTabPage::Create; break;
        case RID_SVXPAGE_INET_MAIL:             fnCreate = &SvxEMailTabPage::Create; break;
        case RID_SVXPAGE_COLORCONFIG:               fnCreate = &SvxColorOptionsTabPage::Create; break;
        case RID_OFAPAGE_HTMLOPT:                   fnCreate = &OfaHtmlTabPage::Create; break;
        case SID_OPTFILTER_MSOFFICE:                fnCreate = &OfaMSFilterTabPage::Create; break;
        case RID_OFAPAGE_MSFILTEROPT2:              fnCreate = &OfaMSFilterTabPage2::Create; break;
        case RID_SVXPAGE_JSEARCH_OPTIONS:           fnCreate = &SvxJSearchOptionsPage::Create ; break;
        case SID_SB_CONNECTIONPOOLING:              fnCreate = &::offapp::ConnectionPoolOptionsPage::Create; break;
        case SID_SB_DBREGISTEROPTIONS:              fnCreate = &::svx::DbRegistrationOptionsPage::Create; break;
        case RID_SVXPAGE_ACCESSIBILITYCONFIG:       fnCreate = &SvxAccessibilityOptionsTabPage::Create; break;
        case RID_SVXPAGE_SSO:                       fnCreate = ( CreateTabPage ) GetSSOCreator(); break;
        case RID_SVXPAGE_OPTIONS_CTL:               fnCreate = &SvxCTLOptionsPage::Create ; break;
        case RID_SVXPAGE_OPTIONS_JAVA:              fnCreate = &SvxJavaOptionsPage::Create ; break;
    }

    SfxTabPage* pRet = fnCreate ? (*fnCreate)( pParent, rSet ) : NULL;
    return pRet;
}




/* -----------------11.02.99 09:56-------------------
 *
 * --------------------------------------------------*/
struct OptionsPageInfo
{
    SfxTabPage*     pPage;
    sal_uInt16          nPageId;
};

struct OptionsGroupInfo
{
    SfxItemSet* pInItemSet;
    SfxItemSet* pOutItemSet;

    SfxShell*   pShell;     //hier werden die Pages erzeugt
    SfxModule*  pModule;    //hier wird der ItemSet erzeugt
    sal_uInt16      nDialogId;  //Id des ehemaligen Dialogs
    sal_Bool        bLoadError; //gab es hier schon einen Ladefehler?

    OptionsGroupInfo( SfxShell* pSh, SfxModule* pMod, sal_uInt16 nId ) :
        pInItemSet( NULL ), pOutItemSet( NULL ), pShell( pSh ),
        pModule( pMod ), nDialogId( nId ), bLoadError( sal_False ) {}
    ~OptionsGroupInfo() { delete pInItemSet; delete pOutItemSet; }
};

/* -----------------04.05.99 15:51-------------------
 *
 * --------------------------------------------------*/
sal_Bool    OfaOptionsTreeListBox::Collapse( SvLBoxEntry* pParent )
{
    bInCollapse = sal_True;
    sal_Bool bRet = SvTreeListBox::Collapse(pParent);
    bInCollapse = sal_False;
    return bRet;
}


/* -----------------11.02.99 07:54-------------------
 *
 * --------------------------------------------------*/

OfaTreeOptionsDialog::OfaTreeOptionsDialog( Window* pParent ) :

    SfxModalDialog( pParent, ResId( RID_OFADLG_OPTIONS_TREE, DIALOG_MGR() ) ),

    aOkPB           ( this, ResId( PB_OK ) ),
    aCancelPB       ( this, ResId( PB_CANCEL ) ),
    aHelpPB         ( this, ResId( PB_HELP ) ),
    aBackPB         ( this, ResId( PB_BACK ) ),
    aTreeLB         ( this, ResId( TLB_PAGES ) ),
    aHiddenGB       ( this, ResId( FB_BORDER ) ),
    aPageTitleFT    ( this, ResId( FT_PAGE_TITLE ) ),
    aLine1FL        ( this, ResId( FL_LINE_1 ) ),
    aHelpFT         ( this, ResId( FT_HELPTEXT ) ),
    aHelpImg        ( this, ResId( IMG_HELP ) ),
    aHelpTextsArr   (       ResId( STR_HELPTEXTS ) ),
    sNotLoadedError (       ResId( ST_LOAD_ERROR ) ),

    sTitle              ( GetText() ),
    pCurrentPageEntry   ( NULL ),
    pColorPageItemSet   ( NULL ),
    pColorTab           ( NULL ),
    nChangeType         ( CT_NONE ),
    nUnknownType        ( COLORPAGE_UNKNOWN ),
    nUnknownPos         ( COLORPAGE_UNKNOWN ),
    bIsAreaTP           ( sal_False ),
    bForgetSelection    ( sal_False ),
    bImageResized       ( sal_False ),
    bInSelectHdl_Impl   ( false )
{
    aTreeLB.SetNodeDefaultImages();

    FreeResource();

    ResMgr* pIsoRes = SFX_APP()->GetLabelResManager();
    ResId aImgLstRes( RID_IMGLIST_TREEOPT );
    aImgLstRes.SetRT( RSC_IMAGELIST );
    if ( pIsoRes->IsAvailable( aImgLstRes ) )
        aPageImages = ImageList( ResId( RID_IMGLIST_TREEOPT, pIsoRes ) );
    ResId aImgLstHCRes( RID_IMGLIST_TREEOPT_HC );
    aImgLstHCRes.SetRT( RSC_IMAGELIST );
    if ( pIsoRes->IsAvailable( aImgLstHCRes ) )
        aPageImagesHC = ImageList( ResId( RID_IMGLIST_TREEOPT_HC, pIsoRes ) );
    aTreeLB.SetHelpId( HID_OFADLG_TREELISTBOX );
    aTreeLB.SetWindowBits( WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                           WB_HASLINES | WB_HASLINESATROOT |
                           WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
    aTreeLB.SetSpaceBetweenEntries( 0 );
    aTreeLB.SetSelectionMode( SINGLE_SELECTION );
    aTreeLB.SetSublistOpenWithLeftRight( TRUE );
    aTreeLB.SetExpandedHdl( LINK( this, OfaTreeOptionsDialog, ExpandedHdl_Impl ) );
    aTreeLB.SetSelectHdl( LINK( this, OfaTreeOptionsDialog, ShowPageHdl_Impl ) );
    aBackPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, BackHdl_Impl ) );
    aOkPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, OKHdl_Impl ) );

    aHiddenGB.Show();
    aSelectTimer.SetTimeout( SELECT_FIRST_TIMEOUT );
    aSelectTimer.SetTimeoutHdl( LINK( this, OfaTreeOptionsDialog, SelectHdl_Impl ) );

    Initialize();
}

/* -----------------11.02.99 07:58-------------------
 *
 * --------------------------------------------------*/

OfaTreeOptionsDialog::~OfaTreeOptionsDialog()
{
    SvLBoxEntry* pEntry = aTreeLB.First();
    //erst children
    while(pEntry)
    {
        //wenn Child, dann OptionsPageInfo
        if(aTreeLB.GetParent(pEntry))
        {
            OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
            if(pPageInfo->pPage)
            {
                pPageInfo->pPage->FillUserData();
                String aPageData(pPageInfo->pPage->GetUserData());
                if ( aPageData.Len() )
                {
                    SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pPageInfo->nPageId ) );
                    SetViewOptUserItem( aTabPageOpt, aPageData );
                }
                delete pPageInfo->pPage;
            }

            if (pPageInfo->nPageId == RID_SFXPAGE_LINGU)
            {
                // write personal dictionaries
                Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
                if (xDicList.is())
                {
                    SvxSaveDictionaries( xDicList );
                }
            }

            delete pPageInfo;
        }
        pEntry = aTreeLB.Next(pEntry);
    }
    pEntry = aTreeLB.First();
    //dann parents
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
        {
            OptionsGroupInfo *pGroupInfo = (OptionsGroupInfo *)pEntry->GetUserData();
            delete pGroupInfo;
        }
        pEntry = aTreeLB.Next(pEntry);
    }
    delete pColorPageItemSet;

}

/* -----------------11.02.99 08:21-------------------
 *
 * --------------------------------------------------*/
void OfaTreeOptionsDialog::AddTabPage( sal_uInt16 nId, const String& rPageName, sal_uInt16 nGroup )
{
    OptionsPageInfo *pPageInfo = new OptionsPageInfo;
    pPageInfo->pPage = 0;
    pPageInfo->nPageId = nId;

    SvLBoxEntry* pParent = aTreeLB.GetEntry( 0, nGroup );
    DBG_ASSERT(pParent, "no group");
    SvLBoxEntry* pEntry = aTreeLB.InsertEntry(rPageName, pParent);
    pEntry->SetUserData(pPageInfo);

}
/* -----------------11.02.99 10:02-------------------
 *  der ItemSet* geht in den Besitz des Dialogs
 * --------------------------------------------------*/
sal_uInt16  OfaTreeOptionsDialog::AddGroup(const String& rGroupName,
                                        SfxShell* pCreateShell,
                                        SfxModule* pCreateModule,
                                        sal_uInt16 nDialogId )
{
    SvLBoxEntry* pEntry = aTreeLB.InsertEntry(rGroupName);
    OptionsGroupInfo* pInfo =
        new OptionsGroupInfo( pCreateShell, pCreateModule, nDialogId );
    pEntry->SetUserData(pInfo);
    sal_uInt16 nRet = 0;
    pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
            nRet++;
        pEntry = aTreeLB.Next(pEntry);
    }
    return nRet - 1;
}

/* -----------------11.02.99 10:31-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(OfaTreeOptionsDialog, ShowPageHdl_Impl, SvTreeListBox*, pBox)
{
    if ( aSelectTimer.GetTimeout() == SELECT_FIRST_TIMEOUT )
    {
        aSelectTimer.SetTimeout( SELECT_TIMEOUT );
        SelectHdl_Impl( NULL );
    }
    else
        aSelectTimer.Start();

    return 0;
}
/* -----------------11.02.99 10:49-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( OfaTreeOptionsDialog, BackHdl_Impl, PushButton*, EMPTYARG )
{
    if ( pCurrentPageEntry )
    {
        OptionsPageInfo* pPageInfo = (OptionsPageInfo*)pCurrentPageEntry->GetUserData();
        OptionsGroupInfo* pGroupInfo =
            (OptionsGroupInfo*)aTreeLB.GetParent( pCurrentPageEntry )->GetUserData();
        if ( RID_SVXPAGE_COLOR == pPageInfo->nPageId )
            pPageInfo->pPage->Reset( *pColorPageItemSet );
        else
            pPageInfo->pPage->Reset( *pGroupInfo->pInItemSet );
    }
    return 0;
}
/* -----------------11.02.99 16:45-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( OfaTreeOptionsDialog, OKHdl_Impl, Button *, EMPTRARG )
{
    aTreeLB.EndSelection();
    if(pCurrentPageEntry)
    {
        OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pCurrentPageEntry->GetUserData();
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)aTreeLB.GetParent(pCurrentPageEntry)->GetUserData();
        if(RID_SVXPAGE_COLOR != pPageInfo->nPageId &&
                    pPageInfo->pPage->HasExchangeSupport())
        {
            int nLeave = pPageInfo->pPage->DeactivatePage(pGroupInfo->pOutItemSet);
            if(nLeave == SfxTabPage::KEEP_PAGE)
            {
                //die Seite darf nicht verlassen werden!
                aTreeLB.Select(pCurrentPageEntry);
                return 0;
            }
        }
        pPageInfo->pPage->Hide();
    }
    SvLBoxEntry* pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(aTreeLB.GetParent(pEntry))
        {
            OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
            if(pPageInfo->pPage && !pPageInfo->pPage->HasExchangeSupport())
            {
                OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)
                                        aTreeLB.GetParent(pEntry)->GetUserData();
                pPageInfo->pPage->FillItemSet(*pGroupInfo->pOutItemSet);
            }
        }
        pEntry = aTreeLB.Next(pEntry);
    }
    EndDialog(RET_OK);
    return 0;
}
/* -----------------17.02.99 09:15-------------------
 * Eine aufgeklappte Gruppe soll vollstaendig sichtbar sein
 * --------------------------------------------------*/
IMPL_LINK(OfaTreeOptionsDialog, ExpandedHdl_Impl, SvTreeListBox*, pBox )
{
    pBox->Update();
    pBox->InitStartEntry();
    SvLBoxEntry* pEntry = pBox->GetHdlEntry();
    if(pEntry && pBox->IsExpanded(pEntry))
    {
        sal_uInt32 nChildCount = pBox->GetChildCount( pEntry );

        SvLBoxEntry* pNext = pEntry;
        for(sal_uInt32 i = 0; i < nChildCount;i++)
        {
            pNext = pBox->GetNextEntryInView(pNext);
            if(!pNext)
            {
                pBox->ScrollOutputArea( -(short)(nChildCount - i + 1) );
                break;
            }
            else
            {
                Size aSz(pBox->GetOutputSizePixel());
                int nHeight = pBox->GetEntryHeight();
                Point aPos(pBox->GetEntryPos(pNext));
                if(aPos.Y()+nHeight > aSz.Height())
                {
                    pBox->ScrollOutputArea( -(short)(nChildCount - i + 1) );
                    break;
                }
            }
        }
    }
    return 0;
}

/* -----------------11.02.99 10:49-------------------
 *
 * --------------------------------------------------*/
void OfaTreeOptionsDialog::ApplyItemSets()
{
    SvLBoxEntry* pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
        {
            OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)pEntry->GetUserData();
            if(pGroupInfo->pOutItemSet)
            {
                if(pGroupInfo->pShell)
                    pGroupInfo->pShell->ApplyItemSet( pGroupInfo->nDialogId, *pGroupInfo->pOutItemSet);
                else
                    ApplyItemSet( pGroupInfo->nDialogId, *pGroupInfo->pOutItemSet);
            }
        }
        pEntry = aTreeLB.Next(pEntry);
    }
}
/* -----------------17.02.99 09:51-------------------
 *
 * --------------------------------------------------*/
void    OfaTreeOptionsDialog::ActivatePage(sal_uInt16 nResId)
{
    bForgetSelection = sal_True;
    sal_uInt16 nTemp = nLastDialogPageId;
    nLastDialogPageId = nResId;
    ActivateLastSelection();
    nLastDialogPageId = nTemp;
}

/* -----------------16.02.99 13:17-------------------
 *
 * --------------------------------------------------*/
void    OfaTreeOptionsDialog::ActivateLastSelection()
{
    SvLBoxEntry* pEntry = 0;
    if(nLastDialogPageId == USHRT_MAX)
    {
        pEntry = aTreeLB.First();
        pEntry = aTreeLB.Next(pEntry);
    }
    else
    {
        SvLBoxEntry* pTemp = aTreeLB.First();
        while(!pEntry && pTemp)
        {
            //wenn Child, dann OptionsPageInfo
            if(aTreeLB.GetParent(pTemp))
            {
                OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pTemp->GetUserData();
                if(pPageInfo->nPageId == nLastDialogPageId)
                    pEntry = pTemp;
            }
            pTemp = aTreeLB.Next(pTemp);
        }
    }
    if(pEntry)
    {
        SvLBoxEntry* pParent = aTreeLB.GetParent(pEntry);
        aTreeLB.Expand(pParent);
        aTreeLB.MakeVisible(pParent); // damit dieser (wenn m"oglich) auch sichtbar ist
        aTreeLB.MakeVisible(pEntry);
        aTreeLB.Select(pEntry);
    }
    aTreeLB.GrabFocus();
}
/* -----------------22.02.99 08:52-------------------
 *
 * --------------------------------------------------*/
long    OfaTreeOptionsDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();

        if( aKeyCode.GetCode() == KEY_PAGEUP ||
                aKeyCode.GetCode() == KEY_PAGEDOWN)
        {
            SvLBoxEntry* pCurEntry = aTreeLB.FirstSelected();
            SvLBoxEntry*  pTemp = 0;
            if(aKeyCode.GetCode() == KEY_PAGEDOWN)
            {
                pTemp =  aTreeLB.Next( pCurEntry ) ;
                if(pTemp && !aTreeLB.GetParent(pTemp))
                {
                    pTemp =  aTreeLB.Next( pTemp ) ;
                    aTreeLB.Select(pTemp);
                }
            }
            else
            {
                pTemp =  aTreeLB.Prev( pCurEntry ) ;
                if(pTemp && !aTreeLB.GetParent(pTemp))
                {
                    pTemp =  aTreeLB.Prev( pTemp ) ;
                }
            }
            if(pTemp)
            {
                if(!aTreeLB.IsExpanded(aTreeLB.GetParent(pTemp)))
                    aTreeLB.Expand(aTreeLB.GetParent(pTemp));
                aTreeLB.MakeVisible(pTemp);
                aTreeLB.Select(pTemp);
            }
        }
    }
    return SfxModalDialog::Notify(rNEvt);
}

// --------------------------------------------------------------------

void OfaTreeOptionsDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModalDialog::DataChanged( rDCEvt );

    SvLBoxEntry* pEntry = aTreeLB.GetCurEntry();
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) &&
        !aTreeLB.GetParent(pEntry))
    {
        OptionsGroupInfo* pInfo = static_cast<OptionsGroupInfo*>(pEntry->GetUserData());
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;
        ImageList* pImgLst = bHighContrast ? &aPageImagesHC : &aPageImages;
        for ( sal_uInt16 i = 0; i < aHelpTextsArr.Count(); ++i )
        {
            if ( aHelpTextsArr.GetValue(i) == pInfo->nDialogId )
            {
                aHelpImg.SetImage( pImgLst->GetImage( pInfo->nDialogId ) );
                break;
            }
        }
    }
}
class FlagSet_Impl
{
    bool & rFlag;
    public:
        FlagSet_Impl(bool& bFlag) : rFlag(bFlag){rFlag = true;}
        ~FlagSet_Impl(){rFlag = false;}
};

IMPL_LINK( OfaTreeOptionsDialog, SelectHdl_Impl, Timer*, EMPTYARG )
{
    SvTreeListBox* pBox = &aTreeLB;
    SvLBoxEntry* pEntry = pBox->GetCurEntry();
    SvLBoxEntry* pParent = pBox->GetParent(pEntry);
    pBox->EndSelection();

    DBG_ASSERT(!bInSelectHdl_Impl, "Timeout handler called twice")
    if(bInSelectHdl_Impl || pCurrentPageEntry == pEntry)
        return 0;
    //#111938# lock the SelectHdl_Impl to prevent multiple executes
    FlagSet_Impl aFlag(bInSelectHdl_Impl);
    TabPage* pOldPage = NULL;
    TabPage* pNewPage = NULL;

    if(pCurrentPageEntry && ((OptionsPageInfo *)pCurrentPageEntry->GetUserData())->pPage->IsVisible())
    {
        OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pCurrentPageEntry->GetUserData();
        pOldPage = pPageInfo->pPage;
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)aTreeLB.GetParent(pCurrentPageEntry)->GetUserData();
        int nLeave = SfxTabPage::LEAVE_PAGE;
        if(RID_SVXPAGE_COLOR != pPageInfo->nPageId &&
                pPageInfo->pPage->HasExchangeSupport())
        {
           nLeave = pPageInfo->pPage->DeactivatePage(pGroupInfo->pOutItemSet);
        }

        if(nLeave == SfxTabPage::KEEP_PAGE)
        {
            //die Seite darf nicht verlassen werden!
            pBox->Select(pCurrentPageEntry);
            return 0;
        }
        else
            pPageInfo->pPage->Hide();
    }
    if(pParent)
    {
        aPageTitleFT.Hide();
        aLine1FL.Hide();
        aHelpFT.Hide();
        aHelpImg.Hide();
        OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)pParent->GetUserData();
        if(!pPageInfo->pPage)
        {
            if(pGroupInfo->bLoadError)
                return 0;
            if ( RID_SVXPAGE_COLOR == pPageInfo->nPageId )
            {
                if(!pColorPageItemSet)
                {
                    pColorPageItemSet = new SfxItemSet( *XOutdevItemPool::Get(),
                                            XATTR_FILLSTYLE, XATTR_FILLCOLOR );
                    pColorPageItemSet->Put( XFillColorItem() );
                }
            }
            else
            {
                if(pGroupInfo->pModule /*&& !pGroupInfo->pModule->IsLoaded()*/)
                {
                    SfxModule* pOldModule = pGroupInfo->pModule;
                    sal_Bool bIdentical = pGroupInfo->pModule == pGroupInfo->pShell;

                    WaitObject aWait(this);
                    //pGroupInfo->pModule = pGroupInfo->pModule->Load();
                    if(!pGroupInfo->pModule)
                    {
                        pGroupInfo->bLoadError = sal_True;
                        InfoBox(pBox, sNotLoadedError).Execute();
                        return 0;
                    }
                    if(bIdentical)
                         pGroupInfo->pShell = pGroupInfo->pModule;
                    //jetzt noch testen, ob es auch in anderen Gruppen das gleiche Module gab (z.B. Text+HTML)
                    SvLBoxEntry* pTemp = aTreeLB.First();
                    while(pTemp)
                    {
                        if(!aTreeLB.GetParent(pTemp) && pTemp != pEntry)
                        {
                            OptionsGroupInfo* pTGInfo = (OptionsGroupInfo *)pTemp->GetUserData();
                            if(pTGInfo->pModule == pOldModule)
                            {
                                pTGInfo->pModule = pGroupInfo->pModule;
                                if(bIdentical)
                                    pTGInfo->pShell = pGroupInfo->pModule;
                            }
                        }
                        pTemp = aTreeLB.Next(pTemp);
                    }
                }

                if ( pPageInfo->nPageId != SID_SCH_TP_DEFCOLORS )
                {
                    if(!pGroupInfo->pInItemSet)
                        pGroupInfo->pInItemSet = pGroupInfo->pShell ? pGroupInfo->pShell->CreateItemSet( pGroupInfo->nDialogId ) : CreateItemSet( pGroupInfo->nDialogId );
                    if(!pGroupInfo->pOutItemSet)
                        pGroupInfo->pOutItemSet = new SfxItemSet(*pGroupInfo->pInItemSet->GetPool(), pGroupInfo->pInItemSet->GetRanges());
                }
            }

            if(pGroupInfo->pModule)
            {
                pPageInfo->pPage = pGroupInfo->pModule->CreateTabPage( pPageInfo->nPageId, this, *pGroupInfo->pInItemSet );
            }
            else if ( SID_SCH_TP_DEFCOLORS == pPageInfo->nPageId )
            {
                // Hack: force chart library to be loaded
                Reference < util::XCloseable > xCloseable ( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.chart.ChartDocument") ), UNO_QUERY );
                if ( xCloseable.is() )
                {
                    Reference < frame::XLoadable > xLoadable( xCloseable, UNO_QUERY );
                    xLoadable->initNew();
                    xCloseable->close( sal_True );
                }

                SfxModule *pSchMod = (*(SfxModule**) GetAppData(SHL_SCH));
                if ( pSchMod )
                {
                    if( !pGroupInfo->pInItemSet )
                        pGroupInfo->pInItemSet = pSchMod->CreateItemSet( pGroupInfo->nDialogId );
                    pPageInfo->pPage = pSchMod->CreateTabPage( pPageInfo->nPageId, this, *pGroupInfo->pInItemSet );
                    if( !pGroupInfo->pOutItemSet )
                        pGroupInfo->pOutItemSet = new SfxItemSet(*pGroupInfo->pInItemSet->GetPool(), pGroupInfo->pInItemSet->GetRanges());
                }
            }
            else if(RID_SVXPAGE_COLOR != pPageInfo->nPageId)
                pPageInfo->pPage = ::CreateGeneralTabPage( pPageInfo->nPageId, this, *pGroupInfo->pInItemSet );
            else
            {
                pPageInfo->pPage = ::CreateGeneralTabPage( pPageInfo->nPageId, this, *pColorPageItemSet );
                SvxColorTabPage& rColPage = *(SvxColorTabPage*)pPageInfo->pPage;
                const OfaPtrItem* pPtr = NULL;
                if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
                    pPtr = (const OfaPtrItem*)SfxViewFrame::Current()->
                        GetDispatcher()->Execute( SID_GET_COLORTABLE, SFX_CALLMODE_SYNCHRON );
                pColorTab = pPtr ? (XColorTable*)pPtr->GetValue() : XColorTable::GetStdColorTable();

                rColPage.SetColorTable( pColorTab );
                rColPage.SetPageType( &nUnknownType );
                rColPage.SetDlgType( &nUnknownType );
                rColPage.SetPos( &nUnknownPos );
                rColPage.SetAreaTP( &bIsAreaTP );
                rColPage.SetColorChgd( (ChangeType*)&nChangeType );
                rColPage.Construct();
            }
            SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pPageInfo->nPageId ) );
            pPageInfo->pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );

            Point aTreePos(aTreeLB.GetPosPixel());
            Size aTreeSize(aTreeLB.GetSizePixel());
            Point aGBPos(aHiddenGB.GetPosPixel());
            DBG_ASSERT(pPageInfo->pPage, "TabPage nicht gefunden!")
            Size aPageSize(pPageInfo->pPage->GetSizePixel());
            Size aGBSize(aHiddenGB.GetSizePixel());
            Point aPagePos( aGBPos.X() + ( aGBSize.Width() - aPageSize.Width() ) / 2,
                            aGBPos.Y() + ( aGBSize.Height() - aPageSize.Height() ) / 2 );
            pPageInfo->pPage->SetPosPixel( aPagePos );
            if ( RID_SVXPAGE_COLOR == pPageInfo->nPageId )
            {
                pPageInfo->pPage->Reset( *pColorPageItemSet );
                pPageInfo->pPage->ActivatePage( *pColorPageItemSet );
            }
            else
            {
                pPageInfo->pPage->Reset( *pGroupInfo->pInItemSet );
            }
        }
        if(RID_SVXPAGE_COLOR != pPageInfo->nPageId &&
                            pPageInfo->pPage->HasExchangeSupport())
        {
            pPageInfo->pPage->ActivatePage(*pGroupInfo->pOutItemSet);
        }
        pPageInfo->pPage->Show();
        String sTmpTitle = sTitle;
        sTmpTitle += String::CreateFromAscii(" - ");
        sTmpTitle += aTreeLB.GetEntryText(pParent);
        sTmpTitle += String::CreateFromAscii(" - ");
        sTmpTitle += aTreeLB.GetEntryText(pEntry);
        SetText(sTmpTitle);
        pCurrentPageEntry = pEntry;
        if(!bForgetSelection)
            nLastDialogPageId = pPageInfo->nPageId;
        pNewPage = pPageInfo->pPage;
    }
    else
    {
        static const sal_uInt16 _aGlobalHelpIds[] =
        {
            HID_OFADLG_TREE_GENERAL         ,
            HID_OFADLG_TREE_LANGUAGE        ,
            HID_OFADLG_TREE_INTERNET        ,
            HID_OFADLG_TREE_TEXT            ,
            HID_OFADLG_TREE_HTML            ,
            HID_OFADLG_TREE_CALC            ,
            HID_OFADLG_TREE_PRESENTATION    ,
            HID_OFADLG_TREE_DRAWING         ,
            HID_OFADLG_TREE_FORMULA         ,
            HID_OFADLG_TREE_CHART           ,
            HID_OFADLG_TREE_BASE            ,
            HID_OFADLG_TREE_FILTER
        };

        DBG_ASSERT( aHelpTextsArr.Count() ==
                    sizeof( _aGlobalHelpIds ) / sizeof(_aGlobalHelpIds[0]),
                    "fehlende HelpIds!!" );

/* !!! pb: #98643# do not expand automatically, only with <+> or <crsr><right>
        if(!aTreeLB.IsInCollapse())
            pBox->Expand(pEntry);
*/
        OptionsGroupInfo* pTGInfo = (OptionsGroupInfo *)pEntry->GetUserData();
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;
        ImageList* pImgLst = bHighContrast ? &aPageImagesHC : &aPageImages;
        //hier den Hilfetext anzeigen
        for(sal_uInt16 i = 0; i < aHelpTextsArr.Count(); i++)
        {
            if(aHelpTextsArr.GetValue(i) == pTGInfo->nDialogId)
            {
                aHelpFT.SetText(aHelpTextsArr.GetString(i));
                aHelpImg.SetImage(pImgLst->GetImage(pTGInfo->nDialogId));
                break;
            }
        }
        aHelpFT.Show();
        aLine1FL.Show();
        aPageTitleFT.Show();
        aHelpImg.Show();

        //auf die Groesse der Bitmap anpassen
        if(!bImageResized)
        {
            const long nCtrlDist = 2;
            bImageResized = sal_True;
            Point aImgPos(aHelpImg.GetPosPixel());
            Size aImgSize(aHelpImg.GetSizePixel());
            Point aTitlePos(aPageTitleFT.GetPosPixel());
            Point aLinePos(aLine1FL.GetPosPixel());
            Point aHelpPos(aHelpFT.GetPosPixel());
            Size aHelpSize(aHelpFT.GetSizePixel());
            long nXDiff = 0;
            long nYDiff = 0;
            if(aTitlePos.X() <= (aImgPos.X() + aImgSize.Width() + nCtrlDist))
            {
                nXDiff = aImgPos.X() + aImgSize.Width() + nCtrlDist - aTitlePos.X();
            }
            if(aLinePos.Y() <= (aImgPos.Y() + aImgSize.Height() + nCtrlDist))
            {
                nYDiff = aImgPos.Y() + aImgSize.Height() + nCtrlDist - aLinePos.Y();
            }
            aLinePos.Y() += nYDiff;
            aLine1FL.SetPosPixel(aLinePos);

            aTitlePos.X() += nXDiff;
            aPageTitleFT.SetPosPixel(aTitlePos);

            aHelpPos.X() += nXDiff;
            aHelpPos.Y() += nYDiff;
            aHelpSize.Width() -= nXDiff;
            aHelpSize.Height() -= nYDiff;
            aHelpFT.SetPosSizePixel(aHelpPos, aHelpSize);

            Font aFont = aHelpFT.GetFont();
            Size aSz = aFont.GetSize();
            aSz.Height() = (aSz.Height() * 14 ) / 10;
            aFont.SetSize(aSz);
            aPageTitleFT.SetFont(aFont);

        }
        aHelpImg.Show();
        String sTmpTitle = sTitle;
        sTmpTitle += String::CreateFromAscii(" - ");
        aPageTitleFT.SetText(aTreeLB.GetEntryText(pEntry));
        sTmpTitle += aPageTitleFT.GetText();
        SetText(sTmpTitle);
        pCurrentPageEntry = 0;
    }

    // restore lost focus, if necessary
    Window* pFocusWin = Application::GetFocusWindow();
    // if the focused window is not the options treebox and the old page has the focus
    if ( pFocusWin && pFocusWin != pBox && pOldPage && pOldPage->HasChildPathFocus() )
        // then set the focus to the new page or if we are on a group set the focus to the options treebox
        pNewPage ? pNewPage->GrabFocus() : pBox->GrabFocus();

    return 0;
}

/* -----------------11.02.99 15:51-------------------
 *
 * --------------------------------------------------*/

OfaPageResource::OfaPageResource() :
    Resource(ResId(RID_OFADLG_OPTIONS_TREE_PAGES, DIALOG_MGR())),
    aGeneralDlgAry(ResId(SID_GENERAL_OPTIONS)),
    aLangDlgAry(ResId(SID_LANGUAGE_OPTIONS)),
    aInetDlgAry(ResId(SID_INET_DLG)),
    aTextDlgAry(ResId(SID_SW_EDITOPTIONS)),
    aHTMLDlgAry(ResId(SID_SW_ONLINEOPTIONS)),
    aCalcDlgAry(ResId(SID_SC_EDITOPTIONS)),
    aStarMathDlgAry(ResId(SID_SM_EDITOPTIONS)),
    aImpressDlgAry(ResId(SID_SD_EDITOPTIONS)),
    aDrawDlgAry(ResId(SID_SD_GRAPHIC_OPTIONS)),
    aChartDlgAry(ResId(SID_SCH_EDITOPTIONS)),
    aFilterDlgAry(ResId(SID_FILTER_DLG)),
    aDatasourcesDlgAry(ResId(SID_SB_STARBASEOPTIONS))
{
    FreeResource();
}

BOOL EnableSSO( void )
{
    // SSO must be enabled if the configuration manager bootstrap settings
    // are configured as follows ...
    //  CFG_Offline=false
    //  CFG_ServerType=uno ( or unspecified )
    //  CFG_BackendService=
    //   com.sun.star.comp.configuration.backend.LdapSingleBackend

    OUString theIniFile;
    osl_getExecutableFile( &theIniFile.pData );
    theIniFile = theIniFile.copy( 0, theIniFile.lastIndexOf( '/' ) + 1 ) +
                 OUString::createFromAscii( SAL_CONFIGFILE( "configmgr" ) );
    ::rtl::Bootstrap theBootstrap( theIniFile );

    OUString theOfflineValue;
    OUString theDefaultOfflineValue = OUString::createFromAscii( "false" );
    theBootstrap.getFrom( OUString::createFromAscii( "CFG_Offline" ),
                          theOfflineValue,
                          theDefaultOfflineValue );

    OUString theServerTypeValue;
    theBootstrap.getFrom( OUString::createFromAscii( "CFG_ServerType" ),
                          theServerTypeValue );

    OUString theBackendServiceTypeValue;
    theBootstrap.getFrom( OUString::createFromAscii( "CFG_BackendService" ),
                          theBackendServiceTypeValue );

    BOOL bSSOEnabled =
        ( theOfflineValue == theDefaultOfflineValue                     &&
          ( theServerTypeValue.getLength() == 0 ||
          theServerTypeValue == OUString::createFromAscii( "uno" ) )    &&
          theBackendServiceTypeValue ==
            OUString::createFromAscii(
                "com.sun.star.comp.configuration.backend.LdapSingleBackend" ) );
    if ( bSSOEnabled && GetSSOCreator() == 0 )
    {
        bSSOEnabled = FALSE;
    }
    return bSSOEnabled;
}

void* GetSSOCreator( void )
{
    static void* theSymbol = 0;
    if ( theSymbol == 0 )
    {
        OUString theModuleName = OUString::createFromAscii( SVLIBRARY( "ssoopt" ) );
        oslModule theModule = osl_loadModule( theModuleName.pData, SAL_LOADMODULE_DEFAULT );
        if ( theModule != 0 )
        {
            OUString theSymbolName = OUString::createFromAscii( "CreateSSOTabPage" );
            theSymbol = osl_getSymbol( theModule, theSymbolName.pData );
        }
    }

    return theSymbol;
}

SfxItemSet* OfaTreeOptionsDialog::CreateItemSet( sal_uInt16 nId )
{
    Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
    SfxItemSet* pRet = 0;
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            pRet = new SfxItemSet(
                SFX_APP()->GetPool(),
                SID_ATTR_METRIC, SID_ATTR_SPELL,
                SID_AUTOSPELL_CHECK, SID_AUTOSPELL_MARKOFF,
                SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                SID_ATTR_YEAR2000, SID_ATTR_YEAR2000,
                0 );

            SfxItemSet aOptSet( SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            SFX_APP()->GetOptions(aOptSet);
            pRet->Put(aOptSet);

            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();
            const SfxPoolItem* pItem;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();

                // Sonstiges - Year2000
                if( SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_YEAR2000, pItem ) )
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, ((const SfxUInt16Item*)pItem)->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );


            // Sonstiges - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, pMisc->IsNotFoundWarning()));

            sal_uInt16 nFlag = pMisc->IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0;
            nFlag  |= pMisc->IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlag ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet = new SfxItemSet(SFX_APP()->GetPool(),
                    SID_ATTR_LANGUAGE, SID_AUTOSPELL_MARKOFF,
                    SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                    SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED,
                    0 );

            // fuer die Linguistik

            Reference< XSpellChecker1 >  xSpell = SvxGetSpellChecker();
            pRet->Put(SfxSpellCheckItem( xSpell, SID_ATTR_SPELL ));
            SfxHyphenRegionItem aHyphen( SID_ATTR_HYPHENREGION );

            sal_Int16   nMinLead  = 2,
                        nMinTrail = 2;
            if (xProp.is())
            {
                xProp->getPropertyValue( String::CreateFromAscii(
                        UPN_HYPH_MIN_LEADING) ) >>= nMinLead;
                xProp->getPropertyValue( String::CreateFromAscii(
                        UPN_HYPH_MIN_TRAILING) ) >>= nMinTrail;
            }
            aHyphen.GetMinLead()  = (sal_uInt8)nMinLead;
            aHyphen.GetMinTrail() = (sal_uInt8)nMinTrail;

            const SfxPoolItem* pItem;
            SfxPoolItem* pClone;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CJK_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CJK_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CTL_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CTL_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));

                pRet->Put(aHyphen);
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_CHECK, pItem))
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                }
                else
                {
                        sal_Bool bVal = sal_False;
                        if (xProp.is())
                        {
                            xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_AUTO) ) >>= bVal;
                        }

                        pRet->Put(SfxBoolItem(SID_AUTOSPELL_CHECK, bVal));
                }

                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_MARKOFF, pItem))
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                }
                else
                {
                    sal_Bool bVal = sal_False;
                    if (xProp.is())
                    {
                        xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_HIDE) ) >>= bVal;
                    }
                    pRet->Put(SfxBoolItem(SID_AUTOSPELL_MARKOFF, bVal));
                }
            }
        }
        break;
        case SID_INET_DLG :
                pRet = new SfxItemSet( SFX_APP()->GetPool(),
                                SID_BASIC_ENABLED, SID_BASIC_ENABLED,
                //SID_OPTIONS_START - ..END
                                SID_OPTIONS_START, SID_INET_PROXY_PORT,
                                SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                                SID_INET_SMTPSERVER, SID_INET_SMTPSERVER,
                                SID_INET_NOPROXY, SID_INET_SOCKS_PROXY_PORT,
                                SID_INET_DNS_AUTO, SID_INET_DNS_SERVER,
                                SID_SECURE_URL, SID_SECURE_URL,
                                0L );
                SFX_APP()->GetOptions(*pRet);
        break;
        case SID_FILTER_DLG:
            pRet = new SfxItemSet( SFX_APP()->GetPool(),
            SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
            SID_SAVEREL_INET, SID_SAVEREL_FSYS,
            SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING,
            0 );
            SFX_APP()->GetOptions(*pRet);
            break;

        case SID_SB_STARBASEOPTIONS:
            pRet = new SfxItemSet( SFX_APP()->GetPool(),
            SID_SB_POOLING_ENABLED, SID_SB_DB_REGISTER,
            0 );
            ::offapp::ConnectionPoolConfig::GetOptions(*pRet);
            ::svx::DbRegisteredNamesConfig::GetOptions(*pRet);
            break;
    }
    return pRet;
}

void OfaTreeOptionsDialog::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();
            const SfxPoolItem* pItem;
            SfxItemSet aOptSet(SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            aOptSet.Put(rSet);
            if(aOptSet.Count())
                SFX_APP()->SetOptions( aOptSet );
            // Dispatcher neu holen, weil SetOptions() ggf. den Dispatcher zerst"ort hat
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
// -------------------------------------------------------------------------
//          Year2000 auswerten
// -------------------------------------------------------------------------
            USHORT nY2K = USHRT_MAX;
            if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, sal_False, &pItem ) )
                nY2K = ((const SfxUInt16Item*)pItem)->GetValue();
            if ( pViewFrame && USHRT_MAX != nY2K)
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                pDispatch->Execute( SID_ATTR_YEAR2000, SFX_CALLMODE_ASYNCHRON, pItem, 0L);
            }

            pMisc->SetYear2000(nY2K);

// -------------------------------------------------------------------------
//          Drucken auswerten
// -------------------------------------------------------------------------
            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_NOTFOUND_WARN, sal_False, &pItem))
                pMisc->SetNotFoundWarning(((const SfxBoolItem*)pItem)->GetValue());

            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_CHANGESTODOC, sal_False, &pItem))
            {
                const SfxFlagItem* pFlag = (const SfxFlagItem*)pItem;
                pMisc->SetPaperSizeWarning(0 != (pFlag->GetValue() &  SFX_PRINTER_CHG_SIZE ));
                pMisc->SetPaperOrientationWarning(0 !=  (pFlag->GetValue() & SFX_PRINTER_CHG_ORIENTATION ));
            }
// -------------------------------------------------------------------------
//          evaluate help options
// -------------------------------------------------------------------------
            if ( SvtHelpOptions().IsHelpTips() != Help::IsQuickHelpEnabled() )
                SvtHelpOptions().IsHelpTips() ? Help::EnableQuickHelp() : Help::DisableQuickHelp();
            if ( SvtHelpOptions().IsExtendedHelp() != Help::IsBalloonHelpEnabled() )
                SvtHelpOptions().IsExtendedHelp() ? Help::EnableBalloonHelp() : Help::DisableBalloonHelp();
        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {

            sal_Bool bSaveSpellCheck = sal_False;
            const SfxPoolItem* pItem;

            if ( SFX_ITEM_SET == rSet.GetItemState( SID_SPELL_MODIFIED, sal_False, &pItem ) )
            {
                bSaveSpellCheck = ( (const SfxBoolItem*)pItem )->GetValue();
            }
            Reference< XMultiServiceFactory >  xMgr( ::comphelper::getProcessServiceFactory() );
            Reference< XPropertySet >  xProp(
                    xMgr->createInstance( OUString::createFromAscii(
                            "com.sun.star.linguistic2.LinguProperties") ),
                    UNO_QUERY );
            if ( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, sal_False, &pItem ) )
            {
                const SfxHyphenRegionItem* pHyphenItem = (const SfxHyphenRegionItem*)pItem;

                if (xProp.is())
                {
                    xProp->setPropertyValue(
                            String::CreateFromAscii(UPN_HYPH_MIN_LEADING),
                            makeAny((sal_Int16) pHyphenItem->GetMinLead()) );
                    xProp->setPropertyValue(
                            String::CreateFromAscii(UPN_HYPH_MIN_TRAILING),
                            makeAny((sal_Int16) pHyphenItem->GetMinTrail()) );
                }
                bSaveSpellCheck = sal_True;
            }

            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                pItem = 0;
                if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_LANGUAGE, sal_False, &pItem ))
                {
                    pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
                }
                if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE, sal_False, &pItem ))
                {
                    pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
                }
                if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE, sal_False, &pItem ))
                {
                    pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
                }

                if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_CHECK, sal_False, &pItem ))
                {
                    sal_Bool bOnlineSpelling = ((const SfxBoolItem*)pItem)->GetValue();
                    pDispatch->Execute(SID_AUTOSPELL_CHECK,
                        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

                    if (xProp.is())
                    {
                        xProp->setPropertyValue(
                                String::CreateFromAscii(UPN_IS_SPELL_AUTO),
                                makeAny(bOnlineSpelling) );
                    }
                }

                if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_MARKOFF, sal_False, &pItem ))
                {
                    sal_Bool bHideSpell = ((const SfxBoolItem*)pItem)->GetValue();
                    pDispatch->Execute(SID_AUTOSPELL_MARKOFF, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

                    if (xProp.is())
                    {
                        xProp->setPropertyValue(
                                String::CreateFromAscii(UPN_IS_SPELL_HIDE),
                                makeAny(bHideSpell) );
                    }
                }

                if( bSaveSpellCheck )
                {
                    //! the config item has changed since we modified the
                    //! property set it uses
                    pDispatch->Execute(SID_SPELLCHECKER_CHANGED, SFX_CALLMODE_ASYNCHRON);
                }
            }

            if( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_LOCALE_CHANGED, sal_False, &pItem ))
            {
                SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
                while ( pViewFrame )
                {
                    pViewFrame->GetDispatcher()->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
                }
            }
        }
        break;
        case SID_INET_DLG :
        case SID_FILTER_DLG:
            SFX_APP()->SetOptions( rSet );

        case SID_SB_STARBASEOPTIONS:
            ::offapp::ConnectionPoolConfig::SetOptions( rSet );
            ::svx::DbRegisteredNamesConfig::SetOptions(rSet);
            break;
        break;
    }

}

void OfaTreeOptionsDialog::Initialize()
{
    USHORT nSlot = SID_OPTIONS_TREEDIALOG;
    OfaPageResource aDlgResource;
    sal_uInt16 nGroup = 0;
    SfxApplication* pApp = SFX_APP();

    if(nSlot != SID_OPTPAGE_PROXY)
    {
        BOOL isSSOEnabled = EnableSSO();

        ResStringArray& rGeneralArray = aDlgResource.GetGeneralArray();
        nGroup = AddGroup(rGeneralArray.GetString(0), 0, 0, SID_GENERAL_OPTIONS );
        sal_uInt16 nEnd = nSlot == SID_OPTIONS_TREEDIALOG ? rGeneralArray.Count() : 2;

        sal_uInt16 i;
        for(i = 1; i < nEnd; i++)
        {
            sal_uInt16 nPageId = (sal_uInt16)rGeneralArray.GetValue(i);
            if ( nPageId != RID_SVXPAGE_SSO || isSSOEnabled )
                AddTabPage( nPageId, rGeneralArray.GetString(i), nGroup );
        }

        //load/save
        ResStringArray& rFilterArray = aDlgResource.GetFilterArray();
        nGroup = AddGroup( rFilterArray.GetString(0), 0, 0, SID_FILTER_DLG );
        for(i = 1; i < rFilterArray.Count(); ++i )
            AddTabPage( (sal_uInt16)rFilterArray.GetValue(i),
                                rFilterArray.GetString(i), nGroup );

        // new spec: always show chart pages in general options
        ResStringArray& rChartArray = aDlgResource.GetChartArray();
        nGroup = AddGroup( rChartArray.GetString(0), 0, 0, SID_SCH_EDITOPTIONS );
        for(USHORT i1 = 1; i1 < rChartArray.Count(); i1++)
            AddTabPage( (sal_uInt16)rChartArray.GetValue(i1), rChartArray.GetString(i1), nGroup);
    }

    SvtLanguageOptions aLanguageOptions;

    if ( nSlot == SID_OPTIONS_TREEDIALOG )
    {
        // language options
        ResStringArray& rLangArray = aDlgResource.GetLangArray();
        nGroup = AddGroup( rLangArray.GetString(0), 0, 0, SID_LANGUAGE_OPTIONS );
        for ( USHORT i = 1; i < rLangArray.Count(); ++i )
        {
            sal_uInt16 nValue = (sal_uInt16)rLangArray.GetValue(i);
            if ( ( RID_SVXPAGE_JSEARCH_OPTIONS != nValue || aLanguageOptions.IsJapaneseFindEnabled() ) &&
                 ( RID_SVXPAGE_ASIAN_LAYOUT != nValue    || aLanguageOptions.IsAsianTypographyEnabled() ) &&
                 ( RID_SVXPAGE_OPTIONS_CTL != nValue     || aLanguageOptions.IsCTLFontEnabled() ) )
                AddTabPage( nValue, rLangArray.GetString(i), nGroup );
        }
    }
    if ( nSlot != SID_OPTPAGE_USERDATA )
    {
        // Internet
        // f"ur SID_OPTPAGE_PROXY wird der komplett INet-Dlg angezeigt
        ResStringArray& rInetArray = aDlgResource.GetInetArray();
        nGroup = AddGroup(rInetArray.GetString(0), 0, 0, SID_INET_DLG );
        //falls doch nur dir Proxy-Page gewuenscht wird, dann diese Zeile
//      sal_uInt16 nEnd = nSlot == SID_OPTPAGE_PROXY ? 2 : rInetArray.Count();
        sal_uInt16 nEnd = rInetArray.Count();

        for ( sal_uInt16 i = 1; i < nEnd; i++ )
        {
#if defined WNT
            // Disable E-mail tab-page on Windows
            if ( i == 3 )
                continue;
#endif
            sal_uInt16 nPageId = (sal_uInt16)rInetArray.GetValue(i);
            AddTabPage( nPageId, rInetArray.GetString(i), nGroup );
        }
        if ( nSlot == SID_OPTPAGE_PROXY )
            ActivatePage(   RID_SVXPAGE_INET_PROXY );
    }

    if ( nSlot == SID_OPTIONS_TREEDIALOG )
    {
        sal_Bool bHasAnyFilter = sal_False;
        SvtModuleOptions aModuleOpt;
        if ( aModuleOpt.IsWriter() )
        {
            // Textdokument
            bHasAnyFilter = sal_True;
            ResStringArray& rTextArray = aDlgResource.GetTextArray();
            SfxModule *pSwMod = (*(SfxModule**) GetAppData(SHL_WRITER));
            if ( pSwMod && pSwMod->IsActive() )
            {
                nGroup = AddGroup(rTextArray.GetString(0), pSwMod, pSwMod, SID_SW_EDITOPTIONS );
                USHORT i;
                for(i = 1; i < rTextArray.Count(); i++)
                {
                    sal_uInt16 nValue = (sal_uInt16)rTextArray.GetValue(i);
                    if((RID_SW_TP_STD_FONT_CJK != nValue || aLanguageOptions.IsCJKFontEnabled())&&
                        (RID_SW_TP_STD_FONT_CTL != nValue || aLanguageOptions.IsCTLFontEnabled()))
                        AddTabPage( nValue, rTextArray.GetString(i), nGroup);
                }
    #ifndef PRODUCT
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup);
    #endif
                // HTML-Dokument
                ResStringArray& rHTMLArray = aDlgResource.GetHTMLArray();
                nGroup = AddGroup(rHTMLArray.GetString(0), pSwMod, pSwMod, SID_SW_ONLINEOPTIONS );
                for(i = 1; i < rHTMLArray.Count(); i++)
                    AddTabPage( (sal_uInt16)rHTMLArray.GetValue(i), rHTMLArray.GetString(i), nGroup);
    #ifndef PRODUCT
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup);
    #endif
            }
        }

        if ( aModuleOpt.IsCalc() )
        {
            // StarCalc-Dialog
            bHasAnyFilter = sal_True;
            SfxModule*      pScMod = ( *( SfxModule** ) GetAppData( SHL_CALC ) );
            if ( pScMod && pScMod->IsActive() )
            {
                ResStringArray& rCalcArray = aDlgResource.GetCalcArray();
                nGroup = AddGroup( rCalcArray.GetString( 0 ), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const sal_Bool  bCTL = aLanguageOptions.IsCTLFontEnabled();
                sal_uInt16      nId;
                const USHORT    nCount = rCalcArray.Count();
                for( USHORT i = 1 ; i < nCount ; ++i )
                {
                    nId = ( sal_uInt16 ) rCalcArray.GetValue( i );
    //              if( bCTL || nId != RID_OFA_TP_INTERNATIONAL )
    //              #103755# if an international tabpage is need one day, this should be used again... ;-)
                    if( nId != RID_OFA_TP_INTERNATIONAL )
                        AddTabPage( nId, rCalcArray.GetString( i ), nGroup );
                }
            }
        }

        if ( aModuleOpt.IsImpress() )
        {
            //Praesentation
            bHasAnyFilter = sal_True;
            SfxModule*      pSdMod = ( *( SfxModule** ) GetAppData( SHL_DRAW ) );
            if ( pSdMod && pSdMod->IsActive() )
            {
                ResStringArray& rImpressArray = aDlgResource.GetImpressArray();
                nGroup = AddGroup( rImpressArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_Bool  bCTL = aLanguageOptions.IsCTLFontEnabled();
                sal_uInt16      nId;
                const USHORT    nCount = rImpressArray.Count();
                for( USHORT i = 1 ; i < nCount ; ++i )
                {
                    nId = ( sal_uInt16 ) rImpressArray.GetValue( i );
                    if( bCTL || nId != RID_OFA_TP_INTERNATIONAL_IMPR )
                        AddTabPage( nId, rImpressArray.GetString( i ), nGroup );
                }
            }
        }

        if ( aModuleOpt.IsDraw() )
        {
            //Zeichnung
            SfxModule*      pSdMod = ( *( SfxModule** ) GetAppData( SHL_DRAW ) );
            if ( pSdMod && pSdMod->IsActive() )
            {
                ResStringArray& rDrawArray = aDlgResource.GetDrawArray();
                nGroup = AddGroup( rDrawArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_Bool  bCTL = aLanguageOptions.IsCTLFontEnabled();
                sal_uInt16      nId;
                const USHORT    nCount = rDrawArray.Count();
                for( USHORT i = 1 ; i < nCount ; ++i )
                {
                    nId = ( sal_uInt16 ) rDrawArray.GetValue( i );
                    if( bCTL || nId != RID_OFA_TP_INTERNATIONAL_SD )
                        AddTabPage( nId, rDrawArray.GetString( i ), nGroup );
                }
            }
        }

        if ( aModuleOpt.IsMath() )
        {
            // StarMath-Dialog
            SfxModule *pSmMod = (*(SfxModule**) GetAppData(SHL_SM));
            if ( pSmMod && pSmMod->IsActive() )
            {
                ResStringArray& rStarMathArray = aDlgResource.GetStarMathArray();
                nGroup = AddGroup(rStarMathArray.GetString(0), pSmMod, pSmMod, SID_SM_EDITOPTIONS );
                for(USHORT i = 1; i < rStarMathArray.Count(); i++)
                    AddTabPage( (sal_uInt16)rStarMathArray.GetValue(i), rStarMathArray.GetString(i), nGroup);
            }
        }
/*
        if ( aModuleOpt.IsChart() )
        {
            //Diagramm
            SfxModule *pSchMod = (*(SfxModule**) GetAppData(SHL_SCH));
            if ( pSchMod && pSchMod->IsActive() )
            {
                ResStringArray& rChartArray = aDlgResource.GetChartArray();
                nGroup = AddGroup(rChartArray.GetString(0), pSchMod, pSchMod, SID_SCH_EDITOPTIONS );
                for(USHORT i = 1; i < rChartArray.Count(); i++)
                    AddTabPage( (sal_uInt16)rChartArray.GetValue(i), rChartArray.GetString(i), nGroup);
            }
        }
*/
        if (sal_True)
        {   // Data access (always installed)
            ResStringArray& rDSArray = aDlgResource.GetDatasourcesArray();
            nGroup = AddGroup(rDSArray.GetString(0), 0, NULL, SID_SB_STARBASEOPTIONS );
            for(USHORT i = 1; i < rDSArray.Count(); i++)
                AddTabPage( (sal_uInt16)rDSArray.GetValue(i), rDSArray.GetString(i), nGroup);
        }

        ResizeTreeLB();
        ActivateLastSelection();
    }
    else
        ResizeTreeLB();

    if ( nSlot == SID_OPTPAGE_USERDATA )
        ActivatePage( RID_SFXPAGE_GENERAL );
}

namespace
{
    void MoveControl( Control& _rCtrl, long _nDeltaPixel )
    {
        Point   aPt( _rCtrl.GetPosPixel() );
        aPt.X() += _nDeltaPixel;
        _rCtrl.SetPosPixel( aPt );
    }
}

void OfaTreeOptionsDialog::ResizeTreeLB( void )
{
    const long          nMax = aHiddenGB.GetSizePixel().Width() * 42 / 100;
                                                    // don't ask where 42 comes from... but it looks / feels ok ;-)
    long                nDelta = 50;                // min.
    USHORT              nDepth = 0;
    const long          nIndent0 = PixelToLogic( Size( 28, 0 ) ).Width();
    const long          nIndent1 = PixelToLogic( Size( 52, 0 ) ).Width();

    SvTreeList*         pTreeList = aTreeLB.GetModel();
    DBG_ASSERT( pTreeList, "-OfaTreeOptionsDialog::ResizeTreeLB(): no model, no cookies!" );

    SvListEntry*        pEntry = pTreeList->First();
    while( pEntry )
    {
        long            n = aTreeLB.GetTextWidth( aTreeLB.GetEntryText( static_cast< SvLBoxEntry* >( pEntry ) ) );
        n += ( nDepth == 0 )? nIndent0 : nIndent1;

        if( n > nDelta )
            nDelta = n;

        pEntry = pTreeList->Next( pEntry, &nDepth );
    }

    nDelta = LogicToPixel( Size( nDelta + 3, 0 ) ).Width();         // + extra space [logic]
    nDelta += GetSettings().GetStyleSettings().GetScrollBarSize();  // + scroll bar, in case it's needed

    if( nDelta > nMax )
        nDelta = nMax;

    // starting resizing with this
    Size            aSize( GetSizePixel() );
    aSize.Width() += nDelta;
    SetSizePixel( aSize );

    // resize treelistbox
    aSize = aTreeLB.GetSizePixel();
    aSize.Width() += nDelta;
    aTreeLB.SetSizePixel( aSize );

    // ... and move depending controls
    MoveControl( aOkPB, nDelta );
    MoveControl( aCancelPB, nDelta );
    MoveControl( aHelpPB, nDelta );
    MoveControl( aBackPB, nDelta );
    MoveControl( aHiddenGB, nDelta );
    MoveControl( aPageTitleFT, nDelta );
    MoveControl( aLine1FL, nDelta );
    MoveControl( aHelpFT, nDelta );
    MoveControl( aHelpImg, nDelta );
}

short OfaTreeOptionsDialog::Execute()
{
    // collect all DictionaryList Events while the dialog is executed
    Reference<com::sun::star::linguistic2::XDictionaryList> xDictionaryList(SvxGetDictionaryList());
    SvxDicListChgClamp aClamp( xDictionaryList );
    short nRet = SfxModalDialog::Execute();

    if( RET_OK == nRet )
    {
        ApplyItemSets();
        if( GetColorTable() )
        {
            GetColorTable()->Save();

            // notify current viewframe it it uses the same color table
            if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
            {
                const OfaPtrItem* pPtr = (const OfaPtrItem*)SfxViewFrame::Current()->GetDispatcher()->Execute( SID_GET_COLORTABLE, SFX_CALLMODE_SYNCHRON );
                if( pPtr )
                {
                    XColorTable* pColorTab = (XColorTable*)pPtr->GetValue();

                    if( pColorTab &&
                        pColorTab->GetPath() == GetColorTable()->GetPath() &&
                        pColorTab->GetName() == GetColorTable()->GetName() )
                        SfxObjectShell::Current()->PutItem( SvxColorTableItem( GetColorTable() ) );
                }
            }
        }

        utl::ConfigManager::GetConfigManager()->StoreConfigItems();
    }

    return nRet;
}
