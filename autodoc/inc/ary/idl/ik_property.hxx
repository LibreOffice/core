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

#ifndef ARY_IDL_IK_PROPERTY_HXX
#define ARY_IDL_IK_PROPERTY_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_property
{

using ifc_ce::DocText;


struct attr: public ifc_ce::attr
{
    static bool         HasAnyStereotype(
                            const CodeEntity &  i_ce );
    static bool         IsReadOnly(
                            const CodeEntity &  i_ce );
    static bool         IsBound(
                            const CodeEntity &  i_ce );
    static bool         IsConstrained(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeAmbiguous(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeDefault(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeVoid(
                            const CodeEntity &  i_ce );
    static bool         IsRemovable(
                            const CodeEntity &  i_ce );
    static bool         IsTransient(
                            const CodeEntity &  i_ce );
    static Type_id      Type(
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};


}   // namespace ifc_property

}   // namespace idl
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
