/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optutil.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:37:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <vcl/svapp.hxx>

#include "optutil.hxx"
#include "global.hxx"       // for pSysLocale

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif

//------------------------------------------------------------------

// static
BOOL ScOptionsUtil::IsMetricSystem()
{
    //! which language should be used here - system language or installed office language?

//  MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    MeasurementSystem eSys = ScGlobal::pLocaleData->getMeasurementSystemEnum();

    return ( eSys == MEASURE_METRIC );
}

//------------------------------------------------------------------

ScLinkConfigItem::ScLinkConfigItem( const rtl::OUString rSubTree ) :
    ConfigItem( rSubTree )
{
}

void ScLinkConfigItem::SetCommitLink( const Link& rLink )
{
    aCommitLink = rLink;
}

void ScLinkConfigItem::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames )
{
    //! not implemented yet...
}

void ScLinkConfigItem::Commit()
{
    aCommitLink.Call( this );
}


