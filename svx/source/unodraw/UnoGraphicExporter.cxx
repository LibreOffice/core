/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoGraphicExporter.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:09:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vector>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
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

#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICRENDERER_HPP_
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONCONTINUATION_HPP_
#include <com/sun/star/task/XInteractionContinuation.hpp>
#endif

#include <framework/interaction.hxx>

#ifndef _COM_SUN_STAR_DRAWING_GRAPHICFILTERREQUEST_HPP_
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>
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

#ifndef SVTOOLS_OUTSTRM_HXX
#include <svtools/outstrm.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif

#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
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
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif

#ifndef _SVX_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

#ifndef _SVX_UNOWPAGE_HXX
#include <svx/unopage.hxx>
#endif

#ifndef _SVX_PAGEITEM_HXX
#include <svx/pageitem.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif

#include "boost/scoped_ptr.hpp"

#define MAX_EXT_PIX         2048

using namespace ::comphelper;
using namespace ::osl;
using namespace ::vos;
using ::rtl::OUString;
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
using namespace ::com::sun::star::task;

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// #114389# use new redirector instead of pPaintProc

class ImplLocalRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
public:
    ImplLocalRedirector() : ViewObjectContactRedirector() {}
    virtual ~ImplLocalRedirector();

    // all default implementations just call the same methods at the original. To do something
    // different, overload the method and at least do what the method does.
    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);
};

ImplLocalRedirector::~ImplLocalRedirector()
{
}

void ImplLocalRedirector::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();
    sal_Bool bDoPaint(sal_False);

    if(pObject)
    {
        if(pObject->IsEmptyPresObj())
        {
            // do not paint empty PresObjs, but the background obejct.
            // SInce the new mechanism does not paint the background object
            // at all, it is not necessary to test for it here and the whole
            // method may be simplified soon. For testing for BackgroundObj
            // You may use pObject->IsMasterPageBackgroundObject().
        }
        else
        {
            // paint non-empty PresObjs
            bDoPaint = sal_True;
        }
    }
    else
    {
        // no SdrObject, may be SdrPage. Do paint.
        bDoPaint = sal_True;
    }

    if(bDoPaint)
    {
        rOriginal.PaintObject(rDisplayInfo);
    }
}

//////////////////////////////////////////////////////////////////////////////

namespace svx
{
    struct ExportSettings
    {
        OUString maFilterName;
        OUString maMediaType;
        URL maURL;
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > mxOutputStream;
        com::sun::star::uno::Reference< com::sun::star::graphic::XGraphicRenderer > mxGraphicRenderer;
        com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator >    mxStatusIndicator;
        com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > mxInteractionHandler;

        sal_Int32 mnWidth;
        sal_Int32 mnHeight;
        sal_Bool mbExportOnlyBackground;
        sal_Bool mbVerboseComments;
        sal_Bool mbScrollText;
        sal_Bool mbUseHighContrast;
        sal_Bool mbTranslucent;

        Sequence< PropertyValue >   maFilterData;

        Fraction    maScaleX;
        Fraction    maScaleY;

        ExportSettings( SdrModel* pDoc );
    };

    ExportSettings::ExportSettings( SdrModel* pDoc )
    : mnWidth( 0 )
    , mnHeight( 0 )
    , mbExportOnlyBackground( false )
    , mbVerboseComments( false )
    , mbScrollText( false )
    , mbUseHighContrast( false )
    , mbTranslucent( sal_False )
    , maScaleX( 1, 1 )
    , maScaleY( 1, 1 )
    {
        if( pDoc )
        {
            maScaleX = pDoc->GetScaleFraction();
            maScaleY = pDoc->GetScaleFraction();
        }
    }

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
        virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

        // XMimeTypeInfo
        virtual sal_Bool SAL_CALL supportsMimeType( const ::rtl::OUString& MimeTypeName ) throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedMimeTypeNames(  ) throw (RuntimeException);

        VirtualDevice* CreatePageVDev( SdrPage* pPage, ULONG nWidthPixel, ULONG nHeightPixel ) const;

        DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

        void ParseSettings( const Sequence< PropertyValue >& aDescriptor, ExportSettings& rSettings );
        bool GetGraphic( ExportSettings& rSettings, Graphic& aGraphic, sal_Bool bVectorType );

