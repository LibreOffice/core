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



#ifndef _VCL_CONTROLLAYOUT_HXX
#define _VCL_CONTROLLAYOUT_HXX

#include <vector>
#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <vcl/dllapi.h>

class Control;

namespace vcl
{

struct VCL_DLLPUBLIC ControlLayoutData
{
    // contains the string really displayed
    // there must be exactly one bounding rectangle in m_aUnicodeBoundRects
    // for every character in m_aDisplayText
    String                              m_aDisplayText;
    // the bounding rectangle of every character
    // where one character may consist of many glyphs
    std::vector< Rectangle >            m_aUnicodeBoundRects;
    // start indices of lines
    std::vector< long >                 m_aLineIndices;
    // notify parent control on destruction
    const Control*                      m_pParent;

    ControlLayoutData() : m_pParent( NULL ) {}
    ~ControlLayoutData();

    Rectangle GetCharacterBounds( long nIndex ) const;
    // returns the character index for corresponding to rPoint (in control coordinates)
    // -1 is returned if no character is at that point
    long GetIndexForPoint( const Point& rPoint ) const;
    // returns the number of lines in the result of GetDisplayText()
    long GetLineCount() const;
    // returns the interval [start,end] of line nLine
    // returns [-1,-1] for an invalid line
    Pair GetLineStartEnd( long nLine ) const;
    /** ToRelativeLineIndex changes a layout data index to a count relative to its line.

    <p>This is equivalent to getting the line start/end pairs with
    <member>GetLineStartEnd</member> until the index lies within [start,end] of a line
    </p>

    @param nIndex
    the absolute index inside the display text to be changed to a relative index

    @returns
    the relative index inside the displayed line or -1 if the absolute index does
    not match any line
    */
    long ToRelativeLineIndex( long nIndex ) const;
};

} // namespace vcl

#endif // _VCL_CONTROLLAYOUT_HXX
