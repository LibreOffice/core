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

#ifndef ARY_CPP_C_TYDEF_HXX
#define ARY_CPP_C_TYDEF_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>



namespace ary
{
namespace cpp
{


/** A C++ typedef declaration.
*/
class Typedef : public CodeEntity
{
  public:
    // LIFECYCLE
    enum E_ClassId { class_id = 1003 };

                        Typedef(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            Lid                 i_nFile,
                            Type_id             i_nDescribingType );
                        ~Typedef();
    // INQUIRY
    Type_id             DescribingType() const;
    E_Protection        Protection() const      { return eProtection; }

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // DATA
    CeEssentials        aEssentials;
    Type_id             nDescribingType;
    E_Protection        eProtection;
};



// IMPLEMENTATION
inline Type_id
Typedef::DescribingType() const
{
    return nDescribingType;
}



}   //  namespace cpp
}   //  namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
