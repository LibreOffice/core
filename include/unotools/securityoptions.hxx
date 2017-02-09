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
#ifndef INCLUDED_UNOTOOLS_SECURITYOPTIONS_HXX
#define INCLUDED_UNOTOOLS_SECURITYOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtSecurityOptions_Impl;

/*-************************************************************************************************************
    @descr          These values present modes to handle StarOffice basic scripts.
                    see GetBasicMode/SetBasicMode() for further information
*//*-*************************************************************************************************************/

enum EBasicSecurityMode
{
    eNEVER_EXECUTE  = 0,
    eFROM_LIST      = 1,
    eALWAYS_EXECUTE = 2
};

/*-************************************************************************************************************
    @short          collect information about security features
    @ATTENTION      This class is partially threadsafe.

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtSecurityOptions : public utl::detail::Options
{
    public:

        enum class EOption
        {
            SecureUrls,
            BasicMode,                    // xmlsec05 deprecated
            ExecutePlugins,               // xmlsec05 deprecated
            Warning,                      // xmlsec05 deprecated
            Confirmation,                 // xmlsec05 deprecated
            DocWarnSaveOrSend,
            DocWarnSigning,
            DocWarnPrint,
            DocWarnCreatePdf,
            DocWarnRemovePersonalInfo,
            DocWarnRecommendPassword,
            MacroSecLevel,
            MacroTrustedAuthors,
            CtrlClickHyperlink,
            BlockUntrustedRefererLinks
        };

        typedef css::uno::Sequence< OUString > Certificate;

        /*
        // MT: Doesn't work for sequence...
        struct Certificate
        {
            OUString SubjectName;
            OUString SerialNumber;
            OUString RawData;
        };
        */

    public:
         SvtSecurityOptions();
        virtual ~SvtSecurityOptions() override;

        /*-****************************************************************************************************
            @short      returns readonly state
            @descr      It can be called to get information about the readonly state of a provided item.
            @param      "eOption", specify, which item is queried
            @return     <TRUE/> if item is readonly; <FALSE/> otherwise

            @onerror    No error should occur!
        *//*-*****************************************************************************************************/

        bool IsReadOnly( EOption eOption ) const;

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

        css::uno::Sequence< OUString >  GetSecureURLs(                                                                      ) const;
        void                                                SetSecureURLs( const css::uno::Sequence< OUString >& seqURLList );

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Security/Scripting/StarOfficeBasic"
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

        sal_Int32           GetMacroSecurityLevel       (                   ) const;
        void                SetMacroSecurityLevel       ( sal_Int32 _nLevel );

        bool            IsMacroDisabled             (                   ) const;

        /**
           Check whether the given uri is either no dangerous macro-execution
           URI at all or else the given referer is a trusted source.
        */
        bool isSecureMacroUri(OUString const & uri, OUString const & referer)
            const;

        /**
           Check whether the given referer URI is untrusted, and links
           originating from it should not be accessed.
         */
        bool isUntrustedReferer(OUString const & referer) const;

        /**
           Check whether the given uri is a trusted location.
        */
        bool isTrustedLocationUri(OUString const & uri) const;

        bool isTrustedLocationUriForUpdatingLinks(OUString const & uri) const;

        css::uno::Sequence< Certificate >  GetTrustedAuthors       (                                                                   ) const;
        void                               SetTrustedAuthors       ( const css::uno::Sequence< Certificate >& rAuthors    );

        // for bool options only!
        bool        IsOptionSet     ( EOption eOption                   ) const;
        void        SetOption       ( EOption eOption, bool bValue      );
        bool        IsOptionEnabled ( EOption eOption                   ) const;

        // xmlsec05 deprecated methods
        bool    IsExecutePlugins() const;
        void        SetExecutePlugins( bool bSet );
        EBasicSecurityMode  GetBasicMode(                           ) const;
        void                SetBasicMode( EBasicSecurityMode eMode  );
        bool IsWarningEnabled() const;
        void SetWarningEnabled( bool bSet );
        bool IsConfirmationEnabled() const;
        void SetConfirmationEnabled( bool bSet );

    //  private methods

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    //  private member

    private:
        std::shared_ptr<SvtSecurityOptions_Impl> m_pImpl;

};      // class SvtSecurityOptions

#endif // INCLUDED_UNOTOOLS_SECURITYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
