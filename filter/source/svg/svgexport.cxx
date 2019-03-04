/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "svgwriter.hxx"
#include "svgfontexport.hxx"
#include "svgfilter.hxx"
#include <svgscript.hxx>

#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/graphic/PrimitiveFactory2D.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>

#include <rtl/bootstrap.hxx>
#include <svtools/miscopt.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <editeng/flditem.hxx>
#include <editeng/numitem.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/lang.h>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/animationexport.hxx>
#include <svx/svdograf.hxx>

#include <memory>

using namespace css::animations;
using namespace css::presentation;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

// - ooo elements and attributes -

#define NSPREFIX "ooo:"

#define SVG_PROP_TINYPROFILE                "TinyMode"
#define SVG_PROP_DTDSTRING                  "DTDString"
#define SVG_PROP_EMBEDFONTS                 "EmbedFonts"
#define SVG_PROP_NATIVEDECORATION           "UseNativeTextDecoration"
#define SVG_PROP_OPACITY                    "Opacity"
#define SVG_PROP_POSITIONED_CHARACTERS      "UsePositionedCharacters"

// ooo xml elements
static const char    aOOOElemTextField[] = NSPREFIX "text_field";


// ooo xml attributes for meta_slide
static const char    aOOOAttrSlide[] = NSPREFIX "slide";
static const char    aOOOAttrMaster[] = NSPREFIX "master";
static const char    aOOOAttrBackgroundVisibility[] = NSPREFIX "background-visibility";
static const char    aOOOAttrMasterObjectsVisibility[] = NSPREFIX "master-objects-visibility";
static const OUString aOOOAttrDateTimeField = NSPREFIX "date-time-field";
static const char    aOOOAttrFooterField[] = NSPREFIX "footer-field";
static const char    aOOOAttrHasTransition[] = NSPREFIX "has-transition";

// ooo xml attributes for pages and shapes
static const char    aOOOAttrName[] = NSPREFIX "name";

static const char    constSvgNamespace[] = "http://www.w3.org/2000/svg";


/** Text Field Class Hierarchy
    This is a set of classes for exporting text field meta info.
  */

class TextField
{
protected:
    SVGFilter::ObjectSet mMasterPageSet;
public:
    TextField() = default;
    TextField(TextField const &) = default;
    TextField(TextField &&) = default;
    TextField & operator =(TextField const &) = default;
    TextField & operator =(TextField &&) = default;

    virtual OUString getClassName() const
    {
        return OUString( "TextField" );
    }
    virtual bool equalTo( const TextField & aTextField ) const = 0;
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const = 0;
    virtual void elementExport( SVGExport* pSVGExport ) const
    {
        pSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", getClassName() );
    }
    void insertMasterPage( const Reference< css::drawing::XDrawPage>& xMasterPage )
    {
        mMasterPageSet.insert( xMasterPage );
    }
    virtual ~TextField() {}
protected:
    void implGrowCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets, const OUString& sText, const OUString& sTextFieldId ) const
    {
        const sal_Unicode * ustr = sText.getStr();
        sal_Int32 nLength = sText.getLength();
        for (auto const& masterPage : mMasterPageSet)
        {
            const Reference< XInterface > & xMasterPage = masterPage;
            for( sal_Int32 i = 0; i < nLength; ++i )
            {
                aTextFieldCharSets[ xMasterPage ][ sTextFieldId ].insert( ustr[i] );
            }
        }
    }
};


class FixedTextField : public TextField
{
public:
    OUString text;

    virtual OUString getClassName() const override
    {
        return OUString( "FixedTextField" );
    }
    virtual bool equalTo( const TextField & aTextField ) const override
    {
        if( const FixedTextField* aFixedTextField = dynamic_cast< const FixedTextField* >( &aTextField ) )
        {
            return ( text == aFixedTextField->text );
        }
        return false;
    }
    virtual void elementExport( SVGExport* pSVGExport ) const override
    {
        TextField::elementExport( pSVGExport );
        SvXMLElementExport aExp( *pSVGExport, XML_NAMESPACE_NONE, "g", true, true );
        pSVGExport->GetDocHandler()->characters( text );
    }
};


class FixedDateTimeField : public FixedTextField
{
public:
    FixedDateTimeField() {}
    virtual OUString getClassName() const override
    {
        return OUString( "FixedDateTimeField" );
    }
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const override
    {
        implGrowCharSet( aTextFieldCharSets, text, aOOOAttrDateTimeField );
    }
};


class FooterField : public FixedTextField
{
public:
    FooterField() {}
    virtual OUString getClassName() const override
    {
        return OUString( "FooterField" );
    }
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const override
    {
        static const OUString sFieldId = aOOOAttrFooterField;
        implGrowCharSet( aTextFieldCharSets, text, sFieldId );
    }
};


class VariableTextField : public TextField
{
public:
    virtual OUString getClassName() const override
    {
        return OUString( "VariableTextField" );
    }
};


class VariableDateTimeField : public VariableTextField
{
public:
    sal_Int32 format;

    VariableDateTimeField()
        : format(0)
    {
    }
    virtual OUString getClassName() const override
    {
        return OUString( "VariableDateTimeField" );
    }
    virtual bool equalTo( const TextField & aTextField ) const override
    {
        if( const VariableDateTimeField* aField = dynamic_cast< const VariableDateTimeField* >( &aTextField ) )
        {
            return ( format == aField->format );
        }
        return false;
    }
    virtual void elementExport( SVGExport* pSVGExport ) const override
    {
        VariableTextField::elementExport( pSVGExport );
        OUString sDateFormat, sTimeFormat;
        SvxDateFormat eDateFormat = static_cast<SvxDateFormat>( format & 0x0f );
        if( eDateFormat != SvxDateFormat::AppDefault )
        {
            switch( eDateFormat )
            {
                case SvxDateFormat::StdSmall:
                case SvxDateFormat::A:       // 13.02.96
                    sDateFormat.clear();
                    break;
                case SvxDateFormat::C:       // 13.Feb 1996
                    sDateFormat.clear();
                    break;
                case SvxDateFormat::D:       // 13.February 1996
                    sDateFormat.clear();
                    break;
                case SvxDateFormat::E:       // Tue, 13.February 1996
                    sDateFormat.clear();
                    break;
                case SvxDateFormat::StdBig:
                case SvxDateFormat::F:       // Tuesday, 13.February 1996
                    sDateFormat.clear();
                    break;
                // default case
                case SvxDateFormat::B:      // 13.02.1996
                default:
                    sDateFormat.clear();
                    break;
            }
        }

        SvxTimeFormat eTimeFormat = static_cast<SvxTimeFormat>( ( format >> 4 ) & 0x0f );
        if( eTimeFormat != SvxTimeFormat::AppDefault )
        {
            switch( eTimeFormat )
            {
                case SvxTimeFormat::HH24_MM_SS:      // 13:49:38
                    sTimeFormat.clear();
                    break;
                case SvxTimeFormat::HH12_MM_AMPM:      // 01:49 PM
                case SvxTimeFormat::HH12_MM:
                    sTimeFormat.clear();
                    break;
                case SvxTimeFormat::HH12_MM_SS_AMPM:     // 01:49:38 PM
                case SvxTimeFormat::HH12_MM_SS:
                    sTimeFormat.clear();
                    break;
                // default case
                case SvxTimeFormat::HH24_MM:     // 13:49
                default:
                    sTimeFormat.clear();
                    break;
            }
        }

        OUString sDateTimeFormat = sDateFormat + " " + sTimeFormat;

        pSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "date-time-format", sDateTimeFormat );
        SvXMLElementExport aExp( *pSVGExport, XML_NAMESPACE_NONE, "g", true, true );
    }
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const override
    {
        // we use the unicode char set in an improper way: we put in the date/time format
        // in order to pass it to the CalcFieldValue method
        static const OUString sFieldId = aOOOAttrDateTimeField + "-variable";
        for (auto const& masterPage : mMasterPageSet)
        {
            aTextFieldCharSets[ masterPage ][ sFieldId ].insert( static_cast<sal_Unicode>( format ) );
        }
    }
};


static bool operator==( const TextField & aLhsTextField, const TextField & aRhsTextField )
{
    return aLhsTextField.equalTo( aRhsTextField );
}


SVGExport::SVGExport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const Reference< XDocumentHandler >& rxHandler,
    const Sequence< PropertyValue >& rFilterData )
    : SvXMLExport( util::MeasureUnit::MM_100TH,
                   rContext, "",
                   xmloff::token::XML_TOKEN_INVALID,
                   SvXMLExportFlags::META|SvXMLExportFlags::PRETTY )
{
    SetDocHandler( rxHandler );
    GetDocHandler()->startDocument();

    // initializing filter settings from filter data
    comphelper::SequenceAsHashMap aFilterDataHashMap = rFilterData;

    // TinyProfile
    mbIsUseTinyProfile = aFilterDataHashMap.getUnpackedValueOrDefault(SVG_PROP_TINYPROFILE, false);

    // DTD string
    mbIsUseDTDString = aFilterDataHashMap.getUnpackedValueOrDefault(SVG_PROP_DTDSTRING, true);

    // Font Embedding
    comphelper::SequenceAsHashMap::const_iterator iter = aFilterDataHashMap.find(SVG_PROP_EMBEDFONTS);
    if(iter==aFilterDataHashMap.end())
    {
        OUString v;
        mbIsEmbedFonts = !rtl::Bootstrap::get("SVG_DISABLE_FONT_EMBEDDING", v);
    }
    else
    {
        if(!(iter->second >>= mbIsEmbedFonts))
            mbIsEmbedFonts = false;
    }

    // Native Decoration
    mbIsUseNativeTextDecoration = !mbIsUseTinyProfile && aFilterDataHashMap.getUnpackedValueOrDefault(SVG_PROP_NATIVEDECORATION, true);

    // Tiny Opacity (supported from SVG Tiny 1.2)
    mbIsUseOpacity = aFilterDataHashMap.getUnpackedValueOrDefault(SVG_PROP_OPACITY, true);

    // Positioned Characters    (The old method)
    mbIsUsePositionedCharacters = aFilterDataHashMap.getUnpackedValueOrDefault(SVG_PROP_POSITIONED_CHARACTERS, false);

    // add namespaces
    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);

    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_SMIL),
        GetXMLToken(XML_N_SMIL_COMPAT),
        XML_NAMESPACE_SMIL);

    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_ANIMATION),
        GetXMLToken(XML_N_ANIMATION),
        XML_NAMESPACE_ANIMATION);
}

SVGExport::~SVGExport()
{
    GetDocHandler()->endDocument();
}

ObjectRepresentation::ObjectRepresentation()
{
}

ObjectRepresentation::ObjectRepresentation( const Reference< XInterface >& rxObject,
                                            const GDIMetaFile& rMtf ) :
    mxObject( rxObject ),
    mxMtf( new GDIMetaFile( rMtf ) )
{
}

ObjectRepresentation::ObjectRepresentation( const ObjectRepresentation& rPresentation ) :
    mxObject( rPresentation.mxObject ),
    mxMtf( rPresentation.mxMtf ? new GDIMetaFile( *rPresentation.mxMtf ) : nullptr )
{
}

