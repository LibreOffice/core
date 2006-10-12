/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contimp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:08:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONTIMP_HXX_
#define _CONTIMP_HXX_

#ifndef _CONTDLG_HXX
#include "contdlg.hxx"
#endif
#ifndef _CONTWND_HXX
#include "contwnd.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif

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
    ULONG               nGrfChanged;
    BOOL                bExecState;
    BOOL                bPipetteMode;
    BOOL                bWorkplaceMode;
    BOOL                bUpdateGraphicLinked;
    BOOL                bGraphicLinked;
    ImageList           maImageList;
    ImageList           maImageListH;

    virtual void        Resize();
    virtual BOOL        Close();

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

    void                SetExecState( BOOL bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( BOOL bLinked ) { bGraphicLinked = bLinked; }
    const Graphic&      GetGraphic() const { return aGraphic; }
    BOOL                IsGraphicChanged() const { return nGrfChanged > 0UL; }

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon( BOOL bRescaleToGraphic = TRUE );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    BOOL                IsUndoPossible() const;
    BOOL                IsRedoPossible() const;

    void                UpdateGraphic( const Graphic& rGraphic, BOOL bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL,
                                void* pEditingObj = NULL );

    /** switches the toolbox images depending on the actuall high contrast display mode state */
    void                ApplyImageList();

    /** virtual method from Window is used to detect change in high contrast display mode
        to switch the toolbox images */
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

};


#endif // _CONTIMP_HXX_

