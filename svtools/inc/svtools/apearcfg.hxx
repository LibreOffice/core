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


#ifndef _SVT_APEARCFG_HXX
#define _SVT_APEARCFG_HXX

#include "svtools/svtdllapi.h"
#include "tools/solar.h"
#include <unotools/configitem.hxx>

class Application;

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
typedef enum {
    SnapToButton = 0,
    SnapToMiddle,
    NoSnap
} SnapType;

typedef enum { // MUST match the order chosen in ListBox LB_DRAG_MODE in optgdlg.src
    DragFullWindow,
    DragFrame,
    DragSystemDep
} DragMode;


class SVT_DLLPUBLIC SvtTabAppearanceCfg : public utl::ConfigItem
{
    short           nDragMode           ;
    short           nScaleFactor        ;
    short           nSnapMode           ;
    short           nMiddleMouse;
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    short           nAAMinPixelHeight   ;
#endif

    sal_Bool            bMenuMouseFollow        ;
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    sal_Bool            bFontAntialiasing       ;
#endif

    static sal_Bool  bInitialized ;

    SVT_DLLPRIVATE const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();

public:
    SvtTabAppearanceCfg( );
    ~SvtTabAppearanceCfg( );

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);

    sal_uInt16  GetDragMode  () const { return nDragMode; }
    void        SetDragMode  ( sal_uInt16 nSet );

    sal_uInt16      GetScaleFactor () const { return nScaleFactor; }
    void        SetScaleFactor ( sal_uInt16 nSet );

    sal_uInt16      GetSnapMode () const { return nSnapMode; }
    void        SetSnapMode ( sal_uInt16 nSet );

    sal_uInt16      GetMiddleMouseButton () const { return nMiddleMouse; }
    void        SetMiddleMouseButton ( sal_uInt16 nSet );

    void        SetApplicationDefaults ( Application* pApp );

    void        SetMenuMouseFollow(sal_Bool bSet) {bMenuMouseFollow = bSet; SetModified();}
    sal_Bool        IsMenuMouseFollow() const{return bMenuMouseFollow;}

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    void        SetFontAntiAliasing( sal_Bool bSet )    { bFontAntialiasing = bSet; SetModified(); }
    sal_Bool        IsFontAntiAliasing() const { return bFontAntialiasing; }

    sal_uInt16      GetFontAntialiasingMinPixelHeight( ) const { return nAAMinPixelHeight; }
    void        SetFontAntialiasingMinPixelHeight( sal_uInt16 _nMinHeight ) { nAAMinPixelHeight = _nMinHeight; SetModified(); }
#endif

    static sal_Bool IsInitialized()  { return bInitialized; }
    static void    SetInitialized() { bInitialized = sal_True; }
};

#endif //  _OFA_APEARCFG_HXX