ObjectRepresentation& ObjectRepresentation::operator=( const ObjectRepresentation& rPresentation )
{
    // Check for self-assignment
    if (this == &rPresentation)
        return *this;

    mxObject = rPresentation.mxObject;
    mxMtf.reset(rPresentation.mxMtf ? new GDIMetaFile(*rPresentation.mxMtf) : nullptr);

    return *this;
}


namespace
{

BitmapChecksum GetBitmapChecksum( const MetaAction* pAction )
{
    BitmapChecksum nChecksum = 0;
    const MetaActionType nType = pAction->GetType();

    switch( nType )
    {
        case MetaActionType::BMPSCALE:
        {
            const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
            if( pA  )
                nChecksum = pA->GetBitmap().GetChecksum();
            else
                OSL_FAIL( "GetBitmapChecksum: MetaBmpScaleAction pointer is null." );
        }
        break;
        case MetaActionType::BMPEXSCALE:
        {
            const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
            if( pA )
                nChecksum = pA->GetBitmapEx().GetChecksum();
            else
                OSL_FAIL( "GetBitmapChecksum: MetaBmpExScaleAction pointer is null." );
        }
        break;
        default: break;
    }
    return nChecksum;
}

} // end anonymous namespace


static void MetaBitmapActionGetPoint( const MetaAction* pAction, Point& rPt )
{
    const MetaActionType nType = pAction->GetType();
    switch( nType )
    {
        case MetaActionType::BMPSCALE:
        {
            const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
            if( pA  )
                rPt = pA->GetPoint();
            else
                OSL_FAIL( "MetaBitmapActionGetPoint: MetaBmpScaleAction pointer is null." );
        }
        break;
        case MetaActionType::BMPEXSCALE:
        {
            const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
            if( pA )
                rPt = pA->GetPoint();
            else
                OSL_FAIL( "MetaBitmapActionGetPoint: MetaBmpExScaleAction pointer is null." );
        }
        break;
        default: break;
    }

}


size_t HashBitmap::operator()( const ObjectRepresentation& rObjRep ) const
{
    const GDIMetaFile& aMtf = rObjRep.GetRepresentation();
    if( aMtf.GetActionSize() == 1 )
    {
        return static_cast< size_t >( GetBitmapChecksum( aMtf.GetAction( 0 ) ) );
    }
    else
    {
        OSL_FAIL( "HashBitmap: metafile should have a single action." );
        return 0;
    }
}


bool EqualityBitmap::operator()( const ObjectRepresentation& rObjRep1,
                                     const ObjectRepresentation& rObjRep2 ) const
{
    const GDIMetaFile& aMtf1 = rObjRep1.GetRepresentation();
    const GDIMetaFile& aMtf2 = rObjRep2.GetRepresentation();
    if( aMtf1.GetActionSize() == 1 && aMtf2.GetActionSize() == 1 )
    {
        BitmapChecksum nChecksum1 = GetBitmapChecksum( aMtf1.GetAction( 0 ) );
        BitmapChecksum nChecksum2 = GetBitmapChecksum( aMtf2.GetAction( 0 ) );
        return ( nChecksum1 == nChecksum2 );
    }
    else
    {
        OSL_FAIL( "EqualityBitmap: metafile should have a single action." );
        return false;
    }
}


bool SVGFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
{
    Reference< XOutputStream >            xOStm;
    std::unique_ptr<SvStream>             pOStm;
    sal_Int32                             nLength = rDescriptor.getLength();
    const PropertyValue*                  pValue = rDescriptor.getConstArray();

    maFilterData.realloc( 0 );

    for ( sal_Int32 i = 0 ; i < nLength; ++i)
    {
        if ( pValue[ i ].Name == "OutputStream" )
            pValue[ i ].Value >>= xOStm;
        else if ( pValue[ i ].Name == "FileName" )
        {
            OUString aFileName;

            pValue[ i ].Value >>= aFileName;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, StreamMode::WRITE | StreamMode::TRUNC );

            if( pOStm )
                xOStm.set( new ::utl::OOutputStreamWrapper ( *pOStm ) );
        }
        else if ( pValue[ i ].Name == "FilterData" )
        {
            pValue[ i ].Value >>= maFilterData;
        }
    }

    if(mbWriterFilter || mbCalcFilter)
       return implExportWriterOrCalc(xOStm);

    return implExportImpressOrDraw(xOStm);
}

bool SVGFilter::implExportImpressOrDraw( const Reference< XOutputStream >& rxOStm)
{
    Reference< XComponentContext >        xContext( ::comphelper::getProcessComponentContext() ) ;
    bool                                  bRet = false;

    if( rxOStm.is() )
    {
        if( !mSelectedPages.empty() && !mMasterPageTargets.empty() )
        {
            Reference< XDocumentHandler > xDocHandler( implCreateExportDocumentHandler( rxOStm ), UNO_QUERY );

            if( xDocHandler.is() )
            {
                mbPresentation = Reference< XPresentationSupplier >( mxSrcDoc, UNO_QUERY ).is();
                mpObjects = new ObjectMap;

                // mpSVGExport = new SVGExport( xDocHandler );
                mpSVGExport = new SVGExport( xContext, xDocHandler, maFilterData );

                // xSVGExport is set up only to manage the life-time of the object pointed by mpSVGExport,
                // and in order to prevent that it is destroyed when passed to AnimationExporter.
                Reference< XInterface > xSVGExport = static_cast< css::document::XFilter* >( mpSVGExport );

                // create an id for each draw page
                for( auto& rPage : mSelectedPages )
                    implRegisterInterface( rPage );

                // create an id for each master page
                for(uno::Reference<drawing::XDrawPage> & mMasterPageTarget : mMasterPageTargets)
                    implRegisterInterface( mMasterPageTarget );

                SdrModel* pSdrModel(nullptr);

                try
                {
                    mxDefaultPage = mSelectedPages[0];

                    if( mxDefaultPage.is() )
                    {
                        SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( mxDefaultPage );

                        if( pSvxDrawPage )
                        {
                            mpDefaultSdrPage = pSvxDrawPage->GetSdrPage();
                            pSdrModel = &mpDefaultSdrPage->getSdrModelFromSdrPage();
                            SdrOutliner& rOutl = pSdrModel->GetDrawOutliner();

                            maOldFieldHdl = rOutl.GetCalcFieldValueHdl();
                            maNewFieldHdl = LINK(this, SVGFilter, CalcFieldHdl);
                            rOutl.SetCalcFieldValueHdl(maNewFieldHdl);
                        }

                        bRet = implExportDocument();
                    }
                }
                catch( ... )
                {
                    delete mpSVGDoc;
                    mpSVGDoc = nullptr;
                    OSL_FAIL( "Exception caught" );
                }

                if( nullptr != pSdrModel )
                {
                    // fdo#62682 The maNewFieldHdl can end up getting copied
                    // into various other outliners which live past this
                    // method, so get the full list of outliners and restore
                    // the maOldFieldHdl for all that have ended up using
                    // maNewFieldHdl
                    std::vector<SdrOutliner*> aOutliners(pSdrModel->GetActiveOutliners());
                    for (auto const& outliner : aOutliners)
                    {
                        if (maNewFieldHdl == outliner->GetCalcFieldValueHdl())
                            outliner->SetCalcFieldValueHdl(maOldFieldHdl);
                    }
                }

                delete mpSVGWriter;
                mpSVGWriter = nullptr;
                mpSVGExport = nullptr; // pointed object is released by xSVGExport dtor at the end of this scope
                delete mpSVGFontExport;
                mpSVGFontExport = nullptr;
                delete mpObjects;
                mpObjects = nullptr;
                mbPresentation = false;
            }
        }
    }
    return bRet;
}


bool SVGFilter::implExportWriterOrCalc( const Reference< XOutputStream >& rxOStm )
{
    Reference< XComponentContext >        xContext( ::comphelper::getProcessComponentContext() ) ;
    bool                                  bRet = false;

    if( rxOStm.is() )
    {
        Reference< XDocumentHandler > xDocHandler( implCreateExportDocumentHandler( rxOStm ), UNO_QUERY );

        if( xDocHandler.is() )
        {
            mpObjects = new ObjectMap;

            // mpSVGExport = new SVGExport( xDocHandler );
            mpSVGExport = new SVGExport( xContext, xDocHandler, maFilterData );

            // xSVGExport is set up only to manage the life-time of the object pointed by mpSVGExport,
            // and in order to prevent that it is destroyed when passed to AnimationExporter.
            Reference< XInterface > xSVGExport = static_cast< css::document::XFilter* >( mpSVGExport );

            try
            {
                mxDefaultPage = mSelectedPages[0];
                bRet = implExportDocument();
            }
            catch( ... )
            {
                delete mpSVGDoc;
                mpSVGDoc = nullptr;
                OSL_FAIL( "Exception caught" );
            }

            delete mpSVGWriter;
            mpSVGWriter = nullptr;
            mpSVGExport = nullptr; // pointed object is released by xSVGExport dtor at the end of this scope
            delete mpSVGFontExport;
            mpSVGFontExport = nullptr;
            delete mpObjects;
            mpObjects = nullptr;
        }
    }
    return bRet;
}

bool SVGFilter::implExportWriterTextGraphic( const Reference< view::XSelectionSupplier >& xSelectionSupplier )
{
    Any selection = xSelectionSupplier->getSelection();
    uno::Reference<lang::XServiceInfo> xSelection;
    selection >>= xSelection;
    if (xSelection.is() && xSelection->supportsService("com.sun.star.text.TextGraphicObject"))
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xSelection, uno::UNO_QUERY);
        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue("Graphic") >>= xGraphic;

        if (!xGraphic.is())
            return false;

        const Graphic aGraphic(xGraphic);

        // Calculate size from Graphic
        Point aPos( OutputDevice::LogicToLogic(aGraphic.GetPrefMapMode().GetOrigin(), aGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)) );
        Size  aSize( OutputDevice::LogicToLogic(aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)) );

        assert(mSelectedPages.size() == 1);
        SvxDrawPage* pSvxDrawPage(SvxDrawPage::getImplementation(mSelectedPages[0]));
        if(pSvxDrawPage == nullptr || pSvxDrawPage->GetSdrPage() == nullptr)
            return false;

        SdrGrafObj* pGraphicObj = new SdrGrafObj(pSvxDrawPage->GetSdrPage()->getSdrModelFromSdrPage(), aGraphic, tools::Rectangle( aPos, aSize ));
        uno::Reference< drawing::XShape > xShape = GetXShapeForSdrObject(pGraphicObj);
        uno::Reference< XPropertySet > xShapePropSet(xShape, uno::UNO_QUERY);
        xShapePropSet->setPropertyValue("Graphic", uno::Any(xGraphic));

        maShapeSelection = drawing::ShapeCollection::create(comphelper::getProcessComponentContext());
        maShapeSelection->add(xShape);
    }

    return true;
}


