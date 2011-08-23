/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifndef SVX_LIGHT
#ifndef _SFX_OBJSH_HXX //autogen
#include <bf_sfx2/objsh.hxx>
#endif
#else
class SfxObjectShell;
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _FM_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _FM_PAGE_HXX
#include "fmpage.hxx"
#endif

#include "svdobj.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(FmFormModel, SdrModel);

struct FmFormModelImplData
{
    XubString				sNextPageId;
    sal_Bool				bOpenInDesignIsDefaulted;
    sal_Bool				bMovingPage;

    FmFormModelImplData()
        :bOpenInDesignIsDefaulted( sal_True )
        ,bMovingPage( sal_False )
    {
    }
};

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers)
/*N*/ 			:SdrModel(rPath, pPool, pPers)
/*N*/ 			,pObjShell(0)
/*N*/ 			,bStreamingOldVersion(sal_False)
/*N*/ 			,m_pImpl(NULL)
/*N*/ 			,m_bOpenInDesignMode(sal_False)
/*N*/ 			,m_bAutoControlFocus(sal_False)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	m_pImpl = new FmFormModelImplData;
/*N*/ 	m_pImpl->sNextPageId = '0';
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers,
/*N*/ 						 FASTBOOL bUseExtColorTable)
/*N*/ 			:SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
/*N*/ 			,pObjShell(0)
/*N*/ 			,bStreamingOldVersion(sal_False)
/*N*/ 			,m_bOpenInDesignMode(sal_False)
/*N*/ 			,m_bAutoControlFocus(sal_False)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	m_pImpl = new FmFormModelImplData;
/*N*/ 	m_pImpl->sNextPageId = '0';
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
/*N*/ FmFormModel::~FmFormModel()
/*N*/ {
/*N*/ 		SetObjectShell(NULL);
/*N*/ 	delete m_pImpl;
/*N*/ }

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/

/*************************************************************************
|*
|* Operator=
|*
\************************************************************************/

/*************************************************************************
|*
|* Operator==
|*
\************************************************************************/


/*************************************************************************
|*
|* Erzeugt eine neue Seite
|*
\************************************************************************/

/*************************************************************************
|*
|* WriteData
|*
\************************************************************************/

/*N*/ void FmFormModel::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 
/*N*/ 	if( rOut.GetVersion() < SOFFICE_FILEFORMAT_50 )
/*N*/ 		((FmFormModel*)this)->bStreamingOldVersion = sal_True;
/*N*/ 
/*N*/ 	SdrModel::WriteData( rOut );
/*N*/ 
/*N*/ 	//////////////////////////////////////////////////////////////////////
/*N*/ 	// Speichern der Option OpenInDesignMode
/*N*/ 	if (!bStreamingOldVersion)
/*N*/ 	{
/*N*/ 		SdrDownCompat aModelFormatCompat(rOut,STREAM_WRITE);
/*N*/ 
/*N*/ 		sal_uInt8 nTemp = m_bOpenInDesignMode;
/*N*/ 		rOut << nTemp;
/*N*/ 
/*N*/ 		nTemp = m_bAutoControlFocus;
/*N*/ 		rOut << nTemp;
/*N*/ 	}
/*N*/ 
/*N*/ 	((FmFormModel*)this)->bStreamingOldVersion = sal_False;
/*N*/ 
/*N*/ #endif
/*N*/ }


/*************************************************************************
|*
|* ReadData
|*
\************************************************************************/
/*N*/ void FmFormModel::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if( rIn.GetVersion() < SOFFICE_FILEFORMAT_50 )
/*N*/ 		((FmFormModel*)this)->bStreamingOldVersion = sal_True;
/*N*/ 
/*N*/ 	SdrModel::ReadData( rHead, rIn );
/*N*/ 
/*N*/ 	//////////////////////////////////////////////////////////////////////
/*N*/ 	// Lesen der Option OpenInDesignMode
/*N*/ 	if (!bStreamingOldVersion)
/*N*/ 	{
/*N*/ 		SdrDownCompat aCompat(rIn,STREAM_READ);
/*N*/ 		sal_uInt8 nTemp = 0;
/*N*/ 		rIn >> nTemp;
/*N*/ 
/*N*/ 		implSetOpenInDesignMode( nTemp ? sal_True : sal_False, sal_True );
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft())
/*N*/ 		{	// it is a version which already wrote the AutoControlFocus flag
/*N*/ 			rIn >> nTemp;
/*N*/ 			m_bAutoControlFocus = nTemp ? sal_True : sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	((FmFormModel*)this)->bStreamingOldVersion = sal_False;
/*N*/ }


