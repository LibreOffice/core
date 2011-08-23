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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "optutil.hxx"
#include "global.hxx"       // for pSysLocale

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <bf_svtools/syslocale.hxx>
#endif
namespace binfilter {

//------------------------------------------------------------------

// static
/*N*/ BOOL ScOptionsUtil::IsMetricSystem()
/*N*/ {
/*N*/ 	//!	which language should be used here - system language or installed office language?
/*N*/ 
/*N*/ //	MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
/*N*/     MeasurementSystem eSys = ScGlobal::pLocaleData->getMeasurementSystemEnum();
/*N*/ 
/*N*/ 	return ( eSys == MEASURE_METRIC );
/*N*/ }

//------------------------------------------------------------------

/*N*/ ScLinkConfigItem::ScLinkConfigItem( const ::rtl::OUString rSubTree ) :
/*N*/ 	ConfigItem( rSubTree )
/*N*/ {
/*N*/ }

/*N*/ void ScLinkConfigItem::SetCommitLink( const Link& rLink )
/*N*/ {
/*N*/ 	aCommitLink = rLink;
/*N*/ }

void  ScLinkConfigItem::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

void  ScLinkConfigItem::Commit()
{
}

}
