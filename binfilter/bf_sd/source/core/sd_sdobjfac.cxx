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


#include "sdpage.hxx"
#include "sdobjfac.hxx"
#include "anminfo.hxx"
#include "imapinfo.hxx"
namespace binfilter {


SdObjectFactory aSdObjectFactory;

/*************************************************************************
|*
|* void SdObjectFactory::MakeUserData()
|*
\************************************************************************/

/*N*/ IMPL_LINK( SdObjectFactory, MakeUserData, SdrObjFactory *, pObjFactory )
/*N*/ {
/*N*/ 	if ( pObjFactory->nInventor == SdUDInventor )
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pObjFactory->pObj;
/*N*/ 
/*N*/ 		switch( pObjFactory->nIdentifier )
/*N*/ 		{
/*N*/ 			case( SD_ANIMATIONINFO_ID ):
/*N*/ 			{
/*N*/ 				SdDrawDocument* pDoc = NULL;
/*N*/ 
/*N*/ 				if ( pObj )
/*N*/ 					pDoc = (SdDrawDocument*) pObj->GetModel();
/*N*/ 
/*N*/ 				DBG_ASSERT(pDoc, "kein Model gefunden");
/*N*/ 
/*N*/ 				pObjFactory->pNewData = new SdAnimationInfo(pDoc);
/*N*/ 			}
/*N*/ 			break;
/*?*/ 
/*?*/ 			case( SD_IMAPINFO_ID ):
/*?*/ 				pObjFactory->pNewData = new SdIMapInfo;
/*?*/ 			break;
/*N*/ 
/*N*/ 			default:
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pObjFactory->pNewData )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	if( aOldMakeUserDataLink.IsSet() )
/*?*/ 		aOldMakeUserDataLink.Call( this );
/*N*/ 
/*N*/ 	return 0;
/*N*/ }


}
