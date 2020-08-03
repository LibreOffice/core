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

#include <jobs/jobdata.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/task/XJobListener.hpp>
#include <com/sun/star/util/XCloseListener.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/conditn.hxx>

namespace framework{

/**
    @short  it represent a job; execute it and control its lifetime

    @descr  This implementation can be used to wrap jobs, execute it
            synchronously or asynchronous, control its lifetime
            and differe between jobs with and without configuration.
 */
class Job final : public  ::cppu::WeakImplHelper<
                        css::task::XJobListener
                      , css::frame::XTerminateListener
                      , css::util::XCloseListener >
{

    // structs

    private:

    /** different possible states for the internal wrapped job.
        It can be started, stopped by a queryClosing() request or
        disposed() by a notifyClosing() request ...
     */
    enum ERunState
    {
        E_NEW,
        E_RUNNING,
        E_STOPPED_OR_FINISHED,
        E_DISPOSED
    };

    // member

    private:

        /**
            hold all necessary information about this job.
            It can be used for both modes: with and without configuration.
         */
        JobData m_aJobCfg;

        /**
            We need it to create own services on demand.
         */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /**
            Hold the (may asynchronous) job alive.
         */
        css::uno::Reference< css::uno::XInterface > m_xJob;

        /**
            Used to wait for finishing of asynchronous started jobs.
         */
        ::osl::Condition m_aAsyncWait;

        /**
            For some special cases we must know the environment, in which
            this job runs. Means the frame inside which we may was triggered.
            We use it too, to listen for closing events of this resource.

            Please note: If m_xFrame is set - m_xModel should be NULL.
            Only one environment can be supported really.
         */
        css::uno::Reference< css::frame::XFrame > m_xFrame;

        /**
            For some special cases we must know the environment, in which
            this job runs. Means the document inside which we may was triggered.
            We use it too, to listen for closing events of this resource.

            Please note: If m_xModel is set - m_xFrame should be NULL.
            Only one environment can be supported really.
         */
        css::uno::Reference< css::frame::XModel > m_xModel;

        /**
            We are registered at this instance to listen for office shutdown events.
            It's necessary suppress it (if possible) or to react in the right way.
         */
        css::uno::Reference< css::frame::XDesktop2 > m_xDesktop;

        /**
            A job can return a dispatch result event after finishing its work.
            We have to transport it to any outside interested listener then.
            (see m_xResultSourceFake for further information too!)
         */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;

        /**
            We can't set ourself as source of a dispatch result event ... nor our job.
            Because the listener (set as m_xResultListener) expect the original instance,
            where it was registered. This original instance is the user of this class.
            It must be set explicitly and will be used to fake the source of the event!
         */
        css::uno::Reference< css::uno::XInterface > m_xResultSourceFake;

        /**
            Holds the state, if we are listen for desktop/frame or model closing events or not.
            The used references are not really enough to detect a valid listener connection.
            That's why we use this additional information here too.
         */
        bool m_bListenOnDesktop;
        bool m_bListenOnFrame;
        bool m_bListenOnModel;

        /**
            In case we got a close request from our desktop/frame/model (on which we listen) ... and
            the ownership was delivered there ... we have to close ourself and this object
            in case the internal wrapped and running job finish his work.
         */
        bool m_bPendingCloseFrame;
        bool m_bPendingCloseModel;

        /**
            indicates in which state the internal job currently exist.

            We can use this information to throw any suitable veto exception
            to prevent the environment against dying or suppress superfluous dispose()
            calls at the job.
         */
        ERunState m_eRunState;

    // native interface

    public:

                 Job( const css::uno::Reference< css::uno::XComponentContext >& xContext  ,
                      const css::uno::Reference< css::frame::XFrame >&              xFrame );
                 Job( const css::uno::Reference< css::uno::XComponentContext >& xContext  ,
                      const css::uno::Reference< css::frame::XModel >&              xModel );
        virtual ~Job(                                                                      ) override;

        void     setDispatchResultFake( const css::uno::Reference< css::frame::XDispatchResultListener >& xListener    ,
                                        const css::uno::Reference< css::uno::XInterface >&                xSourceFake  );
        void     setJobData           ( const JobData&                                                    aData        );
        void     execute              ( const css::uno::Sequence< css::beans::NamedValue >&               lDynamicArgs );
        void     die                  (                                                                                );

    private:

        css::uno::Sequence< css::beans::NamedValue > impl_generateJobArgs  ( const css::uno::Sequence< css::beans::NamedValue >& lDynamicArgs );
        void                                         impl_reactForJobResult( const css::uno::Any&                                aResult      );
        void                                         impl_startListening   (                                                                  );
        void                                         impl_stopListening    (                                                                  );

    // uno interface

    public:

        // XJobListener
        virtual void SAL_CALL jobFinished( const css::uno::Reference< css::task::XAsyncJob >& xJob,
                                           const css::uno::Any&                               aResult ) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination ( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;

        // XCloseListener
        virtual void SAL_CALL queryClosing ( const css::lang::EventObject& aEvent         ,
                                                   sal_Bool                bGetsOwnership ) override;
        virtual void SAL_CALL notifyClosing( const css::lang::EventObject& aEvent         ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
