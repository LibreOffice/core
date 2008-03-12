/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgolbul.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:36:16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "OutlineBulletDlg.hxx"

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _SVX_BULITEM_HXX
#include <svx/bulitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#include <svx/numitem.hxx>

#include <svx/dialogs.hrc>
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

#include "glob.hrc"
#include "dlgolbul.hrc"
#include "bulmaper.hxx"
#include "DrawDocShell.hxx"
#include <svx/svxids.hrc>
#include <svtools/aeitem.hxx>

namespace sd {

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

OutlineBulletDlg::OutlineBulletDlg(
    ::Window* pParent,
    const SfxItemSet* pAttr,
    ::sd::View* pView )
    : SfxTabDialog  ( pParent, SdResId(TAB_OUTLINEBULLET) ),
      aInputSet     ( *pAttr ),
      bTitle            ( FALSE ),
      pSdView           ( pView )
{
    FreeResource();

    aInputSet.MergeRange( SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL );
    aInputSet.Put( *pAttr );

    pOutputSet = new SfxItemSet( *pAttr );
    pOutputSet->ClearItem();

    BOOL bOutliner = FALSE;

    // Sonderbehandlung wenn eine Title Objekt selektiert wurde
    if( pView )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        const ULONG nCount = rMarkList.GetMarkCount();
        for(ULONG nNum = 0; nNum < nCount; nNum++)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor )
            {

                switch(pObj->GetObjIdentifier())
                {
                case OBJ_TITLETEXT:
                    bTitle = TRUE;
                    break;
                case OBJ_OUTLINETEXT:
                    bOutliner = TRUE;
                    break;
                }
            }
        }
    }

    if( SFX_ITEM_SET != aInputSet.GetItemState(EE_PARA_NUMBULLET))
    {
        const SvxNumBulletItem *pItem = NULL;
        if(bOutliner)
        {
            SfxStyleSheetBasePool* pSSPool = pView->GetDocSh()->GetStyleSheetPool();
            String aStyleName((SdResId(STR_LAYOUT_OUTLINE)));
            aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
            if( pFirstStyleSheet )
                pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, FALSE, (const SfxPoolItem**)&pItem);
        }

        if( pItem == NULL )
            pItem = (SvxNumBulletItem*) aInputSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);

        DBG_ASSERT( pItem, "Kein EE_PARA_NUMBULLET im Pool! [CL]" );

        aInputSet.Put(*pItem, EE_PARA_NUMBULLET);
    }

    /* debug
    if( SFX_ITEM_SET == aInputSet.GetItemState(EE_PARA_NUMBULLET, FALSE, &pItem ))
    {
        SvxNumRule& rItem = *((SvxNumBulletItem*)pItem)->GetNumRule();
        for( int i = 0; i < 9; i++ )
        {
            SvxNumberFormat aNumberFormat = rItem.GetLevel(i);
        }
    }
    */

    if(bTitle && aInputSet.GetItemState(EE_PARA_NUMBULLET,TRUE) == SFX_ITEM_ON )
    {
        SvxNumBulletItem* pItem = (SvxNumBulletItem*)aInputSet.GetItem(EE_PARA_NUMBULLET,TRUE);
        SvxNumRule* pRule = pItem->GetNumRule();
        if(pRule)
        {
            SvxNumRule aNewRule( *pRule );
            aNewRule.SetFeatureFlag( NUM_NO_NUMBERS, TRUE );

            SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
            aInputSet.Put(aNewItem);
        }
    }

    SdBulletMapper::PreMapNumBulletForDialog( aInputSet );

    SetInputSet( &aInputSet );

    if(!bTitle)
        AddTabPage(RID_SVXPAGE_PICK_SINGLE_NUM);
    else
        RemoveTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );

    AddTabPage( RID_SVXPAGE_PICK_BULLET  );
    AddTabPage( RID_SVXPAGE_PICK_BMP   );
    AddTabPage(RID_SVXPAGE_NUM_OPTIONS  );
    AddTabPage(RID_SVXPAGE_NUM_POSITION );

}

OutlineBulletDlg::~OutlineBulletDlg()
{
    delete pOutputSet;
}

void OutlineBulletDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch ( nId )
    {
        case RID_SVXPAGE_NUM_OPTIONS:
        {
            if( pSdView )
            {
                FieldUnit eMetric = pSdView->GetDoc()->GetUIUnit();
                SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
                aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,(USHORT)eMetric));
                rPage.PageCreated(aSet);
            }
        }
        break;
        case RID_SVXPAGE_NUM_POSITION:
        {
            if( pSdView )
            {
                FieldUnit eMetric = pSdView->GetDoc()->GetUIUnit();
                SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
                aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,(USHORT)eMetric));
                rPage.PageCreated(aSet);
            }
        }
        break;
    }
}

const SfxItemSet* OutlineBulletDlg::GetOutputItemSet() const
{
    SfxItemSet aSet( *SfxTabDialog::GetOutputItemSet() );
    pOutputSet->Put( aSet );

    const SfxPoolItem *pItem = NULL;
    if( SFX_ITEM_SET == pOutputSet->GetItemState(pOutputSet->GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE), FALSE, &pItem ))
    {
        SdBulletMapper::MapFontsInNumRule( *((SvxNumBulletItem*)pItem)->GetNumRule(), *pOutputSet );

        SfxUInt16Item aBulletState( EE_PARA_BULLETSTATE, 1 );
        pOutputSet->Put(aBulletState);
    }

    SdBulletMapper::PostMapNumBulletForDialog( *pOutputSet );

    if(bTitle && pOutputSet->GetItemState(EE_PARA_NUMBULLET,TRUE) == SFX_ITEM_ON )
    {
        SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)pOutputSet->GetItem(EE_PARA_NUMBULLET,TRUE);
        SvxNumRule* pRule = pBulletItem->GetNumRule();
        if(pRule)
            pRule->SetFeatureFlag( NUM_NO_NUMBERS, FALSE );
    }

    return pOutputSet;
}

} // end of namespace sd
