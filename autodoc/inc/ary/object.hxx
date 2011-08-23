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

#ifndef ARY_OBJECT_HXX
#define ARY_OBJECT_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/types.hxx>


namespace ary
{


/** Interface for every class, that is stored within the
    Autodoc Repository.
*/
class Object : public csv::ConstProcessorClient
{
  public:
    virtual             ~Object() {}

    /// @return Type id of most derived class.
    ClassId             AryClass() const;

  private:
    virtual ClassId     get_AryClass() const = 0;
};



inline ClassId
Object::AryClass() const
{
    return get_AryClass();
}


}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
