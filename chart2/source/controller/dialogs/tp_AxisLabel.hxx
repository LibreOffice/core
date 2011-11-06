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


#ifndef _CHART2_TP_AXISLABEL_HXX
#define _CHART2_TP_AXISLABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <svx/orienthelper.hxx>
#include "TextDirectionListBox.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class SchAxisLabelTabPage : public SfxTabPage
{
private:
    CheckBox            aCbShowDescription;

    FixedLine           aFlOrder;
    RadioButton         aRbSideBySide;
    RadioButton         aRbUpDown;
    RadioButton         aRbDownUp;
    RadioButton         aRbAuto;

    FixedLine           aFlSeparator;
    FixedLine           aFlTextFlow;
    CheckBox            aCbTextOverlap;
    CheckBox            aCbTextBreak;

    FixedLine           aFlOrient;
    svx::DialControl    aCtrlDial;
    FixedText           aFtRotate;
    svx::WrapField      aNfRotate;
    TriStateBox         aCbStacked;
    svx::OrientationHelper aOrientHlp;

    FixedText               m_aFtTextDirection;
    TextDirectionListBox    m_aLbTextDirection;

    sal_Bool                m_bShowStaggeringControls;

    sal_Int32           m_nInitialDegrees;
    bool                m_bHasInitialDegrees;       /// false = DialControl in tristate
    bool                m_bInitialStacking;
    bool                m_bHasInitialStacking;      /// false = checkbox in tristate
    bool                m_bComplexCategories;

    DECL_LINK ( ToggleShowLabel, void* );

public:
    SchAxisLabelTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SchAxisLabelTabPage();

    void    Construct();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );

    void ShowStaggeringControls( sal_Bool bShowStaggeringControls );
    void SetComplexCategories( bool bComplexCategories );
};
//.............................................................................
} //namespace chart
//.............................................................................

#endif

