/*************************************************************************
 *
 *  $RCSfile: xmltree.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: np $Date:  $
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


#include <xmltree.hxx>


// NOT FULLY DEFINED SERVICES
#include <cr_html.hxx>
#include <cr_index.hxx>


char  C_sMODULEDESCRIPTION[] = "module-description";
char  C_sCOMPONENTDESCRIPTION[] = "component-description";
char  C_sAuthor[] = "author";
char  C_sName[] = "name";
char  C_sDescription[] = "description";
char  C_sReferenceDocu[] = "reference-docu";
char  C_sModuleName[] = "module-name";
char  C_sLoaderName[] = "loader-name";
char  C_sSupportedService[] = "supported-service";
char  C_sServiceDependency[] = "service-dependency";
char  C_sProjectBuildDependency[] = "project-build-dependency";
char  C_sRuntimeModuleDependency[] = "runtime-module-dependency";
char  C_sLanguage[] = "language";
char  C_sStatus[] = "status";
char  C_sType[] = "type";
char  C_sAttr_value[] = "value";
char  C_sAttr_xl_href[] = "xlink:href";
char  C_sAttr_xl_role[] = "xlink:role";
char  C_sAttr_xl_title[] = "xlink:title";
char  C_sAttr_xmlns[] = "xmlns:xlink";
char  C_sAttr_xl_type[] = "xlink:type";
char  C_sCompDescrListTitle[] = "Component Descriptions";


FreeChoiceElement * Create_ModuleDescrOptional_Element(
                        MultipleTextElement * &
                                            o_rTypes,
                        MultipleTextElement * &
                                            o_rServiceDependencies );
FreeChoiceElement * Create_CompDescrOptional_Element(
                        MultipleTextElement * &
                                            o_rTypes,
                        MultipleTextElement * &
                                            o_rServiceDependencies );



ModuleDescription::ModuleDescription()
    :   SequenceElement(C_sMODULEDESCRIPTION),
        pModuleName(0),
        pCdList(0),
        pTypes(0),
        pServiceDependencies(0)
{
    pModuleName = new MdName;
    AddChild( *pModuleName );
    pCdList = new CompDescrList;
    AddChild( *pCdList );
    AddChild( *Create_ModuleDescrOptional_Element( pTypes, pServiceDependencies ) );
}

const Simstr &
ModuleDescription::ModuleName() const
{
    return pModuleName->Data();
}

void
ModuleDescription::Get_SupportedServices( List< const MultipleTextElement * > & o_rServices ) const
{
    o_rServices.push_back(pServiceDependencies);
    pCdList->Get_SupportedServices( o_rServices );
}

void
ModuleDescription::Get_Types( List< const MultipleTextElement * > & o_rTypes ) const
{
    o_rTypes.push_back(pTypes);
    pCdList->Get_Types( o_rTypes );
}

void
ModuleDescription::Get_ServiceDependencies( List< const MultipleTextElement * > & o_rServices ) const
{
    pCdList->Get_ServiceDependencies( o_rServices );
}

ComponentDescription::ComponentDescription()
    :   SequenceElement(C_sCOMPONENTDESCRIPTION,1),
        pComponentName(0),
        pSupportedServices(0),
        pTypes(0),
        pServiceDependencies(0)
{
    AddChild( *new SglTextElement(C_sAuthor, lt_nolink, false) );
    pComponentName = new CdName;
    AddChild( *pComponentName );
    AddChild( *new SglTextElement(C_sDescription, lt_nolink, false) );
    AddChild( *new SglTextElement(C_sLoaderName, lt_idl, true) );
    AddChild( *new SglTextElement(C_sLanguage, lt_nolink, false) );
    AddChild( *new SglAttrElement(C_sStatus, C_sAttr_value) );
    pSupportedServices = new SupportedService;
    AddChild( *pSupportedServices );
    AddChild( *Create_CompDescrOptional_Element( pTypes, pServiceDependencies ) );
}

CompDescrList::CompDescrList()
    :   ListElement(C_sCOMPONENTDESCRIPTION, 0)
{
}

void
CompDescrList::Write2Html( HtmlCreator & io_rHC ) const
{
    io_rHC.StartBigCell(C_sCompDescrListTitle);
    ListElement::Write2Html(io_rHC);
    io_rHC.FinishBigCell();
}

XmlElement *
CompDescrList::Create_and_Add_NewElement()
{
    ComponentDescription * pCD = new ComponentDescription;
    Children().push_back(pCD);
    aCDs.push_back(pCD);
    return pCD;
}

void
CompDescrList::Get_SupportedServices( List< const MultipleTextElement * > & o_rResult ) const
{
    unsigned i_max = aCDs.size();;
    for (unsigned i = 0; i < i_max; ++i)
    {
        o_rResult.push_back(& aCDs[i]->SupportedServices());
    }  // end for
}

void
CompDescrList::Get_Types( List< const MultipleTextElement * > & o_rResult ) const
{
    unsigned i_max = aCDs.size();;
    for (unsigned i = 0; i < i_max; ++i)
    {
        o_rResult.push_back(& aCDs[i]->Types());
    }  // end for
}

void
CompDescrList::Get_ServiceDependencies( List< const MultipleTextElement * > & o_rResult ) const
{
    unsigned i_max = aCDs.size();;
    for (unsigned i = 0; i < i_max; ++i)
    {
        o_rResult.push_back(& aCDs[i]->ServiceDependencies());
    }  // end for
}

MdName::MdName()
    :   SglTextElement(C_sModuleName, lt_html, false)
{
}

void
MdName::Write2Html( HtmlCreator & io_rHC ) const
{
    io_rHC.Write_SglTextElement( *this, true );
}

CdName::CdName()
    :   SglTextElement(C_sName, lt_html, true)
{
}

void
CdName::Write2Html( HtmlCreator & io_rHC ) const
{
    io_rHC.Write_SglTextElement( *this, true );
}

SupportedService::SupportedService()
    :   MultipleTextElement(C_sSupportedService, lt_idl, true)
{
}

void
SupportedService::Insert2Index( Index & o_rIndex ) const
{
    for ( unsigned i = 0; i < Size(); ++i )
    {
        o_rIndex.InsertSupportedService( Data(i) );
    }
}

FreeChoiceElement *
Create_ModuleDescrOptional_Element( MultipleTextElement * & o_rTypes,
                                  MultipleTextElement * & o_rServiceDependencies )
{
    FreeChoiceElement * ret = Create_CompDescrOptional_Element( o_rTypes, o_rServiceDependencies );

    ret->AddChild( *new MultipleTextElement(C_sProjectBuildDependency, lt_nolink, false) );
    ret->AddChild( *new MultipleTextElement(C_sRuntimeModuleDependency, lt_nolink, false) );
    return ret;
}

FreeChoiceElement *
Create_CompDescrOptional_Element( MultipleTextElement * & o_rTypes,
                                  MultipleTextElement * & o_rServiceDependencies )
{
    FreeChoiceElement * ret = new FreeChoiceElement;
    const unsigned C_sRefDocuAttrNumber = 5;
    static const char * C_sRefDocuAttrNames[C_sRefDocuAttrNumber]
            = { C_sAttr_xl_href, C_sAttr_xl_role, C_sAttr_xl_title, C_sAttr_xmlns, C_sAttr_xl_type };

    ret->AddChild( *new MultipleAttrElement(C_sReferenceDocu, C_sRefDocuAttrNames, C_sRefDocuAttrNumber) );
    o_rServiceDependencies = new MultipleTextElement(C_sServiceDependency, lt_idl, true);
    ret->AddChild( *o_rServiceDependencies );
    o_rTypes = new MultipleTextElement(C_sType, lt_idl, true);
    ret->AddChild( *o_rTypes );
    return ret;
}


#if 0

const TextElement *
ModuleDescription::ServiceDependencies() const
{
    const unsigned nEarliestPossibleServiceDependenciesIndexInModules = 1;

    for ( unsigned i = nEarliestPossibleServiceDependenciesIndexInModules;
          i < Children().size();
          ++i )
    {
        if ( strcmp(Children()[i]->Name(), C_sServiceDependency) == 0 )
            return Children()[i];
    }
    return 0;
}

const TextElement &
ComponentDescription::SupportedServices() const
{
    return *Children()[C_nSupportedServicesIndex];
}

const TextElement *
ComponentDescription::ServiceDependencies() const
{
    for ( unsigned i = C_nEarliestPossibleServiceDependenciesIndex; i < Children().size(); ++i )
    {
        if ( strcmp(Children()[i]->Name(),C_sServiceDependency) == 0)
            return Children()[i];
    }
    return 0;
}

#endif