Reference< XWriter > SVGFilter::implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm )
{
    Reference< XWriter >       xSaxWriter;

    if( rxOStm.is() )
    {
        xSaxWriter = Writer::create( ::comphelper::getProcessComponentContext() );
        xSaxWriter->setOutputStream( rxOStm );
    }

    return xSaxWriter;
}


bool SVGFilter::implLookForFirstVisiblePage()
{
    sal_Int32 nCurPage = 0, nLastPage = mSelectedPages.size() - 1;

    if(!mbPresentation || mbSinglePage)
    {
        mnVisiblePage = nCurPage;
    }

    while( ( nCurPage <= nLastPage ) && ( -1 == mnVisiblePage ) )
    {
        const Reference< css::drawing::XDrawPage > & xDrawPage = mSelectedPages[nCurPage];

        if( xDrawPage.is() )
        {
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

            if( xPropSet.is() )
            {
                bool bVisible = false;

                if( ( xPropSet->getPropertyValue( "Visible" ) >>= bVisible ) && bVisible )
                {
                    mnVisiblePage = nCurPage;
                }
            }
        }
        ++nCurPage;
    }

    return ( mnVisiblePage != -1 );
}


bool SVGFilter::implExportDocument()
{
    sal_Int32        nDocX = 0, nDocY = 0; // #i124608#
    sal_Int32        nDocWidth = 0, nDocHeight = 0;
    bool             bRet = false;
    sal_Int32        nLastPage = mSelectedPages.size() - 1;

    mbSinglePage = (nLastPage == 0);
    mnVisiblePage = -1;

    const Reference< XPropertySet >             xDefaultPagePropertySet( mxDefaultPage, UNO_QUERY );

    // #i124608#
    mbExportShapeSelection = mbSinglePage && maShapeSelection.is() && maShapeSelection->getCount();

    if(xDefaultPagePropertySet.is())
    {
        xDefaultPagePropertySet->getPropertyValue( "Width" ) >>= nDocWidth;
        xDefaultPagePropertySet->getPropertyValue( "Height" ) >>= nDocHeight;
    }

    if(mbExportShapeSelection)
    {
        // #i124608# create BoundRange and set nDocX, nDocY, nDocWidth and nDocHeight
        basegfx::B2DRange aShapeRange;

        for(sal_Int32 a(0); a < maShapeSelection->getCount(); a++)
        {
            Reference< css::drawing::XShape > xShapeCandidate;
            if((maShapeSelection->getByIndex(a) >>= xShapeCandidate) && xShapeCandidate.is())
            {
                Reference< XPropertySet > xShapePropSet( xShapeCandidate, UNO_QUERY );
                css::awt::Rectangle aBoundRect;
                if( xShapePropSet.is() && ( xShapePropSet->getPropertyValue( "BoundRect" ) >>= aBoundRect ))
                {
                    aShapeRange.expand(basegfx::B2DTuple(aBoundRect.X, aBoundRect.Y));
                    aShapeRange.expand(basegfx::B2DTuple(aBoundRect.X + aBoundRect.Width,
                                                         aBoundRect.Y + aBoundRect.Height));
                }
            }
        }

        if(!aShapeRange.isEmpty())
        {
            nDocX = basegfx::fround(aShapeRange.getMinX());
            nDocY = basegfx::fround(aShapeRange.getMinY());
            nDocWidth  = basegfx::fround(aShapeRange.getWidth());
            nDocHeight = basegfx::fround(aShapeRange.getHeight());
        }
    }

    if(mbWriterFilter || mbCalcFilter)
        implExportDocumentHeaderWriterOrCalc(nDocX, nDocY, nDocWidth, nDocHeight);
    else
        implExportDocumentHeaderImpressOrDraw(nDocX, nDocY, nDocWidth, nDocHeight);


    if( implLookForFirstVisiblePage() )  // OK! We found at least one visible page.
    {
        if( mbPresentation && !mbExportShapeSelection )
        {
            implGenerateMetaData();
            implExportAnimations();
        }
        else
        {
            implGetPagePropSet( mSelectedPages[0] );
        }

        // Create the (Shape, GDIMetaFile) map
        if( implCreateObjects() )
        {
            ::std::vector< ObjectRepresentation >    aObjects( mpObjects->size() );
            sal_uInt32                               nPos = 0;

            for (auto const& elem : *mpObjects)
            {
                aObjects[ nPos++ ] = elem.second;
            }

            mpSVGFontExport = new SVGFontExport( *mpSVGExport, aObjects );
            mpSVGWriter = new SVGActionWriter( *mpSVGExport, *mpSVGFontExport );

            if( mpSVGExport->IsEmbedFonts() )
            {
                mpSVGFontExport->EmbedFonts();
            }
            if( !mpSVGExport->IsUsePositionedCharacters() )
            {
                implExportTextShapeIndex();
                implEmbedBulletGlyphs();
                implExportTextEmbeddedBitmaps();
            }

            // #i124608# export a given object selection, so no MasterPage export at all
            if (!mbExportShapeSelection)
                implExportMasterPages( mMasterPageTargets, 0, mMasterPageTargets.size() - 1 );
            implExportDrawPages( mSelectedPages, 0, nLastPage );

            if( mbPresentation && !mbExportShapeSelection )
            {
                implGenerateScript();
            }

            delete mpSVGDoc;
            mpSVGDoc = nullptr;
            bRet = true;
        }
    }

    return bRet;
}

void SVGFilter::implExportDocumentHeaderImpressOrDraw(sal_Int32 nDocX, sal_Int32 nDocY,
                                                    sal_Int32 nDocWidth, sal_Int32 nDocHeight)
{
    const Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );
    if( xExtDocHandler.is() && !mpSVGExport->IsUseTinyProfile() )
    {
        xExtDocHandler->unknown( SVG_DTD_STRING );
    }

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "version", "1.2" );

    if( mpSVGExport->IsUseTinyProfile() )
         mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "baseProfile", "tiny" );

    // The following if block means that the slide size is not adapted
    // to the size of the browser window, moreover the slide is top left aligned
    // instead of centered:
    OUString aAttr;
    if( !mbPresentation )
    {
        aAttr = OUString::number( nDocWidth * 0.01 ) + "mm";
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

        aAttr = OUString::number( nDocHeight * 0.01 ) + "mm";
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );
    }

    // #i124608# set viewBox explicitly to the exported content
    if (mbExportShapeSelection)
    {
        aAttr = OUString::number(nDocX) + " " + OUString::number(nDocY) + " ";
    }
    else
    {
        aAttr = "0 0 ";
    }

    aAttr += OUString::number(nDocWidth) + " " + OUString::number(nDocHeight);

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", "xMidYMid" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill-rule", "evenodd" );

    // standard line width is based on 1 pixel on a 90 DPI device (0.28222mmm)
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-width", OUString::number( 28.222 ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-linejoin", "round" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns", constSvgNamespace );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", "http://xml.openoffice.org/svg/export" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:presentation", "http://sun.com/xmlns/staroffice/presentation" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:smil", "http://www.w3.org/2001/SMIL20/" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:anim", "urn:oasis:names:tc:opendocument:xmlns:animation:1.0" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xml:space", "preserve" );

    mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", true, true );

    // Create a ClipPath element that will be used for cutting bitmaps and other elements that could exceed the page margins.
    if(!mbExportShapeSelection)
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "ClipPathGroup" );
        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );
        {
            msClipPathId = "presentation_clip_path";
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", msClipPathId );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clipPathUnits", "userSpaceOnUse" );
            SvXMLElementExport aClipPathElem( *mpSVGExport, XML_NAMESPACE_NONE, "clipPath", true, true );
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "x", OUString::number( nDocX ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "y", OUString::number( nDocY ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", OUString::number( nDocWidth ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", OUString::number( nDocHeight ) );
                SvXMLElementExport aRectElem( *mpSVGExport, XML_NAMESPACE_NONE, "rect", true, true );
            }
        }
        // Create a ClipPath element applied to the leaving slide in order
        // to avoid that slide borders are visible during transition
        {
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "presentation_clip_path_shrink" );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clipPathUnits", "userSpaceOnUse" );
            SvXMLElementExport aClipPathElem( *mpSVGExport, XML_NAMESPACE_NONE, "clipPath", true, true );
            {
                sal_Int32 nDocWidthExt = nDocWidth / 500;
                sal_Int32 nDocHeightExt = nDocHeight / 500;
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "x", OUString::number( nDocX + nDocWidthExt / 2 ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "y", OUString::number( nDocY + nDocHeightExt / 2) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", OUString::number( nDocWidth - nDocWidthExt ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", OUString::number( nDocHeight - nDocHeightExt ) );
                SvXMLElementExport aRectElem( *mpSVGExport, XML_NAMESPACE_NONE, "rect", true, true );
            }
        }
    }
}

void SVGFilter::implExportDocumentHeaderWriterOrCalc(sal_Int32 nDocX, sal_Int32 nDocY,
                                               sal_Int32 nDocWidth, sal_Int32 nDocHeight)
{
    OUString aAttr;
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "version", "1.2" );

    aAttr = OUString::number( nDocWidth * 0.01 ) + "mm";
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

    aAttr = OUString::number( nDocHeight * 0.01 ) + "mm";
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );

    aAttr = OUString::number(nDocX) + " " + OUString::number(nDocY) + " ";
    aAttr += OUString::number(nDocWidth) + " " + OUString::number(nDocHeight);

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", "xMidYMid" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill-rule", "evenodd" );

    // standard line width is based on 1 pixel on a 90 DPI device (0.28222mmm)
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-width", OUString::number( 28.222 ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-linejoin", "round" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns", constSvgNamespace );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", "http://xml.openoffice.org/svg/export" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:smil", "urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xml:space", "preserve" );

    mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", true, true );
}

/// Append aField to aFieldSet if it is not already present in the set and create the field id sFieldId
template< typename TextFieldType >
static OUString implGenerateFieldId( std::vector< std::unique_ptr<TextField> > & aFieldSet,
                              const TextFieldType & aField,
                              const OUString & sOOOElemField,
                              const Reference< css::drawing::XDrawPage >& xMasterPage )
{
    bool bFound = false;
    sal_Int32 i;
    sal_Int32 nSize = aFieldSet.size();
    for( i = 0; i < nSize; ++i )
    {
        if( *(aFieldSet[i]) == aField )
        {
            bFound = true;
            break;
        }
    }
    OUString sFieldId(sOOOElemField + "_");
    if( !bFound )
    {
        aFieldSet.emplace_back( new TextFieldType( aField ) );
    }
    aFieldSet[i]->insertMasterPage( xMasterPage );
    sFieldId += OUString::number( i );
    return sFieldId;
}


