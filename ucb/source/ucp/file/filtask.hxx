/*************************************************************************
 *
 *  $RCSfile: filtask.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FILTASK_HXX_
#define _FILTASK_HXX_
#endif

#ifndef INCLUDED_STL_HASH_MAP
#include <hash_map>
#define INCLUDED_STL_HASH_MAP
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef  _COM_SUN_STAR_UCB_DUPLICATECOMMANDIDENTIFIEREXCEPTION_HPP_
#include <com/sun/star/ucb/DuplicateCommandIdentifierException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef  _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef  _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _FILERROR_HXX_
#include "filerror.hxx"
#endif


namespace fileaccess
{
    class shell;
    class BaseContent;

    /*
     * This implementation is inherited by class fileaccess::shell.
     * The relevant methods in this class all have as first argument the CommandId,
     * so if necessary, every method has acess to its relevant XInteractionHandler and
     * XProgressHandler, simply by calling directly the method
     * getInteractionHandler( CommandId )
     * and
     * getProgressHandler();
     */


    class TaskManager
    {
    protected:

        class TaskHandling
        {
        private:

            bool m_bAbort,m_bHandled;
            sal_Int32 m_nErrorCode,m_nMinorCode;
            com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInteractionHandler;
            com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler >     m_xProgressHandler;
            com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >  m_xCommandEnvironment;


        public:

            TaskHandling(
                const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >&  xCommandEnv
                = com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >( 0 ) )
                : m_xInteractionHandler( 0 ),
                  m_xProgressHandler( 0 ),
                  m_xCommandEnvironment( xCommandEnv ),
                  m_bAbort( false ),
                  m_bHandled( false ),
                  m_nErrorCode( TASKHANDLER_NO_ERROR ),
                  m_nMinorCode( TASKHANDLER_NO_ERROR )
            {
            }

            void SAL_CALL abort()
            {
                m_bAbort = true;
            }

            bool SAL_CALL isAborted()
            {
                return m_bAbort;
            }

            void setHandled()
            {
                m_bHandled = true;
            }

            bool isHandled()
            {
                return true;
            }

            void clearError()
            {
                m_nErrorCode = TASKHANDLER_NO_ERROR;
                m_nMinorCode =  TASKHANDLER_NO_ERROR;
            }

            void SAL_CALL installError( sal_Int32 nErrorCode,
                                        sal_Int32 nMinorCode = TASKHANDLER_NO_ERROR )
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

            com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler > SAL_CALL
            getProgressHandler()
            {
                if( ! m_xProgressHandler.is() && m_xCommandEnvironment.is() )
                    m_xProgressHandler = m_xCommandEnvironment->getProgressHandler();

                return m_xProgressHandler;
            }

            com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > SAL_CALL
            getInteractionHandler()
            {
                if( ! m_xInteractionHandler.is() && m_xCommandEnvironment.is() )
                    m_xInteractionHandler = m_xCommandEnvironment->getInteractionHandler();

                return m_xInteractionHandler;
            }

            com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > SAL_CALL
            getCommandEnvironment()
            {
                return m_xCommandEnvironment;
            }

        };  // end class TaskHandling


        typedef std::hash_map< sal_Int32,TaskHandling,std::hash< sal_Int32 > > TaskMap;


    private:

        vos::OMutex                                                         m_aMutex;
        sal_Int32                                                           m_nCommandId;
        TaskMap                                                             m_aTaskMap;


    public:

        TaskManager();
        virtual ~TaskManager();

        void SAL_CALL startTask(
            sal_Int32 CommandId,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >&  xCommandEnv )
            throw( com::sun::star::ucb::DuplicateCommandIdentifierException );

        sal_Int32 SAL_CALL getCommandId( void );
        void SAL_CALL abort( sal_Int32 CommandId );
        bool SAL_CALL isAborted( sal_Int32 CommandId );


        /**
         *  The error code may be one of the error codes defined in
         *  filerror.hxx.
         *  The minor code refines the information given in ErrorCode.
         */

        void SAL_CALL clearError();

        void SAL_CALL installError( sal_Int32 CommandId,
                                    sal_Int32 ErrorCode,
                                    sal_Int32 minorCode = TASKHANDLER_NO_ERROR );


//          void SAL_CALL installError( sal_Int32 CommandId,
//                                      sal_Int32 ErrorCode,
//                                      rtl::OUString message );

//          void SAL_CALL installError( sal_Int32 CommandId,
//                                      sal_Int32 ErrorCode,
//                                      rtl::OUString message );

        void SAL_CALL retrieveError( sal_Int32 CommandId,
                                     sal_Int32 &ErrorCode,
                                     sal_Int32 &minorCode);

        /**
         *  Deinstalls the task and evaluates a possibly set error code.
         *  "endTask" throws in case an error code is set the corresponding exception.
         */

        void SAL_CALL endTask( shell * pShell, // must not be null
                               sal_Int32 CommandId,
                               // the physical URL of the object
                               const rtl::OUString& aUnqPath,
                               BaseContent* pContent);


        /**
         *  Handles an interactionrequest
         */

        void SAL_CALL handleTask( sal_Int32 CommandId,
                                  const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& request );

        /**
         *  Clears any error which are set on the commandid
         */

        void SAL_CALL clearError( sal_Int32 );


        com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > SAL_CALL
        getInteractionHandler( sal_Int32 CommandId );

        com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler > SAL_CALL
        getProgressHandler( sal_Int32 CommandId );

        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > SAL_CALL
        getCommandEnvironment( sal_Int32 CommandId );

    };

} // end namespace TaskHandling
