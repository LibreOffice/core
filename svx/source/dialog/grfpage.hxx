/*************************************************************************
 *
 *  $RCSfile: grfpage.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif


class SvxGrfCropPage : public SfxTabPage
{
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


    GroupBox        aCropGB;
    FixedText       aLeftFT;
    MetricField     aLeftMF;
    FixedText       aRightFT;
    MetricField     aRightMF;
    FixedText       aTopFT;
    MetricField     aTopMF;
    FixedText       aBottomFT;
    MetricField     aBottomMF;
    RadioButton     aSizeConstRB;
    RadioButton     aZoomConstRB;

    GroupBox        aSizeGB;
    FixedText       aWidthFT;
    MetricField     aWidthMF;
    FixedText       aHeightFT;
    MetricField     aHeightMF;

    GroupBox        aZoomGB;
    FixedText       aWidthZoomFT;
    MetricField     aWidthZoomMF;
    FixedText       aHeightZoomFT;
    MetricField     aHeightZoomMF;

    // Example
    SvxCropExample  aExampleWN;
    GroupBox        aExampleGB;

    FixedText       aOrigSizeFT;
    PushButton      aOrigSizePB;

    Timer           aTimer;
    String          aGraphicName;
    Size            aOrigSize;
    Size            aPageSize;
    const MetricField*  pLastCropField;
    long            nOldWidth;
    long            nOldHeight;
    BOOL            bReset;
    BOOL            bInitialized;
    BOOL            bSetOrigSize;


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
    void            GraphicHasChanged(BOOL bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet);

    Size            GetGrfOrigSize( const Graphic& ) const;
public:
    static SfxTabPage *Create( Window *pParent, const SfxItemSet &rSet );

    virtual BOOL FillItemSet( SfxItemSet &rSet );
    virtual void Reset( const SfxItemSet &rSet );
    virtual int DeactivatePage( SfxItemSet *pSet );
};


#endif
