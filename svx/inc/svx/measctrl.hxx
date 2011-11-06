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


#ifndef _SVX_MEASCTRL_HXX
#define _SVX_MEASCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif
#include "svx/svxdllapi.h"


class SfxItemSet;
class SdrMeasureObj;
class SdrModel;

/*************************************************************************
|*
|* SvxXMeasurePreview
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxXMeasurePreview : public Control
{
 friend class SvxMeasurePage;

private:
    const SfxItemSet&   rAttrs;
    SdrMeasureObj*      pMeasureObj;
    SdrModel*           pModel;

public:
            SvxXMeasurePreview( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rInAttrs );
            ~SvxXMeasurePreview();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void         SetAttributes( const SfxItemSet& rInAttrs );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

#endif

