/*************************************************************************
 *
 *  $RCSfile: UnoGraphicExporter.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:05:06 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#include <vector>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XMIMETYPEINFO_HPP_
#include <com/sun/star/document/XMimeTypeInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <svtools/FilterConfigItem.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX
#include <numitem.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif

#ifndef _SVDOGRAF_HXX
#include "svdograf.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _XOUTBMP_HXX
#include "xoutbmp.hxx"
#endif

#ifndef _SVX_IMPGRF_HXX
#include "impgrf.hxx"
#endif

#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVX_FMVIEW_HXX
#include "fmview.hxx"
#endif

#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_UNOWPAGE_HXX
#include "unopage.hxx"
#endif

#ifndef _SVX_PAGEITEM_HXX
#include "pageitem.hxx"
#endif

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

#ifndef _SVDOUTL_HXX
#include "svdoutl.hxx"
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX
#include "flditem.hxx"
#endif

#define MAX_EXT_PIX         2048

using namespace ::comphelper;
using namespace ::osl;
using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

namespace svx
{
    /** implements a component to export shapes or pages to external graphic formats.

        @implements com.sun.star.drawing.GraphicExportFilter
    */
    class GraphicExporter : public WeakImplHelper4< XFilter, XExporter, XServiceInfo, XMimeTypeInfo >
    {
    public:
        GraphicExporter();
        virtual ~GraphicExporter();

        // XFilter
        virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) throw(RuntimeException);
        virtual void SAL_CALL cancel(  ) throw(RuntimeException);

        // XExporter
        virtual void SAL_CALL setSourceDocument( const Reference< lang::XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

        // XMimeTypeInfo
        virtual sal_Bool SAL_CALL supportsMimeType( const ::rtl::OUString& MimeTypeName ) throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedMimeTypeNames(  ) throw (RuntimeException);

        DECL_LINK(PaintProc, SdrPaintProcRec*);

        VirtualDevice* CreatePageVDev( SdrPage* pPage, ULONG nWidthPixel, ULONG nHeightPixel ) const;

        DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

    private:
        Reference< XShape >     mxShape;
        Reference< XDrawPage >  mxPage;
        Reference< XShapes >    mxShapes;

        SvxDrawPage*        mpUnoPage;

        Link                maOldCalcFieldValueHdl;
        sal_Int32           mnPageNumber;
        SdrModel*           mpDoc;
    };

    Reference< XInterface > SAL_CALL GraphicExporter_createInstance(const Reference< XMultiServiceFactory > & rSMgr)
        throw( Exception )
    {
        return (XWeak*)new GraphicExporter();
    }

    Sequence< OUString > SAL_CALL GraphicExporter_getSupportedServiceNames()
        throw()
    {
        Sequence< OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GraphicExportFilter" ) );
        return aSupportedServiceNames;
    }

    OUString SAL_CALL GraphicExporter_getImplementationName()
        throw()
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.GraphicExporter" ) );
    }

    GDIMetaFile RemoveClipRegionActions( const GDIMetaFile& rMtf )
    {
        GDIMetaFile     aMtf;
        const ULONG     nActionCount = rMtf.GetActionCount();

        aMtf.SetPrefSize( rMtf.GetPrefSize() );
        aMtf.SetPrefMapMode( rMtf.GetPrefMapMode() );

        // examine actions and remove ClipRegion-Actions.
        for ( ULONG nAction = 0; nAction < nActionCount; nAction++ )
        {
            MetaAction* pCopyAction = ( (GDIMetaFile&) rMtf ).CopyAction( nAction );

            if( pCopyAction )
            {
                switch( pCopyAction->GetType() )
                {
                    case( META_CLIPREGION_ACTION ) :
                        delete pCopyAction;
                    break;

                    default:
                        aMtf.AddAction( pCopyAction );
                    break;
                }
            }
        }

        return aMtf;
    }

    /** creates a bitmap that is optionaly transparent from a metafile
    */
    BitmapEx GetBitmapFromMetaFile( const GDIMetaFile& rMtf, BOOL bTransparent, const Size* pSize )
    {
        Graphic     aGraphic( rMtf );
        BitmapEx    aBmpEx;

        if( bTransparent )
        {
            Graphic aMaskGraphic( rMtf.GetMonochromeMtf( COL_BLACK ) );
            Bitmap  aMaskBmp( aMaskGraphic.GetBitmap( pSize ) );

            aMaskBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            aBmpEx = BitmapEx( aGraphic.GetBitmap( pSize ), aMaskBmp );
        }
        else
            aBmpEx = BitmapEx( aGraphic.GetBitmap( pSize ) );

        aBmpEx.SetPrefMapMode( rMtf.GetPrefMapMode() );
        aBmpEx.SetPrefSize( rMtf.GetPrefSize() );

        return aBmpEx;
    }

    Size* CalcSize( sal_Int32 nWidth, sal_Int32 nHeight, const Size& aBoundSize, Size& aOutSize )
    {
        if( (nWidth == 0) && (nHeight == 0) )
            return NULL;

        if( (nWidth == 0) && (nHeight != 0) && (aBoundSize.Height() != 0) )
        {
            nWidth = ( nHeight * aBoundSize.Width() ) / aBoundSize.Height();
        }
        else if( (nWidth != 0) && (nHeight == 0) && (aBoundSize.Width() != 0) )
        {
            nHeight = ( nWidth * aBoundSize.Height() ) / aBoundSize.Width();
        }

        aOutSize.Width() = nWidth;
        aOutSize.Height() = nHeight;

        return &aOutSize;
    }
}

