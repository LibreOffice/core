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
#pragma once

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <unordered_map>

/*-************************************************************************************************************
    @short          collect information about security features
    @ATTENTION      This class is partially threadsafe.

    @devstatus      ready to use
*//*-*************************************************************************************************************/

namespace SvtSecurityOptions
{
    enum class EOption
    {
        SecureUrls,
        DocWarnSaveOrSend,
        DocWarnSigning,
        DocWarnPrint,
        DocWarnCreatePdf,
        DocWarnRemovePersonalInfo,
        DocWarnKeepRedlineInfo,
        DocWarnKeepDocUserInfo,
        DocWarnKeepNoteAuthorDateInfo,
        DocWarnKeepDocVersionInfo,
        DocKeepPrinterSettings,
        DocWarnRecommendPassword,
        MacroSecLevel,
        MacroTrustedAuthors,
        CtrlClickHyperlink,
        BlockUntrustedRefererLinks,
        DisableActiveContent
    };

    struct Certificate
    {
        OUString SubjectName;
        OUString SerialNumber;
        OUString RawData;

        bool operator==(const Certificate& other) const
        {
            return SubjectName == other.SubjectName && SerialNumber == other.SerialNumber && RawData == other.RawData;
        }
    };


    /*-****************************************************************************************************
        @short      returns readonly state
        @descr      It can be called to get information about the readonly state of a provided item.
        @param      "eOption", specify, which item is queried
        @return     <TRUE/> if item is readonly; <FALSE/> otherwise

        @onerror    No error should occur!
    *//*-*****************************************************************************************************/

    UNOTOOLS_DLLPUBLIC bool IsReadOnly( EOption eOption );

    /*-****************************************************************************************************
        @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Security/Scripting/SecureURL"
        @descr      These value displays the list of all trustworthy URLs.
                    zB.:    file:/                  => All scripts from the local file system including a LAN;
                            private:explorer        => Scripts from the Explorer;
                            private:help            => Scripts in the help system;
                            private:newmenu         => Scripts that are executed by the commands File-New and AutoPilot;
                            private:schedule        => Scripts of  the scheduler;
                            private:searchfolder    => Scripts of the searchfolder;
                            private:user            => Scripts that are entered in the URL field.
        @param      "seqURLList", new values to set it in configuration.
        @return     The values which represent current state of internal variable.

        @onerror    No error should occur!
    *//*-*****************************************************************************************************/

    UNOTOOLS_DLLPUBLIC std::vector< OUString >  GetSecureURLs();
    UNOTOOLS_DLLPUBLIC void SetSecureURLs( std::vector< OUString >&& seqURLList );

    /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Security/Scripting/MacroSecurityLevel"
            @descr      These value determines how StarOffice Basic scripts should be handled.
                        It exist 3 different modes:
                            0 = never execute
                            1 = from list
                            2 = always execute

            @ATTENTION  These methods don't check for valid or invalid values!
                        Our configuration server can do it ... but these implementation don't get any notifications
                        about wrong commits ...!
                        => If you set an invalid value - nothing will be changed. The information will lost.

            @seealso    enum EBasicSecurityMode

            @param      "eMode" to set new mode ... Value must defined as an enum of type EBasicSecurityMode!
            @return     An enum value, which present current mode.

            @onerror    No error should occur!
        *//*-*****************************************************************************************************/

    UNOTOOLS_DLLPUBLIC sal_Int32 GetMacroSecurityLevel();
    UNOTOOLS_DLLPUBLIC void SetMacroSecurityLevel( sal_Int32 _nLevel );

    UNOTOOLS_DLLPUBLIC bool IsMacroDisabled();

    /**
       Check whether the given uri is either no dangerous macro-execution
       URI at all or else the given referer is a trusted source.
    */
    UNOTOOLS_DLLPUBLIC bool isSecureMacroUri(OUString const & uri, OUString const & referer);

    /**
       Check whether the given referer URI is untrusted, and links
       originating from it should not be accessed.
     */
    UNOTOOLS_DLLPUBLIC bool isUntrustedReferer(OUString const & referer);

    /**
       Check whether the given uri is a trusted location.
    */
    UNOTOOLS_DLLPUBLIC bool isTrustedLocationUri(OUString const & uri);

    UNOTOOLS_DLLPUBLIC bool isTrustedLocationUriForUpdatingLinks(OUString const & uri);

    UNOTOOLS_DLLPUBLIC std::vector< Certificate > GetTrustedAuthors();
    UNOTOOLS_DLLPUBLIC void SetTrustedAuthors( const std::vector< Certificate >& rAuthors );

    // for bool options only!
    UNOTOOLS_DLLPUBLIC bool        IsOptionSet     ( EOption eOption                   );
    UNOTOOLS_DLLPUBLIC void        SetOption       ( EOption eOption, bool bValue      );

} // namespace SvtSecurityOptions

// map personal info strings, e.g. authors to 1, 2, 3... for removing personal info

class UNOTOOLS_DLLPUBLIC SvtSecurityMapPersonalInfo
{
    std::unordered_map< OUString, size_t > aInfoIDs;
public:
    size_t GetInfoID( const OUString sPersonalInfo );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
