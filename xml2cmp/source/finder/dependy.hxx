/*************************************************************************
 *
 *  $RCSfile: dependy.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:32:07 $
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