    private:
        Reference< XShape >     mxShape;
        Reference< XDrawPage >  mxPage;
        Reference< XShapes >    mxShapes;

        SvxDrawPage*        mpUnoPage;

        Link                maOldCalcFieldValueHdl;
        sal_Int32           mnPageNumber;
        SdrPage*            mpCurrentPage;
        SdrModel*           mpDoc;
    };

    Reference< XInterface > SAL_CALL GraphicExporter_createInstance(const Reference< XMultiServiceFactory > & )
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

    GDIMetaFile& RemoveClipRegionActions( GDIMetaFile& o_rMtf, const GDIMetaFile& i_rMtf )
    {
        const ULONG nActionCount( i_rMtf.GetActionCount() );

        o_rMtf.Clear();
        o_rMtf.SetPrefSize( i_rMtf.GetPrefSize() );
        o_rMtf.SetPrefMapMode( i_rMtf.GetPrefMapMode() );

        // examine actions and remove ClipRegion-Actions.
        for ( ULONG nAction = 0; nAction < nActionCount; nAction++ )
        {
            MetaAction* pCopyAction = ( (GDIMetaFile&) i_rMtf ).CopyAction( nAction );

            if( pCopyAction )
            {
                switch( pCopyAction->GetType() )
                {
                    case( META_CLIPREGION_ACTION ) :
                        pCopyAction->Delete();
                    break;

                    default:
                        o_rMtf.AddAction( pCopyAction );
                    break;
                }
            }
        }

        return o_rMtf;
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
            Bitmap  aMaskBmp( aMaskGraphic.GetUnlimitedBitmap( pSize ) );

            aMaskBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            aBmpEx = BitmapEx( aGraphic.GetUnlimitedBitmap( pSize ), aMaskBmp );
        }
        else
            aBmpEx = BitmapEx( aGraphic.GetUnlimitedBitmap( pSize ) );

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

class ImplExportCheckVisisbilityRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
public:
    ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage );
    virtual ~ImplExportCheckVisisbilityRedirector();

    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);

private:
    SdrPage*    mpCurrentPage;
};

ImplExportCheckVisisbilityRedirector::ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage )
:   ViewObjectContactRedirector(), mpCurrentPage( pCurrentPage )
{
}

ImplExportCheckVisisbilityRedirector::~ImplExportCheckVisisbilityRedirector()
{
}

void ImplExportCheckVisisbilityRedirector::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        SdrPage* pPage = mpCurrentPage;
        if( pPage == 0 )
            pPage = pObject->GetPage();

        if( (pPage == 0) || pPage->checkVisibility(rOriginal, rDisplayInfo, false) )
        {
            rOriginal.PaintObject(rDisplayInfo);
        }
    }
    else
    {
        // not an object, maybe a page
        rOriginal.PaintObject(rDisplayInfo);
    }
}

using namespace ::svx;

GraphicExporter::GraphicExporter()
: mpUnoPage( NULL ), mnPageNumber(-1), mpCurrentPage(0), mpDoc( NULL )
{
}

GraphicExporter::~GraphicExporter()
{
}

IMPL_LINK(GraphicExporter, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if( pInfo )
    {
        if( mpCurrentPage )
        {
            pInfo->SetSdrPage( mpCurrentPage );
        }
        else if( mnPageNumber != -1 )
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
    }

    long nRet = maOldCalcFieldValueHdl.Call( pInfo );

    if( pInfo && mpCurrentPage )
        pInfo->SetSdrPage( 0 );

    return nRet;
}

/** creates an virtual device for the given page

    @return the returned VirtualDevice is owned by the caller
*/
VirtualDevice* GraphicExporter::CreatePageVDev( SdrPage* pPage, ULONG nWidthPixel, ULONG nHeightPixel ) const
{
    VirtualDevice*  pVDev = new VirtualDevice();
    MapMode         aMM( MAP_100TH_MM );

    Point aPoint( 0, 0 );
    Size aPageSize(pPage->GetSize());

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
#ifdef DBG_UTIL
    BOOL bAbort = !
#endif
        pVDev->SetOutputSize(aPageSize);
    DBG_ASSERT(!bAbort, "virt. Device nicht korrekt erzeugt");

    SdrView* pView = new SdrView(mpDoc, pVDev);
    pView->SetPageVisible( FALSE );
    pView->SetBordVisible( FALSE );
    pView->SetGridVisible( FALSE );
    pView->SetHlplVisible( FALSE );
    pView->SetGlueVisible( FALSE );
    pView->ShowSdrPage(pPage);
    Region aRegion (Rectangle( aPoint, aPageSize ) );

    ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

    pView->CompleteRedraw(pVDev, aRegion, 0, &aRedirector);

    delete pView;
    return pVDev;
}

