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





#ifndef _FM_FMOBJFAC_HXX
#include "fmobjfac.hxx"
#endif

#ifndef _FM_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _FM_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif

#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif


#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif


#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif



#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif



#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::svxform;//STRIP008 using namespace ::svxform;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ FmFormObjFactory::FmFormObjFactory()
/*N*/ {
/*N*/ 	SdrObjFactory::InsertMakeObjectHdl(LINK(this, FmFormObjFactory, MakeObject));
/*N*/ 
/*N*/ 	// ImplSmartRegisterUnoServices();
/*N*/ 
/*N*/ }


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
/*N*/ FmFormObjFactory::~FmFormObjFactory()
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|* ::com::sun::star::form::Form-Objekte erzeugen
|*
\************************************************************************/

/*N*/ IMPL_LINK(FmFormObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
/*N*/ {
/*N*/ 	if (pObjFactory->nInventor == FmFormInventor)
/*N*/ 	{
/*N*/ 		switch (pObjFactory->nIdentifier)
/*N*/ 		{
/*N*/ 			case OBJ_FM_CONTROL:	// allgemeines Object
/*N*/ 			{
/*N*/ 				pObjFactory->pNewObj = new FmFormObj(pObjFactory->nIdentifier);
/*N*/ 			}	break;
/*?*/ 			default:
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 return 0;
/*?*/ 		}
/*?*/ 	}
/*N*/ 
/*N*/ 	return 0;
/*N*/ }



}