void SVGFilter::implGenerateMetaData()
{
    sal_Int32 nCount = mSelectedPages.size();
    if( nCount != 0 )
    {
        // we wrap all meta presentation info into a svg:defs element
        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", NSPREFIX "meta_slides" );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "number-of-slides", OUString::number( nCount ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "start-slide-number", OUString::number( mnVisiblePage ) );

        if( mpSVGExport->IsUsePositionedCharacters() )
        {
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "use-positioned-chars", "true" );
        }

        // Add a (global) Page Numbering Type attribute for the document
        // NOTE: at present pSdrModel->GetPageNumType() returns always css::style::NumberingType::ARABIC
        // so the following code fragment is pretty useless
        sal_Int32 nPageNumberingType = css::style::NumberingType::ARABIC;
        SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( mSelectedPages[0] );
        if( pSvxDrawPage )
        {
            SdrPage* pSdrPage = pSvxDrawPage->GetSdrPage();
            SdrModel& rSdrModel(pSdrPage->getSdrModelFromSdrPage());
            nPageNumberingType = rSdrModel.GetPageNumType();

            // That is used by CalcFieldHdl method.
            mVisiblePagePropSet.nPageNumberingType = nPageNumberingType;
        }
        if( nPageNumberingType != css::style::NumberingType::NUMBER_NONE )
        {
            OUString sNumberingType;
            switch( nPageNumberingType )
            {
                case css::style::NumberingType::CHARS_UPPER_LETTER:
                    sNumberingType = "alpha-upper";
                    break;
                case css::style::NumberingType::CHARS_LOWER_LETTER:
                    sNumberingType = "alpha-lower";
                    break;
                case css::style::NumberingType::ROMAN_UPPER:
                    sNumberingType = "roman-upper";
                    break;
                case css::style::NumberingType::ROMAN_LOWER:
                    sNumberingType = "roman-lower";
                    break;
                case css::style::NumberingType::ARABIC:
                    // arabic numbering type is the default, so we do not append any attribute for it
                default:
                    // in case the numbering type is not handled we fall back on arabic numbering
                    break;
            }
            if( !sNumberingType.isEmpty() )
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "page-numbering-type", sNumberingType );
        }


        {
            SvXMLElementExport    aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
            const OUString                aId( NSPREFIX "meta_slide" );
            const OUString                aElemTextFieldId( aOOOElemTextField );
            std::vector< std::unique_ptr<TextField> >     aFieldSet;

            // dummy slide - used as leaving slide for transition on the first slide
            if( mbPresentation )
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", NSPREFIX "meta_dummy_slide" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, "dummy-slide" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMaster, "dummy-master-page" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrBackgroundVisibility, "hidden" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMasterObjectsVisibility, "hidden" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrHasTransition, "false" );
                SvXMLElementExport aMetaDummySlideElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
            }

            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                const Reference< css::drawing::XDrawPage > & xDrawPage = mSelectedPages[i];
                Reference< css::drawing::XMasterPageTarget > xMasterPageTarget( xDrawPage, UNO_QUERY );
                Reference< css::drawing::XDrawPage > xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
                OUString aSlideId(aId + "_" + OUString::number( i ));

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", aSlideId );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, implGetValidIDFromInterface( xDrawPage ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMaster, implGetValidIDFromInterface( xMasterPage ) );


                if( mbPresentation )
                {
                    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

                    if( xPropSet.is() )
                    {
                        bool bBackgroundVisibility                = true;     // default: visible
                        bool bBackgroundObjectsVisibility         = true;     // default: visible

                        FixedDateTimeField            aFixedDateTimeField;
                        VariableDateTimeField         aVariableDateTimeField;
                        FooterField                   aFooterField;

                        xPropSet->getPropertyValue( "IsBackgroundVisible" )  >>= bBackgroundVisibility;
                        // in case the attribute is set to its default value it is not appended to the meta-slide element
                        if( !bBackgroundVisibility ) // visibility default value: 'visible'
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrBackgroundVisibility, "hidden" );

                        // Page Number, Date/Time, Footer and Header Fields are regarded as background objects.
                        // So bBackgroundObjectsVisibility overrides visibility of master page text fields.
                        xPropSet->getPropertyValue( "IsBackgroundObjectsVisible" )  >>= bBackgroundObjectsVisibility;
                        if( bBackgroundObjectsVisibility ) // visibility default value: 'visible'
                        {
                            bool bPageNumberVisibility    = false;    // default: hidden
                            bool bDateTimeVisibility      = true;     // default: visible
                            bool bFooterVisibility        = true;     // default: visible

                            //  Page Number Field
                            xPropSet->getPropertyValue( "IsPageNumberVisible" )  >>= bPageNumberVisibility;
                            bPageNumberVisibility = bPageNumberVisibility && ( nPageNumberingType != css::style::NumberingType::NUMBER_NONE );
                            if( bPageNumberVisibility ) // visibility default value: 'hidden'
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "page-number-visibility", "visible" );
                            }

                            // Date/Time Field
                            xPropSet->getPropertyValue( "IsDateTimeVisible" ) >>= bDateTimeVisibility;
                            if( bDateTimeVisibility ) // visibility default value: 'visible'
                            {
                                bool bDateTimeFixed           = true;     // default: fixed
                                xPropSet->getPropertyValue( "IsDateTimeFixed" ) >>= bDateTimeFixed;
                                if( bDateTimeFixed ) // we are interested only in the field text not in the date/time format
                                {
                                    xPropSet->getPropertyValue( "DateTimeText" ) >>= aFixedDateTimeField.text;
                                    if( !aFixedDateTimeField.text.isEmpty() )
                                    {
                                        OUString sFieldId = implGenerateFieldId( aFieldSet, aFixedDateTimeField, aElemTextFieldId, xMasterPage );
                                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeField, sFieldId );
                                    }
                                }
                                else // the inverse applies: we are interested only in the date/time format not in the field text
                                {
                                    xPropSet->getPropertyValue( "DateTimeFormat" ) >>= aVariableDateTimeField.format;
                                    OUString sFieldId = implGenerateFieldId( aFieldSet, aVariableDateTimeField, aElemTextFieldId, xMasterPage );
                                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeField, sFieldId );
                                }
                            }
                            else
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "date-time-visibility", "hidden" );
                            }

                            // Footer Field
                            xPropSet->getPropertyValue( "IsFooterVisible" )  >>= bFooterVisibility;
                            if( bFooterVisibility ) // visibility default value: 'visible'
                            {
                                xPropSet->getPropertyValue( "FooterText" ) >>= aFooterField.text;
                                if( !aFooterField.text.isEmpty() )
                                {
                                    OUString sFieldId = implGenerateFieldId( aFieldSet, aFooterField, aElemTextFieldId, xMasterPage );
                                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrFooterField, sFieldId );
                                }
                            }
                            else
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "footer-visibility", "hidden" );
                            }
                        }
                        else
                        {
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMasterObjectsVisibility, "hidden" );
                        }

                        // We look for a slide transition.
                        // Transition properties are exported together with animations.
                        sal_Int16 nTransitionType(0);
                        if( xPropSet->getPropertySetInfo()->hasPropertyByName( "TransitionType" ) &&
                            (xPropSet->getPropertyValue( "TransitionType" ) >>= nTransitionType) )
                        {
                            sal_Int16 nTransitionSubType(0);
                            if( xPropSet->getPropertyValue( "TransitionSubtype" )  >>= nTransitionSubType )
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrHasTransition, "true" );
                            }
                        }

                    }
                }

                {
                    SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
                }  // when the aExp2 destructor is called the </g> tag is appended to the output file
            }

            // export text field elements
            if( mbPresentation )
            {
                for( sal_Int32 i = 0, nSize = aFieldSet.size(); i < nSize; ++i )
                {
                    OUString sElemId = OUStringLiteral(aOOOElemTextField) + "_" + OUString::number( i );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sElemId );
                    aFieldSet[i]->elementExport( mpSVGExport );
                }
                if( mpSVGExport->IsEmbedFonts() && mpSVGExport->IsUsePositionedCharacters() )
                {
                    for(std::unique_ptr<TextField>& i : aFieldSet)
                    {
                        i->growCharSet( mTextFieldCharSets );
                    }
                }
            }

            // text fields are used only for generating meta info so we don't need them anymore
            aFieldSet.clear();
        }
    }
}


void SVGFilter::implExportAnimations()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "presentation-animations" );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

    for(uno::Reference<drawing::XDrawPage> & mSelectedPage : mSelectedPages)
    {
        Reference< XPropertySet > xProps( mSelectedPage, UNO_QUERY );

        if( xProps.is() && xProps->getPropertySetInfo()->hasPropertyByName( "TransitionType" ) )
        {
            sal_Int16 nTransition = 0;
            xProps->getPropertyValue( "TransitionType" )  >>= nTransition;
            // we have a slide transition ?
            bool bHasEffects = ( nTransition != 0 );

            Reference< XAnimationNodeSupplier > xAnimNodeSupplier( mSelectedPage, UNO_QUERY );
            if( xAnimNodeSupplier.is() )
            {
                Reference< XAnimationNode > xRootNode = xAnimNodeSupplier->getAnimationNode();
                if( xRootNode.is() )
                {
                    if( !bHasEffects )
                    {
                        // first check if there are no animations
                        Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
                        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                        if( xEnumeration->hasMoreElements() )
                        {
                            // first child node may be an empty main sequence, check this
                            Reference< XAnimationNode > xMainNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
                            Reference< XEnumerationAccess > xMainEnumerationAccess( xMainNode, UNO_QUERY_THROW );
                            Reference< XEnumeration > xMainEnumeration( xMainEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );

                            // only export if the main sequence is not empty or if there are additional
                            // trigger sequences
                            bHasEffects = xMainEnumeration->hasMoreElements() || xEnumeration->hasMoreElements();
                        }
                    }
                    if( bHasEffects )
                    {
                        OUString sId = implGetValidIDFromInterface( mSelectedPage );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, sId  );
                        sId += "-animations";
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId  );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Animations" );
                        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

                        rtl::Reference< xmloff::AnimationsExporter > xAnimationsExporter;
                        xAnimationsExporter = new xmloff::AnimationsExporter( *mpSVGExport, xProps );
                        xAnimationsExporter->prepare( xRootNode );
                        xAnimationsExporter->exportAnimations( xRootNode );
                    }
                }
            }
        }
    }
}


void SVGFilter::implExportTextShapeIndex()
{
    if(mbExportShapeSelection)
        return;

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "TextShapeIndex" );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

    sal_Int32 nCount = mSelectedPages.size();
    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        const Reference< css::drawing::XDrawPage > & xDrawPage = mSelectedPages[i];
        if( mTextShapeIdListMap.find( xDrawPage ) != mTextShapeIdListMap.end() )
        {
            OUString sTextShapeIdList = mTextShapeIdListMap[xDrawPage].trim();

            const OUString& rPageId = implGetValidIDFromInterface( Reference<XInterface>(xDrawPage, UNO_QUERY) );
            if( !rPageId.isEmpty() && !sTextShapeIdList.isEmpty() )
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, rPageId  );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "id-list", sTextShapeIdList );
                SvXMLElementExport aGElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
            }
        }
    }
}


