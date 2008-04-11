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
        using configuration::AbsolutePath;
        using configuration::Name;
// ---------------------------------------------------------------------------
      //typedef std::pair<std::auto_ptr<ISubtree>, Name> ComponentData;
        struct ComponentDataStruct
        {
            const std::auto_ptr<ISubtree>& data;
            Name name;
            ComponentDataStruct (const std::auto_ptr<ISubtree>& _data, Name _name)
              : data(_data), name(_name) {}
        };
        typedef struct ComponentDataStruct ComponentData;

        class NodePath
        {
            AbsolutePath m_path;
        public:
            NodePath(AbsolutePath const & _path) : m_path(_path) {};

            AbsolutePath const & location() const { return m_path; }
            AbsolutePath context()          const { return m_path.getParentPath(); }

            bool isEmpty()              const { return m_path.isRoot(); }
            bool isModuleRoot()         const { return m_path.getDepth() == 1; }
            Name getModuleName()        const { return m_path.getModuleName(); }
            rtl::OUString toString()    const { return m_path.toString(); }
        };
// ---------------------------------------------------------------------------
        struct NodeInstance
        {
            typedef std::auto_ptr<ISubtree> Data;

            explicit
            NodeInstance(Data _node, AbsolutePath const & _rootpath)
            : m_node(_node)
            , m_root(_rootpath)
            {
            }

            Data        const & data() const { return m_node; }
            NodePath    const & root() const { return m_root; }

            Data &  mutableData() { return m_node; }
            Data extractData() { return m_node; }
        private:
            Data        m_node;
            NodePath    m_root;
        };
// ---------------------------------------------------------------------------
        struct TemplateInstance
        {
            typedef std::auto_ptr<INode> Data;

            explicit
            TemplateInstance(Data _node, Name const & _name, Name const & _component)
            : m_node(_node)
            , m_name(_name)
            , m_component(_component)
            {
            }

            Data        const & data() const { return m_node; }
            Name        const & name() const { return m_name; }
            Name        const & component() const { return m_component; }

            Data extractData() { return m_node; }
    private:
            Data m_node;
            Name m_name; // if empty, this is a complete set of component templates
            Name m_component;
        };
// ---------------------------------------------------------------------------
        struct ComponentInstance
        {
            typedef std::auto_ptr<ISubtree> Data;

            explicit
            ComponentInstance(Data _node, Data _template, Name const & _component)
            : m_node(_node)
            , m_template(_template)
            , m_component(_component)
            {
            }

            Data        const & data() const { return m_node; }
            Data        const & templateData() const { return m_template; }
            Name        const & component() const { return m_component; }

            ComponentData  componentTemplateData () const { return ComponentData(m_template,m_component);}
            ComponentData  componentNodeData () const { return ComponentData(m_node,m_component);}
            Data &  mutableData() { return m_node; }
            Data extractData() { return m_node; }
            Data extractTemplateData() { return m_template; }
        private:
            Data        m_node;
            Data        m_template;
            Name        m_component;
        };
// ---------------------------------------------------------------------------
        struct UpdateInstance
        {
            typedef SubtreeChange *         Data;
            typedef SubtreeChange const *   ConstData;

            explicit
            UpdateInstance(Data _update, AbsolutePath const & _rootpath)
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
            ConstData           data() const { return m_update; }
            NodePath    const & root() const { return m_root; }
        private:
            Data        m_update;
            NodePath    m_root;
        };
// ---------------------------------------------------------------------------
        struct ConstUpdateInstance
        {
            typedef UpdateInstance::ConstData   Data, ConstData;

            explicit
            ConstUpdateInstance(Data _update, AbsolutePath const & _rootpath)
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
            NodePath    const & root() const { return m_root; }
        private:
            Data        m_update;
            NodePath    m_root;
        };
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Due to the use of auto_ptr, the XxxInstance classes cannot easily be used as return values
// To return them, they should be wrapped into a ResultHolder

        template <class Instance_>
        class ResultHolder
        {
            struct RCInstance : public configmgr::SimpleReferenceObject
            {
                RCInstance(Instance_ & _instance)
                    : instance(_instance) {}
                Instance_ instance;
            };
            typedef rtl::Reference< RCInstance > InstanceRef;

            InstanceRef m_xInstance;
        public:
            typedef Instance_ Instance;

            explicit
            ResultHolder(Instance & _rInstance)
            : m_xInstance( new RCInstance(_rInstance) )
            {}

            bool isEmpty() const { return !m_xInstance.is(); }

            bool is() const { return m_xInstance.is() && m_xInstance->instance.data().get(); }

            Instance const & instance() const { return  m_xInstance->instance; }

            Instance const & operator *() const { return  instance(); }
            Instance const * operator->() const { return &instance(); }
            Instance & mutableInstance() { return m_xInstance->instance; }

            typename Instance::Data extractDataAndClear()
            {
                typename Instance::Data aData = m_xInstance->instance.extractData();
                this->clear();
                return aData;
            }

            void releaseAndClear()
            {
                typename Instance::Data aData = this->extractDataAndClear();
                aData.release();
            }

            void clear() { m_xInstance.clear(); }
        };
// ---------------------------------------------------------------------------
        typedef ResultHolder< NodeInstance >        NodeResult;
        typedef ResultHolder< TemplateInstance >    TemplateResult;
        typedef ResultHolder< ComponentInstance >   ComponentResult;

// ---------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------
} // namespace

#endif
