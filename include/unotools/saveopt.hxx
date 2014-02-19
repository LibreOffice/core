/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_UNOTOOLS_SAVEOPT_HXX
#define INCLUDED_UNOTOOLS_SAVEOPT_HXX

#include <unotools/unotoolsdllapi.h>
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
        E_USEBLOWFISHINODF12,
        E_USERAUTOSAVE,
    };

    // keep enum values sorted that a less or greater compare maps to older and newer versions!
    enum ODFDefaultVersion
    {
        ODFVER_UNKNOWN = 0, // unknown
        ODFVER_010 = 1,         // ODF 1.0
        ODFVER_011 = 2,         // ODF 1.1
        DO_NOT_USE = 3,         // Do not use this, only here for compatibility with pre OOo 3.2 configuration
        ODFVER_012 = 4,         // ODF 1.2
        ODFVER_012_EXT_COMPAT = 8, // ODF 1.2 extended, but with compatibility fallbacks

        ODFVER_LATEST = SAL_MAX_ENUM,      // ODF latest version with enhancements
    };

    SvtSaveOptions();
    virtual ~SvtSaveOptions();

    void                    SetAutoSaveTime( sal_Int32 n );
    sal_Int32               GetAutoSaveTime() const;

    void                    SetUseUserData( bool b );
    bool                IsUseUserData() const;

    void                    SetBackup( bool b );
    bool                IsBackup() const;

    void                    SetAutoSave( bool b );
    bool                IsAutoSave() const;

    void                    SetAutoSavePrompt( bool b );
    bool                IsAutoSavePrompt() const;

    void                    SetUserAutoSave( bool b );
    bool                IsUserAutoSave() const;

    void                    SetDocInfoSave(bool b);
    bool                IsDocInfoSave() const;

    void                    SetSaveWorkingSet( bool b );
    bool                IsSaveWorkingSet() const;

    void                    SetSaveDocView( bool b );
    bool                IsSaveDocView() const;

    void                    SetSaveRelINet( bool b );
    bool                IsSaveRelINet() const;

    void                    SetSaveRelFSys( bool b );
    bool                IsSaveRelFSys() const;

    bool                IsSaveUnpacked() const;

    void                    SetLoadUserSettings(bool b);
    bool                IsLoadUserSettings() const;

    void                    SetPrettyPrinting( bool _bEnable );
    bool                IsPrettyPrinting( ) const;

    void                    SetWarnAlienFormat( bool _bEnable );
    bool                IsWarnAlienFormat( ) const;

    void                    SetLoadDocumentPrinter( bool _bEnable );
    bool                IsLoadDocumentPrinter( ) const;

    void                    SetODFDefaultVersion( ODFDefaultVersion eVersion );
    ODFDefaultVersion       GetODFDefaultVersion() const;

    bool                IsUseSHA1InODF12() const;

    bool                IsUseBlowfishInODF12() const;

    bool                IsReadOnly( EOption eOption ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
