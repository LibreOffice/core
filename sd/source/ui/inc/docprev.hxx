/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docprev.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:00:46 $
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

#ifndef _SD_DOCPREV_HXX_
#define _SD_DOCPREV_HXX_

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#include <rtl/ref.hxx>

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SD_FADEDEF_H
#include "fadedef.h"
#endif
#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

namespace sd {
    class SlideShow;
}

class GDIMetaFile;

class SD_DLLPUBLIC SdDocPreviewWin : public Control, public SfxListener
{
protected:
    GDIMetaFile*    pMetaFile;
    BOOL            bInEffect;
    Link            aClickHdl;
    SfxObjectShell* mpObj;
    sal_uInt16      mnShowPage;
    Color           maDocumentColor;
    rtl::Reference< sd::SlideShow > mxSlideShow;

    virtual void    Paint( const Rectangle& rRect );
    static void     CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint );
    void            ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev );

    static const int FRAME;

    svtools::ColorConfig maColorConfig;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

    void updateViewSettings();

public:
                    SdDocPreviewWin( Window* pParent, const ResId& rResId );
                    SdDocPreviewWin( Window* pParent );
                    ~SdDocPreviewWin();
    void            SetObjectShell( SfxObjectShell* pObj, sal_uInt16 nShowPage = 0 );
    void            SetGDIFile( GDIMetaFile* pFile );
    virtual void    Resize();
    void            startPreview();

    virtual long    Notify( NotifyEvent& rNEvt );

    void            SetClickHdl( const Link& rLink ) { aClickHdl = rLink; }
    const Link&     GetClickHdl() const { return aClickHdl; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

};

#endif