using namespace ::svx;

GraphicExporter::GraphicExporter()
: mpUnoPage( NULL ), mpDoc( NULL ), mnPageNumber(-1)
{
}

GraphicExporter::~GraphicExporter()
{
}

IMPL_LINK(GraphicExporter, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if( pInfo && mnPageNumber != -1 )
    {
        const SvxFieldData* pField = pInfo->GetField().GetField();
        if( pField && pField->ISA( SvxPageField ) )
        {
            String aPageNumValue;
            BOOL bUpper = FALSE;

            switch(mpDoc->GetPageNumType())
            {
                case SVX_CHARS_UPPER_LETTER:
                    aPageNumValue += (sal_Unicode)(char)((mnPageNumber - 1) % 26 + 'A');
                    break;
                case SVX_CHARS_LOWER_LETTER:
                    aPageNumValue += (sal_Unicode)(char)((mnPageNumber - 1) % 26 + 'a');
                    break;
                case SVX_ROMAN_UPPER:
                    bUpper = TRUE;
                case SVX_ROMAN_LOWER:
                    aPageNumValue += SvxNumberFormat::CreateRomanString(mnPageNumber, bUpper);
                    break;
                case SVX_NUMBER_NONE:
                    aPageNumValue.Erase();
                    aPageNumValue += sal_Unicode(' ');
                    break;
                default:
                    aPageNumValue += String::CreateFromInt32( (sal_Int32)mnPageNumber );
            }

            pInfo->SetRepresentation( aPageNumValue );

            return(0);
        }
    }

    return maOldCalcFieldValueHdl.Call( pInfo );
}

IMPL_LINK( GraphicExporter, PaintProc, SdrPaintProcRec *, pRecord )
{
    SdrObject* pObj = pRecord->pObj;
    if( !pObj->IsEmptyPresObj() )
    {
        pObj->Paint( pRecord->rOut, pRecord->rInfoRec );
    }
    else
    {
        // sad but true, background shapes are also empty presentation objects
        // so we need to check if this is one
        if( pObj->GetPage()->IsMasterPage() && (pObj->GetPage() == pObj->GetObjList()) && (pObj->GetOrdNum() == 0) && pObj->ISA( SdrRectObj ) )
        {
            pObj->Paint( pRecord->rOut, pRecord->rInfoRec );
        }
    }

    return 0;
}

