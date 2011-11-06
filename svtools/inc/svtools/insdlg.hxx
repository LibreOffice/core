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


#ifndef _SVTOOLS_INSDLG_HXX
#define _SVTOOLS_INSDLG_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <tools/globname.hxx>
#include <sot/formats.hxx>

#ifndef _OWNCONT_HXX
#include <svl/ownlist.hxx>
#endif
#include <svtools/transfer.hxx>

class SvObjectServer
{
private:
    SvGlobalName    aClassName;
    String          aHumanName;

public:
    SvObjectServer( const SvGlobalName & rClassP, const String & rHumanP ) :
        aClassName( rClassP ),
        aHumanName( rHumanP ) {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetHumanName() const { return aHumanName; }
};

class SVT_DLLPUBLIC SvObjectServerList
{
    PRV_SV_DECL_OWNER_LIST(SvObjectServerList,SvObjectServer)
    const SvObjectServer *  Get( const String & rHumanName ) const;
    const SvObjectServer *  Get( const SvGlobalName & ) const;
    void                    Remove( const SvGlobalName & );
    void                    FillInsertObjects();
};

class SVT_DLLPUBLIC SvPasteObjectHelper
{
public:
    static String GetSotFormatUIName( SotFormatStringId nId );
    static sal_Bool GetEmbeddedName(const TransferableDataHelper& rData, String& _rName, String& _rSource, SotFormatStringId& _nFormat);
};

#endif // _SVTOOLS_INSDLG_HXX

