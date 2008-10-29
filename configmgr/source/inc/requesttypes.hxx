/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: requesttypes.hxx,v $
 * $Revision: 1.10 $
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

#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#define CONFIGMGR_BACKEND_REQUESTTYPES_HXX_

#include "sal/config.h"

#include "salhelper/simplereferenceobject.hxx"

#include "valuenode.hxx"
#include "treechangelist.hxx"
#include "configpath.hxx"

#ifndef _CONFIGMGR_UTILITY_HXX_
#include <utility.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
        struct ComponentDataStruct
        {
            const std::auto_ptr<ISubtree>& data;
            rtl::OUString name;
            ComponentDataStruct (const std::auto_ptr<ISubtree>& _data, rtl::OUString _name)
              : data(_data), name(_name) {}
        };
// ---------------------------------------------------------------------------
        struct NodeInstance
        {
            typedef std::auto_ptr<ISubtree> Data;

            explicit
            NodeInstance(Data _node, configuration::AbsolutePath const & _rootpath)
            : m_node(_node)
            , m_root(_rootpath)
            {
            }

            Data        const & data() const { return m_node; }
            configuration::AbsolutePath const & root() const { return m_root; }

            Data &  mutableData() { return m_node; }
            Data extractData() { return m_node; }
        private:
            Data        m_node;
            configuration::AbsolutePath m_root;
        };
// ---------------------------------------------------------------------------
        struct TemplateInstance
        {
            typedef std::auto_ptr<INode> Data;

            explicit
            TemplateInstance(Data _node, rtl::OUString const & _name, rtl::OUString const & _component)
            : m_node(_node)
            , m_name(_name)
            , m_component(_component)
            {
            }

            Data        const & data() const { return m_node; }
            rtl::OUString const & name() const { return m_name; }
            rtl::OUString const & component() const { return m_component; }

            Data extractData() { return m_node; }
    private:
            Data m_node;
            rtl::OUString m_name; // if empty, this is a complete set of component templates
            rtl::OUString m_component;
        };
// ---------------------------------------------------------------------------
        struct ComponentInstance
        {
            typedef std::auto_ptr<ISubtree> Data;

            explicit
            ComponentInstance(Data _node, Data _template, rtl::OUString const & _component)
            : m_node(_node)
            , m_template(_template)
            , m_component(_component)
            {
            }

            Data        const & data() const { return m_node; }
            Data        const & templateData() const { return m_template; }
            rtl::OUString const & component() const { return m_component; }

            ComponentDataStruct  componentTemplateData () const { return ComponentDataStruct(m_template,m_component);}
            ComponentDataStruct  componentNodeData () const { return ComponentDataStruct(m_node,m_component);}
            Data &  mutableData() { return m_node; }
            Data extractData() { return m_node; }
            Data extractTemplateData() { return m_template; }
        private:
            Data        m_node;
            Data        m_template;
            rtl::OUString m_component;
        };
// ---------------------------------------------------------------------------
        struct UpdateInstance
        {
            typedef SubtreeChange *         Data;

            explicit
            UpdateInstance(Data _update, configuration::AbsolutePath const & _rootpath)
            : m_update(_update)
            , m_root(_rootpath)
            {
            }

            UpdateInstance(UpdateInstance & _aModifiableOther)
            : m_update(_aModifiableOther.m_update)
            , m_root(_aModifiableOther.m_root)
            {
            }

            Data                data()       { return m_update; }
            SubtreeChange const *           data() const { return m_update; }
            configuration::AbsolutePath const & root() const { return m_root; }
        private:
            Data        m_update;
            configuration::AbsolutePath m_root;
        };
// ---------------------------------------------------------------------------
        struct ConstUpdateInstance
        {
            typedef SubtreeChange const *   Data;

            explicit
            ConstUpdateInstance(Data _update, configuration::AbsolutePath const & _rootpath)
            : m_update(_update)
            , m_root(_rootpath)
            {
            }

            // conversion
            ConstUpdateInstance(UpdateInstance const & _aModifiable)
            : m_update(_aModifiable.data())
            , m_root(_aModifiable.root())
            {
            }

            Data                data() const { return m_update; }
            configuration::AbsolutePath const & root() const { return m_root; }
        private:
            Data        m_update;
            configuration::AbsolutePath m_root;
        };
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Due to the use of auto_ptr, the XxxInstance classes cannot easily be used as return values
// To return them, they should be wrapped into a ResultHolder

        template <class Instance_>
        class ResultHolder
        {
            struct RCInstance : public salhelper::SimpleReferenceObject
            {
                RCInstance(Instance_ & _instance)
                    : instance(_instance) {}
                Instance_ instance;
            };

            rtl::Reference< RCInstance > m_xInstance;
        public:
            explicit
            ResultHolder(Instance_ & _rInstance)
            : m_xInstance( new RCInstance(_rInstance) )
            {}

            bool isEmpty() const { return !m_xInstance.is(); }

            bool is() const { return m_xInstance.is() && m_xInstance->instance.data().get(); }

            Instance_ const & instance() const { return  m_xInstance->instance; }

            Instance_ const & operator *() const { return  instance(); }
            Instance_ const * operator->() const { return &instance(); }
            Instance_ & mutableInstance() { return m_xInstance->instance; }

            typename Instance_::Data extractDataAndClear()
            {
                typename Instance_::Data aData = m_xInstance->instance.extractData();
                this->clear();
                return aData;
            }

            void releaseAndClear()
            {
                typename Instance_::Data aData = this->extractDataAndClear();
                aData.release();
            }

            void clear() { m_xInstance.clear(); }
        };
// ---------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------
} // namespace

#endif
