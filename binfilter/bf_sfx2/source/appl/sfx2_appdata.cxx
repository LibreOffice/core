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
#include <tools/config.hxx>

#define _SVSTDARR_STRINGS
#include <bf_svtools/svstdarr.hxx>

#include "app.hxx"
#include "appdata.hxx"
#include "doctempl.hxx"
#include "docfile.hxx"
#include "imestatuswindow.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

/*N*/ SfxAppData_Impl::SfxAppData_Impl( SfxApplication* pApp ) :
/*N*/ 		pProgress(0),
/*N*/ 		pPool(0),
/*N*/ 		pEventConfig(0),
/*N*/ 		nBasicCallLevel(0),
/*N*/ 		nRescheduleLocks(0),
/*N*/ 		nInReschedule(0),
/*N*/ 		pInitLinkList(0),
            pSfxFrameObjectFactoryPtr(0),
            pThisDocument(0),
/*N*/ 		pMatcher( 0 ),
/*N*/ 		pCancelMgr( 0 ),
/*N*/ 		pMiscConfig(0),
/*N*/ 		bInQuit(sal_False),
/*N*/ 		bInException( sal_False ),
/*N*/       m_xImeStatusWindow(new sfx2::appl::ImeStatusWindow(
/*N*/                                *pApp, ::legacy_binfilters::getLegacyProcessServiceFactory()))
/*N*/ {
/*N*/ }

/*N*/ SfxAppData_Impl::~SfxAppData_Impl()
/*N*/ {
/*N*/ 	delete pCancelMgr;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
