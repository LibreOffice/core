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

#ifndef ADC_DOC_DEAL_HXX
#define ADC_DOC_DEAL_HXX

// BASE CLASSES
#include <tokens/tokproct.hxx>
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace doc
{
    class OldCppDocu;
}
}




class DocuDealer
{
  public:
    // INQUIRY
    virtual				~DocuDealer() {}

    // OPERATIONS
    /** @descr
        This distributes the let_drDocu to the matching ary::RepositoryEntity .

        If the docu is not inline, it will be saved and later given to the next
        ary::CodeEntity. Or it will be discarded, if there does not come a matching
        ary::CodeEntity .

        If the docu is inline after a function header or after an enum value
        or after a function parameter or after a base class, it will be stored
        together with the matching function, enum value, parameter or base class.

        If the documentation is @file or @project or @glos(sary) it will be
        stored at the matching ary::cpp::FileGroup, ary::cpp::ProjectGroup
        or ary::Glossary.
    */
    void                TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo );
  private:
    virtual void        do_TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo ) = 0;
};




// IMPLEMENTATION
inline void
DocuDealer::TakeDocu( DYN ary::doc::OldCppDocu & let_drInfo )
    { do_TakeDocu(let_drInfo); }




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
