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

#ifndef ARY_CPP_NAMECHAI_HXX
#define ARY_CPP_NAMECHAI_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER


namespace ary
{
namespace cpp
{
    class Gate;

namespace ut
{
    class List_TplParameter;

class NameSegment
{
  public:
                        NameSegment(
                            const char *		i_sName );
    /** @precond MPT pTemplate.
        This cannot be used, except of inserting a new element
        in the segment list of ary::cpp::ut::NameChain. In that
        case, the template parameter list doe snot yet exist.
    */
                        NameSegment(
                            const NameSegment &	i_rSeg );
                        ~NameSegment();

    // OPERATIONS
    List_TplParameter & AddTemplate();

    // INQUIRY
    const String  &		Name() const;

        /// @return as strcmp().
    intt			   	Compare(
                            const NameSegment &	i_rOther ) const;
    void				Get_Text_AsScope(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
    void				Get_Text_AsMainType(
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::Gate &
                                                i_rGate ) const;

    NameSegment& operator=(const NameSegment&);
  private:
    String 				sName;
    Dyn<List_TplParameter>
                        pTemplate;
};

class NameChain
{
  public:
    typedef std::vector<NameSegment>::const_iterator
            const_iterator;

                        NameChain();
                        ~NameChain();

    // OPERATIONS
    void                Add_Segment(
                            const char *	    i_sSeg );
    /** @precond  aSegments.size() > 0.
        Which means: Add_Segment() has to be called at least once before.
    */
    List_TplParameter & Templatize_LastSegment();

    // INQUIRY
    const_iterator      begin() const          { return aSegments.begin(); }
    const_iterator      end() const            { return aSegments.end(); }

    ///	@return like strcmp.
    intt				Compare(
                            const NameChain &	i_rChain ) const;
    /// @ATTENTION Return value is volatile. Not reentrance enabled.
    const String  &		LastSegment() const;

    void				Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
  private:
    std::vector< NameSegment >
                        aSegments;
};



// IMPLEMENTATION
inline const String  &
NameSegment::Name() const
    { return sName; }






}   // namespace ut
}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
