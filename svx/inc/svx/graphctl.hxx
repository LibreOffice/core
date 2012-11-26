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


#ifndef _GRAPHCTL_HXX
#define _GRAPHCTL_HXX

// include ---------------------------------------------------------------

#include <vcl/ctrl.hxx>
#include <vcl/graph.hxx>
#include "svx/svxdllapi.h"

#include <svx/svdview.hxx>
#include <svx/svdobj.hxx>

/*************************************************************************
|*
|*
|*
\************************************************************************/

class GraphCtrlUserCall;
class SvxGraphCtrlAccessibleContext;

#define WB_SDRMODE      ((WinBits)0x0080)
#define WB_ANIMATION    ((WinBits)0x0100)

class SVX_DLLPUBLIC GraphCtrl : public Control, public SfxListener
{
    friend class GraphCtrlView;
    friend class GraphCtrlUserCall;

    Graphic             aGraphic;
    Timer               aUpdateTimer;
    Link                aMousePosLink;
    Link                aGraphSizeLink;
    Link                aMarkObjLink;
    Link                aUpdateLink;
    MapMode             aMap100;
    Size                aGraphSize;
    Point               aMousePos;
    WinBits             nWinStyle;
    SdrObjectCreationInfo   maSdrObjectCreationInfo;
    sal_uInt16              nPolyEdit;
    sal_Bool                bEditMode;
    sal_Bool                bSdrMode;
    sal_Bool                bAnim;

                        DECL_LINK( UpdateHdl, Timer* );

    SvxGraphCtrlAccessibleContext* mpAccContext;

protected:

    SdrModel*           pModel;
    SdrView*            pView;

    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        KeyInput(const KeyEvent& rKEvt);
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);
    virtual void        MouseButtonUp(const MouseEvent& rMEvt);
    virtual void        MouseMove(const MouseEvent& rMEvt);

    virtual void        InitSdrModel();

    virtual void        SdrObjCreated( const SdrObject& rObj );
    virtual void        SdrObjChanged( const SdrObject& rObj );
    virtual void selectionChange();

public:
    // from SfxListener
    using Control::Notify;
    using SfxListener::Notify;
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // establich connection to given SDrObject to get Notify-calls
    void                ConnectToSdrObject(SdrObject* pObject);

                        GraphCtrl( Window* pParent, const WinBits nWinBits = 0 );
                        GraphCtrl( Window* pParent, const ResId& rResId );
                        ~GraphCtrl();

    void                SetWinStyle( WinBits nWinBits );
    WinBits             GetWinStyle() const { return nWinStyle; }

    void                SetGraphic( const Graphic& rGraphic, sal_Bool bNewModel = sal_True );
    const Graphic&      GetGraphic() const { return aGraphic; }
    const Size&         GetGraphicSize() const { return aGraphSize; }

    const Point&        GetMousePos() const { return aMousePos; }

    void                SetEditMode( const sal_Bool bEditMode );
    sal_Bool                IsEditMode() const { return bEditMode; }

    void                SetPolyEditMode( const sal_uInt16 nPolyEdit );
    sal_uInt16              GetPolyEditMode() const { return nPolyEdit; }

    void setSdrObjectCreationInfo( const SdrObjectCreationInfo& rSdrObjectCreationInfo );
    const SdrObjectCreationInfo& getSdrObjectCreationInfo() const { return maSdrObjectCreationInfo; }

    SdrModel*           GetSdrModel() const { return pModel; }
    SdrView*            GetSdrView() const { return pView; }
    SdrObject*          GetSelectedSdrObject() const;
    sal_Bool                IsChanged() const { return bSdrMode ? pModel->IsChanged() : sal_False; }

    void                SetMousePosLink( const Link& rLink ) { aMousePosLink = rLink; }
    const Link&         GetMousePosLink() const { return aMousePosLink; }

    void                SetGraphSizeLink( const Link& rLink ) { aGraphSizeLink = rLink; }
    const Link&         GetGraphSizeLink() const { return aGraphSizeLink; }

    void                SetMarkObjLink( const Link& rLink ) { aMarkObjLink = rLink; }
    const Link&         GetMarkObjLink() const { return aMarkObjLink; }

    void                SetUpdateLink( const Link& rLink ) { aUpdateLink = rLink; }
    const Link&         GetUpdateLink() const { return aUpdateLink; }

    static String       GetStringFromDouble( const double& rDouble );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

//#if 0 // _SOLAR__PRIVATE

/*************************************************************************
|*
|*
|*
\************************************************************************/

class GraphCtrlView : public SdrView
{
    GraphCtrl&      rGraphCtrl;

protected:

    virtual void handleSelectionChange();

public:

                    GraphCtrlView( SdrModel& rModel, GraphCtrl* pWindow) :
                            SdrView     ( rModel, pWindow ),
                            rGraphCtrl  ( *pWindow ) {};

    virtual         ~GraphCtrlView() {};
};

//#endif // __PRIVATE

#endif // _GRAPHCTL_HXX

