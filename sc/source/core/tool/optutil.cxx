/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <vcl/svapp.hxx>

#include "optutil.hxx"
#include "global.hxx"       // for pSysLocale
#include <unotools/syslocale.hxx>

//------------------------------------------------------------------

// static
sal_Bool ScOptionsUtil::IsMetricSystem()
{
    //! which language should be used here - system language or installed office language?

//  MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    MeasurementSystem eSys = ScGlobal::pLocaleData->getMeasurementSystemEnum();

    return ( eSys == MEASURE_METRIC );
}

//------------------------------------------------------------------

ScLinkConfigItem::ScLinkConfigItem( const rtl::OUString& rSubTree ) :
    ConfigItem( rSubTree )
{
}

ScLinkConfigItem::ScLinkConfigItem( const rtl::OUString& rSubTree, sal_Int16 nMode ) :
    ConfigItem( rSubTree, nMode )
{
}

void ScLinkConfigItem::SetCommitLink( const Link& rLink )
{
    aCommitLink = rLink;
}

void ScLinkConfigItem::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& /* aPropertyNames */ )
{
    //! not implemented yet...
}

void ScLinkConfigItem::Commit()
{
    aCommitLink.Call( this );
}


