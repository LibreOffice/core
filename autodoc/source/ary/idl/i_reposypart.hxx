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

#ifndef ARY_IDL_I_REPOSYPART_HXX
#define ARY_IDL_I_REPOSYPART_HXX



// USED SERVICES
    // BASE CLASSES
#include <idl_internalgate.hxx>
    // OTHER


namespace ary
{
namespace idl
{
    class CeAdmin;
    class TypeAdmin;
    class NameLookup;
}
}





namespace ary
{
namespace idl
{



/** The idl part of the Autodoc repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
    // LIFECYCLE
                        RepositoryPartition();
                        ~RepositoryPartition();
    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath );
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const CePilot &
                        Ces() const;
    virtual const TypePilot &
                        Types() const;
    virtual CePilot &   Ces();
    virtual TypePilot & Types();

  private:
    // DATA
    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<NameLookup>     pNamesDictionary;
};




}   //  namespace idl
}   //  namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
