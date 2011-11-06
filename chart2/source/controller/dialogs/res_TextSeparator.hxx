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


#ifndef CHART2_RES_TEXTSEPARATOR_HXX
#define CHART2_RES_TEXTSEPARATOR_HXX

// header for class FixedText
#include <vcl/fixed.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>

#include <map>

//.............................................................................
namespace chart
{
//.............................................................................

class TextSeparatorResources
{
public:
    TextSeparatorResources( Window* pParent );
    virtual ~TextSeparatorResources();

    void Show( bool bShow );
    void Enable( bool bEnable );

    void PositionBelowControl( const Window& rWindow );
    void AlignListBoxWidthAndXPos( long nWantedLeftBorder /*use -1 to indicate that this can be automatic*/
                                 , long nWantedRightBorder /*use -1 to indicate that this can be automatic*/
                                 , long nMinimumListBoxWidth /*use -1 to indicate that this can be automatic*/ );
    Point GetCurrentListBoxPosition() const;
    Size GetCurrentListBoxSize() const;

    ::rtl::OUString GetValue() const;
    void SetValue( const ::rtl::OUString& rSeparator );
    void SetDefault();

private:
    FixedText   m_aFT_Separator;
    ListBox     m_aLB_Separator;

    ::std::map< ::rtl::OUString, sal_uInt16 > m_aEntryMap;

    const sal_uInt16 m_nDefaultPos;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
