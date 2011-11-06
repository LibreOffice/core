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


#ifndef _SVX_OPTCTL_HXX
#define _SVX_OPTCTL_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

class SvxCTLOptionsPage : public SfxTabPage
{
private:
    FixedLine           m_aSequenceCheckingFL;
    CheckBox            m_aSequenceCheckingCB;
    CheckBox            m_aRestrictedCB;
    CheckBox            m_aTypeReplaceCB;

    FixedLine           m_aCursorControlFL;
    FixedText           m_aMovementFT;
    RadioButton         m_aMovementLogicalRB;
    RadioButton         m_aMovementVisualRB;

    FixedLine           m_aGeneralFL;
    FixedText           m_aNumeralsFT;
    ListBox             m_aNumeralsLB;

    DECL_LINK( SequenceCheckingCB_Hdl, void* );

    SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:

    virtual ~SvxCTLOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTCTL_HXX

