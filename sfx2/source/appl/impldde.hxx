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


#ifndef _IMPLDDE_HXX
#define _IMPLDDE_HXX

#include <sfx2/linksrc.hxx>
#include <tools/string.hxx>

class DdeConnection;
class DdeData;
class DdeLink;
class DdeRequest;
class DdeTransaction;

namespace sfx2
{

class SvDDEObject : public SvLinkSource
{
    String sItem;

    DdeConnection* pConnection;
    DdeLink* pLink;
    DdeRequest* pRequest;
    ::com::sun::star::uno::Any * pGetData;

    sal_uInt8 bWaitForData : 1;                     // wird auf Daten gewartet?
    sal_uInt8 nError        : 7;                    // Error Code fuer den Dialog


    sal_Bool ImplHasOtherFormat( DdeTransaction& );
    DECL_LINK( ImplGetDDEData, DdeData* );
    DECL_LINK( ImplDoneDDEData, void* );

protected:
    virtual ~SvDDEObject();

public:
    SvDDEObject();

    virtual sal_Bool    GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & aMimeType,
                                sal_Bool bSynchron = sal_False );

    virtual sal_Bool    Connect( SvBaseLink * );
    virtual void    Edit( Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl );

    virtual sal_Bool    IsPending() const;
    virtual sal_Bool    IsDataComplete() const;
};

}

#endif
