/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objectregistry.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_API_OBJECTREGISTRY_HXX_
#define CONFIGMGR_API_OBJECTREGISTRY_HXX_

#include "noderef.hxx"

#include <osl/mutex.hxx>
#include <vos/refernce.hxx>

#include <hash_map>
#include "tracer.hxx"

namespace configmgr
{
    namespace configapi
    {

        class NodeElement;

        class ObjectRegistry : public vos::OReference
        {
        public:
            static NodeElement* notFound() { return 0; }

            struct KeyHash
            {
                size_t operator() (const configuration::NodeID& rKey) const {return rKey.hashCode();}
            };
            struct KeyEq
            {
                bool operator() (const configuration::NodeID& lhs,const configuration::NodeID& rhs) const {return lhs == rhs;}
            };
            typedef std::hash_map<configuration::NodeID,NodeElement*,KeyHash, KeyEq> ObjectMap;
        public:
            ObjectRegistry() {}
            ~ObjectRegistry();

            NodeElement*    findElement(configuration::NodeID const& aNode) const
            {
                ObjectMap::const_iterator aFound = m_aMap.find(aNode);

                return (aFound != m_aMap.end()) ? aFound->second : notFound();
            }
            void registerElement(configuration::NodeID const& aNode, NodeElement* aElement)
            {
                OSL_ENSURE(m_aMap.find(aNode) == m_aMap.end(), "ERROR: Node is already registered");

                m_aMap[aNode] = aElement;
            }
            void revokeElement(configuration::NodeID const& aNode, NodeElement* aElement)
            {
                ObjectMap::iterator aFound = m_aMap.find(aNode);

                if (aFound != m_aMap.end())
                {
                    OSL_ENSURE(aFound->second == aElement,"Found unexpected element in map");

                    if (aFound->second == aElement)
                        m_aMap.erase(aFound);
                }
            }
        private:
            ObjectMap m_aMap;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_FACTORY_HXX_
