/*************************************************************************
 *
 *  $RCSfile: change.hxx,v $
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

#ifndef CONFIGMGR_CHANGE_HXX
#define CONFIGMGR_CHANGE_HXX

#include <map>
#include <memory>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#include "valuenode.hxx"

namespace configmgr
{

    namespace uno = com::sun::star::uno;

    class Change;
    class SubtreeChange;
    class ValueChange;
    class AddNode;
    class RemoveNode;

    //==========================================================================
    //= ChangeTreeAction
    //==========================================================================
    struct ChangeTreeAction
    {
        virtual void handle(ValueChange const& aValueNode) = 0;
        virtual void handle(AddNode const& aAddNode) = 0;
        virtual void handle(RemoveNode const& aRemoveNode) = 0;
        virtual void handle(SubtreeChange const& aSubtree) = 0;

        void applyToChange(Change const& aChange);
        void applyToChildren(SubtreeChange const& aSubtree);
    };

    struct ChangeTreeModification
    {
        virtual void handle(ValueChange& aValueNode) = 0;
        virtual void handle(AddNode& aAddNode) = 0;
        virtual void handle(RemoveNode& aRemoveNode) = 0;
        virtual void handle(SubtreeChange& aSubtree) = 0;

        void applyToChange(Change& aChange);
        void applyToChildren(SubtreeChange& aSubtree);
    };

    //==========================================================================
    //= Change
    //==========================================================================
    class Change
    {
    protected:
        rtl::OUString m_aName;

        void swap(Change& aOther);

    public:
        explicit
        Change(rtl::OUString const& _rName) : m_aName(_rName){}
        Change(Change const& _rChange):m_aName(_rChange.m_aName){}
        virtual ~Change() {}

        rtl::OUString getNodeName() const { return m_aName; }
        void setNodeName(const rtl::OUString &aName) {m_aName = aName;}

        Change* getSubChange(rtl::OUString const& _rName) { return doGetChild(_rName); }
        Change const* getSubChange(rtl::OUString const& _rName) const { return doGetChild(_rName); }

        virtual void dispatch(ChangeTreeAction& anAction) const = 0;
        virtual void dispatch(ChangeTreeModification& anAction) = 0;

        // some kind of simple rtti
        RTTI_BASE(Change);
        virtual Change* clone() const = 0;

    private:
        virtual Change* doGetChild(rtl::OUString const& ) const { return 0; }
    };

    //==========================================================================
    //= ValueChange
    //==========================================================================
    class ValueChange : public Change
    {
    public:
        typedef uno::Any Any;
        struct SetToDefault {};
        enum Mode { wasDefault, changeValue, setToDefault, changeDefault, typeIsAny };
        configuration::Attributes   m_aAttributes;

    private:
        uno::Any        m_aValue;
        uno::Any        m_aOldValue;
        Mode            m_eMode;
    public:
        ValueChange(rtl::OUString const& _rName, uno::Any aNewValue, const configuration::Attributes& _rAttributes,
                    Mode aMode = changeValue, uno::Any aOldValue = uno::Any());
        ValueChange(uno::Any aNewValue, ValueNode const& aOldValue);
        ValueChange(SetToDefault, ValueNode const& aOldValue);
        ValueChange(const ValueChange&);
        virtual Change* clone() const;

        uno::Any    getNewValue() const { return m_aValue; }
        uno::Any    getOldValue() const { return m_aOldValue; }

        void        setNewValue(const uno::Any& _rNewVal) { m_aValue = _rNewVal; }
        void        setNewValue(const uno::Any& _rNewVal, Mode aMode)
        { setNewValue(_rNewVal); m_eMode = aMode;}

        bool isReplacing() const {return m_aAttributes.bReplacing;}
        bool isLocalized() const {return m_aAttributes.bLocalized;}

        Mode getMode() const { return m_eMode; }

        const configuration::Attributes& getAttributes() const {return m_aAttributes;}

        rtl::OUString getModeAsString() const;
        void setModeAsString(const rtl::OUString& _rMode);

        void applyTo(ValueNode& aNode);

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }

        // "rtti"
        RTTI(ValueChange, Change);
    };

    //==========================================================================
    //= AddNode
    //==========================================================================
    class AddNode : public Change
    {
        std::auto_ptr<INode>            m_aOwnNewNode;
        std::auto_ptr<INode>            m_aOwnOldNode;
        INode*                          m_pNewNode;
        INode*                          m_pOldNode;
        bool                            m_bReplacing;

        // don't create CopyCTor automatically
        void operator=(AddNode const&);

    public:
        AddNode(std::auto_ptr<INode> aNewNode_,rtl::OUString const& _rName);
        ~AddNode();
        AddNode(AddNode const&);
        virtual Change* clone() const;

        /// marks this as not merely adding a node but replacing another
        void setReplacing() { m_bReplacing = true; }
        /// is this  not merely adding a node but replacing another ?
        bool isReplacing() const { return m_bReplacing || m_pOldNode; }

        /** returns the node this change represents; The Node object is owned by this change until
            <method>releaseAddedNode</method> is called.<BR>
            After ownership is lost this method returns NULL.
        */
        INode*                  getAddedNode()  const { return m_aOwnNewNode.get(); }

        /** returns the node this change represents, even if this node does not own the new Node object any more.
            This is somewhat dangerous if the node referenced by this object dies before the object itself does.<BR>
            In this case all calls to this method will return nonsense. This case can be detected by testing
            whether <method>getAddedNode</method> returns NULL.
        */
        INode*                  getAddedNode_unsafe()   const { return m_pNewNode; }
        /** returns the node the change represents, and releases ownership of it. This means that
            afterwards <method>getAddedNode</method> will return NULL. This change object keeps a reference
            to the node though which can be retrieved using <method>getAddedNode_unsafe</method>.
        */
        std::auto_ptr<INode>    releaseAddedNode() { return m_aOwnNewNode; }

        /** returns the node this change replaces, ihe Node object is owned by this change.
            After ownership is lost this method returns NULL.
        */
        INode*  getReplacedNode()           const   { return m_aOwnOldNode.get(); }
        /** returns the node this change replaces, even if this node does not own the new Node object any more.
            This is somewhat dangerous if the node referenced by this object dies before the object itself does.<BR>
            In this case all calls to this method will return nonsense. This case can be detected by testing
            whether <method>getAddedNode</method> returns NULL.
        */
        INode*  getReplacedNode_Unsafe()    const   { return m_pOldNode; }
        /** returns the node the change replaces, and releases ownership of it. This means that
            afterwards <method>getReplacedNode</method> will return NULL. This change object keeps a reference
            to the node though which can be retrieved using <method>getReplacedNode_unsafe</method>.
        */
        std::auto_ptr<INode> releaseReplacedNode()  { return m_aOwnOldNode; }
        /** forgets about the node the change replaces, returning the previous setting with ownership
        */
        std::auto_ptr<INode> resetReplacedNode()    { m_pOldNode = 0; return m_aOwnOldNode; }

        void    expectReplacedNode(INode* pOldNode);
        void    takeReplacedNode(std::auto_ptr<INode> aNode);

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }

        // "rtti"
        RTTI(AddNode, Change);
    };

    //==========================================================================
    //= RemoveNode
    //==========================================================================
    class RemoveNode : public Change
    {
    protected:
        INode*                          m_pOldNode;
        std::auto_ptr<INode>            m_aOwnOldNode;

    public:
        explicit
        RemoveNode(rtl::OUString const& _rName);
        ~RemoveNode();
        RemoveNode(const RemoveNode&);
        virtual Change* clone() const;

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }

        /** returns the node this change removes, ihe Node object is owned by this change.
            After ownership is lost this method returns NULL.
        */
        INode*  getRemovedNode()            const   { return m_aOwnOldNode.get(); }
        /** returns the node this change removes, even if this node does not own the new Node object any more.
            This is somewhat dangerous if the node referenced by this object dies before the object itself does.<BR>
            In this case all calls to this method will return nonsense. This case can be detected by testing
            whether <method>getRemovedNode</method> returns NULL.
        */
        INode*  getRemovedNode_Unsafe() const   { return m_pOldNode; }
        /** returns the node the change removes, and releases ownership of it. This means that
            afterwards <method>getRemovedNode</method> will return NULL. This change object keeps a reference
            to the node though which can be retrieved using <method>getRemovedNode_unsafe</method>.
        */
        std::auto_ptr<INode> releaseRemovedNode()   { return m_aOwnOldNode; }
        /** forgets about the node the change removes, returning the previous setting with ownership
        */
        std::auto_ptr<INode> resetRemovedNode() { m_pOldNode = 0; return m_aOwnOldNode; }

        void    expectRemovedNode(INode* pOldNode);
        void    takeRemovedNode(std::auto_ptr<INode> aNode);

        // "rtti"
        RTTI(RemoveNode, Change);
    };

    namespace argument { struct NoChildCopy {}; }

    //==========================================================================
    //= SubtreeChange
    //==========================================================================
    class SubtreeChange : public Change
    {
    protected:
        typedef ::std::map< ::rtl::OUString,Change* > Children;
        Children                    m_aChanges;
        ::rtl::OUString             m_sTemplateName;            /// path of the template for child instantiation
        ::rtl::OUString             m_sTemplateModule;          /// module of the template for child instantiation
        configuration::Attributes   m_aAttributes;

        // don't create CopyCTor automatically
        void operator=(SubtreeChange&);

    public:
        class ChildIterator;
        ChildIterator begin() const throw();
        ChildIterator end() const throw();

        class MutatingChildIterator;
        MutatingChildIterator begin_changes() throw();
        MutatingChildIterator end_changes() throw();

        friend class MutatingChildIterator;
    public:
        /// A parameter for disabling copying of children
        typedef argument::NoChildCopy NoChildCopy;

        SubtreeChange(const rtl::OUString& _rName,
                      const configuration::Attributes& _rAttr)
            :Change(_rName)
            ,m_aAttributes(_rAttr)
        {}

        SubtreeChange(const rtl::OUString& _rName,
                      const rtl::OUString& _rTemplateName,
                      const rtl::OUString& _rTemplateModule,
                      const configuration::Attributes& _rAttr)
            :Change(_rName)
            ,m_sTemplateName(_rTemplateName)
            ,m_sTemplateModule(_rTemplateModule)
            ,m_aAttributes(_rAttr)
        {}

        SubtreeChange(const ISubtree& _rTree)
            :Change(_rTree.getName())
            ,m_aAttributes(_rTree.getAttributes())
            ,m_sTemplateName(_rTree.getElementTemplateName())
            ,m_sTemplateModule(_rTree.getElementTemplateModule())
        {}

        SubtreeChange(const SubtreeChange& _rChange, NoChildCopy)
            :Change(_rChange)
            ,m_sTemplateName(_rChange.getElementTemplateName())
            ,m_sTemplateModule(_rChange.getElementTemplateModule())
            ,m_aAttributes(_rChange.getAttributes()){}

        ~SubtreeChange();

        SubtreeChange(const SubtreeChange&);
        virtual Change* clone() const;

        void swap(SubtreeChange& aOther);

        bool isReplacing() const {return m_aAttributes.bReplacing;}
        bool isLocalized() const {return m_aAttributes.bLocalized;}
        const configuration::Attributes& getAttributes() const {return m_aAttributes;}

        bool            isSetNodeChange() const { return m_sTemplateName.getLength() != 0; }

        rtl::OUString   getElementTemplateName() const { return m_sTemplateName; }
        rtl::OUString   getElementTemplateModule() const { return m_sTemplateModule; }

        void            setElementTemplate(const rtl::OUString& _rName, const rtl::OUString& _rModule)
        { m_sTemplateName = _rName; m_sTemplateModule = _rModule; }

        sal_Int32                   size() const { return m_aChanges.size(); }
        uno::Sequence< rtl::OUString >  elementNames() const;

        void                    addChange(std::auto_ptr<Change> aChange);
        ::std::auto_ptr<Change> removeChange(rtl::OUString const& _rName);

        Change* getChange(rtl::OUString const& _rName);
        Change const* getChange(rtl::OUString const& _rName) const;

        virtual void dispatch(ChangeTreeAction& _anAction) const;
        virtual void dispatch(ChangeTreeModification& _anAction);

        void forEachChange(ChangeTreeAction& _anAction) const;
        void forEachChange(ChangeTreeModification& _anAction);

        // "rtti"
        RTTI(SubtreeChange, Change);

    private:
        virtual Change* doGetChild(rtl::OUString const& _rName) const;
    };

    /** iterates through all children of a <type>SubtreeChange</type>. Every non-const action on the object
        which is beeing iterated invalidates the iterator.
        <BR>
        beware of the lifetime of the tree change object : it has to live as long as the iterator does (at least) !!
    */
    class SubtreeChange::ChildIterator
    {
    protected:
        uno::Sequence< rtl::OUString >  m_aNames;
        const SubtreeChange*            m_pTree;
        sal_Int32                       m_nPos;

        friend class SubtreeChange;
        struct EndPos { };
        ChildIterator(const SubtreeChange* _pTree, struct EndPos);

        inline sal_Bool isValid() const { return m_nPos >= 0 && m_nPos < m_aNames.getLength(); }

    public:
        ChildIterator(const SubtreeChange* _pTree);

        const Change& operator*() const;
        const Change* operator->() const;

        ChildIterator& operator++();
        ChildIterator operator++(int) { ChildIterator ret(*this); ++*this; return ret; }

        ChildIterator& operator--();
        ChildIterator operator--(int) { ChildIterator ret(*this); --*this; return ret; }

        friend bool operator==(ChildIterator const& lhs, ChildIterator const& rhs);
        friend bool operator!=(ChildIterator const& lhs, ChildIterator const& rhs) { return !(lhs == rhs); }
    };

    /** iterates through all children of a <type>SubtreeChange</type>. Every non-const action on the object
        which is beeing iterated invalidates the iterator.
        <BR>
        beware of the lifetime of the tree change object : it has to live as long as the iterator does (at least) !!
    */
    class SubtreeChange::MutatingChildIterator
    {
    protected:
        typedef SubtreeChange::Children::iterator Base;
        Base m_aBaseIter;

        friend class SubtreeChange;
        MutatingChildIterator(Base aBase) : m_aBaseIter(aBase) {};

    public:
        Change& current() const { return *m_aBaseIter->second; }

        Change& operator*() const { return current(); }
        Change* operator->() const { return &current(); }

        MutatingChildIterator& operator++()     { ++m_aBaseIter; return *this; }
        MutatingChildIterator operator++(int)   { return MutatingChildIterator(m_aBaseIter++); }

        MutatingChildIterator& operator--()     { ++m_aBaseIter; return *this; }
        MutatingChildIterator operator--(int)   { return MutatingChildIterator(m_aBaseIter++); }

        friend bool operator==(MutatingChildIterator const& lhs, MutatingChildIterator const& rhs)
        { return lhs.m_aBaseIter == rhs.m_aBaseIter; }
        friend bool operator!=(MutatingChildIterator const& lhs, MutatingChildIterator const& rhs) { return !(lhs == rhs); }
    };
    //==========================================================================
    //= SubtreeChangeReferrer
    //==========================================================================
    /** a specialized SubtreeChange, which, upon desctruction, does not delete the changes
        it holds

        <BR>
        This implies that when using this class, you have to beware of the lifetime of the involved objects
    */
    class SubtreeChangeReferrer : public SubtreeChange
    {
        // no explicit construction
        SubtreeChangeReferrer() : SubtreeChange(::rtl::OUString(), configuration::Attributes()) { }

    public:
        SubtreeChangeReferrer(const SubtreeChange& _rSource);
        ~SubtreeChangeReferrer();

        RTTI(SubtreeChangeReferrer, SubtreeChange);
    };

////////////////////////////////////////////////////////////////////////////////

    //==========================================================================
    //= inlines
    //==========================================================================
    inline void ChangeTreeAction::applyToChange(Change const& aChange)
    { aChange.dispatch(*this); }
    inline void ChangeTreeAction::applyToChildren(SubtreeChange const& aSubtree)
    { aSubtree.forEachChange(*this); }

    inline void ChangeTreeModification::applyToChange(Change& aChange)
    { aChange.dispatch(*this); }
    inline void ChangeTreeModification::applyToChildren(SubtreeChange& aSubtree)
    { aSubtree.forEachChange(*this); }

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

