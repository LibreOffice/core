/*************************************************************************
 *
 *  $RCSfile: treechangefactory.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-16 17:02:18 $
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
                                    uno::Any const& _aNewValue,
                                    configuration::Attributes _aAttrs,
                                    ValueChange::Mode _eMode,
                                    uno::Any _aOldValue
                                )
{
    return std::auto_ptr<ValueChange>(new ValueChange(_aName.toString(),_aNewValue,_aAttrs,_eMode,_aOldValue));
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
                                        Name const& _aName,
                                        Name const& _aElementTypeName)
{
    std::auto_ptr<SubtreeChange> pResult;

    if (_aElementTypeName.isEmpty())
    {
        pResult.reset( new SubtreeChange(_aName.toString(),configuration::Attributes()) );
    }
    else
    {
        pResult.reset( new SubtreeChange(_aName.toString(),
                                         _aElementTypeName.toString(),
                                         getDummySetElementModule().toString(),
                                         configuration::Attributes()) );
    }
    return pResult;
}

//-----------------------------------------------
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createGroupNodeChange(
                                Name const& _aName,
                                configuration::Attributes _aAttrs)
{
    return std::auto_ptr<SubtreeChange>(new SubtreeChange(_aName.toString(),_aAttrs));
}

//-----------------------------------------------
std::auto_ptr<SubtreeChange> OTreeChangeFactory::createSetNodeChange(
                                Name const& _aName,
                                Name const& _aTemplateName,
                                Name const& _aTemplateModule,
                                configuration::Attributes _aAttrs)
{
    return std::auto_ptr<SubtreeChange>(new SubtreeChange(_aName.toString(),
                                                        _aTemplateName.toString(),
                                                        _aTemplateModule.toString(),
                                                        _aAttrs));
}
//-----------------------------------------------

//= Set Changes ============================================================
std::auto_ptr<AddNode> OTreeChangeFactory::createAddNodeChange(
                                std::auto_ptr<INode> _aNewNode,
                                Name const& _aName)
{
    return std::auto_ptr<AddNode>(new AddNode(_aNewNode,_aName.toString()));
}

//-----------------------------------------------
std::auto_ptr<RemoveNode> OTreeChangeFactory::createRemoveNodeChange(
                                Name const& _aName)
{
    return std::auto_ptr<RemoveNode>(new RemoveNode(_aName.toString()));
}

//-----------------------------------------------

} // namespace configmgr


