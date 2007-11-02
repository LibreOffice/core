/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namechain.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:57:10 $
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
                            const char *        i_sName );
    /** @precond MPT pTemplate.
        This cannot be used, except of inserting a new element
        in the segment list of ary::cpp::ut::NameChain. In that
        case, the template parameter list doe snot yet exist.
    */
                        NameSegment(
                            const NameSegment & i_rSeg );
                        ~NameSegment();

    // OPERATIONS
    List_TplParameter & AddTemplate();

    // INQUIRY
    const String  &     Name() const;

        /// @return as strcmp().
    intt                Compare(
                            const NameSegment & i_rOther ) const;
    void                Get_Text_AsScope(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
    void                Get_Text_AsMainType(
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::Gate &
                                                i_rGate ) const;

    NameSegment& operator=(const NameSegment&);
  private:
    String              sName;
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
                            const char *        i_sSeg );
    /** @precond  aSegments.size() > 0.
        Which means: Add_Segment() has to be called at least once before.
    */
    List_TplParameter & Templatize_LastSegment();

    // INQUIRY
    const_iterator      begin() const          { return aSegments.begin(); }
    const_iterator      end() const            { return aSegments.end(); }

    /// @return like strcmp.
    intt                Compare(
                            const NameChain &   i_rChain ) const;
    /// @ATTENTION Return value is volatile. Not reentrance enabled.
    const String  &     LastSegment() const;

    void                Get_Text(
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
