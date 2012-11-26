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




#ifndef _BASCTL_DLGEDVIEW_HXX
#define _BASCTL_DLGEDVIEW_HXX

#include <svx/svdview.hxx>

class DlgEditor;

//============================================================================
// DlgEdView
//============================================================================

class DlgEdView : public SdrView
{
private:
    DlgEditor&  mrDlgEditor;

public:
    DlgEdView( SdrModel& rModel, OutputDevice* pOut, DlgEditor& rEditor );
    virtual ~DlgEdView();

    virtual void handleSelectionChange();
    virtual void MakeVisibleAtView( const basegfx::B2DRange& rRange, Window& rWin );

protected:
    // overloaded to handle HitTest for some objects special
    using SdrView::CheckSingleSdrObjectHit;
    virtual SdrObject* CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, sal_uInt32 nOptions, const SetOfByte* pMVisLay) const;
};

#endif //_BASCTL_DLGEDVIEW_HXX
