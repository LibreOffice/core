/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treechangefactory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:58:59 $
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

#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#define CONFIGMGR_TREE_CHANGEFACTORY_HXX

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif


namespace configmgr
{

    namespace uno = ::com::sun::star::uno;

    //==========================================================================
    namespace configuration { class Name;}

    //==========================================================================

    class OTreeChangeFactory
    {
    public:
        typedef rtl::OUString Name;
    public:
    //= ValueChanges ============================================================
        std::auto_ptr<ValueChange> createValueChange(
                                        Name const& _aName,
                                        node::Attributes _aAttrs,
                                        ValueChange::Mode _eMode,
                                        uno::Any const& _aNewValue,
                                        uno::Any const& _aOldValue = uno::Any()
                                        );

        //-----------------------------------------------
        std::auto_ptr<ValueChange> createValueChange(
                                        ValueNode const& _aNewValue,
                                        bool _bWasDefault
                                        );

        //-----------------------------------------------
        std::auto_ptr<ValueChange> createValueChange(
                                        uno::Any const& _aNewValue,
                                        ValueNode const& _aOldValue
                                        );

        //-----------------------------------------------
        std::auto_ptr<ValueChange> createValueChange(
                                        ValueChange::SetToDefault,
                                        ValueNode const& _aOldValue
                                        );

    //= SubtreeChanges ============================================================
        std::auto_ptr<SubtreeChange> createGroupNodeChange(
                                        Name const& _aName,
                                        node::Attributes _aAttrs,
                                        bool _bToDefault = false);

        //-----------------------------------------------
        std::auto_ptr<SubtreeChange> createSetNodeChange(
                                        Name const& _aName,
                                        Name const& _aTemplateName,
                                        Name const& _aTemplateModule,
                                        node::Attributes _aAttrs,
                                        bool _bToDefault = false);
        //-----------------------------------------------

    //= Set Changes ============================================================
        std::auto_ptr<AddNode>      createAddNodeChange(
                                        data::TreeSegment const & _aNewTree,
                                        Name const& _aName,
                                        bool _bToDefault = false);

        //-----------------------------------------------
        std::auto_ptr<RemoveNode>   createRemoveNodeChange(
                                        Name const& _aName,
                                        bool _bToDefault = false);

    //= special case: Dummy ISubtree ============================================================
        static std::auto_ptr<SubtreeChange> createDummyChange(
                                                configuration::Name const& _aName,
                                                configuration::Name const& _aElementTypeName);

    //-----------------------------------------------
    };

    OTreeChangeFactory& getDefaultTreeChangeFactory();
    //==========================================================================
    inline
    std::auto_ptr<Change> base_ptr(std::auto_ptr<SubtreeChange> pChange)
    { return std::auto_ptr<Change>(pChange.release()); }
    //==========================================================================
    inline
    std::auto_ptr<Change> base_ptr(std::auto_ptr<ValueChange> pChange)
    { return std::auto_ptr<Change>(pChange.release()); }
    //==========================================================================
    inline
    std::auto_ptr<Change> base_ptr(std::auto_ptr<AddNode> pChange)
    { return std::auto_ptr<Change>(pChange.release()); }
    //==========================================================================
    inline
    std::auto_ptr<Change> base_ptr(std::auto_ptr<RemoveNode> pChange)
    { return std::auto_ptr<Change>(pChange.release()); }
    //==========================================================================

    bool isGenericSetElementType(OUString const& _aElementType);
    bool isDummySetElementModule(OUString const& _aElementModule);
    configuration::Name getGenericSetElementType();
    configuration::Name getDummySetElementModule();
    //==========================================================================

} // namespace configmgr

#endif
