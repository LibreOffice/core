/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
