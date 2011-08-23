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

#include "axisid.hxx"
#include "schgroup.hxx"
#include "objfac.hxx"
#include "objid.hxx"
#include "objadj.hxx"
#include "datarow.hxx"
#include "datapoin.hxx"
#include "glob.hxx"
namespace binfilter {


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjFactory::SchObjFactory() :
/*N*/ 	bInserted(FALSE)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Chart-interne Objekte erzeugen
|*
\************************************************************************/

/*N*/ IMPL_LINK( SchObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
/*N*/ {
/*N*/ 	if (pObjFactory->nInventor == SchInventor &&
/*N*/ 		pObjFactory->nIdentifier == SCH_OBJGROUP_ID)
/*N*/ 		pObjFactory->pNewObj = new SchObjGroup;
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
|*
|* Chart-Userdata erzeugen
|*
\************************************************************************/

/*N*/ IMPL_LINK( SchObjFactory, MakeUserData, SdrObjFactory* ,pObjFactory)
/*N*/ {
/*N*/ 	if (pObjFactory->nInventor == SchInventor)
/*N*/ 		switch (pObjFactory->nIdentifier)
/*N*/ 		{
/*N*/ 			case SCH_OBJECTID_ID:
/*N*/ 				pObjFactory->pNewData = new SchObjectId;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SCH_OBJECTADJUST_ID:
/*N*/ 				pObjFactory->pNewData = new SchObjectAdjust;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SCH_DATAROW_ID:
/*N*/ 				pObjFactory->pNewData = new SchDataRow;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SCH_DATAPOINT_ID:
/*N*/ 				pObjFactory->pNewData = new SchDataPoint;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SCH_LIGHTFACTOR_ID:
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pObjFactory->pNewData = new SchLightFactor;
/*?*/ 				DBG_ERROR("SCH_LIGHTFACTOR_ID no longer available");
/*?*/ 				break;
/*N*/ 
/*N*/ 			case SCH_AXIS_ID :
/*N*/ 				pObjFactory->pNewData = new SchAxisId;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 		return 0;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
