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


#include "precompiled_rptui.hxx"
#include "GroupExchange.hxx"
#include <sot/formats.hxx>
#include <sot/storage.hxx>

namespace rptui
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    sal_uInt32 OGroupExchange::getReportGroupId()
    {
        static sal_uInt32 s_nReportFormat = (sal_uInt32)-1;
        if ( (sal_uInt32)-1 == s_nReportFormat )
        {
            s_nReportFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"reportdesign.GroupFormat\"" ));
            OSL_ENSURE((sal_uInt32)-1 != s_nReportFormat, "Bad exchange id!");
        }
        return s_nReportFormat;
    }
    OGroupExchange::OGroupExchange(const uno::Sequence< uno::Any >& _aGroupRow)
        : m_aGroupRow(_aGroupRow)
    {
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::AddSupportedFormats()
    {
        if ( m_aGroupRow.getLength() )
        {
            AddFormat(OGroupExchange::getReportGroupId());
        }
    }
    // -----------------------------------------------------------------------------
    sal_Bool OGroupExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uLong nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == OGroupExchange::getReportGroupId() )
        {
            return SetAny(uno::makeAny(m_aGroupRow),rFlavor);
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::ObjectReleased()
    {
        m_aGroupRow.realloc(0);
    }
    // -----------------------------------------------------------------------------
}