/** creates an virtual device for the given page

    @return the returned VirtualDevice is owned by the caller
*/
VirtualDevice* GraphicExporter::CreatePageVDev( SdrPage* pPage, ULONG nWidthPixel, ULONG nHeightPixel ) const
{
    VirtualDevice*  pVDev = new VirtualDevice();
    MapMode         aMM( MAP_100TH_MM );

    Size aPageSize(pPage->GetSize());
    aPageSize.Width()  -= pPage->GetLftBorder();
    aPageSize.Width()  -= pPage->GetRgtBorder();
    aPageSize.Height() -= pPage->GetUppBorder();
    aPageSize.Height() -= pPage->GetLwrBorder();

    // use scaling?
    if( nWidthPixel )
    {
        const Fraction aFrac( (long) nWidthPixel, pVDev->LogicToPixel( aPageSize, aMM ).Width() );

        aMM.SetScaleX( aFrac );

        if( nHeightPixel == 0 )
            aMM.SetScaleY( aFrac );
    }

    if( nHeightPixel )
    {
        const Fraction aFrac( (long) nHeightPixel, pVDev->LogicToPixel( aPageSize, aMM ).Height() );

        if( nWidthPixel == 0 )
            aMM.SetScaleX( aFrac );

        aMM.SetScaleY( aFrac );
    }

    pVDev->SetMapMode( aMM );
    BOOL bAbort = !pVDev->SetOutputSize(aPageSize);
    DBG_ASSERT(!bAbort, "virt. Device nicht korrekt erzeugt");

    SdrView* pView = new SdrView(mpDoc, pVDev);
    pView->SetPageVisible( FALSE );
    pView->SetBordVisible( FALSE );
    pView->SetGridVisible( FALSE );
    pView->SetHlplVisible( FALSE );
    pView->SetGlueVisible( FALSE );
    pView->ShowPage(pPage, Point(-pPage->GetLftBorder(), -pPage->GetUppBorder()));
    SdrPageView* pPageView  = pView->GetPageView(pPage);

//  DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
//  FrameView* pFrameView   = pViewShell->GetFrameView();
//  pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
//  pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
//  pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

    Point aPoint( pPage->GetLftBorder(), pPage->GetUppBorder() );
    Region aRegion (Rectangle( aPoint, aPageSize ) );
    const Link aPaintProcLink( LINK(this, GraphicExporter, PaintProc ) );

//  pView->InitRedraw(pVDev, aRegion, 0, &aPaintProcLink);

    for (USHORT i=0; i<pView->GetPageViewCount(); i++)
    {
        SdrPageView* pPV=pView->GetPageViewPvNum(i);
        pPV->InitRedraw(pVDev,aRegion,0,&aPaintProcLink );
    }

    delete pView;
    return pVDev;
}