void GraphicExporter::ParseSettings( const Sequence< PropertyValue >& aDescriptor, ExportSettings& rSettings )
{
    sal_Int32 nArgs = aDescriptor.getLength();
    const PropertyValue* pValues = aDescriptor.getConstArray();
    while( nArgs-- )
    {
        if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FilterName" ) ) )
        {
            pValues->Value >>= rSettings.maFilterName;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        {
            pValues->Value >>= rSettings.maMediaType;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "URL" ) ) )
        {
            if( !( pValues->Value >>= rSettings.maURL ) )
            {
                pValues->Value >>= rSettings.maURL.Complete;
            }
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OutputStream" ) ) )
        {
            pValues->Value >>= rSettings.mxOutputStream;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicRenderer" ) ) )
        {
            pValues->Value >>= rSettings.mxGraphicRenderer;
        }
        else if ( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StatusIndicator" ) ) )
        {
            pValues->Value >>= rSettings.mxStatusIndicator;
        }
        else if ( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "InteractionHandler" ) ) )
        {
            pValues->Value >>= rSettings.mxInteractionHandler;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnWidth;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnHeight;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportOnlyBackground" ) ) )   // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mbExportOnlyBackground;
        }
        else if ( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FilterData" ) ) )
        {
            pValues->Value >>= rSettings.maFilterData;

            sal_Int32 nFilterArgs = rSettings.maFilterData.getLength();
            PropertyValue* pDataValues = rSettings.maFilterData.getArray();
            while( nFilterArgs-- )
            {
                if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Translucent" ) ) )
                {
                    if ( !( pDataValues->Value >>= rSettings.mbTranslucent ) )  // SJ: TODO: The GIF Transparency is stored as int32 in
                    {                                               // configuration files, this has to be changed to boolean
                        sal_Int32 nTranslucent = 0;
                        if ( pDataValues->Value >>= nTranslucent )
                            rSettings.mbTranslucent = nTranslucent != 0;
                    }
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PixelWidth" ) ) )
                {
                    pDataValues->Value >>= rSettings.mnWidth;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PixelHeight" ) ) )
                {
                    pDataValues->Value >>= rSettings.mnHeight;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnWidth;
                    pDataValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PixelWidth" ) );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnHeight;
                    pDataValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PixelHeight" ) );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportOnlyBackground" ) ) )
                {
                    pDataValues->Value >>= rSettings.mbExportOnlyBackground;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "HighContrast" ) ) )
                {
                    pDataValues->Value >>= rSettings.mbUseHighContrast;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PageNumber" ) ) )
                {
                    pDataValues->Value >>= mnPageNumber;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VerboseComments" ) ) )
                {
                    // #110496# Read flag for verbose metafile comments
                    pDataValues->Value >>= rSettings.mbVerboseComments;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScrollText" ) ) )
                {
                    // #110496# Read flag solitary scroll text metafile
                    pDataValues->Value >>= rSettings.mbScrollText;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CurrentPage" ) ) )
                {
                    Reference< XDrawPage >  xPage;
                    pDataValues->Value >>= xPage;
                    if( xPage.is() )
                    {
                        SvxDrawPage* pUnoPage = SvxDrawPage::getImplementation( xPage );
                        if( pUnoPage && pUnoPage->GetSdrPage() )
                            mpCurrentPage = pUnoPage->GetSdrPage();
                    }
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleXNumerator" ) ) )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleX = Fraction( nVal, rSettings.maScaleX.GetDenominator() );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleXDenominator" ) ) )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleX = Fraction( rSettings.maScaleX.GetNumerator(), nVal );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleYNumerator" ) ) )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleY = Fraction( nVal, rSettings.maScaleY.GetDenominator() );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleYDenominator" ) ) )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleY = Fraction( rSettings.maScaleY.GetNumerator(), nVal );
                }

                pDataValues++;
            }
        }

        pValues++;
    }

    // putting the StatusIndicator that we got from the MediaDescriptor into our local FilterData copy
    if ( rSettings.mxStatusIndicator.is() )
    {
        rtl::OUString sStatusIndicator( RTL_CONSTASCII_USTRINGPARAM( "StatusIndicator" ) );
        int i = rSettings.maFilterData.getLength();
        rSettings.maFilterData.realloc( i + 1 );
        rSettings.maFilterData[ i ].Name = sStatusIndicator;
        rSettings.maFilterData[ i ].Value <<= rSettings.mxStatusIndicator;
    }
}

