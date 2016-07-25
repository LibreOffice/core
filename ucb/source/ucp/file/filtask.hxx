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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILTASK_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILTASK_HXX
#endif

#include <rtl/ustring.hxx>

#include "osl/mutex.hxx"
#include <com/sun/star/ucb/DuplicateCommandIdentifierException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include "filerror.hxx"
#include <unordered_map>
#include <functional>

namespace fileaccess
{
    class BaseContent;

    /*
     * This implementation is inherited by class fileaccess::shell.
     * The relevant methods in this class all have as first argument the CommandId,
     * so if necessary, every method has access to its relevant XInteractionHandler and
     * XProgressHandler.
     */


    class TaskManager
    {
    protected:

        class TaskHandling
        {
        private:

            bool m_bAbort,m_bHandled;
            sal_Int32 m_nErrorCode,m_nMinorCode;
            css::uno::Reference< css::task::XInteractionHandler > m_xInteractionHandler;
            css::uno::Reference< css::ucb::XCommandEnvironment >  m_xCommandEnvironment;


        public:

            explicit TaskHandling(
                const css::uno::Reference< css::ucb::XCommandEnvironment >&  xCommandEnv
                = css::uno::Reference< css::ucb::XCommandEnvironment >( nullptr ) )
                : m_bAbort( false ),
                  m_bHandled( false ),
                  m_nErrorCode( TASKHANDLER_NO_ERROR ),
                  m_nMinorCode( TASKHANDLER_NO_ERROR ),
                  m_xInteractionHandler( nullptr ),
                  m_xCommandEnvironment( xCommandEnv )
            {
            }

            void SAL_CALL abort()
            {
                m_bAbort = true;
            }

            void setHandled()
            {
                m_bHandled = true;
            }

            bool isHandled() const
            {
                return m_bHandled;
            }

            void clearError()
            {
                m_nErrorCode = TASKHANDLER_NO_ERROR;
                m_nMinorCode =  TASKHANDLER_NO_ERROR;
            }

            void SAL_CALL installError( sal_Int32 nErrorCode,
                                        sal_Int32 nMinorCode )
            {
                m_nErrorCode = nErrorCode;
                m_nMinorCode = nMinorCode;
            }

            sal_Int32 SAL_CALL getInstalledError()
            {
                return m_nErrorCode;
            }

            sal_Int32 SAL_CALL getMinorErrorCode()
            {
                return m_nMinorCode;
            }

            css::uno::Reference< css::task::XInteractionHandler > SAL_CALL
            getInteractionHandler()
            {
                if( ! m_xInteractionHandler.is() && m_xCommandEnvironment.is() )
                    m_xInteractionHandler = m_xCommandEnvironment->getInteractionHandler();

                return m_xInteractionHandler;
            }

            const css::uno::Reference< css::ucb::XCommandEnvironment >& SAL_CALL
            getCommandEnvironment()
            {
                return m_xCommandEnvironment;
            }

        };  // end class TaskHandling


        typedef std::unordered_map< sal_Int32,TaskHandling > TaskMap;
    private:

        osl::Mutex                                                         m_aMutex;
        sal_Int32                                                           m_nCommandId;
        TaskMap                                                             m_aTaskMap;


    public:

        TaskManager();
        virtual ~TaskManager();

        void SAL_CALL startTask(
            sal_Int32 CommandId,
            const css::uno::Reference< css::ucb::XCommandEnvironment >&  xCommandEnv )
            throw( css::ucb::DuplicateCommandIdentifierException );

        sal_Int32 SAL_CALL getCommandId();
        void SAL_CALL abort( sal_Int32 CommandId );


        /**
         *  The error code may be one of the error codes defined in
         *  filerror.hxx.
         *  The minor code refines the information given in ErrorCode.
         */

        void SAL_CALL installError( sal_Int32 CommandId,
                                    sal_Int32 ErrorCode,
                                    sal_Int32 minorCode = TASKHANDLER_NO_ERROR );

        void SAL_CALL retrieveError( sal_Int32 CommandId,
                                     sal_Int32 &ErrorCode,
                                     sal_Int32 &minorCode);

        /**
         *  Deinstalls the task and evaluates a possibly set error code.
         *  "endTask" throws in case an error code is set the corresponding exception.
         */

        void SAL_CALL endTask( sal_Int32 CommandId,
                               // the physical URL of the object
                               const OUString& aUnqPath,
                               BaseContent* pContent);


        /**
         *  Handles an interactionrequest
         */

        void SAL_CALL handleTask( sal_Int32 CommandId,
                                  const css::uno::Reference< css::task::XInteractionRequest >& request );

        /**
         *  Clears any error which are set on the commandid
         */

        void SAL_CALL clearError( sal_Int32 );

    };

} // end namespace TaskHandling

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