void SVGFilter::implEmbedBulletGlyphs()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "EmbeddedBulletChars" );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

    OUString sPathData = "M 580,1141 L 1163,571 580,0 -4,571 580,1141 Z";
    implEmbedBulletGlyph( 57356, sPathData );
    sPathData = "M 8,1128 L 1137,1128 1137,0 8,0 8,1128 Z";
    implEmbedBulletGlyph( 57354, sPathData );
    sPathData = "M 174,0 L 602,739 174,1481 1456,739 174,0 Z M 1358,739 L 309,1346 659,739 1358,739 Z";
    implEmbedBulletGlyph( 10146, sPathData );
    sPathData = "M 2015,739 L 1276,0 717,0 1260,543 174,543 174,936 1260,936 717,1481 1274,1481 2015,739 Z";
    implEmbedBulletGlyph( 10132, sPathData );
    sPathData = "M 0,-2 C -7,14 -16,27 -25,37 L 356,567 C 262,823 215,952 215,954 215,979 228,992 255,992 264,992 276,990 289,987 310,991 331,999 354,1012 L 381,999 492,748 772,1049 836,1024 860,1049 C 881,1039 901,1025 922,1006 886,937 835,863 770,784 769,783 710,716 594,584 L 774,223 C 774,196 753,168 711,139 L 727,119 C 717,90 699,76 672,76 641,76 570,178 457,381 L 164,-76 C 142,-110 111,-127 72,-127 30,-127 9,-110 8,-76 1,-67 -2,-52 -2,-32 -2,-23 -1,-13 0,-2 Z";
    implEmbedBulletGlyph( 10007, sPathData );
    sPathData = "M 285,-33 C 182,-33 111,30 74,156 52,228 41,333 41,471 41,549 55,616 82,672 116,743 169,778 240,778 293,778 328,747 346,684 L 369,508 C 377,444 397,411 428,410 L 1163,1116 C 1174,1127 1196,1133 1229,1133 1271,1133 1292,1118 1292,1087 L 1292,965 C 1292,929 1282,901 1262,881 L 442,47 C 390,-6 338,-33 285,-33 Z";
    implEmbedBulletGlyph( 10004, sPathData );
    sPathData = "M 813,0 C 632,0 489,54 383,161 276,268 223,411 223,592 223,773 276,916 383,1023 489,1130 632,1184 813,1184 992,1184 1136,1130 1245,1023 1353,916 1407,772 1407,592 1407,412 1353,268 1245,161 1136,54 992,0 813,0 Z";
    implEmbedBulletGlyph( 9679, sPathData );
    sPathData = "M 346,457 C 273,457 209,483 155,535 101,586 74,649 74,723 74,796 101,859 155,911 209,963 273,989 346,989 419,989 480,963 531,910 582,859 608,796 608,723 608,648 583,586 532,535 482,483 420,457 346,457 Z";
    implEmbedBulletGlyph( 8226, sPathData );
    sPathData = "M -4,459 L 1135,459 1135,606 -4,606 -4,459 Z";
    implEmbedBulletGlyph( 8211, sPathData );
    sPathData = "M 173,740 C 173,903 231,1043 346,1159 462,1274 601,1332 765,1332 928,1332 1067,1274 1183,1159 1299,1043 1357,903 1357,740 1357,577 1299,437 1183,322 1067,206 928,148 765,148 601,148 462,206 346,322 231,437 173,577 173,740 Z";
    implEmbedBulletGlyph( 61548, sPathData );
}


void SVGFilter::implEmbedBulletGlyph( sal_Unicode cBullet, const OUString & sPathData )
{
    OUString sId = "bullet-char-template-" + OUString::number( static_cast<sal_Int32>(cBullet) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId );

    OUString sFactor = OUString::number( 1.0 / 2048 );
    OUString sTransform = "scale(" + sFactor + ",-" + sFactor + ")";
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "transform", sTransform );

    SvXMLElementExport aGElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "d", sPathData );
    SvXMLElementExport aPathElem( *mpSVGExport, XML_NAMESPACE_NONE, "path", true, true );

}


/** SVGFilter::implExportTextEmbeddedBitmaps
    We export bitmaps embedded into text shapes, such as those used by list
    items with image style, only once in a specific defs element.
 */
void SVGFilter::implExportTextEmbeddedBitmaps()
{
    if (mEmbeddedBitmapActionSet.empty())
        return;

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "TextEmbeddedBitmaps" );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", true, true );

    OUString sId;

    for (auto const& embeddedBitmapAction : mEmbeddedBitmapActionSet)
    {
        const GDIMetaFile& aMtf = embeddedBitmapAction.GetRepresentation();

        if( aMtf.GetActionSize() == 1 )
        {
            MetaAction* pAction = aMtf.GetAction( 0 );
            if( pAction )
            {
                BitmapChecksum nId = GetBitmapChecksum( pAction );
                sId = "bitmap(" + OUString::number( nId ) + ")";
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId );

                const Reference< XInterface >& rxShape = embeddedBitmapAction.GetObject();
                Reference< XPropertySet > xShapePropSet( rxShape, UNO_QUERY );
                css::awt::Rectangle    aBoundRect;
                if( xShapePropSet.is() && ( xShapePropSet->getPropertyValue( "BoundRect" ) >>= aBoundRect ) )
                {
                    // Origin of the coordinate device must be (0,0).
                    const Point aTopLeft;
                    const Size  aSize( aBoundRect.Width, aBoundRect.Height );

                    Point aPt;
                    MetaBitmapActionGetPoint( pAction, aPt );
                    // The image must be exported with x, y attribute set to 0,
                    // on the contrary when referenced by a <use> element,
                    // specifying the wanted position, they will result
                    // misplaced.
                    pAction->Move( -aPt.X(), -aPt.Y() );
                    mpSVGWriter->WriteMetaFile( aTopLeft, aSize, aMtf, 0xffffffff );
                    // We reset to the original values so that when the <use>
                    // element is created the x, y attributes are correct.
                    pAction->Move( aPt.X(), aPt.Y() );
                }
                else
                {
                    OSL_FAIL( "implExportTextEmbeddedBitmaps: no shape bounding box." );
                    return;
                }
            }
            else
            {
                OSL_FAIL( "implExportTextEmbeddedBitmaps: metafile should have MetaBmpExScaleAction only." );
                return;
            }
        }
        else
        {
            OSL_FAIL( "implExportTextEmbeddedBitmaps: metafile should have a single action." );
            return;
        }
    }
}

void SVGFilter::implGenerateScript()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "type", "text/ecmascript" );

    {
        SvXMLElementExport                       aExp( *mpSVGExport, XML_NAMESPACE_NONE, "script", true, true );
        Reference< XExtendedDocumentHandler >    xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

        if( xExtDocHandler.is() )
        {
            for (size_t i = 0; i < N_SVGSCRIPT_FRAGMENTS; ++i)
            {
                xExtDocHandler->unknown(OUString::createFromAscii(g_SVGScripts[i]));
            }
        }
    }
}


Any SVGFilter::implSafeGetPagePropSet( const OUString & sPropertyName,
                                            const Reference< XPropertySet > & rxPropSet,
                                            const Reference< XPropertySetInfo > & rxPropSetInfo )
{
    Any result;
    if( rxPropSetInfo->hasPropertyByName( sPropertyName ) )
    {
        result = rxPropSet->getPropertyValue( sPropertyName );
    }
    return result;
}


/** We collect info on master page elements visibility, and placeholder text shape content.
    This method is used when exporting a single page as implGenerateMetaData is not invoked.
 */
void SVGFilter::implGetPagePropSet( const Reference< css::drawing::XDrawPage > & rxPage )
{
    mVisiblePagePropSet.bIsBackgroundVisible                = true;
    mVisiblePagePropSet.bAreBackgroundObjectsVisible        = true;
    mVisiblePagePropSet.bIsPageNumberFieldVisible           = false;
    mVisiblePagePropSet.bIsHeaderFieldVisible               = false;
    mVisiblePagePropSet.bIsFooterFieldVisible               = true;
    mVisiblePagePropSet.bIsDateTimeFieldVisible             = true;
    mVisiblePagePropSet.bIsDateTimeFieldFixed               = true;
    mVisiblePagePropSet.nDateTimeFormat                     = SvxDateFormat::B;
    mVisiblePagePropSet.nPageNumberingType                  = css::style::NumberingType::ARABIC;

    //  We collect info on master page elements visibility, and placeholder text shape content.
    Reference< XPropertySet > xPropSet( rxPage, UNO_QUERY );
    if( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        if( xPropSetInfo.is() )
        {
            implSafeGetPagePropSet( "IsBackgroundVisible", xPropSet, xPropSetInfo )          >>= mVisiblePagePropSet.bIsBackgroundVisible;
            implSafeGetPagePropSet( "IsBackgroundObjectsVisible", xPropSet, xPropSetInfo )   >>= mVisiblePagePropSet.bAreBackgroundObjectsVisible;
            implSafeGetPagePropSet( "IsPageNumberVisible", xPropSet, xPropSetInfo )          >>= mVisiblePagePropSet.bIsPageNumberFieldVisible;
            implSafeGetPagePropSet( "IsHeaderVisible", xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsHeaderFieldVisible;
            implSafeGetPagePropSet( "IsFooterVisible", xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsFooterFieldVisible;
            implSafeGetPagePropSet( "IsDateTimeVisible", xPropSet, xPropSetInfo )            >>= mVisiblePagePropSet.bIsDateTimeFieldVisible;

            implSafeGetPagePropSet( "IsDateTimeFixed", xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsDateTimeFieldFixed;
            sal_Int32 nTmp;
            if (implSafeGetPagePropSet( "DateTimeFormat", xPropSet, xPropSetInfo ) >>= nTmp)
                mVisiblePagePropSet.nDateTimeFormat = static_cast<SvxDateFormat>(nTmp);

            if( mVisiblePagePropSet.bIsPageNumberFieldVisible )
            {
                SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( rxPage );
                if( pSvxDrawPage )
                {
                    SdrPage* pSdrPage = pSvxDrawPage->GetSdrPage();
                    SdrModel& rSdrModel(pSdrPage->getSdrModelFromSdrPage());
                    mVisiblePagePropSet.nPageNumberingType = rSdrModel.GetPageNumType();
                }
            }
        }
    }
}


bool SVGFilter::implExportMasterPages( const std::vector< Reference< css::drawing::XDrawPage > > & rxPages,
                                           sal_Int32 nFirstPage, sal_Int32 nLastPage )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportMasterPages: nFirstPage > nLastPage" );

    // When the exported slides are more than one we wrap master page elements
    // with a svg <defs> element.
    OUString aContainerTag = (!mbPresentation) ? OUString( "g" ) : OUString( "defs" );
    SvXMLElementExport aContainerElement( *mpSVGExport, XML_NAMESPACE_NONE, aContainerTag, true, true );

    // dummy slide - used as leaving slide for transition on the first slide
    if( mbPresentation )
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "dummy-master-page" );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrName, "dummy-master-page" );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Master_Slide" );
        SvXMLElementExport aMasterSlideElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
        {
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "bg-dummy-master-page" );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Background" );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );
            SvXMLElementExport aBackgroundElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
        }
        {
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "bo-dummy-master-page" );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "BackgroundObjects" );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );
            SvXMLElementExport aBackgroundObjectElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
        }
    }

    bool bRet = false;
    for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
    {
        if( rxPages[i].is() )
        {
            Reference< css::drawing::XShapes > xShapes( rxPages[i], UNO_QUERY );

            if( xShapes.is() )
            {
                // add id attribute
                const OUString & sPageId = implGetValidIDFromInterface( rxPages[i] );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sPageId );

                bRet = implExportPage( sPageId, rxPages[i], xShapes, true /* is a master page */ ) || bRet;
            }
        }
    }
    return bRet;
}


