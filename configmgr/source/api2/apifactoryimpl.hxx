/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apifactoryimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:06:47 $
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

#ifndef CONFIGMGR_API_FACTORYIMPL_HXX_
#define CONFIGMGR_API_FACTORYIMPL_HXX_

#include "apifactory.hxx"

namespace configmgr
{
    namespace configapi
    {
        class ApiProvider;
        // used to create UNO objects
        class ReadOnlyObjectFactory : public Factory
        {
            ApiProvider& m_rProvider;
        public:
            ReadOnlyObjectFactory(ApiProvider& rProvider,ObjectRegistryHolder pRegistry);
            ~ReadOnlyObjectFactory();

            virtual NodeElement*    doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate);
            virtual TreeElement*    doCreateAccessRoot(configuration::Tree const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions);
            virtual SetElement* doCreateSetElement(configuration::ElementTree const& aTree, configuration::Template* pSetElementTemplate);
        };
        // used to create UNO objects
        class UpdateObjectFactory : public Factory
        {
            ApiProvider& m_rProvider;
        public:
            UpdateObjectFactory(ApiProvider& rProvider,ObjectRegistryHolder pRegistry);
            ~UpdateObjectFactory();

            virtual NodeElement*    doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate);
            virtual TreeElement*    doCreateAccessRoot(configuration::Tree const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions);
            virtual SetElement* doCreateSetElement(configuration::ElementTree const& aTree, configuration::Template* pSetElementTemplate);
        private:
            bool implIsReadOnly(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
        };

    }
}

#endif // CONFIGMGR_API_FACTORYIMPL_HXX_
