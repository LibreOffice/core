/*************************************************************************
 *
 *  $RCSfile: fmexch.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-26 15:04:39 $
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
#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_DBEXCH_HRC
#include <dbexch.hrc>
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

static sal_uInt32 nFieldFormat = 0;
static sal_uInt32 nControlFormat = 0;

//========================================================================
// class SvxFmExplCtrlExch
//========================================================================

//------------------------------------------------------------------------
SvxFmExplCtrlExch::SvxFmExplCtrlExch( const vector<SvLBoxEntry*>& lstWhich, FmFormShell* pShell, FmFormPage* pPage )
    :m_aDraggedEntries(lstWhich)
    ,m_pShell(pShell)
    ,m_pPage(pPage)
{
    m_aDataTypeList.Insert( SvDataType(Exchange::RegisterFormatName(SVX_FM_CONTROL_EXCH)) );
}

//------------------------------------------------------------------------
SvxFmExplCtrlExch::SvxFmExplCtrlExch( SvLBoxEntry* pEntry )
    :m_pShell(NULL)
    ,m_pPage(NULL)
{
    m_aDataTypeList.Insert( SvDataType(Exchange::RegisterFormatName(SVX_FM_CONTROL_EXCH)) );

    AddItem(pEntry);
}

//------------------------------------------------------------------------
sal_Bool SvxFmExplCtrlExch::GetData( SvData* pData )
{
    return sal_False;
}

//------------------------------------------------------------------------
void SvxFmExplCtrlExch::BuildPathFormat(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot)
{
    if (m_aControlPaths.getLength() == 0)
    {
        // Unterstuetzung des neuen Formats anzeigen
        m_aDataTypeList.Insert( SvDataType(Exchange::RegisterFormatName(SVX_FM_CONTROLS_AS_PATH)) );
    }
    m_aControlPaths.realloc(0);

    sal_Int32 nEntryCount = m_aDraggedEntries.size();
    if (nEntryCount == 0)
        return;

    m_aControlPaths.realloc(nEntryCount);
    ::com::sun::star::uno::Sequence<sal_uInt32>* pAllPaths = m_aControlPaths.getArray();
    for (sal_Int32 i=0; i<nEntryCount; ++i)
    {
        // erst mal sammeln wir den Pfad in einem Array ein
        SvULongs arrCurrentPath;
        SvLBoxEntry* pCurrentEntry = m_aDraggedEntries[i];

        SvLBoxEntry* pLoop = pCurrentEntry;
        while (pLoop != pRoot)
        {
            arrCurrentPath.Insert(pLoop->GetChildListPos(), arrCurrentPath.Count());
            pLoop = pTreeBox->GetParent(pLoop);
            DBG_ASSERT((pLoop != NULL) || (pRoot == 0), "SvxFmExplCtrlExch::BuildPathFormat : invalid root or entry !");
                // pLoop == NULL heisst, dass ich am oberen Ende angelangt bin, dann sollte das Ganze abbrechen, was nur bei pRoot == NULL der Fall sein wird
        }

        // dann koennen wir ihn in die ::com::sun::star::uno::Sequence uebertragen
        ::com::sun::star::uno::Sequence<sal_uInt32>& rCurrentPath = pAllPaths[i];
        sal_Int32 nDepth = arrCurrentPath.Count();

        rCurrentPath.realloc(nDepth);
        sal_uInt32* pSeq = rCurrentPath.getArray();
        sal_Int32 j,k;
        for (j = nDepth - 1, k = 0; k<nDepth; --j, ++k)
        {
            pSeq[j] = arrCurrentPath.GetObject(k);
        }
    }
}

//------------------------------------------------------------------------
void SvxFmExplCtrlExch::BuildListFromPath(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot)
{
    m_aDraggedEntries.clear();

    sal_Int32 nControls = m_aControlPaths.getLength();
    const ::com::sun::star::uno::Sequence<sal_uInt32>* pPaths = m_aControlPaths.getConstArray();
    for (sal_Int32 i=0; i<nControls; ++i)
    {
        sal_Int32 nThisPatLength = pPaths[i].getLength();
        const sal_uInt32* pThisPath = pPaths[i].getConstArray();
        SvLBoxEntry* pSearch = pRoot;
        for (sal_Int32 j=0; j<nThisPatLength; ++j)
            pSearch = pTreeBox->GetEntry(pSearch, pThisPath[j]);

        m_aDraggedEntries.push_back(pSearch);
    }
}

//------------------------------------------------------------------------
void SvxFmExplCtrlExch::AddHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqInterfaces)
{
    m_aDataTypeList.Insert( SvDataType(Exchange::RegisterFormatName(SVX_FM_HIDDEN_CONTROLS)) );
    m_seqControls = seqInterfaces;
}

//========================================================================
// class SvxFmFieldExch
//========================================================================

//------------------------------------------------------------------------
//SvxFmFieldExch::SvxFmFieldExch(const String& rFieldDesc)
//             :aFieldDesc(rFieldDesc)
//{
//  if (!nFieldFormat)
//      nFieldFormat = Exchange::RegisterFormatName(String::CreateFromAscii(SBA_FIELDEXCHANGE_FORMAT));
//  if(!nControlFormat)
//      nControlFormat = Exchange::RegisterSotFormatName(SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE);
//
//  aDataTypeList.Insert( *new SvDataType(nControlFormat) );
//  aDataTypeList.Insert( *new SvDataType(nFieldFormat) );
//}

//------------------------------------------------------------------------
//sal_Bool SvxFmFieldExch::GetData( SvData* pData )
//{
//  if (pData->GetFormat() == nFieldFormat || pData->GetFormat() == nControlFormat)
//  {
//      pData->SetData(aFieldDesc);
//      return sal_True;
//  }
//  return sal_False;
//}


