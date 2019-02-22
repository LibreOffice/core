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

#ifndef INCLUDED_FRAMEWORK_INC_JOBS_JOBDATA_HXX
#define INCLUDED_FRAMEWORK_INC_JOBS_JOBDATA_HXX

#include <jobs/configaccess.hxx>
#include <jobs/jobresult.hxx>
#include <stdtypes.h>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <rtl/ustring.hxx>

namespace framework{

/**
    @short  holds all necessary information about a job and
            handle it's configuration (if any exist!)
    @descr  It can be used rom different use cases as a container
            (or proxy) for all config data of a registered job.
            But it doesn't implement any execute functionality!
 */
class JobData final
{
    public:

        /** These values can be used to differe between jobs with and jobs without
            a configuration. Of course an "unknown state" should be available too,
            to detect a missing initialization.
         */
        enum EMode
        {
            /// indicates a missing initialization
            E_UNKNOWN_MODE,
            /// indicates a job with configuration (They alias represent the config key name.)
            E_ALIAS,
            /// indicates a job without configuration (The pure UNO implementation is used only.)
            E_SERVICE,
            /// indicates a job with configuration, which was triggered by an event
            E_EVENT
        };

        /** These values represent the environment type, in which a job can run.
            A job must known, from which binding it will be started. Because
            it's initialization data depends from that!
         */
        enum EEnvironment
        {
            /// indicates a missing initialization
            E_UNKNOWN_ENVIRONMENT,
            /// this job is used by the global JobExecutor service
            E_EXECUTION,
            /// this job is used by the global dispatch framework
            E_DISPATCH,
            /// this job is used by the global event broadcaster
            E_DOCUMENTEVENT
        };

        /** Some jobs can be registered to "logical events", which are generated on demand if another document event
            occurs. E.g. "onDocumentOpened" in case "OnNew" or "OnLoad" was notified to the JobExecutor instance.
            And normally the original event is transported as parameter set to the executed job. But then such job
            can't differ between e.g. "OnNew" and "onDocumentOpened".
            That's why we must know, for which type of event the job was really triggered .-)

            The information "sDocEvent" from this struct must be set on the member JobData::m_sEvent from outside
            user of such Jobdata structure.
        */
        struct TJob2DocEventBinding
        {
            OUString m_sJobName;
            OUString m_sDocEvent;

            TJob2DocEventBinding(const OUString& sJobName ,
                                 const OUString& sDocEvent)
                : m_sJobName (sJobName )
                , m_sDocEvent(sDocEvent)
            {}
        };

    // member

    private:

        /**
            reference to the uno service manager.
            We need it for creating of own uno services ... e.g. for
            opening the configuration.
         */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /**
            An instance of this class can be used in two different modes:
                - as a configured job
                - as a job without any configuration
            First mode is triggered by an alias, which points to the
            configuration entries. Second mode is specified by an uno service
            or implementation name. Then we do the same things (use the same interfaces)
            but don't handle any configuration data.
            The effect: This mode can be detected by this member.
         */
        EMode m_eMode;

        /**
            Because jobs can be bind to different mechanism inside office, a job
            should know inside which environment it runs. E.g. a job can be executed
            by the global JobExecutor service (triggered by an event) or e.g. as part
            of the global dispatch framework (triggered by an UI control e.g. a menu entry).
         */
        EEnvironment m_eEnvironment;

        /**
            the alias name of this job.
            Is used as entry of configuration set for job registration, to find all
            necessary properties of it..
         */
        OUString m_sAlias;

        /**
            the uno implementation name of this job.
            It's read from the configuration. Don't set it from outside!
         */
        OUString m_sService;

        /**
            the module context list of this job.
            It's read from the configuration. Don't set it from outside!
         */
        OUString m_sContext;

        /**
            a job can be registered for an event.
            It can be an empty value! But it will be set from outside any times.
            Because it's not clear which job this instance should represent if an event
            (instaed of an alias) comes in. Because there can be multiple registrations
            for this event. We use this information only, to merge it with the job specific
            arguments. A job can be called so, with a) it's onw config data and b) some dynamic
            environment data.
         */
        OUString m_sEvent;

        /**
            job specific configuration items... unknown for us!
            It's read from the configuration. Don't set it from outside!
         */
        std::vector< css::beans::NamedValue > m_lArguments;

    // native interface

    public:

                 JobData( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                 JobData( const JobData&                                                rCopy );
                 ~JobData(                                                                     );

        JobData& operator=( const JobData& rCopy );

        EMode                                        getMode                 () const;
        EEnvironment                                 getEnvironment          () const;
        OUString                              getEnvironmentDescriptor() const;
        OUString                              getService              () const;
        OUString                              getEvent                () const;
        css::uno::Sequence< css::beans::NamedValue > getConfig               () const;
        std::vector< css::beans::NamedValue >    getJobConfig            () const;

        bool                                     hasConfig               () const;
        bool                                     hasCorrectContext       ( const OUString& rModuleIdent ) const;

        void                                         setEnvironment (       EEnvironment                                  eEnvironment );
        void                                         setAlias       ( const OUString&                              sAlias       );
        void                                         setService     ( const OUString&                              sService     );
        void                                         setEvent       ( const OUString&                              sEvent       ,
                                                                      const OUString&                              sAlias       );
        void                                         setJobConfig   ( const std::vector< css::beans::NamedValue >& lArguments   );
        void                                         disableJob     (                                                                  );

        static std::vector< OUString > getEnabledJobsForEvent( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                const OUString&                                    sEvent );

        static void appendEnabledJobsForEvent( const css::uno::Reference< css::uno::XComponentContext >&              rxContext,
                                               const OUString&                                                 sEvent ,
                                                     ::std::vector< JobData::TJob2DocEventBinding >& lJobs  );

    // private helper

    private:

        void impl_reset();
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_JOBS_JOBDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
