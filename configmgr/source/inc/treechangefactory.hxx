/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treechangefactory.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#define CONFIGMGR_TREE_CHANGEFACTORY_HXX

#include "change.hxx"

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
