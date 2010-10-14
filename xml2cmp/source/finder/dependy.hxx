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

#ifndef X2C_DEPENDY_HXX
#define X2C_DEPENDY_HXX


#include <vector>
#include <map>
#include <set>
#include <../support/sistr.hxx>

class Service;
class ServiceInfo;

typedef std::vector< Simstr>            StringVector;
typedef std::vector< ServiceInfo* >     List_Implementations;
typedef std::map< Simstr, Service* >    Map_Services;

class Service
{
  public:
                        Service(
                            const char *        i_sName );

    ServiceInfo &       AddImplementation(
                            const char *        i_sLibrary );   /// That is: module-name.

    const Simstr &      Name() const            { return sName; }
    const ServiceInfo & FirstImplementation() const
                                                { return *aImplementations[0]; }

  private:
    Simstr              sName;
    List_Implementations
                        aImplementations;
};

class ServiceInfo
{
  public:
    typedef StringVector List_NeededServices;

                        ServiceInfo(
                            const char *        i_sLibrary );

    void                AddDependency(
                            const char *        i_sNeededService );

    const Simstr &      Library() const         { return sImplementingLibrary; }
    const List_NeededServices &
                        NeededServices() const  { return aNeededServices; }


  private:
    Simstr              sImplementingLibrary;
    List_NeededServices aNeededServices;
};


class DependencyFinder
{
  public:
                        DependencyFinder();
                        ~DependencyFinder();

    void                GatherData(
                            const char *        i_sSearchDirectory );

    void                FindNeededServices(
                            StringVector &      o_rLibraries,
                            StringVector &      o_rServices,
                            const Simstr &      i_rService );
  private:
    void                ReadFile(
                            const char *        i_sFilename );
    void                Add2Result(
                            const Service &     i_rService );

    // Data
    Map_Services        aServices;

    // Temporary data
    std::set< Simstr >  aResult_Libraries;
    std::set< Simstr >  aResult_Services;
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
