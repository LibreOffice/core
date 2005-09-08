/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: committer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:10:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_COMMITTER_HXX_
#define CONFIGMGR_API_COMMITTER_HXX_

namespace configmgr
{
    class ITreeManager;
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------

        class ApiRootTreeImpl;
//-----------------------------------------------------------------------------

        /// allows to update values of a simple type within a <type>NodeRef</type> that refers to a Group
        class Committer
        {
            ApiRootTreeImpl& m_rTree;
        public:
            Committer(ApiRootTreeImpl& rTree);

            void commit();
        private:
            ITreeManager* getUpdateProvider();
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_COMMITTER_HXX_
