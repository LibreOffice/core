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



#ifndef SC_AREALINK_HXX
#define SC_AREALINK_HXX

#include "global.hxx"
#include "refreshtimer.hxx"
#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include "scdllapi.h"

class ScDocShell;
class SfxObjectShell;
class AbstractScLinkedAreaDlg;
struct AreaLink_Impl;

class SC_DLLPUBLIC ScAreaLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    AreaLink_Impl*  pImpl;
    String          aFileName;
    String          aFilterName;
    String          aOptions;
    String          aSourceArea;
    ScRange         aDestArea;
    sal_Bool            bAddUndo;
    sal_Bool            bInCreate;
    sal_Bool            bDoInsert;      // wird fuer das erste Update auf sal_False gesetzt

    sal_Bool        FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const String& rAreaName );

public:
    ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt,
                    const String& rArea, const ScRange& rDest, sal_uLong nRefresh );
    virtual ~ScAreaLink();

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    sal_Bool    Refresh( const String& rNewFile, const String& rNewFilter,
                    const String& rNewArea, sal_uLong nNewRefresh );

    void    SetInCreate(sal_Bool bSet)                  { bInCreate = bSet; }
    void    SetDoInsert(sal_Bool bSet)                  { bDoInsert = bSet; }
    void    SetDestArea(const ScRange& rNew);
    void    SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rArea);

    sal_Bool    IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
                        const String& rSource, const ScRange& rDest ) const;

    const String&   GetFile() const         { return aFileName;     }
    const String&   GetFilter() const       { return aFilterName;   }
    const String&   GetOptions() const      { return aOptions;      }
    const String&   GetSource() const       { return aSourceArea;   }
    const ScRange&  GetDestArea() const     { return aDestArea;     }

    DECL_LINK( RefreshHdl, ScAreaLink* );
    DECL_LINK( AreaEndEditHdl, void* );
};

#endif

