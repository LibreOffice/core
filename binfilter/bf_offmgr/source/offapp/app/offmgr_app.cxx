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

#include "app.hxx"

#include <bf_svx/dialdll.hxx>
#include "bf_basic/basrdll.hxx"
#include <bf_svx/editdll.hxx>
#include <bf_svx/itemdata.hxx>
#include <bf_svx/svdetc.hxx>
#include <bf_svx/svdoutl.hxx>
#include <bf_svx/svxerr.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <comphelper/processfactory.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svx/fmobjfac.hxx>
#include <bf_svx/siimport.hxx>
#define ITEMID_FIELD 0
#include <bf_svx/flditem.hxx>
#include <bf_svx/svdfield.hxx>
#include <bf_svx/objfac3d.hxx>
#include <bf_svx/unoshcol.hxx>
#include <bf_svx/xtable.hxx>
#include <bf_svtools/moduleoptions.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002

using namespace ::com::sun::star::uno;

namespace binfilter {

static SvxDialogDll*   pSvxDLL  = NULL;
static BasicDLL*       pBasic   = NULL;
static EditDLL*        pEditDLL = NULL;
static SvxGlobalItemData* pItemData = NULL;
static SvxErrorHandler* pSvxErrorHdl = NULL;

class OfficeData_Impl 
{
public:
    XColorTable*		pStdColorTable;
    ResMgr*				pResMgr;
    OfficeData_Impl() :
         pStdColorTable( NULL ),
        pResMgr(0)
    {}

    ~OfficeData_Impl()
    {
        delete pStdColorTable;
        delete pResMgr;
    }
};

/*N*/ OfficeApplication::OfficeApplication() :
/*N*/
/*N*/ 	SfxApplication(),
/*N*/ 	pDataImpl   ( NULL )
/*N*/
/*N*/ {
/*N*/     SetName( UniString(RTL_CONSTASCII_USTRINGPARAM("StarOffice") ) );
/*N*/
/*N*/     sal_uInt32 nFeatures = SvtModuleOptions().GetFeatures();
/*N*/ 	  if ( nFeatures )
/*N*/ 		SetFeatures( nFeatures );
/*N*/
/*N*/ 	  pDataImpl = new OfficeData_Impl;
/*N*/
/*N*/     pSvxDLL  = new SvxDialogDll;
/*N*/     pBasic   = new BasicDLL;
/*N*/     pEditDLL = new EditDLL;
/*N*/     pItemData = new SvxGlobalItemData;
/*N*/
/*N*/ 	  SfxApplication::SetApp( this );
/*N*/ 	  pSvxErrorHdl = new SvxErrorHandler();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ OfficeApplication::~OfficeApplication()
/*N*/ {
/*N*/     Deinitialize();
/*N*/ 	  delete pSvxErrorHdl;
/*N*/ 	  delete ( &GetSdrGlobalData() )->pOutliner;  // vor EditDLL, wegen Static-Defaults
/*N*/ 	  ( &GetSdrGlobalData() )->pOutliner = NULL;
/*N*/ 	  delete pEditDLL;
/*N*/ 	  delete pBasic;
/*N*/ 	  delete pSvxDLL;
/*N*/ 	  delete pDataImpl;
/*N*/     delete pItemData;
/*N*/ }

/*N*/ TYPEINIT1(OfficeApplication,SfxApplication);

/*N*/  Reference< XInterface > SAL_CALL SvxShapeCollection_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr ) throw( Exception )
/*N*/  {
/*?*/  	  return *( new SvxShapeCollection() );
/*N*/  }

/*N*/ void OfficeApplication::Init()
/*N*/ {
/*N*/   SfxApplication::Init();
/*N*/
/*N*/ 	SvClassManager& rClassManager = SvxFieldItem::GetClassManager();
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxFieldData );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxURLField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxDateField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxPageField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxTimeField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxExtTimeField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxExtFileField );
/*N*/ 	rClassManager.SV_CLASS_REGISTER( SvxAuthorField );
/*N*/
/*N*/     // SvDraw-Felder registrieren
/*N*/     SdrRegisterFieldClasses();
/*N*/
/*N*/     // 3D-Objekt-Factory eintragen
/*N*/     E3dObjFactory();
/*N*/
/*N*/     // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
/*N*/     FmFormObjFactory();
/*N*/
/*N*/     // factory for dummy import of old si-controls in 3.1 documents
/*N*/     SiImportFactory();
/*N*/
/*N*/ 	// Servies etc. registrieren
/*N*/ 	Reference< ::com::sun::star::lang::XMultiServiceFactory >  xSMgr = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	Reference< ::com::sun::star::container::XSet >  xSet( xSMgr, UNO_QUERY );
/*N*/ 	Sequence< ::rtl::OUString >			aName( 1 );
/*N*/ 	Reference< ::com::sun::star::lang::XSingleServiceFactory > 	xFact;
/*N*/
/*N*/ 	aName.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeCollection");
/*N*/ 	xFact = ::cppu::createSingleFactory( xSMgr, ::rtl::OUString::createFromAscii("ShapeCollection"), SvxShapeCollection_CreateInstance, aName );
/*N*/ 	xSet->insert( makeAny(xFact) );
/*N*/ }

/*N*/ XColorTable* OfficeApplication::GetStdColorTable()
/*N*/ {
/*N*/ 	if(!pDataImpl->pStdColorTable)
/*N*/ 		pDataImpl->pStdColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
/*N*/ 	return pDataImpl->pStdColorTable;
/*N*/ }

} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
