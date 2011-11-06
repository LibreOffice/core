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
#include "precompiled_sw.hxx"


#include <dbconfig.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <swdbdata.hxx>

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

/*--------------------------------------------------------------------
     Beschreibung: Ctor
 --------------------------------------------------------------------*/

const Sequence<OUString>& SwDBConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "AddressBook/DataSourceName",        //  0
            "AddressBook/Command",              //  1
            "AddressBook/CommandType",          //  2
            "Bibliography/CurrentDataSource/DataSourceName",        //  4
            "Bibliography/CurrentDataSource/Command",              //  5
            "Bibliography/CurrentDataSource/CommandType"          //  6
        };
        const int nCount = sizeof(aPropNames)/sizeof(const char*);
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/* -----------------------------06.09.00 16:44--------------------------------

 ---------------------------------------------------------------------------*/
SwDBConfig::SwDBConfig() :
    ConfigItem(C2U("Office.DataAccess"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    pAdrImpl(0),
    pBibImpl(0)
{
};
/* -----------------------------06.09.00 16:50--------------------------------

 ---------------------------------------------------------------------------*/
SwDBConfig::~SwDBConfig()
{
    delete pAdrImpl;
    delete pBibImpl;
}
/* -----------------------------20.02.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
void SwDBConfig::Load()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    if(!pAdrImpl)
    {

        pAdrImpl = new SwDBData;
        pAdrImpl->nCommandType = 0;
        pBibImpl = new SwDBData;
        pBibImpl->nCommandType = 0;
    }
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == rNames.getLength())
    {
        for(int nProp = 0; nProp < rNames.getLength(); nProp++)
        {
            switch(nProp)
            {
                case  0: pValues[nProp] >>= pAdrImpl->sDataSource;  break;
                case  1: pValues[nProp] >>= pAdrImpl->sCommand;     break;
                case  2: pValues[nProp] >>= pAdrImpl->nCommandType; break;
                case  3: pValues[nProp] >>= pBibImpl->sDataSource;  break;
                case  4: pValues[nProp] >>= pBibImpl->sCommand;     break;
                case  5: pValues[nProp] >>= pBibImpl->nCommandType; break;
            }
        }
    }
}
/* -----------------------------20.02.01 12:36--------------------------------

 ---------------------------------------------------------------------------*/
const SwDBData& SwDBConfig::GetAddressSource()
{
    if(!pAdrImpl)
        Load();
    return *pAdrImpl;
}
/* -----------------29.11.2002 11:43-----------------
 *
 * --------------------------------------------------*/
const SwDBData& SwDBConfig::GetBibliographySource()
{
    if(!pBibImpl)
        Load();
    return *pBibImpl;
}

void SwDBConfig::Commit() {}
void SwDBConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}


