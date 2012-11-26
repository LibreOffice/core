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


#ifndef _SVX_CONNCTRL_HXX
#define _SVX_CONNCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif
#include "svx/svxdllapi.h"

class SfxItemSet;
class SdrEdgeObj;
class SdrView;
class SdrObjList;

/*************************************************************************
|*
|* SvxXConnectionPreview
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxXConnectionPreview : public Control
{
 friend class SvxConnectionPage;

private:
    const SfxItemSet&   rAttrs;
    SdrEdgeObj*         pEdgeObj;
    SdrObjectVector maSdrObjectVector;
    const SdrView*      pView;

    SVX_DLLPRIVATE void SetStyles();
public:
            SvxXConnectionPreview( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rInAttrs );
            ~SvxXConnectionPreview();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void         SetAttributes( const SfxItemSet& rInAttrs );
    sal_uInt16       GetLineDeltaAnz();

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};


#endif // _SVX_CONNCTRL_HXX

