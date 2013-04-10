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
#include "LineWidthValueSet.hxx"
#include <svl/poolitem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

class SfxBindings;

namespace svx { namespace sidebar {

class LinePropertyPanel;

class LineWidthControl
    : public svx::sidebar::PopupControl
{
public:
    LineWidthControl (Window* pParent, LinePropertyPanel& rPanel);
    virtual ~LineWidthControl (void);

    virtual void GetFocus();
    virtual void Paint(const Rectangle& rect);

    void SetWidthSelect( long lValue, bool bValuable, SfxMapUnit eMapUnit);
    ValueSet& GetValueSet();
    bool IsCloseByEdit();
    long GetTmpCustomWidth();

private:
    LinePropertyPanel& mrLinePropertyPanel;
    SfxBindings*                        mpBindings;
    LineWidthValueSet maVSWidth;
    FixedText                           maFTCus;
    FixedText                           maFTWidth;
    MetricField                         maMFWidth;
    SfxMapUnit                          meMapUnit;
    XubString*                          rStr;
    XubString                           mstrPT; //
    long                                mnCustomWidth;
    bool                                mbCustom;
    bool                                mbColseByEdit;
    long                                mnTmpCusomWidth;
    bool                                mbVSFocus;

    Image                               maIMGCus;
    Image                               maIMGCusGray;

    void Initialize();
    DECL_LINK(VSSelectHdl, void *);
    DECL_LINK(MFModifyHdl, void *);
};

} } // end of namespace svx::sidebar

// eof
