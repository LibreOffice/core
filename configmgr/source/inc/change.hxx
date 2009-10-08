/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: change.hxx,v $
 * $Revision: 1.28 $
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

#ifndef CONFIGMGR_CHANGE_HXX
#define CONFIGMGR_CHANGE_HXX
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable : 4350) // behavior change: 'member1' called instead of 'member2'
#endif

#include "valuenode.hxx"
#include "treesegment.hxx"
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

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
    protected:
        virtual ~ChangeTreeAction() {}
    };

    struct ChangeTreeModification
    {
        virtual void handle(ValueChange& aValueNode) = 0;
        virtual void handle(AddNode& aAddNode) = 0;
        virtual void handle(RemoveNode& aRemoveNode) = 0;
        virtual void handle(SubtreeChange& aSubtree) = 0;

        void applyToChange(Change& aChange);
        void applyToChildren(SubtreeChange& aSubtree);
    protected:
        virtual ~ChangeTreeModification() {}
    };

    //==========================================================================
    //= Change
    //==========================================================================
    class Change
    {
    protected:
        rtl::OUString m_aName;
        bool m_bIsToDefault;

        void swap(Change& aOther);

    public:
        explicit
        Change(rtl::OUString const& _rName, bool _bToDefault)
        : m_aName(_rName)
        , m_bIsToDefault(_bToDefault)
        {}

        virtual ~Change() {}

        rtl::OUString getNodeName() const { return m_aName; }
        void setNodeName(const rtl::OUString &aName) {m_aName = aName;}

        bool isToDefault() const { return m_bIsToDefault;  }

        Change* getSubChange(rtl::OUString const& _rName) { return doGetChild(_rName); }
        Change const* getSubChange(rtl::OUString const& _rName) const { return doGetChild(_rName); }

        virtual void dispatch(ChangeTreeAction& anAction) const = 0;
        virtual void dispatch(ChangeTreeModification& anAction) = 0;

        virtual std::auto_ptr<Change> clone() const = 0;

    private:
        virtual Change* doGetChild(rtl::OUString const& ) const { return 0; }
    };

    //==========================================================================
    //= ValueChange
    //==========================================================================
    class ValueChange : public Change
    {
    public:
        struct SetToDefault {};
        enum Mode { wasDefault, changeValue, setToDefault, changeDefault };

    private:
        uno::Type           m_aValueType;
        uno::Any            m_aValue;
        uno::Any            m_aOldValue;
        node::Attributes    m_aAttributes;
        Mode                m_eMode;
    public:
        ValueChange(
            rtl::OUString const& _rName,
            const node::Attributes& _rAttributes,
            Mode aMode,
            uno::Any const & aNewValue,
            uno::Any const & aOldValue = uno::Any());

        virtual std::auto_ptr<Change> clone() const;

        bool isChange() const;

        uno::Type   getValueType() const { return m_aValueType; }

        uno::Any    getNewValue() const { return m_aValue; }
        uno::Any    getOldValue() const { return m_aOldValue; }

        void        setNewValue(const uno::Any& _rNewVal);
        void        setNewValue(const uno::Any& _rNewVal, Mode aMode)
        { setNewValue(_rNewVal); m_eMode = aMode;}

        bool isReplacedValue()   const {return m_aAttributes.isReplacedForUser();}
        bool isLocalizedValue()  const {return m_aAttributes.isLocalized();}

        Mode getMode() const { return m_eMode; }

        const node::Attributes& getAttributes() const {return m_aAttributes;}

        void applyChangeNoRecover(ValueNode& aNode) const;

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }

        friend class ApplyValueChange;
    };

    //==========================================================================
    //= AddNode
    //==========================================================================
    class AddNode : public Change
    {
        rtl::Reference< data::TreeSegment > m_aOwnNewNode;
        rtl::Reference< data::TreeSegment > m_aOwnOldNode;
        sharable::TreeFragment *    m_aInsertedTree;
        bool                m_bReplacing;

    private:
        void operator=(AddNode const&); // not implemented

        // needed for clone()
        AddNode(AddNode const&);
    public:
        AddNode(rtl::Reference< data::TreeSegment > const & _aAddedTree, rtl::OUString const& _rName, bool _bToDefault);
        ~AddNode();

        virtual std::auto_ptr<Change> clone() const;

        /// marks this as not merely adding a node but replacing another
        void setReplacing() { m_bReplacing = true; }
        /// is this  not merely adding a node but replacing another ?
        bool isReplacing() const { return m_bReplacing; }

        /// has this been applied and inserted
        bool wasInserted() const { return m_aInsertedTree != NULL; }

        /** returns the node this change represents, even if this node does not own the new Node object any more.
            This is somewhat dangerous if the node referenced by this object dies before the object itself does.<BR>
            In this case all calls to this method will return nonsense. This case can be detected by testing
            whether <method>getAddedNode</method> returns NULL.
        */
        sharable::TreeFragment *   getInsertedTree()    const { return m_aInsertedTree; }

        /** returns the node this change represents; The Node object is owned by this change until
            <method>releaseAddedNode</method> is called.<BR>
            After ownership is lost this method returns NULL.
        */
        sharable::TreeFragment const * getNewTreeData() const  { return m_aOwnNewNode.is() ? m_aOwnNewNode->fragment : 0; }

        /** returns the node the change represents. .
        */
        rtl::Reference< data::TreeSegment > getNewTree() const   { return m_aOwnNewNode; }

        /** returns the node the change represents, and releases ownership of it. This means that
            afterwards <method>getAddedNode</method> will return NULL. This change object keeps a reference
            to the node though which can be retrieved using <method>getAddedNode_unsafe</method>.
        */
        void clearNewTree() { m_aOwnNewNode.clear(); }

        /** .
        */
        void setInsertedAddress(sharable::TreeFragment * const & _aInsertedAddress);


        /** returns the node this change replaces, ihe Node object is owned by this change.
            After ownership is lost this method returns NULL.
        */
        sharable::TreeFragment const * getReplacedTreeData() const  { return m_aOwnOldNode.is() ? m_aOwnOldNode->fragment : 0; }

        /** returns the node the change replaces.
        */
        rtl::Reference< data::TreeSegment > getReplacedTree() const { return m_aOwnOldNode; }

        /** forgets about the node the change replaces
        */
        void clearReplacedTree() { m_aOwnOldNode.clear(); }

        void takeReplacedTree(rtl::Reference< data::TreeSegment > const& _aTree);

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }
    };

    //==========================================================================
    //= RemoveNode
    //==========================================================================
    class RemoveNode : public Change
    {
    protected:
        rtl::Reference< data::TreeSegment > m_aOwnOldNode;
        bool                            m_bIsToDefault;

    private:
        RemoveNode& operator=(const RemoveNode&); // not implemented
        // needed for clone()
        RemoveNode(const RemoveNode&);
    public:
        explicit
        RemoveNode(rtl::OUString const& _rName, bool _bToDefault);
        ~RemoveNode();

        virtual std::auto_ptr<Change> clone() const;

        virtual void dispatch(ChangeTreeAction& anAction) const { anAction.handle(*this); }
        virtual void dispatch(ChangeTreeModification& anAction) { anAction.handle(*this); }

        /** returns the node this change removes, ihe Node object is owned by this change.
            After ownership is lost this method returns NULL.
        */
        sharable::TreeFragment const * getRemovedTreeData() const { return m_aOwnOldNode.is() ? m_aOwnOldNode->fragment : 0; }
        /** returns the node the change removes.
        */
        rtl::Reference< data::TreeSegment > getRemovedTree()            const   { return m_aOwnOldNode; }

        /** forgets about the node the change removes, returning the previous setting with ownership
        */
        void clearRemovedTree() { m_aOwnOldNode.clear(); }

        void takeRemovedTree(rtl::Reference< data::TreeSegment > const & _aTree);
    };

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
        node::Attributes            m_aAttributes;

        // don't create copy ops automatically
        SubtreeChange(const SubtreeChange&);
        void operator=(SubtreeChange&);

              using Change::swap;

    public:
        class ChildIterator;
        ChildIterator begin() const throw();
        ChildIterator end() const throw();

        class MutatingChildIterator;
        MutatingChildIterator begin_changes() throw();
        MutatingChildIterator end_changes() throw();

        friend class MutatingChildIterator;
    public:
        SubtreeChange(const rtl::OUString& _rName,
                      const node::Attributes& _rAttr,
                      bool _bToDefault = false)
        : Change(_rName,_bToDefault)
        , m_aAttributes(_rAttr)
        {
            m_aAttributes.markAsDefault(_bToDefault);
        }

        SubtreeChange(const rtl::OUString& _rName,
                      const rtl::OUString& _rTemplateName,
                      const rtl::OUString& _rTemplateModule,
                      const node::Attributes& _rAttr,
                      bool _bToDefault = false)
        : Change(_rName,_bToDefault)
        , m_sTemplateName(_rTemplateName)
        , m_sTemplateModule(_rTemplateModule)
        , m_aAttributes(_rAttr)
        {
            m_aAttributes.markAsDefault(_bToDefault);
        }

        SubtreeChange(const ISubtree& _rTree, bool _bToDefault = false)
        : Change(_rTree.getName(),_bToDefault)
        , m_sTemplateName(_rTree.getElementTemplateName())
        , m_sTemplateModule(_rTree.getElementTemplateModule())
        , m_aAttributes(_rTree.getAttributes())
        {
            m_aAttributes.markAsDefault(_bToDefault);
        }

        SubtreeChange(const SubtreeChange& _rChange, treeop::NoChildCopy)
        : Change(_rChange)
        , m_sTemplateName(_rChange.getElementTemplateName())
        , m_sTemplateModule(_rChange.getElementTemplateModule())
        , m_aAttributes(_rChange.getAttributes())
        {}

        ~SubtreeChange();

        SubtreeChange(const SubtreeChange&, treeop::DeepChildCopy);

        virtual std::auto_ptr<Change> clone() const;

        void swap(SubtreeChange& aOther);

        bool isReplacedNode()       const { return m_aAttributes.isReplacedForUser();  }
        bool isLocalizedContainer() const { return m_aAttributes.isLocalized(); }

        const node::Attributes& getAttributes() const {return m_aAttributes;}

        bool            isSetNodeChange() const { return m_sTemplateName.getLength() != 0; }

        rtl::OUString   getElementTemplateName() const { return m_sTemplateName; }
        rtl::OUString   getElementTemplateModule() const { return m_sTemplateModule; }

        void            setElementTemplate(const rtl::OUString& _rName, const rtl::OUString& _rModule)
        { m_sTemplateName = _rName; m_sTemplateModule = _rModule; }

        sal_Int32                       size() const { return m_aChanges.size(); }
        uno::Sequence< rtl::OUString >  elementNames() const;

        void                    addChange(std::auto_ptr<Change> aChange);
        ::std::auto_ptr<Change> removeChange(rtl::OUString const& _rName);

        Change* getChange(rtl::OUString const& _rName);
        Change const* getChange(rtl::OUString const& _rName) const;

        virtual void dispatch(ChangeTreeAction& _anAction) const;
        virtual void dispatch(ChangeTreeModification& _anAction);

        void forEachChange(ChangeTreeAction& _anAction) const;
        void forEachChange(ChangeTreeModification& _anAction);

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
        SubtreeChange::Children::iterator m_aBaseIter;

        friend class SubtreeChange;
        MutatingChildIterator(SubtreeChange::Children::iterator aBase) : m_aBaseIter(aBase) {};

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
        SubtreeChangeReferrer() : SubtreeChange(::rtl::OUString(), node::Attributes()) { }

    public:
        SubtreeChangeReferrer(const SubtreeChange& _rSource);
        ~SubtreeChangeReferrer();
    };

////////////////////////////////////////////////////////////////////////////////
    //==========================================================================
    extern bool isLocalizedValueSet(SubtreeChange const& _aSubtree);
    extern bool isValueSet(SubtreeChange const& _aSubtree);

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

