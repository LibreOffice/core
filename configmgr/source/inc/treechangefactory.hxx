/*************************************************************************
 *
 *  $RCSfile: treechangefactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:08 $
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