bool GraphicExporter::GetGraphic( ExportSettings& rSettings, Graphic& aGraphic, sal_Bool bVectorType )
{
    if( !mpDoc || !mpUnoPage )
        return false;

    SdrPage* pPage = mpUnoPage->GetSdrPage();
    if( !pPage )
        return false;

    VirtualDevice       aVDev;
    const MapMode       aMap( mpDoc->GetScaleUnit(), Point(), rSettings.maScaleX, rSettings.maScaleY );

    // create a view
    SdrView*        pView;

    if( PTR_CAST( FmFormModel, mpDoc ) )
    {
        pView = new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev );
    }
    else
    {
        pView = new SdrView( mpDoc, &aVDev );
    }

    pView->SetBordVisible( FALSE );
    pView->SetPageVisible( FALSE );
    pView->ShowSdrPage( pPage );

    SdrOutliner& rOutl=mpDoc->GetDrawOutliner(NULL);
    maOldCalcFieldValueHdl = rOutl.GetCalcFieldValueHdl();
    rOutl.SetCalcFieldValueHdl( LINK(this, GraphicExporter, CalcFieldValueHdl) );
    rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor(pView->GetSdrPageView()) );

    std::vector< SdrObject* > aShapes;

    bool bRet = true;

    // export complete page?
    if ( !mxShape.is() )
    {
        if( rSettings.mbExportOnlyBackground )
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
            const Size aSize( pPage->GetSize() );

            // generate a bitmap to convert it to a pixel format.
            // For gif pictures there can also be a vector format used (bTranslucent)
            if ( !bVectorType && !rSettings.mbTranslucent )
            {
                long nWidthPix = 0;
                long nHeightPix = 0;
                if ( rSettings.mnWidth > 0 && rSettings.mnHeight > 0 )
                {
                    nWidthPix = rSettings.mnWidth;
                    nHeightPix = rSettings.mnHeight;
                }
                else
                {
                    const Size aSizePix( Application::GetDefaultDevice()->LogicToPixel( aSize, aMap ) );
                    if (aSizePix.Width() > MAX_EXT_PIX || aSizePix.Height() > MAX_EXT_PIX)
                    {
                        if (aSizePix.Width() > MAX_EXT_PIX)
                            nWidthPix = MAX_EXT_PIX;
                        else
                            nWidthPix = aSizePix.Width();
                        if (aSizePix.Height() > MAX_EXT_PIX)
                            nHeightPix = MAX_EXT_PIX;
                        else
                            nHeightPix = aSizePix.Height();

                        double fWidthDif = aSizePix.Width() / nWidthPix;
                        double fHeightDif = aSizePix.Height() / nHeightPix;

                        if (fWidthDif > fHeightDif)
                            nHeightPix = static_cast<long>(aSizePix.Height() / fWidthDif);
                        else
                            nWidthPix = static_cast<long>(aSizePix.Width() / fHeightDif);
                    }
                    else
                    {
                        nWidthPix = aSizePix.Width();
                        nHeightPix = aSizePix.Height();
                    }
                }

                boost::scoped_ptr< SdrView > pLocalView;
                if( PTR_CAST( FmFormModel, mpDoc ) )
                {
                    pLocalView.reset( new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev ) );
                }
                else
                {
                    pLocalView.reset( new SdrView( mpDoc, &aVDev ) );
                }


                VirtualDevice*  pVDev = CreatePageVDev( pPage, nWidthPix, nHeightPix );

                if( pVDev )
                {
                    aGraphic = pVDev->GetBitmap( Point(), pVDev->GetOutputSize() );
                    aGraphic.SetPrefMapMode( aMap );
                    aGraphic.SetPrefSize( aSize );
                    delete pVDev;
                }
            }
            // create a metafile to export a vector format
            else
            {
                GDIMetaFile aMtf;

                aVDev.SetMapMode( aMap );
                if( rSettings.mbUseHighContrast )
                    aVDev.SetDrawMode( aVDev.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
                aVDev.EnableOutput( FALSE );
                aMtf.Record( &aVDev );

                Size aNewSize;
                if ( pView && pPage )
                {
                    pView->SetBordVisible( FALSE );
                    pView->SetPageVisible( FALSE );
                    pView->ShowSdrPage( pPage );

                    const Point aNewOrg( pPage->GetLftBorder(), pPage->GetUppBorder() );
                    aNewSize = Size( aSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder(),
                                          aSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder() );
                    const Rectangle aClipRect( aNewOrg, aNewSize );
                    MapMode         aVMap( aMap );

                    aVDev.Push();
                    aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                    aVDev.SetRelativeMapMode( aVMap );
                    aVDev.IntersectClipRegion( aClipRect );

                    sal_uInt16 nPaintMode(0);

                    // #110496# Enable verbose metafile comments
                    if( rSettings.mbVerboseComments )
                        nPaintMode|=SDRPAINTMODE_VERBOSE_MTF;

                    // Use new StandardCheckVisisbilityRedirector
                    ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

                    pView->CompleteRedraw(&aVDev, Region(Rectangle(Point(), aNewSize)), 0, &aRedirector);

                    aVDev.Pop();

                    aMtf.Stop();
                    aMtf.WindStart();
                    aMtf.SetPrefMapMode( aMap );
                    aMtf.SetPrefSize( aNewSize );

                    GDIMetaFile aMtf2;
                    aGraphic = Graphic( RemoveClipRegionActions( aMtf2, aMtf ) );
                }

                if( rSettings.mbTranslucent )
                {
                    Size aOutSize;
                    aGraphic = GetBitmapFromMetaFile( aGraphic.GetGDIMetaFile(), TRUE, CalcSize( rSettings.mnWidth, rSettings.mnHeight, aNewSize, aOutSize ) );
                }
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

        if( 1 == aShapes.size() )
        {
            if( !bVectorType )
            {
                SdrObject* pObj = aShapes.front();
                if( pObj && pObj->ISA( SdrGrafObj ) && !( (SdrGrafObj*) pObj )->HasText() )
                {
                    aGraphic = ( (SdrGrafObj*) pObj )->GetTransformedGraphic();
                    if ( aGraphic.GetType() == GRAPHIC_BITMAP )
                    {
                        Size aSizePixel( aGraphic.GetSizePixel() );
                        if ( rSettings.mnWidth && rSettings.mnHeight && ( rSettings.mnWidth != aSizePixel.Width() ) || ( rSettings.mnHeight != aSizePixel.Height() ) )
                        {
                            BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                            aBmpEx.Scale( Size( rSettings.mnWidth, rSettings.mnHeight ) );
                            aGraphic = aBmpEx;
                        }
                    }
                    bSingleGraphic = sal_True;
                }
            }
            else if( rSettings.mbScrollText )
            {
                SdrObject* pObj = aShapes.front();
                if( pObj && pObj->ISA( SdrTextObj )
                    && ( (SdrTextObj*) pObj )->HasText() )
                {
                    Rectangle aScrollRectangle;
                    Rectangle aPaintRectangle;

                    const boost::scoped_ptr< GDIMetaFile > pMtf(
                        ( (SdrTextObj*) pObj )->GetTextScrollMetaFileAndRectangle(
                        aScrollRectangle, aPaintRectangle ) );

                    // take the larger one of the two rectangles (that
                    // should be the bound rect of the retrieved
                    // metafile)
                    Rectangle aTextRect;

                    if( aScrollRectangle.IsInside( aPaintRectangle ) )
                        aTextRect = aScrollRectangle;
                    else
                        aTextRect = aPaintRectangle;

                    // setup pref size and mapmode
                    pMtf->SetPrefSize( aTextRect.GetSize() );

                    // set actual origin (mtf is at actual shape
                    // output position)
                    MapMode aLocalMapMode( aMap );
                    aLocalMapMode.SetOrigin(
                        Point( -aPaintRectangle.Left(),
                               -aPaintRectangle.Top() ) );
                    pMtf->SetPrefMapMode( aLocalMapMode );

                    pMtf->AddAction( new MetaCommentAction(
                                         "XTEXT_SCROLLRECT", 0,
                                         reinterpret_cast<BYTE const*>(&aScrollRectangle),
                                         sizeof( Rectangle ) ) );
                    pMtf->AddAction( new MetaCommentAction(
                                         "XTEXT_PAINTRECT", 0,
                                         reinterpret_cast<BYTE const*>(&aPaintRectangle),
                                         sizeof( Rectangle ) ) );

                    aGraphic = Graphic( *pMtf );

                    bSingleGraphic = sal_True;
                }
            }
        }

        if( !bSingleGraphic )
        {
            // create a metafile for all shapes
            VirtualDevice   aOut;
            XOutputDevice aXOut( &aOut );

            // calculate bound rect for all shapes
            Rectangle aBound;

            std::vector< SdrObject* >::iterator aIter = aShapes.begin();
            const std::vector< SdrObject* >::iterator aEnd = aShapes.end();

            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                Rectangle aR1(pObj->GetCurrentBoundRect());
                if (aBound.IsEmpty())
                    aBound=aR1;
                else
                    aBound.Union(aR1);
            }

            aOut.EnableOutput( FALSE );
            aOut.SetMapMode( aMap );
            if( rSettings.mbUseHighContrast )
                aOut.SetDrawMode( aVDev.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

            GDIMetaFile aMtf;
            aMtf.Clear();
            aMtf.Record( &aOut );

            MapMode aOutMap( aMap );
            aOutMap.SetOrigin( Point( -aBound.TopLeft().X(), -aBound.TopLeft().Y() ) );
            aOut.SetRelativeMapMode( aOutMap );

            SdrPaintInfoRec aInfoRec;
            aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;

            // #110496# Enable verbose metafile comments
            if( rSettings.mbVerboseComments )
                aInfoRec.nPaintMode|=SDRPAINTMODE_VERBOSE_MTF;

            sdr::contact::DisplayInfo aDisplayInfo;
            aDisplayInfo.SetProcessedPage( mpCurrentPage );
            aDisplayInfo.SetMasterPagePainting( pPage->IsMasterPage() );
            if( mpCurrentPage && pPage->IsMasterPage() )
                aDisplayInfo.SetProcessLayers( mpCurrentPage->TRG_GetMasterPageVisibleLayers() );

            aIter = aShapes.begin();
            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);

                sdr::contact::ViewContactOfSdrObj aViewContact( *pObj );
                sdr::contact::ObjectContactOfPagePainter aObjectContact( pPage, false );
                sdr::contact::ViewObjectContact aOriginal( aObjectContact, aViewContact);

                // #i70852# why was this pObj->SdrObject::GetLayer()? This would not use the correct GetLayer()
                // for FmFormObj. Changed.
                if( aDisplayInfo.GetProcessLayers().IsSet(pObj->GetLayer()) )
                    if( (pObj->GetPage() == 0) || pObj->GetPage()->checkVisibility(aOriginal, aDisplayInfo, false) )
                        pObj->SingleObjectPainter(aXOut,aInfoRec); // #110094#-17


                aOriginal.PrepareDelete();
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
                aGraphic = GetBitmapFromMetaFile( aMtf, rSettings.mbTranslucent, CalcSize( rSettings.mnWidth, rSettings.mnHeight, aBoundSize, aOutSize ) );
            }
            else
            {
                aGraphic = aMtf;
            }
        }
    }

    if ( pView )
    {
        pView->HideSdrPage();
        delete pView;
    }

    rOutl.SetCalcFieldValueHdl( maOldCalcFieldValueHdl );

    return bRet;

}

