/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: template.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:34:13 $
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

#include "template.hxx"
#include "templateimpl.hxx"

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_API_APITYPES_HXX_
#include "apitypes.hxx"
#endif

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

