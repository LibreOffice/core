/*************************************************************************
 *
 *  $RCSfile: template.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:35:59 $
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

#include <vos/refernce.hxx>
#include <stl/map>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

        class Name;
        class AbsolutePath;
        //---------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;

//-----------------------------------------------------------------------------
class TemplateHelper
{
public:
    //-----------------------------------------------------------------------------
    static Name makeSimpleTypeName(UnoType const& aType)
    {
        OUString aTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("uno:")).concat(aType.getTypeName());
        return Name(aTypeName, Name::NoValidate());
    }
    //-----------------------------------------------------------------------------
    static Name makeSimpleTypeModuleName()
    {
        OUString aModuleName = OUString(RTL_CONSTASCII_USTRINGPARAM("cfg:native-types"));
        return Name(aModuleName, Name::NoValidate());
    }
    //-----------------------------------------------------------------------------

    static UnoType getUnoInterfaceType()
    {
        uno::Reference<uno::XInterface> const * const p = 0;
        return getCppuType(p);
    }
    //-----------------------------------------------------------------------------

    struct Names
    {
        Name aName, aModule;

        Names(UnoType const& aType)
        : aName(makeSimpleTypeName(aType))
        , aModule(makeSimpleTypeModuleName())
        {}

        Names(Name const& aName_)
        : aName(aName_)
        , aModule()
        {}

        Names(Name const& aName_, Name const& aModule_)
        : aName(aName_)
        , aModule(aModule_)
        {}

        Names(OUString const& sName_, OUString const& sModule_)
        : aName(sName_, Name::NoValidate())
        , aModule(sModule_, Name::NoValidate())
        {}

        bool operator<(Names const& aOther) const
        {
            return (aModule == aOther.aModule) ? (aName < aOther.aName) : (aModule < aOther.aModule);
        }
    };

    typedef std::map<Names, TemplateHolder> Repository;
    typedef Repository::value_type Entry;
    //-----------------------------------------------------------------------------
    static Repository& repository();

    //-----------------------------------------------------------------------------
    static TemplateHolder createNew(Name const& aName, Name const& aModule,UnoType const& aType)
    {
        return new Template(aName,aModule,aType);
    }
    //-----------------------------------------------------------------------------
    static TemplateHolder createNew (Names const& aNames,UnoType const& aType)
    {
        return createNew(aNames.aName, aNames.aModule, aType);
    }
    //-----------------------------------------------------------------------------
    static TemplateHolder findTemplate (Names const& aNames,UnoType const& aType);
    //-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
TemplateHolder TemplateHelper::findTemplate (Names const& aNames,UnoType const& aType)
{
    Repository& rep = repository();

    Repository::iterator it = rep.find(aNames);
    if (it == rep.end())
        it = rep.insert( Entry(aNames,createNew(aNames,aType)) ).first;

    OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    return it->second;
}
//-----------------------------------------------------------------------------

TemplateHelper::Repository& TemplateHelper::repository()
{
    static Repository aTheRepository;
    return aTheRepository;
}
//-----------------------------------------------------------------------------

Template::Template(Name const& aName, Name const& aModule,UnoType const& aType)
: m_aName(aName)
, m_aModule(aModule)
, m_aInstanceType(aType)
{
}
//-----------------------------------------------------------------------------

/// checks if this is a 'value' template
bool Template::isInstanceValue() const
{
    return m_aInstanceType.getTypeClass() != uno::TypeClass_INTERFACE;
}
//-----------------------------------------------------------------------------

/// get the path where the template is located
AbsolutePath Template::getPath() const
{
    Path::Components aPath;
    aPath.push_back(Name());
    aPath.push_back(m_aModule);
    aPath.push_back(m_aName);
    return AbsolutePath( aPath  );
}
//-----------------------------------------------------------------------------

TemplateHolder Template::fromPath(OUString const& sName)
{
    Path::Components aPath = Path::parse(sName);
    if (aPath.empty())
        return 0;

    TemplateHelper::Names aNames( aPath.back() );

    switch(aPath.size())
    {
    case 1: break;
    case 2: aNames.aModule = aPath.front(); break;

    case 3: if (aPath[0].isEmpty())
            {
                aNames.aModule = aPath[1];
                break;
            }
            // fall through
    default:
            OSL_ENSURE(false, "Invalid template path - too many components");
            // hack - cram it all into the module part
            aPath.pop_back();
            aNames.aModule = Name( PathRep(aPath).toString(), Name::NoValidate() );
            break;
    }
    return TemplateHelper::findTemplate(aNames, TemplateHelper::getUnoInterfaceType() );
}
//-----------------------------------------------------------------------------
TemplateHolder locate(Name const& aName, Name const& aModule)
{
    return TemplateHelper::findTemplate(TemplateHelper::Names(aName,aModule),
                                        TemplateHelper::getUnoInterfaceType() );
}
//-----------------------------------------------------------------------------

TemplateHolder makeSimpleTemplate(UnoType const& aType)
{
    return TemplateHelper::findTemplate( TemplateHelper::Names(aType), aType);
}
//-----------------------------------------------------------------------------
    }
}

