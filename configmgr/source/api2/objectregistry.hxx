/*************************************************************************
 *
 *  $RCSfile: objectregistry.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-07 17:18:22 $
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

#ifndef CONFIGMGR_API_OBJECTREGISTRY_HXX_
#define CONFIGMGR_API_OBJECTREGISTRY_HXX_

#include "apitypes.hxx"
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

            osl::Mutex& mutex() const { return m_aMutex; }

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
            mutable osl::Mutex m_aMutex;
            ObjectMap m_aMap;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_FACTORY_HXX_