// XFilter
sal_Bool SAL_CALL GraphicExporter::filter( const Sequence< PropertyValue >& aDescriptor )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpUnoPage )
        return sal_False;

    GraphicFilter*              pFilter = GetGrfFilter();

    if( NULL == pFilter || NULL == mpUnoPage->GetSdrPage() || NULL == mpDoc )
        return sal_False;

    // get the arguments from the descriptor
    ExportSettings aSettings( mpDoc );
    ParseSettings( aDescriptor, aSettings );

    const sal_uInt16    nFilter = aSettings.maMediaType.getLength()
                            ? pFilter->GetExportFormatNumberForMediaType( aSettings.maMediaType )
                            : pFilter->GetExportFormatNumberForShortName( aSettings.maFilterName );
    sal_Bool            bVectorType = !pFilter->IsExportPixelFormat( nFilter );

    // create the output stuff
    Graphic aGraphic;

    USHORT nStatus = GetGraphic( aSettings, aGraphic, bVectorType ) ? GRFILTER_OK : GRFILTER_FILTERERROR;

    if( nStatus == GRFILTER_OK )
    {
        // export graphic only if it has a size
        const Size aGraphSize( aGraphic.GetPrefSize() );
        if ( ( aGraphSize.Width() == 0 ) || ( aGraphSize.Height() == 0 ) )
        {
            nStatus = GRFILTER_FILTERERROR;
        }
        else
        {
            // now we have a graphic, so export it
            if( aSettings.mxGraphicRenderer.is() )
            {
                // render graphic directly into given renderer
                aSettings.mxGraphicRenderer->render( aGraphic.GetXGraphic() );
            }
            else if( aSettings.mxOutputStream.is() )
            {
                // TODO: Either utilize optional XSeekable functionality for the
                // SvOutputStream, or adapt the graphic filter to not seek anymore.
                SvMemoryStream aStream( 1024, 1024 );

                nStatus = pFilter->ExportGraphic( aGraphic, String(), aStream, nFilter, &aSettings.maFilterData );

                // copy temp stream to XOutputStream
                SvOutputStream aOutputStream( aSettings.mxOutputStream );
                aStream.Seek(0);
                aOutputStream << aStream;
            }
            else
            {
                INetURLObject aURLObject( aSettings.maURL.Complete );
                DBG_ASSERT( aURLObject.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

                nStatus = XOutBitmap::ExportGraphic( aGraphic, aURLObject, *pFilter, nFilter, &aSettings.maFilterData );
            }
        }
    }

    if ( aSettings.mxInteractionHandler.is() && ( nStatus != GRFILTER_OK ) )
    {
        Any aInteraction;
        Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > lContinuations(1);
        ::framework::ContinuationApprove* pApprove = new ::framework::ContinuationApprove();
        lContinuations[0] = Reference< XInteractionContinuation >(static_cast< XInteractionContinuation* >(pApprove), UNO_QUERY);

        GraphicFilterRequest aErrorCode;
        aErrorCode.ErrCode = nStatus;
        aInteraction <<= aErrorCode;
        framework::InteractionRequest* pRequest = new framework::InteractionRequest( aInteraction, lContinuations );
        Reference< XInteractionRequest >xRequest( static_cast< XInteractionRequest* >(pRequest), UNO_QUERY );
        aSettings.mxInteractionHandler->handle( xRequest );
    }
    return nStatus == GRFILTER_OK;
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
    Sequence< OUString > aSeq( GraphicExporter_getSupportedServiceNames() );
    sal_Int32 nArgs = aSeq.getLength();
    const OUString* pService = aSeq.getConstArray();
    while( nArgs-- )
        if( *pService++ == ServiceName )
            return sal_True;

    return sal_False;
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

Graphic SvxGetGraphicForShape( SdrObject& rShape, bool bVector )
{
    Graphic aGraphic;
    try
    {
        rtl::Reference< GraphicExporter > xExporter( new GraphicExporter() );
        Reference< XComponent > xComp( rShape.getUnoShape(), UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );
        ExportSettings aSettings( rShape.GetModel() );
        xExporter->GetGraphic( aSettings, aGraphic, bVector );
    }
    catch( Exception& )
    {
        DBG_ERROR("SvxGetGraphicForShape(), exception caught!");
    }
    return aGraphic;
}

