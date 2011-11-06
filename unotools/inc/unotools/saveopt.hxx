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


#ifndef INCLUDED_unotools_SAVEOPT_HXX
#define INCLUDED_unotools_SAVEOPT_HXX

#include "unotools/unotoolsdllapi.h"
#include <unotools/options.hxx>

struct SvtLoadSaveOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtSaveOptions: public utl::detail::Options
{
    SvtLoadSaveOptions_Impl*    pImp;

public:

    enum EOption
    {
        E_AUTOSAVETIME,
        E_USEUSERDATA,
        E_BACKUP,
        E_AUTOSAVE,
        E_AUTOSAVEPROMPT,
        E_DOCINFSAVE,
        E_SAVEWORKINGSET,
        E_SAVEDOCVIEW,
        E_SAVERELINET,
        E_SAVERELFSYS,
        E_SAVEUNPACKED,
        E_DOPRETTYPRINTING,
        E_WARNALIENFORMAT,
        E_LOADDOCPRINTER,
        E_ODFDEFAULTVERSION,
        E_USESHA1INODF12,
        E_USEBLOWFISHINODF12
    };

    // keep enum values sorted that a less or greater compare maps to older and newer versions!
    enum ODFDefaultVersion
    {
        ODFVER_UNKNOWN = 0, // unknown
        ODFVER_010 = 1,         // ODF 1.0
        ODFVER_011 = 2,         // ODF 1.1
        DO_NOT_USE = 3,         // Do not use this, only here for compatibility with pre OOo 3.2 configuration
        ODFVER_012 = 4,         // ODF 1.2

        ODFVER_LATEST = SAL_MAX_ENUM,      // ODF latest version with enhancements
    };

    SvtSaveOptions();
    virtual ~SvtSaveOptions();

    void                    SetAutoSaveTime( sal_Int32 n );
    sal_Int32               GetAutoSaveTime() const;

    void                    SetUseUserData( sal_Bool b );
    sal_Bool                IsUseUserData() const;

    void                    SetBackup( sal_Bool b );
    sal_Bool                IsBackup() const;

    void                    SetAutoSave( sal_Bool b );
    sal_Bool                IsAutoSave() const;

    void                    SetAutoSavePrompt( sal_Bool b );
    sal_Bool                IsAutoSavePrompt() const;

    void                    SetDocInfoSave(sal_Bool b);
    sal_Bool                IsDocInfoSave() const;

    void                    SetSaveWorkingSet( sal_Bool b );
    sal_Bool                IsSaveWorkingSet() const;

    void                    SetSaveDocView( sal_Bool b );
    sal_Bool                IsSaveDocView() const;

    void                    SetSaveRelINet( sal_Bool b );
    sal_Bool                IsSaveRelINet() const;

    void                    SetSaveRelFSys( sal_Bool b );
    sal_Bool                IsSaveRelFSys() const;

    void                    SetSaveUnpacked( sal_Bool b );
    sal_Bool                IsSaveUnpacked() const;

    void                    SetLoadUserSettings(sal_Bool b);
    sal_Bool                IsLoadUserSettings() const;

    void                    SetPrettyPrinting( sal_Bool _bEnable );
    sal_Bool                IsPrettyPrinting( ) const;

    void                    SetWarnAlienFormat( sal_Bool _bEnable );
    sal_Bool                IsWarnAlienFormat( ) const;

    void                    SetLoadDocumentPrinter( sal_Bool _bEnable );
    sal_Bool                IsLoadDocumentPrinter( ) const;

    void                    SetODFDefaultVersion( ODFDefaultVersion eVersion );
    ODFDefaultVersion       GetODFDefaultVersion() const;

    void                    SetUseSHA1InODF12( sal_Bool bUse );
    sal_Bool                IsUseSHA1InODF12() const;

    void                    SetUseBlowfishInODF12( sal_Bool bUse );
    sal_Bool                IsUseBlowfishInODF12() const;

    sal_Bool                IsReadOnly( EOption eOption ) const;
};

#endif

