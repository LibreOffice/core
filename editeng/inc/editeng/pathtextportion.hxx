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

#ifndef _PATHTEXTPORTION_HXX
#define _PATHTEXTPORTION_HXX

#include <editeng/svxfont.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class DrawPortionInfo;

//////////////////////////////////////////////////////////////////////////////

class EDITENG_DLLPUBLIC PathTextPortion
{
    basegfx::B2DVector                          maOffset;
    String                                      maText;
    xub_StrLen                                  mnTextStart;
    xub_StrLen                                  mnTextLength;
    sal_uInt16                                  mnParagraph;
    xub_StrLen                                  mnIndex;
    SvxFont                                     maFont;
    ::std::vector< double >                     maDblDXArray;   // double DXArray, font size independent -> unit coordinate system
    ::com::sun::star::lang::Locale              maLocale;

    // bitfield
    bool                                        mbRTL : 1;

public:
    PathTextPortion(DrawPortionInfo& rInfo);

    // for ::std::sort
    bool operator<(const PathTextPortion& rComp) const;

    const basegfx::B2DVector& getOffset() const { return maOffset; }
    const String& getText() const { return maText; }
    xub_StrLen getTextStart() const { return mnTextStart; }
    xub_StrLen getTextLength() const { return mnTextLength; }
    sal_uInt16 getParagraph() const { return mnParagraph; }
    xub_StrLen getIndex() const { return mnIndex; }
    const SvxFont& getFont() const { return maFont; }
    bool isRTL() const { return mbRTL; }
    const ::std::vector< double >& getDoubleDXArray() const { return maDblDXArray; }
    const ::com::sun::star::lang::Locale& getLocale() const { return maLocale; }
    xub_StrLen getPortionIndex(xub_StrLen nIndex, xub_StrLen nLength) const;
    double getDisplayLength(xub_StrLen nIndex, xub_StrLen nLength) const;
};

//////////////////////////////////////////////////////////////////////////////

#endif // _PATHTEXTPORTION_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
