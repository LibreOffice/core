/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treechangefactory.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <stdio.h>

#include "treechangefactory.hxx"
#include "change.hxx"
#include "configpath.hxx"

namespace configmgr
{
//= dummy helpe ============================================================
bool isGenericSetElementType(rtl::OUString const& _aElementType)
{
    return !! _aElementType.equals( getGenericSetElementType() );
}

bool isDummySetElementModule(rtl::OUString const& _aElementModule)
{
    return !! _aElementModule.equals( getDummySetElementModule() );
}

rtl::OUString getGenericSetElementType()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
}

rtl::OUString getDummySetElementModule()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cfg:dummy-change"));
}

//= static default ============================================================
OTreeChangeFactory& getDefaultTreeChangeFactory()
{
    static OTreeChangeFactory aDefaultFactory;
    return aDefaultFactory;
}

//= SubtreeChanges ============================================================
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createDummyChange(
    rtl::OUString const& _aName, rtl::OUString const& _aElementTypeName)
{
    std::auto_ptr<SubtreeChange> pResult;

    if (_aElementTypeName.getLength() == 0)
    {
        pResult.reset( new SubtreeChange(_aName, node::Attributes()) );
    }
    else
    {
        pResult.reset( new SubtreeChange(_aName,
                                         _aElementTypeName,
                                         getDummySetElementModule(),
                                         node::Attributes()) );
    }
    return pResult;
}

//-----------------------------------------------
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createSetNodeChange(
                                rtl::OUString const& _aName,
                                rtl::OUString const& _aTemplateName,
                                rtl::OUString const& _aTemplateModule,
                                node::Attributes _aAttrs,
                                bool _bToDefault)
{
    return std::auto_ptr<SubtreeChange>(new SubtreeChange(_aName,
                                                        _aTemplateName,
                                                        _aTemplateModule,
                                                        _aAttrs,_bToDefault));
}
//-----------------------------------------------

//= Set Changes ============================================================
std::auto_ptr<AddNode> OTreeChangeFactory::createAddNodeChange(
    rtl::Reference< data::TreeSegment > const & _aNewTree,
                                rtl::OUString const& _aName,
                                bool _bToDefault)
{
    return std::auto_ptr<AddNode>(new AddNode(_aNewTree,_aName,_bToDefault));
}

//-----------------------------------------------
std::auto_ptr<RemoveNode> OTreeChangeFactory::createRemoveNodeChange(
                                rtl::OUString const& _aName,
                                bool _bToDefault)
{
    return std::auto_ptr<RemoveNode>(new RemoveNode(_aName,_bToDefault));
}

//-----------------------------------------------

} // namespace configmgr


