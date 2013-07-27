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

#include "svx/sidebar/PopupControl.hxx"

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svx/xenum.hxx>

class XFillFloatTransparenceItem;
class SfxBindings;

namespace svx { namespace sidebar {

class AreaPropertyPanel;

class AreaTransparencyGradientControl : public PopupControl
{
public:
    AreaTransparencyGradientControl (
        Window* pParent,
        AreaPropertyPanel& rPanel);
    virtual ~AreaTransparencyGradientControl (void);

    void ToGetFocus();
    void Rearrange(XFillFloatTransparenceItem* pGradientItem);
    void InitStatus(XFillFloatTransparenceItem* pGradientItem);
    void ExecuteValueModify( sal_uInt8 nStartCol, sal_uInt8 nEndCol );
    void SetControlState_Impl(XGradientStyle eXGS);

private:
    FixedText           maFtTrgrCenterX;
    MetricField         maMtrTrgrCenterX;
    FixedText           maFtTrgrCenterY;
    MetricField         maMtrTrgrCenterY;
    FixedText           maFtTrgrAngle;
    MetricField         maMtrTrgrAngle;
    ToolBox             maBtnLeft45;
    ToolBox             maBtnRight45;
    FixedText           maFtTrgrStartValue;
    MetricField         maMtrTrgrStartValue;
    FixedText           maFtTrgrEndValue;
    MetricField         maMtrTrgrEndValue;
    FixedText           maFtTrgrBorder;
    MetricField         maMtrTrgrBorder;
    Image               maRotLeft;
    Image               maRotRight;

    AreaPropertyPanel& mrAreaPropertyPanel;
    SfxBindings*        mpBindings;
    DECL_LINK( ModifiedTrgrHdl_Impl, void* );
    DECL_LINK( Left_Click45_Impl, void* );
    DECL_LINK( Right_Click45_Impl, void* );
};


} } // end of namespace svx::sidebar

// eof