void SVGFilter::implExportDrawPages( const std::vector< Reference< css::drawing::XDrawPage > > & rxPages,
                                           sal_Int32 nFirstPage, sal_Int32 nLastPage )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportDrawPages: nFirstPage > nLastPage" );

    // dummy slide - used as leaving slide for transition on the first slide
    if( mbPresentation && !mbExportShapeSelection)
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "DummySlide" );
        SvXMLElementExport aDummySlideElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
        {
            SvXMLElementExport aGElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", "dummy-slide" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Slide" );
                OUString sClipPathAttrValue = "url(#" + msClipPathId + ")";
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clip-path", sClipPathAttrValue );
                SvXMLElementExport aSlideElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrName, "dummy-page" );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Page" );
                    SvXMLElementExport aPageElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );
                }
            }
        }
    }

    if(!mbExportShapeSelection)
    {
        // We wrap all slide in a group element with class name "SlideGroup".
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "SlideGroup" );
        SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

        for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
        {
            Reference< css::drawing::XShapes > xShapes;

            if (mbExportShapeSelection)
            {
                // #i124608# export a given object selection
                xShapes = maShapeSelection;
            }
            else
            {
                xShapes.set( rxPages[i], UNO_QUERY );
            }

            if( xShapes.is() )
            {
                // Insert the <g> open tag related to the svg element for
                // handling a slide visibility.
                // In case the exported slides are more than one the initial
                // visibility of each slide is set to 'hidden'.
                if( mbPresentation )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );
                }
                SvXMLElementExport aGElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );


                {
                    // Insert a further inner the <g> open tag for handling elements
                    // inserted before or after a slide: that is used for some
                    // when switching from the last to the first slide.
                    const OUString & sPageId = implGetValidIDFromInterface( rxPages[i] );
                    OUString sContainerId = "container-";
                    sContainerId += sPageId;
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sContainerId );
                    SvXMLElementExport aContainerExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                    {
                        // add id attribute
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sPageId );

                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Slide" );

                        // Adding a clip path to each exported slide , so in case
                        // bitmaps or other elements exceed the slide margins, they are
                        // trimmed, even when they are shown inside a thumbnail view.
                        OUString sClipPathAttrValue = "url(#" + msClipPathId + ")";
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clip-path", sClipPathAttrValue );

                        SvXMLElementExport aSlideElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                        implExportPage( sPageId, rxPages[i], xShapes, false /* is not a master page */ );
                    }
                } // append the </g> closing tag related to inserted elements
            } // append the </g> closing tag related to the svg element handling the slide visibility
        }
    }
    else
    {
        assert(maShapeSelection.is());
        assert(rxPages.size() == 1);

        const OUString & sPageId = implGetValidIDFromInterface( rxPages[0] );
        implExportPage( sPageId, rxPages[0], maShapeSelection, false /* is not a master page */ );
     }
}


bool SVGFilter::implExportPage( const OUString & sPageId,
                                    const Reference< css::drawing::XDrawPage > & rxPage,
                                    const Reference< css::drawing::XShapes > & xShapes,
                                    bool bMaster )
{
    bool bRet = false;

    {
        OUString sPageName = implGetInterfaceName( rxPage );
        if( mbPresentation && !sPageName.isEmpty() )
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrName, sPageName );

        {
            Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

            if( xExtDocHandler.is() )
            {
                OUString aDesc;

                if( bMaster )
                    aDesc = "Master_Slide";
                else
                    aDesc = "Page";

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aDesc );
            }
        }

        // insert the <g> open tag related to the DrawPage/MasterPage
        SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

        // In case the page has a background object we append it .
        if( mpObjects->find( rxPage ) != mpObjects->end() )
        {
            const GDIMetaFile& rMtf = (*mpObjects)[ rxPage ].GetRepresentation();
            if( rMtf.GetActionSize() )
            {
                // background id = "bg-" + page id
                OUString sBackgroundId = "bg-";
                sBackgroundId += sPageId;
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sBackgroundId );

                // At present (LibreOffice 3.4.0) the 'IsBackgroundVisible' property is not handled
                // by Impress; anyway we handle this property as referring only to the visibility
                // of the master page background. So if a slide has its own background object,
                // the visibility of such a background object is always inherited from the visibility
                // of the parent slide regardless of the value of the 'IsBackgroundVisible' property.
                // This means that we need to set up the visibility attribute only for the background
                // element of a master page.
                if( !mbPresentation && bMaster )
                {
                    if( !mVisiblePagePropSet.bIsBackgroundVisible )
                    {
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );
                    }
                }

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class",  "Background" );

                // insert the <g> open tag related to the Background
                SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                // append all elements that make up the Background
                const Point aNullPt;
                mpSVGWriter->WriteMetaFile( aNullPt, rMtf.GetPrefSize(), rMtf, SVGWRITER_WRITE_FILL );
            }   // insert the </g> closing tag related to the Background
        }

        // In case we are dealing with a master page we need to group all its shapes
        // into a group element, this group will make up the so named "background objects"
        if( bMaster )
        {
            // background objects id = "bo-" + page id
            OUString sBackgroundObjectsId = "bo-";
            sBackgroundObjectsId += sPageId;
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sBackgroundObjectsId );
            if( !mbPresentation )
            {
                if( !mVisiblePagePropSet.bAreBackgroundObjectsVisible )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );
                }
            }
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class",  "BackgroundObjects" );

            // insert the <g> open tag related to the Background Objects
            SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

            // append all shapes that make up the Master Slide
            bRet = implExportShapes( xShapes, true ) || bRet;
        }   // append the </g> closing tag related to the Background Objects
        else
        {
            // append all shapes that make up the Slide
            bRet = implExportShapes( xShapes, false ) || bRet;
        }
    }  // append the </g> closing tag related to the Slide/Master_Slide

    return bRet;
}


bool SVGFilter::implExportShapes( const Reference< css::drawing::XShapes >& rxShapes, bool bMaster )
{
    Reference< css::drawing::XShape > xShape;
    bool bRet = false;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implExportShape( xShape, bMaster ) || bRet;

        xShape = nullptr;
    }

    return bRet;
}


bool SVGFilter::implExportShape( const Reference< css::drawing::XShape >& rxShape, bool bMaster )
{
    Reference< XPropertySet > xShapePropSet( rxShape, UNO_QUERY );
    bool bRet = false;

    if( xShapePropSet.is() )
    {
        const OUString   aShapeType( rxShape->getShapeType() );
        bool                    bHideObj = false;

        if( mbPresentation )
        {
            xShapePropSet->getPropertyValue( "IsEmptyPresentationObject" )  >>= bHideObj;
        }

        OUString aShapeClass = implGetClassFromShape( rxShape );
        if( bMaster )
        {
            if( aShapeClass == "TitleText" || aShapeClass == "Outline" )
                bHideObj = true;
        }

        if( !bHideObj )
        {
            if( aShapeType.lastIndexOf( "drawing.GroupShape" ) != -1 )
            {
                Reference< css::drawing::XShapes > xShapes( rxShape, UNO_QUERY );

                if( xShapes.is() )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "Group" );
                    const OUString& rShapeId = implGetValidIDFromInterface( Reference<XInterface>(rxShape, UNO_QUERY) );
                    if( !rShapeId.isEmpty() )
                    {
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", rShapeId );
                    }
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                    bRet = implExportShapes( xShapes, bMaster );
                }
            }

            if( !bRet && mpObjects->find( rxShape ) !=  mpObjects->end() )
            {
                css::awt::Rectangle    aBoundRect;
                const GDIMetaFile&     rMtf = (*mpObjects)[ rxShape ].GetRepresentation();

                xShapePropSet->getPropertyValue( "BoundRect" ) >>= aBoundRect;

                const Point aTopLeft( aBoundRect.X, aBoundRect.Y );
                const Size  aSize( aBoundRect.Width, aBoundRect.Height );

                if( rMtf.GetActionSize() )
                {   // for text field shapes we set up text-adjust attributes
                    // and set visibility to hidden
                    const OUString* pElementId = nullptr;
                    if( mbPresentation )
                    {
                        bool bIsPageNumber  = ( aShapeClass == "Slide_Number" );
                        bool bIsFooter      = ( aShapeClass == "Footer" );
                        bool bIsDateTime    = ( aShapeClass == "Date/Time" );
                        if( bIsPageNumber || bIsDateTime || bIsFooter )
                        {
                            // to notify to the SVGActionWriter::ImplWriteActions method
                            // that we are dealing with a placeholder shape
                            pElementId = &sPlaceholderTag;

                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", "hidden" );

                            sal_uInt16 nTextAdjust = sal_uInt16(ParagraphAdjust_LEFT);
                            OUString sTextAdjust;
                            xShapePropSet->getPropertyValue( "ParaAdjust" ) >>= nTextAdjust;

                            switch( static_cast<ParagraphAdjust>(nTextAdjust) )
                            {
                                case ParagraphAdjust_LEFT:
                                        sTextAdjust = "left";
                                        break;
                                case ParagraphAdjust_CENTER:
                                        sTextAdjust = "center";
                                        break;
                                case ParagraphAdjust_RIGHT:
                                        sTextAdjust = "right";
                                        break;
                                default:
                                    break;
                            }
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, NSPREFIX "text-adjust", sTextAdjust );
                        }
                    }
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aShapeClass );
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                    Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                    OUString aTitle;
                    xShapePropSet->getPropertyValue( "Title" ) >>= aTitle;
                    if( !aTitle.isEmpty() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "title", true, true );
                        xExtDocHandler->characters( aTitle );
                    }

                    OUString aDescription;
                    xShapePropSet->getPropertyValue( "Description" ) >>= aDescription;
                    if( !aDescription.isEmpty() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", true, true );
                        xExtDocHandler->characters( aDescription );
                    }


                    const OUString& rShapeId = implGetValidIDFromInterface( Reference<XInterface>(rxShape, UNO_QUERY) );
                    if( !rShapeId.isEmpty() )
                    {
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", rShapeId );
                    }

                    const GDIMetaFile* pEmbeddedBitmapsMtf = nullptr;
                    if( mEmbeddedBitmapActionMap.find( rxShape ) !=  mEmbeddedBitmapActionMap.end() )
                    {
                        pEmbeddedBitmapsMtf = &( mEmbeddedBitmapActionMap[ rxShape ].GetRepresentation() );
                    }

                    {
                        OUString aBookmark;
                        Reference<XPropertySetInfo> xShapePropSetInfo = xShapePropSet->getPropertySetInfo();
                        if(xShapePropSetInfo->hasPropertyByName("Bookmark"))
                        {
                            xShapePropSet->getPropertyValue( "Bookmark" ) >>= aBookmark;
                        }

                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", true, true );

                        // export the shape bounding box
                        {
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", "BoundingBox" );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke", "none" );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill", "none" );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "x", OUString::number( aBoundRect.X ) );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "y", OUString::number( aBoundRect.Y ) );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", OUString::number( aBoundRect.Width ) );
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", OUString::number( aBoundRect.Height ) );
                            SvXMLElementExport aBB( *mpSVGExport, XML_NAMESPACE_NONE, "rect", true, true );
                        }

                        if( !aBookmark.isEmpty() )
                        {
                            INetURLObject aINetURLObject(aBookmark);
                            if (!aINetURLObject.HasError()
                                && aINetURLObject.GetProtocol() != INetProtocol::Javascript)
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xlink:href", aBookmark);
                                SvXMLElementExport alinkA( *mpSVGExport, XML_NAMESPACE_NONE, "a", true, true );
                                mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf,
                                                            0xffffffff,
                                                            pElementId,
                                                            &rxShape,
                                                            pEmbeddedBitmapsMtf );
                            }
                        }
                        else
                        {
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf,
                                                        0xffffffff,
                                                        pElementId,
                                                        &rxShape,
                                                        pEmbeddedBitmapsMtf );
                        }
                    }
                }

                bRet = true;
            }
        }
    }

    return bRet;
}


