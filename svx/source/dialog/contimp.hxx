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


#ifndef _CONTIMP_HXX_
#define _CONTIMP_HXX_

#ifndef _CONTDLG_HXX
#include <svx/contdlg.hxx>
#endif
#include "contwnd.hxx"
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>

#define CONT_RESID(nId)  ResId( nId, DIALOG_MGR() )

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SvxSuperContourDlg : public SvxContourDlg
{
    using SvxContourDlg::GetPolyPolygon;

    Graphic             aGraphic;
    Graphic             aUndoGraphic;
    Graphic             aRedoGraphic;
    Graphic             aUpdateGraphic;
    PolyPolygon         aUpdatePolyPoly;
    Timer               aUpdateTimer;
    Timer               aCreateTimer;
    Size                aLastSize;
    void*               pUpdateEditingObject;
    void*               pCheckObj;
    SvxContourDlgItem   aContourItem;
    ToolBox             aTbx1;
    MetricField         aMtfTolerance;
    ContourWindow       aContourWnd;
    StatusBar           aStbStatus;
    sal_uIntPtr             nGrfChanged;
    sal_Bool                bExecState;
    sal_Bool                bPipetteMode;
    sal_Bool                bWorkplaceMode;
    sal_Bool                bUpdateGraphicLinked;
    sal_Bool                bGraphicLinked;
    ImageList           maImageList;
    ImageList           maImageListH;

    virtual void        Resize();
    virtual sal_Bool        Close();

    void                DoAutoCreate();
    void                ReducePoints( const long nTol = 8 );

                        DECL_LINK( Tbx1ClickHdl, ToolBox* );
                        DECL_LINK( MousePosHdl, ContourWindow* );
                        DECL_LINK( GraphSizeHdl, ContourWindow* );
                        DECL_LINK( UpdateHdl, Timer* );
                        DECL_LINK( CreateHdl, Timer* );
                        DECL_LINK( StateHdl, ContourWindow* );
                        DECL_LINK( PipetteHdl, ContourWindow* );
                        DECL_LINK( PipetteClickHdl, ContourWindow* );
                        DECL_LINK( WorkplaceClickHdl, ContourWindow* );
                        DECL_LINK( MiscHdl, void* );

public:

                        SvxSuperContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                            Window* pParent, const ResId& rResId );
                        ~SvxSuperContourDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( sal_Bool bLinked ) { bGraphicLinked = bLinked; }
    const Graphic&      GetGraphic() const { return aGraphic; }
    sal_Bool                IsGraphicChanged() const { return nGrfChanged > 0UL; }

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon( sal_Bool bRescaleToGraphic = sal_True );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    sal_Bool                IsUndoPossible() const;
    sal_Bool                IsRedoPossible() const;

    void                UpdateGraphic( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL,
                                void* pEditingObj = NULL );

    /** switches the toolbox images depending on the actuall high contrast display mode state */
    void                ApplyImageList();

    /** virtual method from Window is used to detect change in high contrast display mode
        to switch the toolbox images */
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

};


#endif // _CONTIMP_HXX_

