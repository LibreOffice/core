/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
