/*************************************************************************
 *
 *  $RCSfile: valuenode.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:35:11 $
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

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#define _CONFIGMGR_TREE_VALUENODE_HXX

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif
#ifndef CFGMGR_ANYPAIR_HXX
#include "anypair.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef CONFIGMGR_RTTIMACROS_HXX
#include "rttimacros.hxx"
#endif

#include <string.h>
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{

    namespace css = com::sun::star;
    namespace uno = css::uno;

    class INode;
    class ISubtree;
    class ValueNode;

    using rtl::OUString;

    // helper (tag) class
    namespace treeop { struct NoChildCopy {}; struct DeepChildCopy {}; enum { ALL_LEVELS = -1 }; }
    //==========================================================================
    //= Visitors
    //==========================================================================
    struct NodeAction
    {
        virtual void handle(ValueNode const&) = 0;
        virtual void handle(ISubtree const&) = 0;

        void applyToNode(INode const&);
        void applyToChildren(ISubtree const&);
    };

    struct NodeModification
    {
        virtual void handle(ValueNode&) = 0;
        virtual void handle(ISubtree&) = 0;

        void applyToNode(INode&);
        void applyToChildren(ISubtree&);
    };

    class INode
    {
        OUString                        m_aName;
        node::Attributes            m_aAttributes;

    protected:
        INode(){}

        void markAsDefault(bool _bDefault = true)
        {
            m_aAttributes.markAsDefault(_bDefault);
        }
    public:
        explicit
        INode(node::Attributes);
        INode(OUString const& aName, node::Attributes);

        virtual ~INode();

        virtual std::auto_ptr<INode> clone() const = 0;
    public:

        const OUString& getName() const { return m_aName; }
        node::Attributes getAttributes() const { return m_aAttributes; }

        bool isDefault()   const { return m_aAttributes.isDefault(); }
        bool isLocalized() const { return m_aAttributes.isLocalized(); }

        void modifyState(node::State _eNewState);
        void modifyAccess(node::Access _aAccessLevel);
        void markMandatory();
        void markRemovable();
        void promoteAccessToDefault();
          void forceReadonlyToFinalized();

            // to be used with caution. If the node is referenced from somewhere else under it's old name,
            // you may have problems with this inconsistence
        void setName(const OUString& _rNewName) { m_aName = _rNewName; }

        virtual ValueNode* asValueNode();
        virtual ValueNode const* asValueNode() const;
        virtual ISubtree* asISubtree();
        virtual ISubtree const* asISubtree() const;

        // double dispatch support
        virtual void dispatch(NodeAction&) const = 0;
        virtual void dispatch(NodeModification&) = 0;

        // "rtti"
        RTTI_BASE(INode);
    };

// -----------------------------------------------------------------------------

    //==========================================================================
    //= ISubtree
    //==========================================================================
    class ISubtree : public INode
    {
        sal_Int16       m_nLevel;                   /// determines if everything is read
        sal_Int16       m_nDefaultLevels;           /// determines if defaults are read
        OUString        m_sId;
        OUString        m_sTemplateName;            /// path of the template for child instantiation
        OUString        m_sTemplateModule;          /// module of the template for child instantiation

        virtual INode* doGetChild(OUString const& name) const = 0;

    protected:
        ISubtree():m_nLevel(0){}

        ISubtree(ISubtree const& other)
        :INode(other)
        ,m_nLevel(other.m_nLevel)
        ,m_nDefaultLevels(other.m_nDefaultLevels)
        ,m_sTemplateName(other.m_sTemplateName)
        ,m_sTemplateModule(other.m_sTemplateModule)
        ,m_sId() // do not copy ID while cloning !
        {}

    public:
        // Ctor for group trees
        ISubtree(const rtl::OUString& aName, const node::Attributes& _rAttrs)
            :INode(aName, _rAttrs)
            ,m_nLevel(0)
            ,m_nDefaultLevels(0)
            {}

        // Ctor for set trees
        ISubtree(const rtl::OUString& aName,
                 const rtl::OUString& _rTemplateName,
                 const rtl::OUString& _rTemplateModule,
                 const node::Attributes& _rAttrs)
            :INode(aName, _rAttrs)
            ,m_nLevel(0)
            ,m_nDefaultLevels(0)
            ,m_sTemplateName(_rTemplateName)
            ,m_sTemplateModule(_rTemplateModule){}

        bool        hasId() const               { return m_sId.getLength() != 0; }
        OUString    getId() const               { return m_sId; }
        void        setId(const OUString& _rId) { m_sId = _rId; }

        INode* getChild(OUString const& name)               { return doGetChild(name); }
        INode const* getChild(OUString const& name) const   { return doGetChild(name); }

        ISubtree* asISubtree();
        ISubtree const* asISubtree() const;

        using INode::markAsDefault;

        sal_Int16 getLevel()         const { return m_nLevel; }
        sal_Int16 getDefaultsLevel() const { return m_nDefaultLevels; }

        void setLevels(sal_Int16 _nLevel,sal_Int16 _nDefaultsLevel);

        bool isSetNode() const { return m_sTemplateName.getLength() != 0; }

        void makeSetNode(OUString const& _sTemplateName, OUString const& _sTemplateModule)
        { m_sTemplateName = _sTemplateName; m_sTemplateModule = _sTemplateModule; }

        OUString const& getElementTemplateName()   const { return m_sTemplateName; }
        OUString const& getElementTemplateModule() const { return m_sTemplateModule; }

        virtual INode* addChild(std::auto_ptr<INode> node) =0;                      // takes ownership
        virtual ::std::auto_ptr<INode> removeChild(rtl::OUString const& name) =0;   // releases ownership

        // Iteration support, stop if (action returns true)
        virtual void forEachChild(NodeAction& anAction) const = 0;
        virtual void forEachChild(NodeModification& anAction) = 0;

        // double dispatch support
        virtual void dispatch(NodeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(NodeModification& anAction) { anAction.handle(*this); }

        // "rtti"
        RTTI(ISubtree, INode);
    };

    //==========================================================================
    //= ValueNode
    //==========================================================================
    class ValueNode : public INode
    {
        AnyPair m_aValuePair;
        // uno::Type        m_aType;
        // uno::Any     m_aValue;
        // uno::Any     m_aDefaultValue;

    public:
        //ValueNode(){}

        //explicit ValueNode(node::Attributes _aAttrs):INode(_aAttrs){}

        /*
        ValueNode(OUString const& aName, node::Attributes _aAttrs)
        : INode(aName, _aAttrs)
        , m_aValuePair()
        {}
        */
        ValueNode(OUString const& aName,uno::Type const& aType, node::Attributes _aAttrs)
        : INode(aName, _aAttrs)
        , m_aValuePair(aType)
        {
        }
        ValueNode(OUString const& aName,uno::Any const& anAny, node::Attributes _aAttrs)
        : INode(aName, _aAttrs)
        , m_aValuePair(anAny, selectMember(_aAttrs.isDefault()))
        {
        }
        ValueNode(OUString const& aName,uno::Any const& anAny,uno::Any const& aDefault, node::Attributes _aAttrs)
        : INode(aName, _aAttrs)
        , m_aValuePair(anAny, aDefault)
        {
        }

        bool isEmpty()      const {return  m_aValuePair.isEmpty();}
        bool isValid()      const {return !m_aValuePair.isEmpty();}

        bool isNull()       const {return m_aValuePair.isNull();}
        bool hasUsableDefault()   const {return getAttributes().isNullable() || m_aValuePair.hasSecond();}

        uno::Type   getValueType()  const {return m_aValuePair.getValueType();}
        uno::Any    getValue()      const {return m_aValuePair.getValue( selectMember(this->isDefault()) );}
        uno::Any    getUserValue()  const {return m_aValuePair.getFirst();}
        uno::Any    getDefault()    const {return m_aValuePair.getSecond();}

        bool setValueType(uno::Type const& _aType);
        bool setValue(uno::Any const& _aValue);
        void setDefault();

        bool changeDefault(uno::Any const& _aValue);
        void promoteToDefault();

        virtual std::auto_ptr<INode> clone() const;

        ValueNode* asValueNode();
        ValueNode const* asValueNode() const;
        // double dispatch support
        virtual void dispatch(NodeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(NodeModification& anAction) { anAction.handle(*this); }

        // "rtti"
        RTTI(ValueNode, INode);
    private:
        static AnyPair::SelectMember selectValue() { return AnyPair::SELECT_FIRST; }
        static AnyPair::SelectMember selectDeflt() { return AnyPair::SELECT_SECOND; }
        static AnyPair::SelectMember selectMember(bool bDeflt)
        { return bDeflt ? AnyPair::SELECT_SECOND : AnyPair::SELECT_FIRST; }
    };
    //==========================================================================

    extern bool isLocalizedValueSet(ISubtree const& _aSubtree);

    //==========================================================================
    //= inlines
    //==========================================================================
    inline void NodeAction::applyToNode(INode const& aNode)
    { aNode.dispatch(*this); }
    inline void NodeAction::applyToChildren(ISubtree const& aSubtree)
    { aSubtree.forEachChild(*this); }

    inline void NodeModification::applyToNode(INode& aNode)
    { aNode.dispatch(*this); }
    inline void NodeModification::applyToChildren(ISubtree& aSubtree)
    { aSubtree.forEachChild(*this); }


} // namespace configmgr

#endif
