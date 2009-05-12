/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: segmentedname.hxx,v $
 * $Revision: 1.4 $
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

#ifndef ARY_SEGMENTEDNAME_HXX
#define ARY_SEGMENTEDNAME_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER



namespace ary
{
namespace sn    // segmented name
{

/** One segment in a name like "::global::abc<hello,xy>::def".
    A segment being the part between "::"s.

    @collab implements ->SegmentedName
*/
class NameSegment
{
  public:
    explicit            NameSegment(
                            const char *        i_name );
    explicit            NameSegment(
                            const String &      i_name );
                        NameSegment(
                            const NameSegment & i_segment );
                        ~NameSegment();
    // OPERATORS
    NameSegment &       operator=(
                            const NameSegment & i_other );
    bool                operator<(
                            const NameSegment & i_other ) const;
    // OPERATIONS
    String &            Make_Template();

    // INQUIRY
    const String  &     Name() const;
    const String *      TemplateArguments() const;

    void                Get_Text(
                            StreamStr &         o_out ) const;
  private:
    // DATA
    String              sName;
    Dyn<String>         pTemplateArguments;
};



typedef std::vector<NameSegment>        NameChain;

}   // namespace sn


/** A name of the form "::global::abc<hello,xy>::def".
*/
class SegmentedName
{
  public:
    explicit            SegmentedName(
                            const char *        i_text );
    explicit            SegmentedName(
                            const String &      i_text );
                        ~SegmentedName();
    // OPERATIONS
    bool                operator<(
                            const SegmentedName &
                                                i_other ) const;

    // INQUIRY
    bool                IsAbsolute() const;
    const sn::NameChain &
                        Sequence() const;

    void                Get_Text(
                            StreamStr &         o_out ) const;
  private:
    void                Interpret_Text(
                            const char *        i_text ); /// [valid C++ or IDL name].

    // DATA
    sn::NameChain       aSegments;
    bool                bIsAbsolute;
};


// IMPLEMENTATION
namespace sn
{

inline const String  &
NameSegment::Name() const
{
    return sName;
}

inline const String *
NameSegment::TemplateArguments() const
{
    return pTemplateArguments.Ptr();
}

}   // namespace sn

inline bool
SegmentedName::IsAbsolute() const
{
    return bIsAbsolute;
}

inline const sn::NameChain &
SegmentedName::Sequence() const
{
    return aSegments;
}



}   // namespace ary
#endif
