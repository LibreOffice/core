/*************************************************************************
 *
 *  $RCSfile: print2.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:58:03 $
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

#define _SV_PRINT_CXX
#define _SPOOLPRINTER_EXT

#include <functional>
#include <algorithm>
#include <utility>
#include <list>
#include <vector>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

// -----------
// - Defines -
// -----------

#define MAX_TILE_WIDTH  1024
#define MAX_TILE_HEIGHT 1024

// -----------
// - Printer -
// -----------

// #i10613# Extracted from Printer::GetPreparedMetaFile
static bool ImplIsActionSpecial( const MetaAction& rAct )
{
    bool bRet( false );

    if( META_TRANSPARENT_ACTION == rAct.GetType() )
        bRet = true;
    else if( META_FLOATTRANSPARENT_ACTION == rAct.GetType() )
        bRet = true;
    else if( META_BMPEX_ACTION == rAct.GetType() )
        bRet = static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().IsAlpha() != 0;
    else if( META_BMPEXSCALE_ACTION == rAct.GetType() )
        bRet = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx().IsAlpha() != 0;
    else if( META_BMPEXSCALEPART_ACTION == rAct.GetType() )
        bRet = static_cast<const MetaBmpExScalePartAction&>(rAct).GetBitmapEx().IsAlpha() != 0;

    return bRet;
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
// Returns true, if given action creates visible (i.e. non-transparent) output
static bool ImplIsNotTransparent( const MetaAction& rAct, const OutputDevice& rOut )
{
    const bool  bLineTransparency( rOut.GetLineColor().GetTransparency() == 255 );
    const bool  bFillTransparency( rOut.GetFillColor().GetTransparency() == 255 );
    bool        bRet( false );

    switch( rAct.GetType() )
    {
        case META_POINT_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_LINE_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_RECT_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ROUNDRECT_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ELLIPSE_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ARC_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_PIE_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_CHORD_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_POLYLINE_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_POLYGON_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_POLYPOLYGON_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_TEXT_ACTION:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const XubString aString( rTextAct.GetText(), rTextAct.GetIndex(), rTextAct.GetLen() );

            if( aString.Len() )
                bRet = true;
        }
        break;

        case META_TEXTARRAY_ACTION:
        {
            const MetaTextArrayAction& rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const XubString aString( rTextAct.GetText(), rTextAct.GetIndex(), rTextAct.GetLen() );

            if( aString.Len() )
                bRet = true;
        }
        break;

        case META_PIXEL_ACTION:
        case META_BMP_ACTION:
        case META_BMPSCALE_ACTION:
        case META_BMPSCALEPART_ACTION:
        case META_BMPEX_ACTION:
        case META_BMPEXSCALE_ACTION:
        case META_BMPEXSCALEPART_ACTION:
        case META_MASK_ACTION:
        case META_MASKSCALE_ACTION:
        case META_MASKSCALEPART_ACTION:
        case META_GRADIENT_ACTION:
        case META_GRADIENTEX_ACTION:
        case META_HATCH_ACTION:
        case META_WALLPAPER_ACTION:
        case META_TRANSPARENT_ACTION:
        case META_FLOATTRANSPARENT_ACTION:
        case META_EPS_ACTION:
        case META_TEXTRECT_ACTION:
        case META_STRETCHTEXT_ACTION:
        case META_TEXTLINE_ACTION:
            // all other actions: generate non-transparent output
            bRet = true;
            break;

        default:
            break;
    }

    return bRet;
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
static Rectangle ImplCalcActionBounds( const MetaAction& rAct, const OutputDevice& rOut )
{
    Rectangle aActionBounds;

    switch( rAct.GetType() )
    {
        case META_PIXEL_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaPixelAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case META_POINT_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaPointAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case META_LINE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaLineAction&>(rAct).GetStartPoint(),
                                       static_cast<const MetaLineAction&>(rAct).GetEndPoint() );
            break;

        case META_RECT_ACTION:
            aActionBounds = static_cast<const MetaRectAction&>(rAct).GetRect();
            break;

        case META_ROUNDRECT_ACTION:
            aActionBounds = Polygon( static_cast<const MetaRoundRectAction&>(rAct).GetRect(),
                                     static_cast<const MetaRoundRectAction&>(rAct).GetHorzRound(),
                                     static_cast<const MetaRoundRectAction&>(rAct).GetVertRound() ).GetBoundRect();
            break;

        case META_ELLIPSE_ACTION:
        {
            const Rectangle& rRect = static_cast<const MetaEllipseAction&>(rAct).GetRect();
            aActionBounds = Polygon( rRect.Center(),
                                     rRect.GetWidth() >> 1,
                                     rRect.GetHeight() >> 1 ).GetBoundRect();
            break;
        }

        case META_ARC_ACTION:
            aActionBounds = Polygon( static_cast<const MetaArcAction&>(rAct).GetRect(),
                                     static_cast<const MetaArcAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaArcAction&>(rAct).GetEndPoint(), POLY_ARC ).GetBoundRect();
            break;

        case META_PIE_ACTION:
            aActionBounds = Polygon( static_cast<const MetaPieAction&>(rAct).GetRect(),
                                     static_cast<const MetaPieAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaPieAction&>(rAct).GetEndPoint(), POLY_PIE ).GetBoundRect();
            break;

        case META_CHORD_ACTION:
            aActionBounds = Polygon( static_cast<const MetaChordAction&>(rAct).GetRect(),
                                     static_cast<const MetaChordAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaChordAction&>(rAct).GetEndPoint(), POLY_CHORD ).GetBoundRect();
            break;

        case META_POLYLINE_ACTION:
            aActionBounds = static_cast<const MetaPolyLineAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case META_POLYGON_ACTION:
            aActionBounds = static_cast<const MetaPolygonAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case META_POLYPOLYGON_ACTION:
            aActionBounds = static_cast<const MetaPolyPolygonAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_BMP_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case META_BMPSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpScaleAction&>(rAct).GetSize() );
            break;

        case META_BMPSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_BMPEX_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().GetSizePixel() ) );
            break;

        case META_BMPEXSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize() );
            break;

        case META_BMPEXSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_MASK_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaMaskAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case META_MASKSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaMaskScaleAction&>(rAct).GetSize() );
            break;

        case META_MASKSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaMaskScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_GRADIENT_ACTION:
            aActionBounds = static_cast<const MetaGradientAction&>(rAct).GetRect();
            break;

        case META_GRADIENTEX_ACTION:
            aActionBounds = static_cast<const MetaGradientExAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_HATCH_ACTION:
            aActionBounds = static_cast<const MetaHatchAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_WALLPAPER_ACTION:
            aActionBounds = static_cast<const MetaWallpaperAction&>(rAct).GetRect();
            break;

        case META_TRANSPARENT_ACTION:
            aActionBounds = static_cast<const MetaTransparentAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_FLOATTRANSPARENT_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaFloatTransparentAction&>(rAct).GetPoint(),
                                       static_cast<const MetaFloatTransparentAction&>(rAct).GetSize() );
            break;

        case META_EPS_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaEPSAction&>(rAct).GetPoint(),
                                       static_cast<const MetaEPSAction&>(rAct).GetSize() );
            break;

        case META_TEXT_ACTION:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const XubString aString( rTextAct.GetText(), rTextAct.GetIndex(), rTextAct.GetLen() );

            if( aString.Len() )
            {
                const Point aPtLog( rTextAct.GetPoint() );

                // #105987# Use API method instead of Impl* methods
                // #107490# Set base parameter equal to index parameter
                rOut.GetTextBoundRect( aActionBounds, rTextAct.GetText(), rTextAct.GetIndex(),
                                       rTextAct.GetIndex(), rTextAct.GetLen() );
                aActionBounds.Move( aPtLog.X(), aPtLog.Y() );
            }
        }
        break;

        case META_TEXTARRAY_ACTION:
        {
            const MetaTextArrayAction&  rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const XubString             aString( rTextAct.GetText(), rTextAct.GetIndex(), rTextAct.GetLen() );
            const long                  nLen = aString.Len();

            if( nLen )
            {
                // #105987# ImplLayout takes everything in logical coordinates
                SalLayout* pSalLayout = rOut.ImplLayout( rTextAct.GetText(), rTextAct.GetIndex(),
                                                         rTextAct.GetLen(), rTextAct.GetPoint(),
                                                         0, rTextAct.GetDXArray() );
                if( pSalLayout )
                {
                    Rectangle aBoundRect( const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect( *pSalLayout ) );
                    aActionBounds = rOut.PixelToLogic( aBoundRect );
                    pSalLayout->Release();
                }
            }
        }
        break;

        case META_TEXTRECT_ACTION:
            aActionBounds = static_cast<const MetaTextRectAction&>(rAct).GetRect();
            break;

        case META_STRETCHTEXT_ACTION:
            DBG_ERROR("META_STRETCHTEXT_ACTION not supported");
            break;

        case META_TEXTLINE_ACTION:
            DBG_ERROR("META_TEXTLINE_ACTION not supported");
            break;

        default:
            break;
    }

    if( !aActionBounds.IsEmpty() )
        return rOut.LogicToPixel( aActionBounds );
    else
        return Rectangle();
}

static bool ImplIsActionHandlingTransparency( const MetaAction& rAct )
{
    // META_FLOATTRANSPARENT_ACTION can contain a whole metafile,
    // which is to be rendered with the given transparent gradient. We
    // currently cannot emulate transparent painting on a white
    // background reliably.

    // For the time being, bitmaps are also not handled correctly,
    // since they don't calculate explicit alpha against white
    // background in ImplQPrinter::ImplPrintMtf.

    // Leaves us with META_TRANSPARENT_ACTION
    if( rAct.GetType() == META_TRANSPARENT_ACTION )
        return true;
    else
        return false;
}

// predicate functor for checking whether given element is fully transparent
class Impl_IsNotTransparent : public ::std::unary_function< ::std::pair< const MetaAction*, int >, bool >
{
public:
    Impl_IsNotTransparent( const OutputDevice& rOut ) : mrOut(rOut) {}
    bool operator()( ::std::pair< const MetaAction*, int > rElem )
    {
        return ImplIsNotTransparent( *rElem.first, mrOut );
    }

private:
    const OutputDevice& mrOut;
};

typedef ::std::pair< const MetaAction*, int > Component; // MetaAction plus index in metafile

// List of (intersecting) actions, plus overall bounds
struct ConnectedComponents
{
    ::std::list< Component >    aComponentList;
    Rectangle                   aBounds;
    bool                        bIsSpecial;
};

typedef ::std::list< ConnectedComponents > ConnectedComponentsList;

// remove comment to enable highlighting of generated output
#ifdef DBG_UTIL
//#define DEBUG_GetPreparedMetaFile
#endif

void Printer::GetPreparedMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                   long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    const PrinterOptions&   rPrinterOptions = GetPrinterOptions();
    MetaAction*             pCurrAct;
    bool                    bTransparent( false );

    rOutMtf.Clear();

    if( !rPrinterOptions.IsReduceTransparency() ||
        ( PRINTER_TRANSPARENCY_AUTO == rPrinterOptions.GetReducedTransparencyMode() ) )
    {
        // watch for transparent drawing actions
        for( pCurrAct = ( (GDIMetaFile&) rInMtf ).FirstAction();
             pCurrAct && !bTransparent;
             pCurrAct = ( (GDIMetaFile&) rInMtf ).NextAction() )
        {
            // #i10613# Extracted "specialness" predicate into extra method
            if( ImplIsActionSpecial( *pCurrAct ) )
                bTransparent = true;
        }
    }

    // #i10613# Determine set of connected components containing transparent objects. These are
    // then processed as bitmaps, the original actions are removed from the metafile.
    if( !bTransparent )
    {
        // nothing transparent -> just copy
        rOutMtf = rInMtf;
    }
    else
    {
        // #i10613#
        // This works as follows: we want a number of distinct sets of
        // connected components, where each set contains metafile
        // actions that are intersecting (note: there are possibly
        // more actions contained as are directly intersecting,
        // because we can only produce rectangular bitmaps later
        // on. Thus, each set of connected components is the smallest
        // enclosing, axis-aligned rectangle that completely bounds a
        // number of intersecting metafile actions, plus any action
        // that would otherwise be cut in two). Therefore, we
        // iteratively add metafile actions from the original metafile
        // to this connected components list (aCCList), by checking
        // each element's bounding box against intersection with the
        // metaaction at hand.
        // All those intersecting elements are removed from aCCList
        // and collected in a temporary list (aCCMergeList). After all
        // elements have been checked, the aCCMergeList elements are
        // merged with the metaaction at hand into one resulting
        // connected component, with one big bounding box, and
        // inserted into aCCList again.
        // The time complexity of this algorithm is O(n^3), where n is
        // the number of metafile actions, and it finds all distinct
        // regions of rectangle-bounded connected components. This
        // algorithm was designed by AF.
        //

        ConnectedComponentsList aCCList;        // list containing distinct sets of connected components as elements.
        int                     nActionNum;

        // create an OutputDevice to record mapmode changes and the like
        VirtualDevice aMapModeVDev;
        aMapModeVDev.mnDPIX = mnDPIX;
        aMapModeVDev.mnDPIY = mnDPIY;
        aMapModeVDev.EnableOutput(FALSE);

        //
        //  STAGE 1: Generate connected components list
        //  ===========================================
        //

        // iterate over all actions
        for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            // execute action to get correct MapModes etc.
            pCurrAct->Execute( &aMapModeVDev );

            // cache bounds of current action
            const Rectangle aBBCurrAct( ImplCalcActionBounds(*pCurrAct, aMapModeVDev) );

            // accumulate collected bounds here, initialize with current action
            Rectangle                               aTotalBounds( aBBCurrAct ); // thus,
                                                                                // aTotalComponents.aBounds
                                                                                // is
                                                                                // empty
                                                                                // for
                                                                                // non-output-generating
                                                                                // actions
            bool                                    bTreatSpecial( false );
            ConnectedComponents                     aTotalComponents;

            //
            //  STAGE 1.1: Search for intersecting cc entries
            //  =============================================
            //

            // if aBBCurrAct is empty, it will intersect with no
            // aCCList member. Thus, we can safe us the check.
            // Furthermore, this ensures that non-output-generating
            // actions get their own aCCList entry, which is necessary
            // when copying them to the output metafile (see stage 3
            // below).
            if( !aBBCurrAct.IsEmpty() )
            {
                ConnectedComponentsList::iterator       aCurrCC;
                const ConnectedComponentsList::iterator aLastCC( aCCList.end() );
                bool                                    bSomeComponentsChanged;

                // now, this is unfortunate: since changing anyone of
                // the aCCList elements (e.g. by merging or addition
                // of an action) might generate new intersection with
                // other aCCList elements, have to repeat the whole
                // element scanning, until nothing changes anymore.
                // Thus, this loop here makes us O(n^3) in the worst
                // case.
                do
                {
                    // only loop here if 'intersects' branch below was hit
                    bSomeComponentsChanged = false;

                    // iterate over all current members of aCCList
                    for( aCurrCC=aCCList.begin(); aCurrCC != aLastCC; )
                    {
                        // first check if current element's bounds are
                        // empty. This ensures that empty actions are not
                        // merged into one component, as a matter of fact,
                        // they have no position.
                        if( !aCurrCC->aBounds.IsEmpty() &&
                            aCurrCC->aBounds.IsOver( aTotalBounds ) )
                        {
                            // union the intersecting aCCList element into aTotalComponents

                            // calc union bounding box
                            aTotalBounds.Union( aCurrCC->aBounds );

                            // extract all aCurr actions to aTotalComponents
                            aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                                    aCurrCC->aComponentList );

                            if( aCurrCC->bIsSpecial )
                                bTreatSpecial = true;

                            // remove and delete aCurrCC element from list (we've now merged its content)
                            aCurrCC = aCCList.erase( aCurrCC );

                            // at least one component changed, need to rescan everything
                            bSomeComponentsChanged = true;
                        }
                        else
                        {
                            ++aCurrCC;
                        }
                    }
                }
                while( bSomeComponentsChanged );
            }

            //
            //  STAGE 1.2: Determine special state for cc element
            //  =================================================
            //

            // now test whether the whole connected component must be
            // treated specially (i.e. rendered as a bitmap): if the
            // added action is the very first action, or all actions
            // before it are completely transparent, the connected
            // component need not be treated specially, not even if
            // the added action contains transparency. This is because
            // painting of transparent objects on _white background_
            // works without alpha compositing (you just calculate the
            // color). Note that for the test "all objects before me
            // are transparent" no sorting is necessary, since the
            // added metaaction pCurrAct is always in the order the
            // metafile is painted. Generally, the order of the
            // metaactions in the ConnectedComponents are not
            // guaranteed to be the same as in the metafile.
            if( bTreatSpecial )
            {
                // prev component(s) special -> this one, too
                aTotalComponents.bIsSpecial = true;
            }
            else if( !ImplIsActionSpecial( *pCurrAct ) )
            {
                // added action and none of prev components special ->
                // this one normal, too
                aTotalComponents.bIsSpecial = false;
            }
            else
            {
                // added action is special and none of prev components
                // special -> do the detailed tests

                // can the action handle transparency correctly
                // (i.e. when painted on white background, does the
                // action still look correct)?
                if( !ImplIsActionHandlingTransparency( *pCurrAct ) )
                {
                    // no, action cannot handle its transparency on
                    // a printer device, render to bitmap
                    aTotalComponents.bIsSpecial = true;
                }
                else
                {
                    // yes, action can handle its transparency, so
                    // check whether we're on white background
                    if( aTotalComponents.aComponentList.empty() )
                    {
                        // nothing between pCurrAct and empty page
                        // background -> don't be special
                        aTotalComponents.bIsSpecial = false;
                    }
                    else
                    {
                        // check if _every_ object between pCurrAct and
                        // empty sheet of paper is fully transparent
                        Impl_IsNotTransparent aSpecialPredicate( aMapModeVDev );
                        if( ::std::find_if( aTotalComponents.aComponentList.begin(),
                                            aTotalComponents.aComponentList.end(),
                                            aSpecialPredicate ) == aTotalComponents.aComponentList.end() )
                        {
                            // predicate Impl_IsNotTransparent never
                            // returned true, so every object is
                            // transparent -> don't be special
                            aTotalComponents.bIsSpecial = false;
                        }
                        else
                        {
                            // predicate Impl_IsNotTransparent returned
                            // true at least once, so there's a visible
                            // object between pCurrAct and the empty sheet
                            // of paper -> be special, then
                            aTotalComponents.bIsSpecial = true;
                        }
                    }
                }
            }

            //
            //  STAGE 1.3: Add newly generated CC list element
            //  ==============================================
            //

            // set new bounds and add action to list
            aTotalComponents.aBounds = aTotalBounds;
            aTotalComponents.aComponentList.push_back( ::std::make_pair(
                        const_cast<const MetaAction*>(pCurrAct), nActionNum) );

            // add aTotalComponents as a new entry to aCCList
            aCCList.push_back( aTotalComponents );

            DBG_ASSERT( !aTotalComponents.aComponentList.empty(),
                        "Printer::GetPreparedMetaFile empty component" );
            DBG_ASSERT( !aTotalComponents.aBounds.IsEmpty() ||
                        (aTotalComponents.aBounds.IsEmpty() && aTotalComponents.aComponentList.size() == 1),
                        "Printer::GetPreparedMetaFile non-output generating actions must be solitary");
        }

        // well now, we've got the list of disjunct connected
        // components. Now we've got to create a map, which contains
        // the corresponding aCCList element for every
        // metaaction. Later on, we always process the complete
        // metafile for each bitmap to be generated, but switch on
        // output only for actions contained in the then current
        // aCCList element. This ensures correct mapmode and attribute
        // settings for all cases.

        // maps mtf actions to CC list entries
        ::std::vector< const ConnectedComponents* > aCCList_MemberMap( rInMtf.GetActionCount() );

        // iterate over all aCCList members and their contained metaactions
        ConnectedComponentsList::iterator       aCurr( aCCList.begin() );
        const ConnectedComponentsList::iterator aLast( aCCList.end() );
        for( ; aCurr != aLast; ++aCurr )
        {
            ::std::list< Component >::iterator          aCurrAct( aCurr->aComponentList.begin() );
            const ::std::list< Component >::iterator    aLastAct( aCurr->aComponentList.end() );
            for( ; aCurrAct != aLastAct; ++aCurrAct )
            {
                // set pointer to aCCList element for corresponding index
                aCCList_MemberMap[ aCurrAct->second ] = &(*aCurr);
            }
        }

        //
        //  STAGE 2: Generate banded bitmaps for special regions
        //  ====================================================
        //

        Point aTmpPoint;
        const Rectangle aOutputRect( aTmpPoint, GetOutputSizePixel() );

        // iterate over all aCCList members and generate bitmaps for the special ones
        for( aCurr = aCCList.begin(); aCurr != aLast; ++aCurr )
        {
            if( aCurr->bIsSpecial )
            {
                Rectangle aBoundRect( aCurr->aBounds );
                aBoundRect.Intersection( aOutputRect );

                const double fBmpArea( (double) aBoundRect.GetWidth() * aBoundRect.GetHeight() );
                const double fOutArea( (double) aOutputRect.GetWidth() * aOutputRect.GetHeight() );

                // check if output doesn't exceed given size
                if( rPrinterOptions.IsReduceTransparency() &&
                    ( PRINTER_TRANSPARENCY_AUTO == rPrinterOptions.GetReducedTransparencyMode() ) &&
                    ( fBmpArea > ( 0.25 * fOutArea ) ) )
                {
                    // output normally. Therefore, we simply clear the
                    // special attribute, as everything non-special is
                    // copied to rOutMtf further below.
                    aCurr->bIsSpecial = false;
                }
                else
                {
                    // create new bitmap action first
                    if( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                    {
                        Point           aDstPtPix( aBoundRect.TopLeft() );
                        Size            aDstSzPix;

                        VirtualDevice   aMapVDev;   // here, we record only mapmode information
                        aMapVDev.EnableOutput(FALSE);

                        VirtualDevice   aPaintVDev; // into this one, we render.

                        rOutMtf.AddAction( new MetaPushAction( PUSH_MAPMODE ) );
                        rOutMtf.AddAction( new MetaMapModeAction() );

                        aPaintVDev.SetDrawMode( GetDrawMode() );

                        while( aDstPtPix.Y() <= aBoundRect.Bottom() )
                        {
                            aDstPtPix.X() = aBoundRect.Left();
                            aDstSzPix = Size( MAX_TILE_WIDTH, MAX_TILE_HEIGHT );

                            if( ( aDstPtPix.Y() + aDstSzPix.Height() - 1L ) > aBoundRect.Bottom() )
                                aDstSzPix.Height() = aBoundRect.Bottom() - aDstPtPix.Y() + 1L;

                            while( aDstPtPix.X() <= aBoundRect.Right() )
                            {
                                if( ( aDstPtPix.X() + aDstSzPix.Width() - 1L ) > aBoundRect.Right() )
                                    aDstSzPix.Width() = aBoundRect.Right() - aDstPtPix.X() + 1L;

                                if( !Rectangle( aDstPtPix, aDstSzPix ).Intersection( aBoundRect ).IsEmpty() &&
                                    aPaintVDev.SetOutputSizePixel( aDstSzPix ) )
                                {
                                    aPaintVDev.Push();
                                    aMapVDev.Push();

                                    aMapVDev.mnDPIX = aPaintVDev.mnDPIX = mnDPIX;
                                    aMapVDev.mnDPIY = aPaintVDev.mnDPIY = mnDPIY;

                                    aPaintVDev.EnableOutput(FALSE);

                                    // iterate over all actions
                                    for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
                                         pCurrAct;
                                         pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
                                    {
                                        // enable output only for
                                        // actions that are members of
                                        // the current aCCList element
                                        // (aCurr)
                                        if( aCCList_MemberMap[nActionNum] == &(*aCurr) )
                                            aPaintVDev.EnableOutput(TRUE);

                                        // but process every action
                                        const USHORT nType( pCurrAct->GetType() );

                                        if( META_MAPMODE_ACTION == nType )
                                        {
                                            pCurrAct->Execute( &aMapVDev );

                                            MapMode     aMtfMap( aMapVDev.GetMapMode() );
                                            const Point aNewOrg( aMapVDev.PixelToLogic( aDstPtPix ) );

                                            aMtfMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                                            aPaintVDev.SetMapMode( aMtfMap );
                                        }
                                        else if( ( META_PUSH_ACTION == nType ) || ( META_POP_ACTION ) == nType )
                                        {
                                            pCurrAct->Execute( &aMapVDev );
                                            pCurrAct->Execute( &aPaintVDev );
                                        }
                                        else if( META_GRADIENT_ACTION == nType )
                                        {
                                            MetaGradientAction* pGradientAction = static_cast<MetaGradientAction*>(pCurrAct);
                                            DrawGradientEx( &aPaintVDev, pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                        }
                                        else
                                        {
                                            pCurrAct->Execute( &aPaintVDev );
                                        }

                                        if( !( nActionNum % 4 ) )
                                            Application::Reschedule();
                                    }

                                    const BOOL bOldMap = mbMap;
                                    mbMap = aPaintVDev.mbMap = FALSE;

                                    Bitmap aBandBmp( aPaintVDev.GetBitmap( Point(), aDstSzPix ) );

                                    // scale down bitmap, if requested
                                    if( rPrinterOptions.IsReduceBitmaps() && rPrinterOptions.IsReducedBitmapIncludesTransparency() )
                                    {
                                        aBandBmp = GetPreparedBitmap( aDstPtPix, aDstSzPix,
                                                                      Point(), aBandBmp.GetSizePixel(),
                                                                      aBandBmp, nMaxBmpDPIX, nMaxBmpDPIY );
                                    }

#ifdef DEBUG_GetPreparedMetaFile
                                    //aBandBmp.Invert();
#endif

                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) );
                                    rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_END" ) );

                                    aPaintVDev.mbMap = TRUE;
                                    mbMap = bOldMap;
                                    aMapVDev.Pop();
                                    aPaintVDev.Pop();
                                }

                                // overlapping bands to avoid missing lines (e.g. PostScript)
                                aDstPtPix.X() += aDstSzPix.Width();
                            }

                            // overlapping bands to avoid missing lines (e.g. PostScript)
                            aDstPtPix.Y() += aDstSzPix.Height();
                        }

                        rOutMtf.AddAction( new MetaPopAction() );
                    }
                }
            }
        }

        //
        //  STAGE 3: Copy actions to output metafile
        //  ========================================
        //

        // iterate over all actions and duplicate the ones not in a
        // special aCCList member into rOutMtf
        for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            // NOTE: This relies on the fact that map-mode or draw
            // mode changing actions are solitary aCCList elements and
            // have empty bounding boxes, see comment on stage 1.1
            // above
            if( aCCList_MemberMap[nActionNum] &&
                (aCCList_MemberMap[nActionNum]->aBounds.IsEmpty() ||
                 !aCCList_MemberMap[nActionNum]->bIsSpecial) )
            {
                rOutMtf.AddAction( ( pCurrAct->Duplicate(), pCurrAct ) );
            }
        }

        rOutMtf.SetPrefMapMode( rInMtf.GetPrefMapMode() );
        rOutMtf.SetPrefSize( rInMtf.GetPrefSize() );

#ifdef DEBUG_GetPreparedMetaFile
        // iterate over all aCCList members and generate rectangles for the bounding boxes
        rOutMtf.AddAction( new MetaFillColorAction( COL_WHITE, FALSE ) );
        for( aCurr = aCCList.begin(); aCurr != aLast; ++aCurr )
        {
            if( aCurr->bIsSpecial )
                rOutMtf.AddAction( new MetaLineColorAction( COL_RED, TRUE) );
            else
                rOutMtf.AddAction( new MetaLineColorAction( COL_BLUE, TRUE) );

            rOutMtf.AddAction( new MetaRectAction( aMapModeVDev.PixelToLogic( aCurr->aBounds ) ) );
        }
#endif
    }
}

// -----------------------------------------------------------------------------

Bitmap Printer::GetPreparedBitmap( const Point& rDstPt, const Size& rDstSz,
                                   const Point& rSrcPt, const Size& rSrcSz,
                                   const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    Bitmap aBmp( rBmp );

    if( !aBmp.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmp.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if neccessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            // do downsampling if neccessary
            Size aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );

            // #103209# Normalize size (mirroring has to happen outside of this method)
            aDstSizeTwip = Size( labs(aDstSizeTwip.Width()), labs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmp.Scale( aNewBmpSize );
                else
                    aBmp.SetEmpty();
            }
        }
    }

    return aBmp;
}

// -----------------------------------------------------------------------------

BitmapEx Printer::GetPreparedBitmapEx( const Point& rDstPt, const Size& rDstSz,
                                       const Point& rSrcPt, const Size& rSrcSz,
                                       const BitmapEx& rBmpEx, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    BitmapEx aBmpEx( rBmpEx );

    if( !aBmpEx.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmpEx.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if neccessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmpEx.Crop( aSrcRect );
            else
                aBmpEx.SetEmpty();
        }

        if( !aBmpEx.IsEmpty() )
        {
            // do downsampling if neccessary
            Size aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );

            // #103209# Normalize size (mirroring has to happen outside of this method)
            aDstSizeTwip = Size( labs(aDstSizeTwip.Width()), labs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmpEx.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmpEx.Scale( aNewBmpSize );
                else
                    aBmpEx.SetEmpty();
            }
        }
    }

    return aBmpEx;
}

// -----------------------------------------------------------------------------

void Printer::DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( PRINTER_GRADIENT_STRIPES == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rRect, aNewGradient );
            }
            else
                pOut->DrawGradient( rRect, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const long      nR = ( ( (long) rStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nG = ( ( (long) rStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nB = ( ( (long) rStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const Color     aColor( (BYTE) nR, (BYTE) nG, (BYTE) nB );

            pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rRect, rGradient );
}

// -----------------------------------------------------------------------------

void Printer::DrawGradientEx( OutputDevice* pOut, const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( PRINTER_GRADIENT_STRIPES == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rPolyPoly, aNewGradient );
            }
            else
                pOut->DrawGradient( rPolyPoly, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const long      nR = ( ( (long) rStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nG = ( ( (long) rStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nB = ( ( (long) rStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const Color     aColor( (BYTE) nR, (BYTE) nG, (BYTE) nB );

            pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawPolyPolygon( rPolyPoly );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rPolyPoly, rGradient );
}
