/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formatclipboard.cxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "formatclipboard.hxx"
#include <svx/globl3d.hxx>

// header for class SfxItemIter
#include <svl/itemiter.hxx>

// header for class SfxStyleSheet
#include <svl/style.hxx>

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
        case OBJ_OUTLINETEXT:
        case OBJ_GRAF:
        case OBJ_OLE2:
        case OBJ_EDGE:
        case OBJ_CAPTION:
            return false;
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
            return true;
        case OBJ_PAGE:
        case OBJ_MEASURE:
        case OBJ_DUMMY:
        case OBJ_FRAME:
        case OBJ_UNO:
            return false;
        case OBJ_CUSTOMSHAPE:
            return true;
        default:
            return false;
    }
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

void SdFormatClipboard::Paste( ::sd::View& rDrawView, bool, bool )
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
            boost::shared_ptr< SfxItemSet > pTargetSet;

            if( pObj->GetStyleSheet() )
            {
                pTargetSet.reset( new SfxItemSet( pObj->GetStyleSheet()->GetItemSet() ) );
            }
            else
            {
                SdrModel* pModel = pObj->GetModel();
                if( pModel )
                {
                    pTargetSet.reset( new SfxItemSet( pModel->GetItemPool() ) );
                }
            }

            if( pTargetSet.get() )
            {
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
                            nTargetState = pTargetSet->GetItemState( nWhich );
                            pTargetItem = pTargetSet->GetItem( nWhich );

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
