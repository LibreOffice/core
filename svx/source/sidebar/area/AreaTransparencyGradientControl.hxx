/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
