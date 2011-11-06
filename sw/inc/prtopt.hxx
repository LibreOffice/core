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


#ifndef _PRTOPT_HXX
#define _PRTOPT_HXX

#include <unotools/configitem.hxx>
#include <printdata.hxx>

class SwPrintOptions : public SwPrintData, public utl::ConfigItem
{
    sal_Bool            bIsWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwPrintOptions(sal_Bool bWeb);
    virtual ~SwPrintOptions();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            doSetModified( ) { bModified = sal_True; SetModified();}

    SwPrintOptions& operator=(const SwPrintData& rData)
    {
        SwPrintData::operator=( rData );
        SetModified();
        return *this;
    }
};

#endif

