/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apifactoryimpl.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
            ReadOnlyObjectFactory(ApiProvider& rProvider,rtl::Reference<ObjectRegistry> pRegistry);
            ~ReadOnlyObjectFactory();

            virtual NodeElement*    doCreateGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate);
            virtual TreeElement*    doCreateAccessRoot(rtl::Reference< configuration::Tree > const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions);
            virtual SetElement* doCreateSetElement(rtl::Reference< configuration::ElementTree > const& aTree, configuration::Template* pSetElementTemplate);
        };
        // used to create UNO objects
        class UpdateObjectFactory : public Factory
        {
            ApiProvider& m_rProvider;
        public:
            UpdateObjectFactory(ApiProvider& rProvider,rtl::Reference<ObjectRegistry> pRegistry);
            ~UpdateObjectFactory();

            virtual NodeElement*    doCreateGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate);
            virtual TreeElement*    doCreateAccessRoot(rtl::Reference< configuration::Tree > const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions);
            virtual SetElement* doCreateSetElement(rtl::Reference< configuration::ElementTree > const& aTree, configuration::Template* pSetElementTemplate);
        private:
            bool implIsReadOnly(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
        };

    }
}

#endif // CONFIGMGR_API_FACTORYIMPL_HXX_