// XFilter
sal_Bool SAL_CALL GraphicExporter::filter( const Sequence< PropertyValue >& aDescriptor )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpUnoPage )
        return sal_False;

    GraphicFilter*              pFilter = GetGrfFilter();
    SdrPage*                    pPage = mpUnoPage->GetSdrPage();
    Sequence< PropertyValue >   aFilterData;

    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;
    sal_Bool bExportOnlyBackground = false;

    if( NULL == pFilter || NULL == pPage || NULL == mpDoc )
        return sal_False;

    // get the arguments from the descriptor

    sal_Bool bTranslucent = sal_False; /* pFilter->GetExportFormatShortName( nFilter ).ToLowerAscii().EqualsAscii( "gif" ); */

    OUString aFilterName, aMediaType;
    URL aURL;

    {
        sal_Int32 nArgs = aDescriptor.getLength();
        const PropertyValue* pValues = aDescriptor.getConstArray();
        while( nArgs-- )
        {
            if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FilterName" ) ) )
            {
                pValues->Value >>= aFilterName;
            }
            else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
            {
                pValues->Value >>= aMediaType;
            }
            else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "URL" ) ) )
            {
                if( !( pValues->Value >>= aURL ) )
                {
                    pValues->Value >>= aURL.Complete;
                }
            }
            else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
            {
                pValues->Value >>= nWidth;
            }
            else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
            {
                pValues->Value >>= nHeight;
            }
            else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportOnlyBackground" ) ) )   // for compatibility reasons, deprecated
            {
                pValues->Value >>= bExportOnlyBackground;
            }
            else if ( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FilterData" ) ) )
            {
                pValues->Value >>= aFilterData;

                sal_Int32 nArgs = aFilterData.getLength();
                PropertyValue* pDataValues = aFilterData.getArray();
                while( nArgs-- )
                {
                    if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Translucent" ) ) )
                    {
                        pDataValues->Value >>= bTranslucent;
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PixelWidth" ) ) )
                    {
                        pDataValues->Value >>= nWidth;
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PixelHeight" ) ) )
                    {
                        pDataValues->Value >>= nHeight;
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
                    {
                        pDataValues->Value >>= nWidth;
                        pDataValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PixelWidth" ) );
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
                    {
                        pDataValues->Value >>= nHeight;
                        pDataValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PixelHeight" ) );
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportOnlyBackground" ) ) )
                    {
                        pDataValues->Value >>= bExportOnlyBackground;
                    }
                    else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PageNumber" ) ) )
                    {
                        pDataValues->Value >>= mnPageNumber;
                    }

                    pDataValues++;
                }
            }

            pValues++;
        }
    }

    // create the output stuff

    Graphic             aGraphic;
    VirtualDevice       aVDev;
    const Fraction      aFrac( mpDoc->GetScaleFraction() );
    const MapMode       aMap( mpDoc->GetScaleUnit(), Point(), aFrac, aFrac );
    const sal_uInt16    nFilter = aMediaType.getLength()
                            ? pFilter->GetExportFormatNumberForMediaType( aMediaType )
                            : pFilter->GetExportFormatNumberForShortName( aFilterName );
    sal_Bool            bVectorType = !pFilter->IsExportPixelFormat( nFilter );
    bool bRet = true;

    SdrOutliner& rOutl=mpDoc->GetDrawOutliner(NULL);
    maOldCalcFieldValueHdl = rOutl.GetCalcFieldValueHdl();
    rOutl.SetCalcFieldValueHdl( LINK(this, GraphicExporter, CalcFieldValueHdl) );

    std::vector< SdrObject* > aShapes;

    // export complete page?
    if ( !mxShape.is() )
    {
        if( bExportOnlyBackground )
        {
            if( pPage->IsMasterPage() )
            {
                aShapes.push_back( pPage->GetObj(0) );
            }
            else
            {
                if( pPage->GetBackgroundObj() )
                    aShapes.push_back( pPage->GetBackgroundObj() );
            }
        }
        else
        {
            sal_uInt16 nPage = pPage->GetPageNum() ? ( pPage->GetPageNum() - 1 ) >> 1 : 0;

            const Size aSize( pPage->GetSize() );

            // generate a bitmap to convert it to a pixel format.
            // For gif pictures there can also be a vector format used (bTranslucent)
            if ( !bVectorType && !bTranslucent )
            {
                const Size      aSizePix( Application::GetDefaultDevice()->LogicToPixel( aSize, aMap ) );
                long    nWidthPix = ( aSizePix.Width()>MAX_EXT_PIX || aSizePix.Height()>MAX_EXT_PIX ) ? MAX_EXT_PIX : 0;
                long    nHeightPix = 0;

                if( (nWidth != -1) && (nWidth < MAX_EXT_PIX) )
                    nWidthPix = nWidth;

                if( (nHeight != -1) && (nHeight < MAX_EXT_PIX) )
                    nHeightPix = nHeight;

                SdrView*        pView;

                if( (nWidth > 0) && (nWidth <= MAX_EXT_PIX)  )
                    nWidthPix = nWidth;

                if( PTR_CAST( FmFormModel, mpDoc ) )
                {
                    pView = new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev );
                }
                else
                {
                    pView = new SdrView( mpDoc, &aVDev );
                }


                VirtualDevice*  pVDev = CreatePageVDev( pPage, nWidthPix, nHeightPix );

                if( pVDev )
                {
                    aGraphic = pVDev->GetBitmap( Point(), pVDev->GetOutputSize() );
                    aGraphic.SetPrefMapMode( aMap );
                    aGraphic.SetPrefSize( aSize );
                    delete pVDev;
                }

                delete pView;
            }
            // create a metafile to export a vector format
            else
            {
                GDIMetaFile aMtf;

                aVDev.SetMapMode( aMap );
                aMtf.Record( &aVDev );

                SdrView*        pView;

                if( PTR_CAST( FmFormModel, mpDoc ) )
                {
                    pView = new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev );
                }
                else
                {
                    pView = new SdrView( mpDoc, &aVDev );
                }

                Size aNewSize;
                if ( pView && pPage )
                {
                    pView->SetBordVisible( FALSE );
                    pView->SetPageVisible( FALSE );
                    pView->ShowPage( pPage, Point() );

                    const Point aNewOrg( pPage->GetLftBorder(), pPage->GetUppBorder() );
                    aNewSize = Size( aSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder(),
                                          aSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder() );
                    const Rectangle aClipRect( aNewOrg, aNewSize );
                    MapMode         aVMap( aMap );

                    SdrPageView* pPageView  = pView->GetPageView( pPage );

                    aVDev.Push();
                    aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                    aVDev.SetRelativeMapMode( aVMap );
                    aVDev.IntersectClipRegion( aClipRect );
                    const Link aPaintProcLink( LINK(this, GraphicExporter, PaintProc ) );

//                  pView->InitRedraw( &aVDev, Region( Rectangle( Point(), aNewSize ), 0, &aPaintProcLink ) );

                    for (USHORT i=0; i<pView->GetPageViewCount(); i++)
                    {
                        SdrPageView* pPV=pView->GetPageViewPvNum(i);
                        pPV->InitRedraw(&aVDev,Region( Rectangle( Point(), aNewSize ) ),0,&aPaintProcLink );
                    }

                    aVDev.Pop();

                    aMtf.Stop();
                    aMtf.WindStart();
                    aMtf.SetPrefMapMode( aMap );
                    aMtf.SetPrefSize( aNewSize );

                     aGraphic = Graphic( RemoveClipRegionActions( aMtf ) );
                }

                if( bTranslucent )
                {
                    Size aOutSize;
                    aGraphic = GetBitmapFromMetaFile( aGraphic.GetGDIMetaFile(), TRUE, CalcSize( nWidth, nHeight, aNewSize, aOutSize ) );
                }

                if ( pView )
                    delete pView;
            }
        }
    }

    // export only single shape or shape collection
    else
    {
        // build list of SdrObject
        if( mxShapes.is() )
        {
            Reference< XShape > xShape;
            const sal_Int32 nCount = mxShapes->getCount();

            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                mxShapes->getByIndex( nIndex ) >>= xShape;
                SdrObject* pObj = GetSdrObjectFromXShape( xShape );
                if( pObj )
                    aShapes.push_back( pObj );
            }
        }
        else
        {
            // only one shape
            SdrObject* pObj = GetSdrObjectFromXShape( mxShape );
            if( pObj )
                aShapes.push_back( pObj );
        }

        if( 0 == aShapes.size() )
            bRet = false;
    }

    if( bRet && aShapes.size() )
    {
        // special treatment for only one SdrGrafObj that has text
        sal_Bool bSingleGraphic = sal_False;

        if( !bVectorType && 1 == aShapes.size() )
        {
            SdrObject* pObj = aShapes.front();
            if( pObj && pObj->ISA( SdrGrafObj ) && !( (SdrGrafObj*) pObj )->HasText() )
            {
                aGraphic = ( (SdrGrafObj*) pObj )->GetTransformedGraphic();
                bSingleGraphic = sal_True;
            }
        }

        if( !bSingleGraphic )
        {
            // create a metafile for all shapes
            VirtualDevice   aOut;
            ExtOutputDevice aXOut( &aOut );

            // calculate bound rect for all shapes
            Rectangle aBound;

            std::vector< SdrObject* >::iterator aIter = aShapes.begin();
            const std::vector< SdrObject* >::iterator aEnd = aShapes.end();

            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                Rectangle aR1(pObj->GetBoundRect());
                if (aBound.IsEmpty())
                    aBound=aR1;
                else
                    aBound.Union(aR1);
            }

            aOut.EnableOutput( FALSE );
            aOut.SetMapMode( aMap );

            GDIMetaFile aMtf;
            aMtf.Clear();
            aMtf.Record( &aOut );

            MapMode aOutMap( aMap );
            aOutMap.SetOrigin( Point( -aBound.TopLeft().X(), -aBound.TopLeft().Y() ) );
            aOut.SetRelativeMapMode( aOutMap );

            SdrPaintInfoRec aInfoRec;
            aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;

            aIter = aShapes.begin();
            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                pObj->Paint(aXOut,aInfoRec);
            }

            aMtf.Stop();
            aMtf.WindStart();

            const Size  aExtSize( aOut.PixelToLogic( Size( 0, 0  ) ) );
            Size        aBoundSize( aBound.GetWidth() + ( aExtSize.Width() ),
                                    aBound.GetHeight() + ( aExtSize.Height() ) );

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundSize );

            if( !bVectorType )
            {
                Size aOutSize;
                aGraphic = GetBitmapFromMetaFile( aMtf, bTranslucent, CalcSize( nWidth, nHeight, aBoundSize, aOutSize ) );
            }
            else
            {
                aGraphic = aMtf;
            }
        }
    }

    if( bRet )
    {
        // export graphic only if it has a size
        const Size aGraphSize( aGraphic.GetPrefSize() );
        if ( ( aGraphSize.Width() == 0 ) || ( aGraphSize.Height() == 0 ) )
        {
            bRet = false;
        }
        else
        {
            // now we have a graphic, so export it
            INetURLObject aURLObject( aURL.Complete );
            DBG_ASSERT( aURLObject.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            bRet = 0 == XOutBitmap::ExportGraphic( aGraphic, aURLObject, *pFilter, nFilter, FALSE, &aFilterData );
        }
    }

    rOutl.SetCalcFieldValueHdl( maOldCalcFieldValueHdl );

    return bRet;
}

