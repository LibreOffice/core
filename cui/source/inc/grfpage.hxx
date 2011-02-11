/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_GRFPAGE_HXX
#define _SVX_GRFPAGE_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <vcl/graph.hxx>
#include <sfx2/tabdlg.hxx>


class SvxGrfCropPage : public SfxTabPage
{
    using Window::CalcZoom;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    class SvxCropExample : public Window
    {
        Size        aFrameSize;
        Point       aTopLeft, aBottomRight;
        Graphic     aGrf;

    public:
        SvxCropExample( Window* pPar, const ResId& rResId );

        virtual void Paint( const Rectangle& rRect );

        void SetTopLeft( const Point& rNew )    { aTopLeft = rNew; }
        void SetTop( long nVal )                { aTopLeft.X() = nVal; }
        void SetBottom( long nVal )             { aBottomRight.X() = nVal; }
        void SetLeft( long nVal )               { aTopLeft.Y() = nVal; }
        void SetRight( long nVal)               { aBottomRight.Y() = nVal; }
        void SetBottomRight(const Point& rNew ) { aBottomRight = rNew; }
        void SetFrameSize( const Size& rSz );
        void SetGraphic( const Graphic& rGrf )  { aGrf = rGrf; }
        const Graphic& GetGraphic() const       { return aGrf; }
    };

    FixedLine       aCropFL;
    RadioButton     aZoomConstRB;
    RadioButton     aSizeConstRB;
    FixedText       aLeftFT;
    MetricField     aLeftMF;
    FixedText       aRightFT;
    MetricField     aRightMF;
    FixedText       aTopFT;
    MetricField     aTopMF;
    FixedText       aBottomFT;
    MetricField     aBottomMF;

    FixedLine       aZoomFL;
    FixedText       aWidthZoomFT;
    MetricField     aWidthZoomMF;
    FixedText       aHeightZoomFT;
    MetricField     aHeightZoomMF;

    FixedLine       aSizeFL;
    FixedText       aWidthFT;
    MetricField     aWidthMF;
    FixedText       aHeightFT;
    MetricField     aHeightMF;
    FixedText       aOrigSizeFT;
    PushButton      aOrigSizePB;

    // Example
    SvxCropExample  aExampleWN;


    Timer           aTimer;
    String          aGraphicName;
    Size            aOrigSize;
    Size            aPageSize;
    const MetricField*  pLastCropField;
    long            nOldWidth;
    long            nOldHeight;
    sal_Bool            bReset;
    sal_Bool            bInitialized;
    sal_Bool            bSetOrigSize;


    SvxGrfCropPage( Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxGrfCropPage();

    DECL_LINK( ZoomHdl, MetricField * );
    DECL_LINK( SizeHdl, MetricField * );
    DECL_LINK( CropHdl, const MetricField * );
    DECL_LINK( CropLoseFocusHdl, MetricField * );
    DECL_LINK( CropModifyHdl, MetricField * );
    DECL_LINK( OrigSizeHdl, PushButton * );
    DECL_LINK( Timeout, Timer * );

    void            CalcZoom();
    void            CalcMinMaxBorder();
    void            GraphicHasChanged(sal_Bool bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet);

    Size            GetGrfOrigSize( const Graphic& ) const;
public:
    static SfxTabPage *Create( Window *pParent, const SfxItemSet &rSet );

    virtual sal_Bool FillItemSet( SfxItemSet &rSet );
    virtual void Reset( const SfxItemSet &rSet );
    virtual int DeactivatePage( SfxItemSet *pSet );
};


#endif
