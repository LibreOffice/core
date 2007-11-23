/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectregistry.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:07:42 $
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

#ifndef CONFIGMGR_API_OBJECTREGISTRY_HXX_
#define CONFIGMGR_API_OBJECTREGISTRY_HXX_

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

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
            typedef configuration::NodeID Key;
            typedef NodeElement*    Element;
            typedef NodeElement*    ElementArg;
            static Element notFound() { return 0; }

            struct KeyHash
            {
                size_t operator() (const Key& rKey) const {return rKey.hashCode();}
            };
            struct KeyEq
            {
                bool operator() (const Key& lhs,const Key& rhs) const {return lhs == rhs;}
            };
            typedef std::hash_map<Key,Element,KeyHash, KeyEq> ObjectMap;
        public:
            ObjectRegistry() {}
            ~ObjectRegistry();

            Element findElement(Key const& aNode) const
            {
                ObjectMap::const_iterator aFound = m_aMap.find(aNode);

                return (aFound != m_aMap.end()) ? aFound->second : notFound();
            }
            void registerElement(Key const& aNode, ElementArg aElement)
            {
                OSL_ENSURE(m_aMap.find(aNode) == m_aMap.end(), "ERROR: Node is already registered");

                m_aMap[aNode] = aElement;
            }
            void revokeElement(Key const& aNode, ElementArg aElement)
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
