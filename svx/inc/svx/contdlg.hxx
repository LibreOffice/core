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



#ifndef _CONTDLG_HXX_
#define _CONTDLG_HXX_

#include <sfx2/basedlgs.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/childwin.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Float
|*
\************************************************************************/

class Graphic;

class SVX_DLLPUBLIC SvxContourDlgChildWindow : public SfxChildWindow
{
 public:

    SvxContourDlgChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxContourDlgChildWindow );

    static void UpdateContourDlg( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                  const PolyPolygon* pPolyPoly = NULL,
                                  void* pEditingObj = NULL );
};

#ifndef _REDUCED_ContourDlg_HXX_
#define _REDUCED_CONTDLG_HXX_

class SvxSuperContourDlg;

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SvxContourDlgItem : public SfxControllerItem
{
    SvxSuperContourDlg& rDlg;

protected:

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

public:
                        SvxContourDlgItem( sal_uInt16 nId, SvxSuperContourDlg& rDlg, SfxBindings& rBindings );
};

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxContourDlg : public SfxFloatingWindow
{
    using Window::Update;

    SvxSuperContourDlg* pSuperClass;

//#if 0 // _SOLAR__PRIVATE

protected:

    void                SetSuperClass( SvxSuperContourDlg& rSuperClass ) { pSuperClass = &rSuperClass; }

//#endif // __PRIVATE

public:

                        SvxContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                       Window* pParent, const ResId& rResId );
                        ~SvxContourDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( sal_Bool bLinked );
    const Graphic&      GetGraphic() const;
    sal_Bool                IsGraphicChanged() const;

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon();

    void                SetEditingObject( void* pObj );
    const void*         GetEditingObject() const;

    void                Update( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL, void* pEditingObj = NULL );

    static PolyPolygon  CreateAutoContour(  const Graphic& rGraphic,
                                            const Rectangle* pRect = NULL,
                                            const sal_uIntPtr nFlags = 0L );
    static void         ScaleContour( PolyPolygon& rContour, const Graphic& rGraphic,
                                      const MapUnit eUnit, const Size& rDisplaySize );
};

/*************************************************************************
|*
|* Defines
|*
\************************************************************************/

#define SVXCONTOURDLG() ( (SvxContourDlg*) ( SfxViewFrame::Current()->GetChildWindow(   \
                          SvxContourDlgChildWindow::GetChildWindowId() )->  \
                          GetWindow() ) )

#endif // _REDUCED_CONTDLG_HXX_
#endif // _CONTDLG_HXX_

