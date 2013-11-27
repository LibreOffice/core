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
#include <editsh.hxx>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <doc.hxx>
#include <docary.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <edimp.hxx>
#include <flddat.hxx>
#include <switerator.hxx>

using namespace com::sun::star;
using ::rtl::OUString;

sal_Bool SwEditShell::IsFieldDataSourceAvailable(String& rUsedDataSource) const
{
    const SwFldTypes * pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();
    uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( !xMgr.is() )
        return sal_False;
    uno::Reference<uno::XInterface> xInstance = xMgr->createInstance( OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ));
    uno::Reference<container::XNameAccess> xDBContext(xInstance, uno::UNO_QUERY) ;
    if(!xDBContext.is())
        return sal_False;
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        sal_uInt16 nWhich = rFldType.Which();
        if(IsUsed(rFldType))
        {
            switch(nWhich)
            {
                case RES_DBFLD:
                {
                    SwIterator<SwFmtFld,SwFieldType> aIter( rFldType );
                    SwFmtFld* pFmtFld = aIter.First();
                    while(pFmtFld)
                    {
                        if(pFmtFld->IsFldInDoc())
                        {
                            const SwDBData& rData =
                                    ((SwDBFieldType*)pFmtFld->GetField()->GetTyp())->GetDBData();
                            try
                            {
                                return xDBContext->getByName(rData.sDataSource).hasValue();
                            }
                            catch(uno::Exception const &)
                            {
                                rUsedDataSource = rData.sDataSource;
                                return sal_False;
                            }
                        }
                        pFmtFld = aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return sal_True;
}