/*************************************************************************
|*
|* InsertPage
|*
\************************************************************************/
/*N*/ void FmFormModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
/*N*/ {
/*?*/ 		SetObjectShell(pObjShell);
/*N*/ 
/*N*/ 	SdrModel::InsertPage( pPage, nPos );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( !m_pImpl->bMovingPage )
/*N*/ 	{
/*N*/ 		// this flag here is kind of a hack.
/*N*/ 		// When a page is moved, the SdrModel::MovePage calls an InsertPage only, but
/*N*/ 		// no preceding RemovePage. Thus, we (as a derivee) don't have a chance to see
/*N*/ 		// that the page which is just being inserted is (in real) already a part of the
/*N*/ 		// model. Especially, we do not have a change to notice that the UndoEnvironment
/*N*/ 		// already _knows_ the forms we're just going to add below.
/*N*/ 		// 
/*N*/ 		// The real solution to this would have been to fix SdrModel::MovePage, which
/*N*/ 		// is buggy in it's current form (as it violates the semantics of InsertPage, which
/*N*/ 		// is: insert a page which /currently is not part of any model/).
/*N*/ 		// However, this change in the SdrModel is much too risky.
/*N*/ 		//
/*N*/ 		// Another solution to this would have been to track (in the UndoEnv) which pages
/*N*/ 		// we know, and ignore any AddForms calls which are for such a page.
/*N*/ 		// But I refuse to do this (much more) work to hack a bug in the SdrModel.
/*N*/ 		// 
/*N*/ 		// I the decision is to do this "small hack" here (which I don't consider really
/*N*/ 		// bad).
/*N*/ 		//
/*N*/ 		// 2002-01-10 - #i3235# - fs@openoffice.org
/*N*/ 		//
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* MovePage
|*
\************************************************************************/
/*?*/ void FmFormModel::MovePage( USHORT nPgNum, USHORT nNewPos )
/*?*/ {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*?*/ }

/*************************************************************************
|*
|* RemovePage
|*
\************************************************************************/
/*N*/ SdrPage* FmFormModel::RemovePage(sal_uInt16 nPgNum)
/*N*/ {
/*N*/ 	FmFormPage* pPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);
/*N*/ 
/*N*/ 
/*N*/ 	return pPage;
/*N*/ }

/*************************************************************************
|*
|* InsertMasterPage
|*
\************************************************************************/
/*N*/ void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
/*N*/ {
/*N*/ 		SetObjectShell(pObjShell);
/*N*/ 
/*N*/ 	SdrModel::InsertMasterPage(pPage, nPos);
/*N*/ 
/*N*/ }

/*************************************************************************
|*
|* RemoveMasterPage
|*
\************************************************************************/
/*N*/ SdrPage* FmFormModel::RemoveMasterPage(sal_uInt16 nPgNum)
/*N*/ {
/*N*/ 	FmFormPage* pPage = (FmFormPage*)SdrModel::RemoveMasterPage(nPgNum);
/*N*/ 
/*N*/ 	return pPage;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ SdrLayerID FmFormModel::GetControlExportLayerId( const SdrObject& rObj ) const
/*N*/ {
/*N*/ 	return rObj.GetLayer();
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::implSetOpenInDesignMode( sal_Bool _bOpenDesignMode, sal_Bool _bForce )
/*N*/ {
/*N*/ 	if( ( _bOpenDesignMode != m_bOpenInDesignMode ) || _bForce )
/*N*/ 	{
/*N*/ 		m_bOpenInDesignMode = _bOpenDesignMode;
/*N*/ 
/*N*/ 		if ( pObjShell )
/*N*/ 			pObjShell->SetModified( sal_True );
/*N*/ 	}
/*N*/ 	// no matter if we really did it or not - from now on, it does not count as defaulted anymore
/*N*/ 	m_pImpl->bOpenInDesignIsDefaulted = sal_False;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetOpenInDesignMode( sal_Bool bOpenDesignMode )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	implSetOpenInDesignMode( bOpenDesignMode, sal_False );
/*N*/ #endif
/*N*/ }

#ifndef SVX_LIGHT
//------------------------------------------------------------------------
/*N*/ sal_Bool FmFormModel::OpenInDesignModeIsDefaulted( )
/*N*/ {
/*N*/ 	return m_pImpl->bOpenInDesignIsDefaulted;
/*N*/ }
#endif

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetAutoControlFocus( sal_Bool _bAutoControlFocus )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( _bAutoControlFocus != m_bAutoControlFocus )
/*N*/ 	{
/*?*/ 		m_bAutoControlFocus = _bAutoControlFocus;
/*?*/ 		pObjShell->SetModified( sal_True );
/*N*/ 	}
/*N*/ #endif
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
/*N*/ {
/*N*/ 	if (pShell == pObjShell)
/*N*/ 		return;
/*N*/ 	
/*N*/ 
/*N*/ 	pObjShell = pShell;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ XubString FmFormModel::GetUniquePageId()
/*N*/ {
/*N*/ 	XubString sReturn = m_pImpl->sNextPageId;
/*N*/ 
/*N*/ 	xub_Unicode aNextChar = m_pImpl->sNextPageId.GetChar(m_pImpl->sNextPageId.Len() - 1);
/*N*/ 	sal_Bool bNeedNewChar = sal_False;
/*N*/ 	switch (aNextChar)
/*N*/ 	{
/*N*/ 		case '9' : aNextChar = 'A'; break;
/*N*/ 		case 'Z' : aNextChar = 'a'; break;
/*N*/ 		case 'z' : aNextChar = '0'; bNeedNewChar = sal_True; break;
/*N*/ 		default: ++aNextChar; break;
/*N*/ 	}
/*N*/ 	m_pImpl->sNextPageId.SetChar(m_pImpl->sNextPageId.Len() - 1, aNextChar);
/*N*/ 	if (bNeedNewChar)
/*N*/ 		m_pImpl->sNextPageId += '0';
/*N*/ 
/*N*/ 	return sReturn;
/*N*/ }


}