void SAL_CALL GraphicExporter::cancel()
    throw(RuntimeException)
{
}

// XExporter

/** the source 'document' could be a XDrawPage, a XShape or a generic XShapes */
void SAL_CALL GraphicExporter::setSourceDocument( const Reference< lang::XComponent >& xComponent )
    throw(IllegalArgumentException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    mxShapes = NULL;
    mpUnoPage = NULL;

    try
    {
    // any break inside this one loop while will throw a IllegalArgumentException
    do
    {
        mxPage = Reference< XDrawPage >::query( xComponent );
        mxShapes = Reference< XShapes >::query( xComponent );
        mxShape = Reference< XShape >::query( xComponent );

        // Step 1: try a generic XShapes
        if( !mxPage.is() && !mxShape.is() && mxShapes.is() )
        {
            // we do not support empty shape collections
            if( 0 == mxShapes->getCount() )
                break;

            // get first shape to detect corresponding page and model
            mxShapes->getByIndex(0) >>= mxShape;
        }
        else
        {
            mxShapes = NULL;
        }

        // Step 2: try a shape
        if( mxShape.is() )
        {
            if( NULL == GetSdrObjectFromXShape( mxShape ) )
                break;

            // get page for this shape
            Reference< XChild > xChild( mxShape, UNO_QUERY );
            if( !xChild.is() )
                break;

            Reference< XInterface > xInt;
            do
            {
                xInt = xChild->getParent();
                mxPage = Reference< XDrawPage >::query( xInt );
                if( !mxPage.is() )
                    xChild = Reference< XChild >::query( xInt );
            }
            while( !mxPage.is() && xChild.is() );

            if( !mxPage.is() )
                break;
        }

        // Step 3: check the page
        if( !mxPage.is() )
            break;

        mpUnoPage = SvxDrawPage::getImplementation( mxPage );

        if( NULL == mpUnoPage || NULL == mpUnoPage->GetSdrPage() )
            break;

        mpDoc = mpUnoPage->GetSdrPage()->GetModel();

        // Step 4:  If we got a generic XShapes test all contained shapes
        //          if they belong to the same XDrawPage

        if( mxShapes.is() )
        {
            SdrPage* pPage = mpUnoPage->GetSdrPage();
            SdrObject* pObj;
            Reference< XShape > xShape;

            sal_Bool bOk = sal_True;

            const sal_Int32 nCount = mxShapes->getCount();

            // test all but the first shape if they have the same page than
            // the first shape
            for( sal_Int32 nIndex = 1; bOk && ( nIndex < nCount ); nIndex++ )
            {
                mxShapes->getByIndex( nIndex ) >>= xShape;
                pObj = GetSdrObjectFromXShape( xShape );
                bOk = pObj && pObj->GetPage() == pPage;
            }

            if( !bOk )
                break;
        }

        // no errors so far
        return;
    }
    while( 0 );
    }
    catch( Exception& )
    {
    }

    throw IllegalArgumentException();
}

