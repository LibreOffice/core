/*************************************************************************
 *
 *  $RCSfile: valuenode.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-20 08:01:46 $
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

#include <memory>
#include "rttimacros.hxx"
#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{

    namespace css = com::sun::star;
    namespace uno = css::uno;

    class INode;
    class ISubtree;
    class ValueNode;

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
        rtl::OUString                       m_aName;
        configuration::Attributes           m_aAttributes;

    protected:
        INode(){}

    public:
        explicit INode(configuration::Attributes);
        INode(rtl::OUString const& aName, configuration::Attributes);
        virtual ~INode();
        virtual INode* clone() const = 0;

        const rtl::OUString& getName() const {return m_aName;}
        configuration::Attributes getAttributes() const {return m_aAttributes;}

        bool isLocalized() const {return m_aAttributes.bLocalized;}
        bool isReplacing() const {return m_aAttributes.bReplacing;}
        void setName(const rtl::OUString& _rNewName) { m_aName = _rNewName; }
            // to be used with caution. If the node is referenced from somewhere else under it's old name,
            // you may have problems with this inconsistence

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

    //==========================================================================
    //= ISubtree
    //==========================================================================
    class ISubtree : public INode
    {
        sal_Int16           m_nLevel;                   /// determines if everything is read
        ::rtl::OUString     m_sId;
        ::rtl::OUString     m_sTemplateName;            /// path of the template for child instantiation
        ::rtl::OUString     m_sTemplateModule;          /// module of the template for child instantiation

        virtual INode* doGetChild(rtl::OUString const& name) const = 0;

    protected:
        ISubtree():m_nLevel(0){}

        ISubtree(ISubtree const& other)
        :INode(other)
        ,m_nLevel(other.m_nLevel)
        ,m_sTemplateName(other.m_sTemplateName)
        ,m_sTemplateModule(other.m_sTemplateModule)
        ,m_sId() // do not copy ID while cloning !
        {}

    public:
        // subtree does current not support attributes in the right way
        // ISubtree(const configuration::Attributes& _rAttrs) : INode(_rAttrs), m_nLevel(0) {}

        // Ctor for group trees
        ISubtree(const rtl::OUString& aName, const configuration::Attributes& _rAttrs)
            :INode(aName, _rAttrs)
            ,m_nLevel(0)
            {}

        // Ctor for set trees
        ISubtree(const rtl::OUString& aName,
                 const rtl::OUString& _rTemplateName,
                 const rtl::OUString& _rTemplateModule,
                 const configuration::Attributes& _rAttrs)
            :INode(aName, _rAttrs)
            ,m_nLevel(0)
            ,m_sTemplateName(_rTemplateName)
            ,m_sTemplateModule(_rTemplateModule){}

        bool            hasId() const           { return m_sId.getLength() != 0; }
        ::rtl::OUString getId() const           { return m_sId; }
        void            setId(const ::rtl::OUString& _rId)  { m_sId = _rId; }

        INode* getChild(rtl::OUString const& name)              { return doGetChild(name); }
        INode const* getChild(rtl::OUString const& name) const  { return doGetChild(name); }

        ISubtree* asISubtree();
        ISubtree const* asISubtree() const;

        sal_Int16 getLevel() const {return m_nLevel;}
        void setLevel(sal_Int16);

        bool                isSetNode() const { return m_sTemplateName.getLength() != 0; }
        ::rtl::OUString     getElementTemplateName() const { return m_sTemplateName; }
        ::rtl::OUString     getElementTemplateModule() const { return m_sTemplateModule; }

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
        uno::Type       m_aType;
        uno::Any        m_aValue;
        uno::Any        m_aDefaultValue;

    public:
        ValueNode(){}

        explicit ValueNode(configuration::Attributes _aAttrs):INode(_aAttrs){}

        ValueNode(rtl::OUString const& aName, configuration::Attributes _aAttrs)
            :INode(aName, _aAttrs)
        {}
        ValueNode(rtl::OUString const& aName,uno::Type const& aType, configuration::Attributes _aAttrs)
            :INode(aName, _aAttrs), m_aType(aType)
        { check_init(); }
        ValueNode(rtl::OUString const& aName,uno::Any const& anAny, configuration::Attributes _aAttrs)
            :INode(aName, _aAttrs), m_aValue(anAny)
        { init(); }
        ValueNode(rtl::OUString const& aName,uno::Any const& anAny,uno::Any const& aDefault, configuration::Attributes _aAttrs)
            :INode(aName, _aAttrs), m_aValue(anAny), m_aDefaultValue(aDefault)
        { init(); }


        bool isNull() const {return !m_aValue.hasValue() && !m_aDefaultValue.hasValue();}
        bool hasDefault() const {return getAttributes().bNullable || m_aDefaultValue.hasValue();}
        bool isDefault() const {return !m_aValue.hasValue() && hasDefault();}

        uno::Type getValueType() const {return m_aType;}
        uno::Any getValue() const {return isDefault() ? m_aDefaultValue : m_aValue;}
        const uno::Any& getDefault() const {return m_aDefaultValue;}

        void setValue(uno::Any aValue);
        void changeDefault(uno::Any aValue);
        void setDefault();

        virtual INode* clone() const;

        ValueNode* asValueNode();
        ValueNode const* asValueNode() const;
        // double dispatch support
        virtual void dispatch(NodeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(NodeModification& anAction) { anAction.handle(*this); }

        // "rtti"
        RTTI(ValueNode, INode);
    private:
        void init();
        void check_init();
    };
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