bool SVGFilter::implCreateObjects()
{
    if (mbExportShapeSelection)
    {
        // #i124608# export a given object selection
        if (!mSelectedPages.empty() && mSelectedPages[0].is())
        {
            implCreateObjectsFromShapes(mSelectedPages[0], maShapeSelection);
            return true;
        }
        return false;
    }

    sal_Int32 i, nCount;

    for( i = 0, nCount = mMasterPageTargets.size(); i < nCount; ++i )
    {
        const Reference< css::drawing::XDrawPage > & xMasterPage = mMasterPageTargets[i];

        if( xMasterPage.is() )
        {
            mCreateOjectsCurrentMasterPage = xMasterPage;
            implCreateObjectsFromBackground( xMasterPage );

            if( xMasterPage.is() )
                implCreateObjectsFromShapes( xMasterPage, xMasterPage );
        }
    }

    for( i = 0, nCount = mSelectedPages.size(); i < nCount; ++i )
    {
        const Reference< css::drawing::XDrawPage > & xDrawPage = mSelectedPages[i];

        if( xDrawPage.is() )
        {
            // TODO complete the implementation for exporting custom background for each slide
            // implementation status:
            // - hatch stroke color is set to 'none' so the hatch is not visible, why?
            // - gradient look is not really awesome, too few colors are used;
            // - stretched bitmap, gradient and hatch are not exported only once
            //   and then referenced in case more than one slide uses them.
            // - tiled bitmap: an image element is exported for each tile,
            //   this is really too expensive!
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
            if( xPropSet.is() )
            {
                Reference< XPropertySet > xBackground;
                xPropSet->getPropertyValue( "Background" ) >>= xBackground;
                if( xBackground.is() )
                {
                    drawing::FillStyle aFillStyle;
                    bool assigned = ( xBackground->getPropertyValue( "FillStyle" ) >>= aFillStyle );
                    if( assigned && aFillStyle != drawing::FillStyle_NONE
                                 && aFillStyle != drawing::FillStyle_BITMAP )
                    {
                        implCreateObjectsFromBackground( xDrawPage );
                    }
                }
            }
            implCreateObjectsFromShapes( xDrawPage, xDrawPage );
        }
    }
    return true;
}


bool SVGFilter::implCreateObjectsFromShapes( const Reference< css::drawing::XDrawPage > & rxPage, const Reference< css::drawing::XShapes >& rxShapes )
{
    Reference< css::drawing::XShape > xShape;
    bool            bRet = false;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implCreateObjectsFromShape( rxPage, xShape ) || bRet;

        xShape = nullptr;
    }

    return bRet;
}


bool SVGFilter::implCreateObjectsFromShape( const Reference< css::drawing::XDrawPage > & rxPage, const Reference< css::drawing::XShape >& rxShape )
{
    bool bRet = false;
    if( rxShape->getShapeType().lastIndexOf( "drawing.GroupShape" ) != -1 )
    {
        Reference< css::drawing::XShapes > xShapes( rxShape, UNO_QUERY );

        if( xShapes.is() )
            bRet = implCreateObjectsFromShapes( rxPage, xShapes );
    }
    else
    {
        SdrObject*  pObj = GetSdrObjectFromXShape( rxShape );

        if( pObj )
        {
            Graphic aGraphic(SdrExchangeView::GetObjGraphic(*pObj));

            // Writer graphic shapes are handled differently
            if( mbWriterFilter && aGraphic.GetType() == GraphicType::NONE )
            {
                if (rxShape->getShapeType() == "com.sun.star.drawing.GraphicObjectShape")
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(rxShape, uno::UNO_QUERY);
                    uno::Reference<graphic::XGraphic> xGraphic;
                    xPropertySet->getPropertyValue("Graphic") >>= xGraphic;

                    if (!xGraphic.is())
                        return false;

                    aGraphic = Graphic(xGraphic);
                }
            }

            if( aGraphic.GetType() != GraphicType::NONE )
            {
                if( aGraphic.GetType() == GraphicType::Bitmap )
                {
                    GDIMetaFile    aMtf;
                    const Point    aNullPt;
                    const Size    aSize( pObj->GetCurrentBoundRect().GetSize() );

                    aMtf.AddAction( new MetaBmpExScaleAction( aNullPt, aSize, aGraphic.GetBitmapEx() ) );
                    aMtf.SetPrefSize( aSize );
                    aMtf.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aMtf );
                }
                else
                {
                    if( aGraphic.GetGDIMetaFile().GetActionSize() )
                    {
                        Reference< XText > xText( rxShape, UNO_QUERY );
                        bool bIsTextShape = xText.is();

                        if( !mpSVGExport->IsUsePositionedCharacters() && bIsTextShape )
                        {
                            Reference< XPropertySet >   xShapePropSet( rxShape, UNO_QUERY );

                            if( xShapePropSet.is() )
                            {
                                bool bHideObj = false;

                                if( mbPresentation )
                                {
                                    xShapePropSet->getPropertyValue( "IsEmptyPresentationObject" )  >>= bHideObj;
                                }

                                if( !bHideObj )
                                {
                                    // We create a map of text shape ids.
                                    implRegisterInterface( rxShape );
                                    const OUString& rShapeId = implGetValidIDFromInterface( Reference<XInterface>(rxShape, UNO_QUERY) );
                                    if( !rShapeId.isEmpty() )
                                    {
                                        mTextShapeIdListMap[rxPage] += rShapeId;
                                        mTextShapeIdListMap[rxPage] += " ";
                                    }

                                    // We create a set of bitmaps embedded into text shape.
                                    GDIMetaFile   aMtf;
                                    const Size    aSize( pObj->GetCurrentBoundRect().GetSize() );
                                    MetaAction*   pAction;
                                    bool bIsTextShapeStarted = false;
                                    const GDIMetaFile& rMtf = aGraphic.GetGDIMetaFile();
                                    sal_uLong nCount = rMtf.GetActionSize();
                                    for( sal_uLong nCurAction = 0; nCurAction < nCount; ++nCurAction )
                                    {
                                        pAction = rMtf.GetAction( nCurAction );
                                        const MetaActionType nType = pAction->GetType();

                                        if( nType == MetaActionType::COMMENT )
                                        {
                                            const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);
                                            if( pA->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTSHAPE_BEGIN") )
                                            {
                                                bIsTextShapeStarted = true;
                                            }
                                            else if( pA->GetComment().equalsIgnoreAsciiCase( "XTEXT_PAINTSHAPE_END" ) )
                                            {
                                                bIsTextShapeStarted = false;
                                            }
                                        }
                                        if( bIsTextShapeStarted && ( nType == MetaActionType::BMPSCALE  || nType == MetaActionType::BMPEXSCALE ) )
                                        {
                                            GDIMetaFile aEmbeddedBitmapMtf;
                                            aEmbeddedBitmapMtf.AddAction( pAction );
                                            aEmbeddedBitmapMtf.SetPrefSize( aSize );
                                            aEmbeddedBitmapMtf.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                                            mEmbeddedBitmapActionSet.insert( ObjectRepresentation( rxShape, aEmbeddedBitmapMtf ) );
                                            aMtf.AddAction( pAction );
                                        }
                                    }
                                    aMtf.SetPrefSize( aSize );
                                    aMtf.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                                    mEmbeddedBitmapActionMap[ rxShape ] = ObjectRepresentation( rxShape, aMtf );
                                }
                            }
                        }
                    }
                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aGraphic.GetGDIMetaFile() );
                }
                bRet = true;
            }
        }
    }

    return bRet;
}


void SVGFilter::implCreateObjectsFromBackground( const Reference< css::drawing::XDrawPage >& rxDrawPage )
{
    Reference< css::drawing::XGraphicExportFilter >  xExporter = drawing::GraphicExportFilter::create( mxContext );

    GDIMetaFile             aMtf;

    utl::TempFile aFile;
    aFile.EnableKillingFile();

    Sequence< PropertyValue > aDescriptor( 3 );
    aDescriptor[0].Name = "FilterName";
    aDescriptor[0].Value <<= OUString( "SVM" );
    aDescriptor[1].Name = "URL";
    aDescriptor[1].Value <<= aFile.GetURL();
    aDescriptor[2].Name = "ExportOnlyBackground";
    aDescriptor[2].Value <<= true;

    xExporter->setSourceDocument( Reference< XComponent >( rxDrawPage, UNO_QUERY ) );
    xExporter->filter( aDescriptor );
    aMtf.Read( *aFile.GetStream( StreamMode::READ ) );

    (*mpObjects)[ rxDrawPage ] = ObjectRepresentation( rxDrawPage, aMtf );
}


OUString SVGFilter::implGetClassFromShape( const Reference< css::drawing::XShape >& rxShape )
{
    OUString            aRet;
    const OUString      aShapeType( rxShape->getShapeType() );

    if( aShapeType.lastIndexOf( "drawing.GroupShape" ) != -1 )
        aRet = "Group";
    else if( aShapeType.lastIndexOf( "drawing.GraphicObjectShape" ) != -1 )
        aRet = "Graphic";
    else if( aShapeType.lastIndexOf( "drawing.OLE2Shape" ) != -1 )
        aRet = "OLE2";
    else if( aShapeType.lastIndexOf( "presentation.HeaderShape" ) != -1 )
        aRet = "Header";
    else if( aShapeType.lastIndexOf( "presentation.FooterShape" ) != -1 )
        aRet = "Footer";
    else if( aShapeType.lastIndexOf( "presentation.DateTimeShape" ) != -1 )
        aRet = "Date/Time";
    else if( aShapeType.lastIndexOf( "presentation.SlideNumberShape" ) != -1 )
        aRet = "Slide_Number";
    else if( aShapeType.lastIndexOf( "presentation.TitleTextShape" ) != -1 )
        aRet = "TitleText";
    else if( aShapeType.lastIndexOf( "presentation.OutlinerShape" ) != -1 )
        aRet = "Outline";
    else
        aRet = aShapeType;

    return aRet;
}


