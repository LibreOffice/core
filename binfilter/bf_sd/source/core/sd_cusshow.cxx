/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "sdiocmpt.hxx"
#include "cusshow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

// #90477#
#include <tools/tenccvt.hxx>
namespace binfilter {

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc)
/*N*/   : List(),
/*N*/   pDoc(pDrawDoc)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/
/*N*/ SdCustomShow::SdCustomShow( const SdCustomShow& rShow )
/*N*/ 	: List( rShow )
/*N*/ {
/*N*/ 	aName = rShow.GetName();
/*N*/ 	pDoc = rShow.GetDoc();
/*N*/ }

/*N*/ SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow )
/*N*/   : List(),
/*N*/   pDoc(pDrawDoc),
/*N*/   mxUnoCustomShow( xShow )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
/*N*/ SdCustomShow::~SdCustomShow()
/*N*/ {
/*N*/ 	uno::Reference< uno::XInterface > xShow( mxUnoCustomShow );
/*N*/ 	uno::Reference< lang::XComponent > xComponent( xShow, uno::UNO_QUERY );
/*N*/ 	if( xComponent.is() )
/*N*/ 		xComponent->dispose();
/*N*/ }


/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/
/*N*/ SvStream& operator << (SvStream& rOut, const SdCustomShow& rCustomShow)
/*N*/ {
/*N*/ 	// Letzter Parameter ist die aktuelle Versionsnummer des Codes
/*N*/ 	SdIOCompat aIO(rOut, STREAM_WRITE, 0);
/*N*/ 
/*N*/ 	// Name
/*N*/ 	// #90477# rOut.WriteByteString( rCustomShow.aName, ::GetStoreCharSet( gsl_getSystemTextEncoding() ) );
/*N*/ 	rOut.WriteByteString(rCustomShow.aName, 
/*N*/ 		GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion()));
/*N*/ 
/*N*/ 	// Anzahl Seiten
/*N*/ 	UINT32 nCount = rCustomShow.Count();
/*N*/ 	rOut << nCount;
/*N*/ 
/*N*/ 	for (UINT32 i = 0; i < nCount; i++)
/*N*/ 	{
/*N*/ 		// Seite aus Liste holen
/*N*/ 		SdPage* pPage = (SdPage*) rCustomShow.GetObject(i);
/*N*/ 
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			// SdPage-Seitennummer
/*N*/ 			UINT16 nPageNum = (pPage->GetPageNum() - 1) / 2;
/*N*/ 			rOut << nPageNum;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/
/*N*/ SvStream& operator >> (SvStream& rIn, SdCustomShow& rCustomShow)
/*N*/ {
/*N*/ 	SdIOCompat aIO(rIn, STREAM_READ);
/*N*/ 
/*N*/ 	// Name
/*N*/ 	// #90477# rIn.ReadByteString( rCustomShow.aName, ::GetStoreCharSet( gsl_getSystemTextEncoding() ) );
/*N*/ 	rIn.ReadByteString(rCustomShow.aName, 
/*N*/ 		GetSOLoadTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rIn.GetVersion()));
/*N*/ 
/*N*/ 	// Anzahl Seiten
/*N*/ 	UINT32 nCount = 0;
/*N*/ 	rIn >> nCount;
/*N*/ 
/*N*/ 	rCustomShow.Clear();
/*N*/ 
/*N*/ 	for (UINT32 i = 0; i < nCount; i++)
/*N*/ 	{
/*N*/ 		// Seitennummer
/*N*/ 		UINT16 nPageNum;
/*N*/ 		rIn >> nPageNum;
/*N*/ 
/*N*/ 		// Seite in Liste einfuegen
/*N*/ 		SdPage* pPage = (SdPage*) rCustomShow.pDoc->GetSdPage(nPageNum, PK_STANDARD);
/*N*/ 		rCustomShow.Insert(pPage, LIST_APPEND);
/*N*/ 	}
/*N*/ 
/*N*/ 	return rIn;
/*N*/ }

/*N*/ extern uno::Reference< uno::XInterface > createUnoCustomShow( SdCustomShow* pShow );

/*N*/ uno::Reference< uno::XInterface > SdCustomShow::getUnoCustomShow()
/*N*/ {
/*N*/ 	// try weak reference first
/*N*/ 	uno::Reference< uno::XInterface > xShow( mxUnoCustomShow );
/*N*/ 
/*N*/ 	if( !xShow.is() )
/*N*/ 	{
/*N*/ 		xShow = createUnoCustomShow( this );
/*N*/ 	}
/*N*/ 
/*N*/ 	return xShow;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
