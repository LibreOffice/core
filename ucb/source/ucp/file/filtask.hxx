/*************************************************************************
 *
 *  $RCSfile: filtask.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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

#ifndef __SGI_STL_HASH_MAP
#include <stl/hash_map>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef  _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

namespace fileaccess
{
    class TaskManager
    {
    protected:
        // Typ definitions
        struct equal_sal_Int32
        {
            bool operator() ( sal_Int32 a, sal_Int32 b ) const
            {
                return a == b;
            }
        };


        class TaskHandling
        {
        private:
            sal_Bool m_bAbort;
            com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInteractionHandler;
            com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler >     m_xProgressHandler;

        public:
            TaskHandling()
                : m_xInteractionHandler( 0 ),
                  m_xProgressHandler( 0 ),
                  m_bAbort( false )
            {

            }
            TaskHandling(
                const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& m_xIH,
                const com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler >& m_xPH )
                : m_xInteractionHandler( m_xIH ),
                  m_xProgressHandler( m_xPH ),
                  m_bAbort( false )
            {
            }

            void SAL_CALL setAbort()
            {
                m_bAbort = true;
            }

        };

        typedef std::hash_map< sal_Int32,TaskHandling,std::hash< sal_Int32 >, equal_sal_Int32 > TaskMap;

    private:
        vos::OMutex                                                         m_aMutex;
        sal_Int32                                                           m_nCommandId;
        TaskMap                                                             m_aTaskMap;

    public:

        TaskManager();
        virtual ~TaskManager();

        void SAL_CALL startTask(
            sal_Int32 CommandId,
            const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xIH,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XProgressHandler >& xPH )
            throw( com::sun::star::ucb::CommandAbortedException );

        void SAL_CALL endTask( sal_Int32 CommandId );
        sal_Int32 SAL_CALL getCommandId( void );
        void SAL_CALL abort( sal_Int32 CommandId );
    };

} // end namespace TaskHandling