void SVGFilter::implRegisterInterface( const Reference< XInterface >& rxIf )
{
    if( rxIf.is() )
        mpSVGExport->getInterfaceToIdentifierMapper().registerReference( rxIf );
}


const OUString & SVGFilter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
   return mpSVGExport->getInterfaceToIdentifierMapper().getIdentifier( rxIf );
}


OUString SVGFilter::implGetInterfaceName( const Reference< XInterface >& rxIf )
{
    Reference< XNamed > xNamed( rxIf, UNO_QUERY );
    OUString            aRet;
    if( xNamed.is() )
    {
        aRet = xNamed->getName().replace( ' ', '_' );
    }
    return aRet;
}


IMPL_LINK( SVGFilter, CalcFieldHdl, EditFieldInfo*, pInfo, void )
{
    bool bFieldProcessed = false;
    if( pInfo && mbPresentation )
    {
        bFieldProcessed = true;
        if( mpSVGExport->IsEmbedFonts() && mpSVGExport->IsUsePositionedCharacters() )
        {
            // to notify to the SVGActionWriter::ImplWriteText method
            // that we are dealing with a placeholder shape
            OUStringBuffer aRepresentation = sPlaceholderTag;

            if( !mCreateOjectsCurrentMasterPage.is() )
            {
                OSL_FAIL( "error: !mCreateOjectsCurrentMasterPage.is()" );
                return;
            }
            bool bHasCharSetMap = mTextFieldCharSets.find( mCreateOjectsCurrentMasterPage ) != mTextFieldCharSets.end();

            static const OUString aHeaderId( NSPREFIX "header-field" );
            static const OUString aFooterId( aOOOAttrFooterField );
            static const OUString aDateTimeId( aOOOAttrDateTimeField );
            static const OUString aVariableDateTimeId( aOOOAttrDateTimeField + "-variable" );

            const UCharSet * pCharSet = nullptr;
            UCharSetMap * pCharSetMap = nullptr;
            if( bHasCharSetMap )
            {
                pCharSetMap = &( mTextFieldCharSets[ mCreateOjectsCurrentMasterPage ] );
            }
            const SvxFieldData* pField = pInfo->GetField().GetField();
            if( bHasCharSetMap && ( pField->GetClassId() == text::textfield::Type::PRESENTATION_HEADER ) && ( pCharSetMap->find( aHeaderId ) != pCharSetMap->end() ) )
            {
                pCharSet = &( (*pCharSetMap)[ aHeaderId ] );
            }
            else if( bHasCharSetMap && ( pField->GetClassId() == text::textfield::Type::PRESENTATION_FOOTER ) && ( pCharSetMap->find( aFooterId ) != pCharSetMap->end() ) )
            {
                pCharSet = &( (*pCharSetMap)[ aFooterId ] );
            }
            else if( pField->GetClassId() == text::textfield::Type::PRESENTATION_DATE_TIME )
            {
                if( bHasCharSetMap && ( pCharSetMap->find( aDateTimeId ) != pCharSetMap->end() ) )
                {
                    pCharSet = &( (*pCharSetMap)[ aDateTimeId ] );
                }
                if( bHasCharSetMap && ( pCharSetMap->find( aVariableDateTimeId ) != pCharSetMap->end() ) && !(*pCharSetMap)[ aVariableDateTimeId ].empty() )
                {
                    SvxDateFormat eDateFormat = SvxDateFormat::B, eCurDateFormat;
                    const UCharSet & aCharSet = (*pCharSetMap)[ aVariableDateTimeId ];
                    // we look for the most verbose date format
                    for (auto const& elem : aCharSet)
                    {
                        eCurDateFormat = static_cast<SvxDateFormat>( static_cast<int>(elem) & 0x0f );
                        switch( eDateFormat )
                        {
                            case SvxDateFormat::StdSmall:
                            case SvxDateFormat::A:       // 13.02.96
                            case SvxDateFormat::B:       // 13.02.1996
                                switch( eCurDateFormat )
                                {
                                    case SvxDateFormat::C:       // 13.Feb 1996
                                    case SvxDateFormat::D:       // 13.February 1996
                                    case SvxDateFormat::E:       // Tue, 13.February 1996
                                    case SvxDateFormat::StdBig:
                                    case SvxDateFormat::F:       // Tuesday, 13.February 1996
                                        eDateFormat = eCurDateFormat;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            case SvxDateFormat::C:       // 13.Feb 1996
                            case SvxDateFormat::D:       // 13.February 1996
                                switch( eCurDateFormat )
                                {
                                    case SvxDateFormat::E:       // Tue, 13.February 1996
                                    case SvxDateFormat::StdBig:
                                    case SvxDateFormat::F:       // Tuesday, 13.February 1996
                                        eDateFormat = eCurDateFormat;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    // Independently of the date format, we always put all these characters by default.
                    // They should be enough to cover every time format.
                    aRepresentation.append( "0123456789.:/-APM" );

                    if( eDateFormat != SvxDateFormat::AppDefault )
                    {
                        OUStringBuffer sDate;
                        LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
                        SvNumberFormatter * pNumberFormatter = new SvNumberFormatter( ::comphelper::getProcessComponentContext(), LANGUAGE_SYSTEM );
                        // We always collect the characters obtained by using the SvxDateFormat::B (as: 13.02.1996)
                        // so we are sure to include any unusual day|month|year separator.
                        Date aDate( 1, 1, 1996 );
                        sDate.append( SvxDateField::GetFormatted( aDate, SvxDateFormat::B, *pNumberFormatter, eLang ) );
                        switch( eDateFormat )
                        {
                            case SvxDateFormat::E:       // Tue, 13.February 1996
                            case SvxDateFormat::StdBig:
                            case SvxDateFormat::F:       // Tuesday, 13.February 1996
                                for( sal_uInt16 i = 1; i <= 7; ++i )  // we get all days in a week
                                {
                                    aDate.SetDay( i );
                                    sDate.append( SvxDateField::GetFormatted( aDate, eDateFormat, *pNumberFormatter, eLang ) );
                                }
                                [[fallthrough]]; // We need months too!
                            case SvxDateFormat::C:       // 13.Feb 1996
                            case SvxDateFormat::D:       // 13.February 1996
                                for( sal_uInt16 i = 1; i <= 12; ++i ) // we get all months in a year
                                {
                                    aDate.SetMonth( i );
                                    sDate.append( SvxDateField::GetFormatted( aDate, eDateFormat, *pNumberFormatter, eLang ) );
                                }
                                break;
                            // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                            case SvxDateFormat::StdSmall:
                            case SvxDateFormat::A:       // 13.02.96
                            case SvxDateFormat::B:       // 13.02.1996
                            default:
                                // nothing to do here, we always collect the characters needed for these cases.
                                break;
                        }
                        aRepresentation.append( sDate.makeStringAndClear() );
                    }
                }
            }
            else if( pField->GetClassId() == text::textfield::Type::PAGE )
            {
                switch( mVisiblePagePropSet.nPageNumberingType )
                {
                    case css::style::NumberingType::CHARS_UPPER_LETTER:
                        aRepresentation.append( "QWERTYUIOPASDFGHJKLZXCVBNM" );
                        break;
                    case css::style::NumberingType::CHARS_LOWER_LETTER:
                        aRepresentation.append( "qwertyuiopasdfghjklzxcvbnm" );
                        break;
                    case css::style::NumberingType::ROMAN_UPPER:
                        aRepresentation.append( "IVXLCDM" );
                        break;
                    case css::style::NumberingType::ROMAN_LOWER:
                        aRepresentation.append( "ivxlcdm" );
                        break;
                    // arabic numbering type is the default
                    case css::style::NumberingType::ARABIC:
                    // in case the numbering type is not handled we fall back on arabic numbering
                    default:
                        aRepresentation.append( "0123456789" );
                        break;
                }
            }
            else
            {
                bFieldProcessed = false;
            }
            if( bFieldProcessed )
            {
                if( pCharSet != nullptr )
                {
                    for (auto const& elem : *pCharSet)
                    {
                        aRepresentation.append(elem);
                    }
                }
                pInfo->SetRepresentation( aRepresentation.makeStringAndClear() );
            }
        }
        else
        {
            bFieldProcessed = false;
        }

    }
    if (!bFieldProcessed)
        maOldFieldHdl.Call( pInfo );
}


void SVGExport::writeMtf( const GDIMetaFile& rMtf )
{
    const Size aSize( OutputDevice::LogicToLogic(rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MapMode(MapUnit::MapMM)) );
    OUString aAttr;
    Reference< XExtendedDocumentHandler> xExtDocHandler( GetDocHandler(), UNO_QUERY );

    if( xExtDocHandler.is() && IsUseDTDString() )
        xExtDocHandler->unknown( SVG_DTD_STRING );

    aAttr = OUString::number( aSize.Width() );
    aAttr += "mm";
    AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

    aAttr = OUString::number( aSize.Height() );
    aAttr += "mm";
    AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );

    aAttr = "0 0 ";
    aAttr += OUString::number( aSize.Width() * 100L );
    aAttr += " ";
    aAttr += OUString::number( aSize.Height() * 100L );
    AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );

    AddAttribute( XML_NAMESPACE_NONE, "version", "1.1" );

    if( IsUseTinyProfile() )
         AddAttribute( XML_NAMESPACE_NONE, "baseProfile", "tiny" );

    AddAttribute( XML_NAMESPACE_NONE, "xmlns", constSvgNamespace );
    // For <image xlink:href="...">.
    AddAttribute(XML_NAMESPACE_XMLNS, "xlink", "http://www.w3.org/1999/xlink");
    AddAttribute( XML_NAMESPACE_NONE, "stroke-width", OUString::number( 28.222 ) );
    AddAttribute( XML_NAMESPACE_NONE, "stroke-linejoin", "round" );
    AddAttribute( XML_NAMESPACE_NONE, "xml:space", "preserve" );

    {
        SvXMLElementExport  aSVG( *this, XML_NAMESPACE_NONE, "svg", true, true );

        std::vector< ObjectRepresentation > aObjects;

        aObjects.emplace_back( Reference< XInterface >(), rMtf );
        SVGFontExport aSVGFontExport( *this, aObjects );

        Point aPoint100thmm( OutputDevice::LogicToLogic(rMtf.GetPrefMapMode().GetOrigin(), rMtf.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)) );
        Size  aSize100thmm( OutputDevice::LogicToLogic(rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)) );

        SVGActionWriter aWriter( *this, aSVGFontExport );
        aWriter.WriteMetaFile( aPoint100thmm, aSize100thmm, rMtf,
            SVGWRITER_WRITE_FILL | SVGWRITER_WRITE_TEXT );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
