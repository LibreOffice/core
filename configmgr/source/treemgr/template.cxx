/*************************************************************************
 *
 *  $RCSfile: template.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-12 14:59:05 $
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

#include "template.hxx"
#include "templateimpl.hxx"

#include "valuenode.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// class TemplateProvider
//-----------------------------------------------------------------------------

TemplateProvider::TemplateProvider()
: m_aImpl()
{
}

//-----------------------------------------------------------------------------
TemplateProvider::TemplateProvider(ITemplateProvider& rProvider)
: m_aImpl(new TemplateProvider_Impl(&rProvider))
{
}

//-----------------------------------------------------------------------------
TemplateProvider::TemplateProvider(TemplateProvider const& aOther)
: m_aImpl(aOther.m_aImpl)
{
}
//-----------------------------------------------------------------------------
TemplateProvider::~TemplateProvider()
{
}


//-----------------------------------------------------------------------------
// class Template
//-----------------------------------------------------------------------------

Template::Template(Name const& aName, Name const& aModule,UnoType const& aType, Attributes const& aAttrs)
: m_aName(aName)
, m_aModule(aModule)
, m_aInstanceType(aType)
, m_aAttributes(aAttrs)
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
RelativePath Template::getPath() const
{
    TemplateName aNames(m_aName,m_aModule);
    return aNames.makePath( );
}
//-----------------------------------------------------------------------------

#if 0
TemplateHolder Template::fromPath(OUString const& sPath, TemplateProvider const& aProvider)
{
    TemplateName aNames( TemplateName::parseTemplatePath(sPath) );
    return TemplateImplHelper::findTemplate(aNames, aProvider );
}
#endif
//-----------------------------------------------------------------------------
TemplateHolder Template::fromNames(OUString const& sName, OUString const& sModule, TemplateProvider const& aProvider)
{
    TemplateName aNames( TemplateName::parseTemplateNames(sName,sModule) );
    return TemplateImplHelper::findTemplate(aNames, aProvider );
}
//-----------------------------------------------------------------------------
TemplateHolder locate(Name const& aName, Name const& aModule, TemplateProvider const& aProvider)
{
    TemplateName aNames(aName,aModule);
    return TemplateImplHelper::findTemplate(aNames, aProvider );
}
//-----------------------------------------------------------------------------

TemplateHolder makeSimpleTemplate(UnoType const& aType, Attributes const& aAttrs, TemplateProvider const& aProvider)
{
    TemplateName aNames(aType);
    return TemplateImplHelper::makeTemplate( aNames, aProvider, aType, aAttrs);
}
//-----------------------------------------------------------------------------
#if 0
TemplateHolder makeTreeTemplate(OUString const& sPath, OUString const& sModule, TemplateProvider const& aProvider)
{
    TemplateName aNames( TemplateName::parseTemplatePath(sName) );
    return TemplateImplHelper::makeTemplate( aNames,aProvider, TemplateImplHelper::getUnoInterfaceType(), Attributes());
}
#endif
//-----------------------------------------------------------------------------

TemplateHolder makeTreeTemplate(OUString const& sName, OUString const& sModule, TemplateProvider const& aProvider)
{
    TemplateName aNames( TemplateName::parseTemplateNames(sName,sModule) );
    return TemplateImplHelper::makeTemplate( aNames,aProvider, TemplateImplHelper::getUnoInterfaceType(), Attributes());
}
//-----------------------------------------------------------------------------

TemplateHolder makeSetElementTemplate(ISubtree const& aSet, TemplateProvider const& aProvider)
{
    TemplateName aNames( TemplateName::parseTemplateNames( aSet.getElementTemplateName(), aSet.getElementTemplateModule() ) );
    return TemplateImplHelper::makeElementTemplateWithType(aNames, aProvider, aSet);
}
//-----------------------------------------------------------------------------
    }
}

