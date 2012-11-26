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
    void            GraphicHasChanged(bool bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet);

    Size            GetGrfOrigSize( const Graphic& ) const;
public:
    static SfxTabPage *Create( Window *pParent, const SfxItemSet &rSet );

    virtual sal_Bool FillItemSet( SfxItemSet &rSet );
    virtual void Reset( const SfxItemSet &rSet );
    virtual int DeactivatePage( SfxItemSet *pSet );
};


#endif
