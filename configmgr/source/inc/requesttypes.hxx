/*************************************************************************
 *
 *  $RCSfile: requesttypes.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:03 $
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

#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#define CONFIGMGR_BACKEND_REQUESTTYPES_HXX_

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
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
            struct RCInstance : public salhelper::SimpleReferenceObject
            {
                RCInstance(Instance_ & _instance) : instance(_instance) {}
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
