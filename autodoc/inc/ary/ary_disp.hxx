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

#ifndef ARY_ARY_DISP_HXX
#define ARY_ARY_DISP_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/types.hxx>
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
}

class Display : public csv::ProcessorIfc
{
  public:
    virtual 			~Display() {}

    // OPERATIONS
    void                StartSlot();
    void                FinishSlot();

    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_Rid() is called
        with just the id as parameter.
    */
    void                DisplaySlot_Rid(
                            ary::Rid            i_nId );
//    void                DisplaySlot_Lid(
//							ary::Lid            i_nId );
    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_LocalCe() is called
        with just the id as parameter.
    */
    void                DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &		i_sName );
    // INQUIRY
    const cpp::Gate *   Get_ReFinder() const;

  private:
    virtual void        do_StartSlot();
    virtual void        do_FinishSlot();


    virtual void        do_DisplaySlot_Rid(
                            ary::Rid            i_nId );
    virtual void        do_DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &		i_sName );
    virtual const cpp::Gate *
                        inq_Get_ReFinder() const = 0;
};


// IMPLEMENTATION


inline void
Display::StartSlot()
    { do_StartSlot(); }
inline void
Display::FinishSlot()
    { do_FinishSlot(); }
inline const cpp::Gate *
Display::Get_ReFinder() const
    { return inq_Get_ReFinder(); }




} // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
