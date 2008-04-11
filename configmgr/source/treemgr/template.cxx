/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: template.cxx,v $
 * $Revision: 1.17 $
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

#include "template.hxx"
#include "templateimpl.hxx"
#include "setnodeaccess.hxx"
#include "apitypes.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// class TemplateProvider
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
TemplateProvider::TemplateProvider()
: m_aImpl()
{
}

//-----------------------------------------------------------------------------
TemplateProvider::TemplateProvider(TemplateManagerRef const & xProvider, RequestOptions const& aOptions)
: m_aImpl( new TemplateProvider_Impl(xProvider,aOptions) )
{
}

//-----------------------------------------------------------------------------
TemplateProvider::TemplateProvider(TemplateProvider const& aOther)
: m_aImpl(aOther.m_aImpl)
{
}

//-----------------------------------------------------------------------------
TemplateProvider& TemplateProvider::operator =(TemplateProvider const& aOther)
{
    m_aImpl = aOther.m_aImpl;
    return *this;
}

//-----------------------------------------------------------------------------
TemplateProvider::~TemplateProvider()
{
}

//-----------------------------------------------------------------------------
// class SpecialTemplateProvider
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
SpecialTemplateProvider::SpecialTemplateProvider()
: m_aImpl( new SpecialTemplateProvider_Impl() )
{
}

//-----------------------------------------------------------------------------
SpecialTemplateProvider::SpecialTemplateProvider(SpecialTemplateProvider const& aOther)
: m_aImpl(aOther.m_aImpl)
{
}

//-----------------------------------------------------------------------------
SpecialTemplateProvider& SpecialTemplateProvider::operator =(SpecialTemplateProvider const& aOther)
{
    m_aImpl = aOther.m_aImpl;
    return *this;
}

//-----------------------------------------------------------------------------
SpecialTemplateProvider::~SpecialTemplateProvider()
{
}

//-----------------------------------------------------------------------------
// class Template
//-----------------------------------------------------------------------------

Template::Template(Name const& aName, Name const& aModule,UnoType const& aType)
: m_aName(aName)
, m_aModule(aModule)
, m_aInstanceType(aType)
{
}
//-----------------------------------------------------------------------------

bool Template::isInstanceTypeKnown() const
{
    OSL_ASSERT( TemplateImplHelper::getNoTypeAvailable().getTypeClass() == uno::TypeClass_VOID );
    return m_aInstanceType.getTypeClass() != uno::TypeClass_VOID;
}
//-----------------------------------------------------------------------------

/// checks if this is a 'value' template
bool Template::isInstanceValue() const
{
    OSL_ENSURE( isInstanceTypeKnown(), "Template instance type unknown - cannot determine kind");
    return m_aInstanceType.getTypeClass() != uno::TypeClass_INTERFACE;
}
//-----------------------------------------------------------------------------

UnoType Template::getInstanceType() const
{
    OSL_ENSURE( isInstanceTypeKnown(), "Template instance type unknown - returning invalid (VOID) type");
    return m_aInstanceType;
}
//-----------------------------------------------------------------------------

/// get the path where the template is located
OUString Template::getPathString() const
{
    TemplateName aNames(m_aName,m_aModule);
    return aNames.makePathString( );
}
//-----------------------------------------------------------------------------

TemplateHolder makeSimpleTemplate(UnoType const& aType, SpecialTemplateProvider const& aProvider)
{
    TemplateName aNames(aType,false);
    return TemplateImplHelper::makeSpecialTemplate( aNames, aProvider, aType);
}
//-----------------------------------------------------------------------------

TemplateHolder makeTreeTemplate(OUString const& sName, OUString const& sModule, SpecialTemplateProvider const& aProvider)
{
    TemplateName aNames( sName,sModule );
    return TemplateImplHelper::makeSpecialTemplate( aNames,aProvider, configapi::getUnoInterfaceType());
}
//-----------------------------------------------------------------------------

TemplateHolder makeSetElementTemplate(data::SetNodeAccess const& _aSet, TemplateProvider const& _aProvider)
{
    TemplateName aNames( _aSet.getElementTemplateName(), _aSet.getElementTemplateModule() );
    return TemplateImplHelper::makeElementTemplateWithType(aNames, _aProvider, _aSet);
}
//-----------------------------------------------------------------------------
    }
}

