/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "dependy.hxx"
#include <iostream>
#include "../support/syshelp.hxx"
#include "../support/list.hxx"
#include "../xcd/xmltree.hxx"
#include "../xcd/parse.hxx"



Simstr              ShortName(const Simstr & i_rService);



Service::Service( const char * i_sName )
    :   sName(i_sName)
        // aImplementations
{
}

ServiceInfo &
Service::AddImplementation( const char * i_sLibrary )
{
    ServiceInfo * ret = new ServiceInfo(i_sLibrary);
    aImplementations.push_back(ret);
    return *ret;
}

ServiceInfo::ServiceInfo( const char * i_sLibrary )
    :   sImplementingLibrary(i_sLibrary)
        // aNeededServices
{
}

void
ServiceInfo::AddDependency( const char * i_sNeededService )
{
    aNeededServices.push_back(i_sNeededService);
}

DependencyFinder::DependencyFinder()
{
}

DependencyFinder::~DependencyFinder()
{
}

void
DependencyFinder::GatherData( const char * i_sSearchDirectory )
{
    List<Simstr> aFiles;
    GatherFileNames( aFiles, i_sSearchDirectory );

    for ( unsigned i = 0; i < aFiles.size(); ++i )
    {
        ReadFile( aFiles[i].str() );
    }
}

void
DependencyFinder::FindNeededServices( StringVector &        o_rLibraries,
                                      StringVector &        o_rServices,
                                      const Simstr &        i_rService )
{
    Map_Services::const_iterator itService = aServices.find(i_rService);
    if ( itService == aServices.end() )
    {
        std::cerr << "Error: Service \""
                  << i_rService.str()
                  << "\" not found."
                  << std::endl;
        return ;
    }

    aResult_Libraries.erase( aResult_Libraries.begin(), aResult_Libraries.end() );
    aResult_Services.erase( aResult_Services.begin(), aResult_Services.end() );

    Add2Result( *(*itService).second );

    for ( std::set< Simstr >::const_iterator il = aResult_Libraries.begin();
          il != aResult_Libraries.end();
          ++il )
    {
        o_rLibraries.push_back(*il);
    }

    for ( std::set< Simstr >::const_iterator is = aResult_Services.begin();
          is != aResult_Services.end();
          ++is )
    {
        o_rServices.push_back(*is);
    }
}

void
DependencyFinder::ReadFile(  const char * i_sFilename )
{
    ModuleDescription   aModule;
    X2CParser           aParser(aModule);

    if ( !aParser.Parse(i_sFilename) )
    {
        std::cerr << "Error: File \""
             << i_sFilename
             << "\" could not be parsed."
             << std::endl;
        return;
    }

    // GetResults:
    Simstr sModule = aModule.ModuleName();

    List < const MultipleTextElement* > aImplServices;
    List < const MultipleTextElement* > aNeededServices;

    aModule.Get_SupportedServices(aImplServices);
    aModule.Get_ServiceDependencies(aNeededServices);

    unsigned nImplServicesSize = aImplServices.size();
    unsigned nNeededServicesSize = aNeededServices.size();

    for ( unsigned i = 0; i < nImplServicesSize; ++i )
    {
        const MultipleTextElement & rImpl = *aImplServices[i];

        unsigned nImplDataSize = rImpl.Size();
        for ( unsigned di = 0; di < nImplDataSize; ++di )
        {
            Simstr sService = ShortName(rImpl.Data(di));
            Service * pService = aServices[sService];
            if (pService == 0)
            {
                pService = new Service(rImpl.Data(di));
                aServices[sService] = pService;
            }
            ServiceInfo & rSInfo = pService->AddImplementation(sModule);

            for ( unsigned n = 0; n < nNeededServicesSize; ++n )
            {
                unsigned nNeededDataSize = aNeededServices[n]->Size();
                for ( unsigned dn = 0; dn < nNeededDataSize; ++dn )
                {
                    if (! aNeededServices[n]->Data(dn).is_no_text())
                        rSInfo.AddDependency( ShortName(aNeededServices[n]->Data(dn)) );
                }   // end for dn
            }   // end for n
        }   //  end for di
    }   // end for i
}

void
DependencyFinder::Add2Result( const Service & i_rService )
{
    const ServiceInfo & rSInfo = i_rService.FirstImplementation();
    aResult_Libraries.insert(rSInfo.Library());

    const ServiceInfo::List_NeededServices & rNeededs
            = rSInfo.NeededServices();
    for ( StringVector::const_iterator it = rNeededs.begin();
          it != rNeededs.end();
          ++it )
    {
        std::pair< std::set< Simstr >::iterator, bool > aInsertResult
                = aResult_Services.insert(*it);
        if (aInsertResult.second)
        {   // Needed service not yet known
            Map_Services::const_iterator itFound = aServices.find(*it);
            if ( itFound == aServices.end() )
            {
                std::cerr << "Needed service \""
                          << (*it).str()
                          << "\" not found,"
                          << std::endl;
            }
            else
            {
                Add2Result( *(*itFound).second );
            }
        }   // endif (! aInsertResult.second)
    }   // end for (it)
}



Simstr
ShortName(const Simstr & i_rService)
{
    const char * pStart = i_rService.str();
    const char * pEnd = strchr(pStart,' ');
    if (pEnd != 0)
        return Simstr(pStart, 0, int(pEnd-pStart));
    else
        return i_rService;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