// XServiceInfo
OUString SAL_CALL GraphicExporter::getImplementationName(  )
    throw(RuntimeException)
{
    return GraphicExporter_getImplementationName();
}

sal_Bool SAL_CALL GraphicExporter::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return sal_True;
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return GraphicExporter_getSupportedServiceNames();
}

// XMimeTypeInfo
sal_Bool SAL_CALL GraphicExporter::supportsMimeType( const OUString& MimeTypeName ) throw (RuntimeException)
{
    const String aMimeTypeName( MimeTypeName );

    GraphicFilter*  pFilter = GetGrfFilter();
    sal_uInt16 nCount = pFilter->GetExportFormatCount();
    sal_uInt16 nFilter;
    for( nFilter = 0; nFilter < nCount; nFilter++ )
    {
        if( aMimeTypeName.Equals( pFilter->GetExportFormatMediaType( nFilter ) ) )
        {
            return sal_True;
        }
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedMimeTypeNames(  ) throw (RuntimeException)
{
    GraphicFilter*  pFilter = GetGrfFilter();
    sal_uInt16 nCount = pFilter->GetExportFormatCount();
    sal_uInt16 nFilter;
    sal_uInt16 nFound = 0;

    Sequence< OUString > aSeq( nCount );
    OUString* pStr = aSeq.getArray();

    for( nFilter = 0; nFilter < nCount; nFilter++ )
    {
        OUString aMimeType( pFilter->GetExportFormatMediaType( nFilter ) );
        if( aMimeType.getLength() )
        {
            *pStr++ = aMimeType;
            nFound++;
        }
    }

    if( nFound < nCount )
        aSeq.realloc( nFound );

    return aSeq;
}

