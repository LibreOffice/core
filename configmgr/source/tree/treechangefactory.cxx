/*************************************************************************
 *
 *  $RCSfile: treechangefactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:38 $
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

#include <stdio.h>

#include "treechangefactory.hxx"

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
//= dummy helpe ============================================================
bool isGenericSetElementType(OUString const& _aElementType)
{
    return !! _aElementType.equals( getGenericSetElementType().toString() );
}

bool isDummySetElementModule(OUString const& _aElementModule)
{
    return !! _aElementModule.equals( getDummySetElementModule().toString() );
}

configuration::Name getGenericSetElementType()
{
    using namespace configuration;
    static const Name c_aGenericTypeName =
        makeName( OUString(RTL_CONSTASCII_USTRINGPARAM("*")), Name::NoValidate() );

    return c_aGenericTypeName;
}

configuration::Name getDummySetElementModule()
{
    using namespace configuration;
    static const Name c_aDummyModuleName =
        makeName( OUString(RTL_CONSTASCII_USTRINGPARAM("cfg:dummy-change")), Name::NoValidate() );

    return c_aDummyModuleName;
}

//= static default ============================================================
OTreeChangeFactory& getDefaultTreeChangeFactory()
{
    static OTreeChangeFactory aDefaultFactory;
    return aDefaultFactory;
}

//= ValueNodes ============================================================
std::auto_ptr<ValueChange> OTreeChangeFactory::createValueChange(
                                    Name const& _aName,
                                    node::Attributes _aAttrs,
                                    ValueChange::Mode _eMode,
                                    uno::Any const& _aNewValue,
                                    uno::Any const& _aOldValue
                                )
{
    return std::auto_ptr<ValueChange>(new ValueChange(_aName,_aAttrs,_eMode,_aNewValue,_aOldValue));
}

//-----------------------------------------------
std::auto_ptr<ValueChange> OTreeChangeFactory::createValueChange(ValueNode const& _aNewValue, bool _bWasDefault)
{
    Name aName              = _aNewValue.getName();
    uno::Any aValue         = _aNewValue.getValue();
    node::Attributes aAttrs = _aNewValue.getAttributes();

    ValueChange::Mode eMode = aAttrs.isDefault() ?
                                    _bWasDefault ? ValueChange::changeDefault   : ValueChange:: setToDefault:
                                    _bWasDefault ? ValueChange::wasDefault      : ValueChange::changeValue;

    if (aValue.hasValue())
    {
        return std::auto_ptr<ValueChange>(new ValueChange(aName,aAttrs,eMode,aValue));
    }
    else
    {
        return std::auto_ptr<ValueChange>(new ValueChange(aName,aAttrs,eMode,_aNewValue.getValueType()));
    }
}

//-----------------------------------------------
std::auto_ptr<ValueChange> OTreeChangeFactory::createValueChange(
                                uno::Any const& _aNewValue,
                                ValueNode const& _aOldValue
                                )
{
    return std::auto_ptr<ValueChange>(new ValueChange(_aNewValue,_aOldValue));
}

//-----------------------------------------------
std::auto_ptr<ValueChange> OTreeChangeFactory::createValueChange(
                                ValueChange::SetToDefault _aSetToDefault,
                                ValueNode const& _aOldValue
                                )
{
    return std::auto_ptr<ValueChange>(new ValueChange(_aSetToDefault,_aOldValue));
}

//= SubtreeChanges ============================================================
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createDummyChange(
                                    configuration::Name const& _aName,
                                    configuration::Name const& _aElementTypeName)
{
    std::auto_ptr<SubtreeChange> pResult;

    if (_aElementTypeName.isEmpty())
    {
        pResult.reset( new SubtreeChange(_aName.toString(),node::Attributes()) );
    }
    else
    {
        pResult.reset( new SubtreeChange(_aName.toString(),
                                         _aElementTypeName.toString(),
                                         getDummySetElementModule().toString(),
                                         node::Attributes()) );
    }
    return pResult;
}

//-----------------------------------------------
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createGroupNodeChange(
                                Name const& _aName,
                                node::Attributes _aAttrs,
                                bool _bToDefault)
{
    return std::auto_ptr<SubtreeChange>(new SubtreeChange(_aName,_aAttrs,_bToDefault));
}

//-----------------------------------------------
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createSetNodeChange(
                                Name const& _aName,
                                Name const& _aTemplateName,
                                Name const& _aTemplateModule,
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
                                data::TreeSegment const & _aNewTree,
                                Name const& _aName,
                                bool _bToDefault)
{
    return std::auto_ptr<AddNode>(new AddNode(_aNewTree,_aName,_bToDefault));
}

//-----------------------------------------------
std::auto_ptr<RemoveNode> OTreeChangeFactory::createRemoveNodeChange(
                                Name const& _aName,
                                bool _bToDefault)
{
    return std::auto_ptr<RemoveNode>(new RemoveNode(_aName,_bToDefault));
}

//-----------------------------------------------

} // namespace configmgr


