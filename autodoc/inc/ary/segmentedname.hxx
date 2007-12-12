/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: segmentedname.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-12 14:56:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
