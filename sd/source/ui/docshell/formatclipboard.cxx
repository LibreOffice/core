/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatclipboard.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 11:31:28 $
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

#include "formatclipboard.hxx"

#ifndef _E3D_GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif

// header for class SfxItemIter
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif

// header for class SfxStyleSheet
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

/*--------------------------------------------------------------------
 --------------------------------------------------------------------*/

SdFormatClipboard::SdFormatClipboard()
        : m_pItemSet(0)
        , m_bPersistentCopy(false)
        , m_nType_Inventor(0)
        , m_nType_Identifier(0)
{
}
SdFormatClipboard::~SdFormatClipboard()
{
    if(m_pItemSet)
        delete m_pItemSet;
}

bool SdFormatClipboard::HasContent() const
{
    return m_pItemSet!=0;
}

bool SdFormatClipboard::CanCopyThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier ) const
{
    if( nObjectInventor != SdrInventor && nObjectInventor != E3dInventor )
        return false;
    switch(nObjectIdentifier)
    {
        case OBJ_NONE:
        case OBJ_GRUP:
            return false;
            break;
        case OBJ_LINE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_TEXT:
        case OBJ_TEXTEXT:
        case OBJ_TITLETEXT:
            return true;
            break;
        case OBJ_OUTLINETEXT:
        case OBJ_GRAF:
        case OBJ_OLE2:
        case OBJ_EDGE:
        case OBJ_CAPTION:
            return false;
            break;
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
            return true;
            break;
        case OBJ_PAGE:
        case OBJ_MEASURE:
        case OBJ_DUMMY:
        case OBJ_FRAME:
        case OBJ_UNO:
            return false;
            break;
        case OBJ_CUSTOMSHAPE:
            return true;
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool SdFormatClipboard::HasContentForThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier ) const
{
    if( !HasContent() )
        return false;
    if( !CanCopyThisType( nObjectInventor, nObjectIdentifier ) )
        return false;
    return true;
}

void SdFormatClipboard::Copy( ::sd::View& rDrawView, bool bPersistentCopy )
{
    this->Erase();
    m_bPersistentCopy = bPersistentCopy;

    const SdrMarkList& rMarkList = rDrawView.GetMarkedObjectList();
    if( rMarkList.GetMarkCount() >= 1 )
    {
        BOOL bOnlyHardAttr = FALSE;
        m_pItemSet = new SfxItemSet( rDrawView.GetAttrFromMarked(bOnlyHardAttr) );

        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        m_nType_Inventor = pObj->GetObjInventor();
        m_nType_Identifier = pObj->GetObjIdentifier();
    }
}

void SdFormatClipboard::Paste( ::sd::View& rDrawView
                              , bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    if( !rDrawView.AreObjectsMarked() )
    {
        if(!m_bPersistentCopy)
            this->Erase();
        return;
    }

    SdrObject* pObj = 0;

    bool bWrongTargetType = false;
    {
        const SdrMarkList& rMarkList = rDrawView.GetMarkedObjectList();
        if( rMarkList.GetMarkCount() != 1 )
            bWrongTargetType = true;
        else
        {
            pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if( pObj && pObj->GetStyleSheet() )
                bWrongTargetType = !this->HasContentForThisType( pObj->GetObjInventor(), pObj->GetObjIdentifier() );
        }
    }
    if( bWrongTargetType )
    {
        if(!m_bPersistentCopy)
            this->Erase();
        return;
    }
    if(m_pItemSet)
    {
        //modify source itemset
        {
            BOOL bOnlyHardAttr = FALSE;
            SfxItemSet aTargetSet( pObj->GetStyleSheet()->GetItemSet() );

            USHORT nWhich=0;
            SfxItemState nSourceState;
            SfxItemState nTargetState;
            const SfxPoolItem* pSourceItem=0;
            const SfxPoolItem* pTargetItem=0;
            SfxItemIter aSourceIter(*m_pItemSet);
            pSourceItem = aSourceIter.FirstItem();
            while( pSourceItem!=NULL )
            {
                if (!IsInvalidItem(pSourceItem))
                {
                    nWhich = pSourceItem->Which();
                    if(nWhich)
                    {
                        nSourceState = m_pItemSet->GetItemState( nWhich );
                        nTargetState = aTargetSet.GetItemState( nWhich );
                        pTargetItem = aTargetSet.GetItem( nWhich );
                        ::com::sun::star::uno::Any aSourceValue, aTargetValue;

                        if(!pTargetItem)
                            m_pItemSet->ClearItem(nWhich);
                        else if( (*pSourceItem) == (*pTargetItem) )
                        {
                            //do not set items which have the same content in source and target
                            m_pItemSet->ClearItem(nWhich);
                        }
                    }
                }
                pSourceItem = aSourceIter.NextItem();
            }//end while
        }
        BOOL bReplaceAll = TRUE;
        rDrawView.SetAttrToMarked(*m_pItemSet, bReplaceAll);
    }
    if(!m_bPersistentCopy)
        this->Erase();
}

void SdFormatClipboard::Erase()
{
    if(m_pItemSet)
    {
        delete m_pItemSet;
        m_pItemSet = 0;
    }
    m_nType_Inventor=0;
    m_nType_Identifier=0;
    m_bPersistentCopy = false;
}
