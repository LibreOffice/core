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

#include <vcl/status.hxx>
#include <vcl/msgbox.hxx>
#include <bf_svtools/whiter.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/filter.hxx>
#include <bf_svtools/internaloptions.hxx>
#include <bf_svtools/pathoptions.hxx>

#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/Reference.h>
#include <tools/config.hxx>
#include <tools/rcid.h>
#include <osl/mutex.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif
#include <unotools/configmgr.hxx>
#include <com/sun/star/frame/XDesktop.hpp>

#include <unotools/ucbstreamhelper.hxx>
#include <framework/menuconfiguration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/bootstrap.hxx>
#include <bf_svtools/moduleoptions.hxx>
#include <osl/file.hxx>

#include "app.hxx"
#include "appdata.hxx"
#include "cfgmgr.hxx"

#include <bf_svtools/imgdef.hxx>

#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "request.hxx"
#include "fcontnr.hxx"
#include "sfx.hrc"
#include "module.hxx"
#include "openflag.hxx"
#include "appimp.hxx"
#include "helper.hxx"	// SfxContentHelper::Kill()

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//===================================================================
namespace binfilter {


/*N*/ SfxProgress* SfxApplication::GetProgress() const
/*N*/ {
/*N*/     return pAppData_Impl->pProgress;
/*N*/ }

/*N*/ SfxCancelManager *SfxApplication::GetCancelManager() const
/*N*/ {
/*N*/ 	if ( !pAppData_Impl->pCancelMgr )
/*N*/ 	{
/*N*/ 		pAppData_Impl->pCancelMgr = new SfxCancelManager;
/*N*/ 	}
/*N*/ 	return pAppData_Impl->pCancelMgr;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
