/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include "svgwriter.hxx"
#include "svgfontexport.hxx"
#include "svgfilter.hxx"
#include "svgscript.hxx"
#include "impsvgdialog.hxx"

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

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
#include <i18npool/lang.h>
#include <svl/zforlist.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/animationexport.hxx>


#include <boost/preprocessor/repetition/repeat.hpp>


using ::rtl::OUString;
using namespace ::com::sun::star;




// -------------------------------
// - ooo elements and attributes -
// -------------------------------

#define NSPREFIX "ooo:"

// ooo xml elements
static const char    aOOOElemMetaSlides[] = NSPREFIX "meta_slides";
static const char    aOOOElemMetaSlide[] = NSPREFIX "meta_slide";
static const char    aOOOElemTextField[] = NSPREFIX "text_field";

// ooo xml attributes for meta_slides
static const char    aOOOAttrNumberOfSlides[] = NSPREFIX "number-of-slides";
static const char    aOOOAttrStartSlideNumber[] = NSPREFIX "start-slide-number";
static const char    aOOOAttrNumberingType[] = NSPREFIX "page-numbering-type";

// ooo xml attributes for meta_slide
static const char    aOOOAttrSlide[] = NSPREFIX "slide";
static const char    aOOOAttrMaster[] = NSPREFIX "master";
static const char    aOOOAttrBackgroundVisibility[] = NSPREFIX "background-visibility";
static const char    aOOOAttrMasterObjectsVisibility[] = NSPREFIX "master-objects-visibility";
static const char    aOOOAttrPageNumberVisibility[] = NSPREFIX "page-number-visibility";
static const char    aOOOAttrDateTimeVisibility[] = NSPREFIX "date-time-visibility";
static const char    aOOOAttrFooterVisibility[] = NSPREFIX "footer-visibility";
static const char    aOOOAttrDateTimeField[] = NSPREFIX "date-time-field";
static const char    aOOOAttrFooterField[] = NSPREFIX "footer-field";
static const char    aOOOAttrHeaderField[] = NSPREFIX "header-field";
static const char    aOOOAttrHasTransition[] = NSPREFIX "has-transition";
static const char    aOOOAttrIdList[] = NSPREFIX "id-list";

// ooo xml attributes for pages and shapes
static const char    aOOOAttrName[] = NSPREFIX "name";

// ooo xml attributes for date_time_field
static const char    aOOOAttrDateTimeFormat[] = NSPREFIX "date-time-format";

// ooo xml attributes for Placeholder Shapes
static const char    aOOOAttrTextAdjust[] = NSPREFIX "text-adjust";



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * - Text Field Class Hierarchy -                                  *
 *                                                                 *
 *  This is a set of classes for exporting text field meta info.   *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define TEXT_FIELD_GET_CLASS_NAME_METHOD( class_name ) \
virtual ::rtl::OUString getClassName() const                            \
{                                                                       \
    static const char className[] = #class_name;                        \
    return B2UCONST( className );                                       \
}


class TextField
{
protected:
    SVGFilter::ObjectSet mMasterPageSet;
public:

    TEXT_FIELD_GET_CLASS_NAME_METHOD( TextField )
    virtual sal_Bool equalTo( const TextField & aTextField ) const = 0;
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const = 0;
    virtual void elementExport( SVGExport* pSVGExport ) const
    {
        pSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", getClassName() );
    }
    void insertMasterPage( Reference< XDrawPage> xMasterPage )
    {
        mMasterPageSet.insert( xMasterPage );
    }
    virtual ~TextField() {}
protected:
    void implGrowCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets, ::rtl::OUString sText, ::rtl::OUString sTextFieldId ) const
    {
        const sal_Unicode * ustr = sText.getStr();
        sal_Int32 nLength = sText.getLength();
        SVGFilter::ObjectSet::const_iterator aMasterPageIt = mMasterPageSet.begin();
        for( ; aMasterPageIt != mMasterPageSet.end(); ++aMasterPageIt )
        {
            const Reference< XInterface > & xMasterPage = *aMasterPageIt;
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
    ::rtl::OUString text;

    TEXT_FIELD_GET_CLASS_NAME_METHOD( FixedTextField )
    virtual sal_Bool equalTo( const TextField & aTextField ) const
    {
        if( const FixedTextField* aFixedTextField = dynamic_cast< const FixedTextField* >( &aTextField ) )
        {
            return ( text == aFixedTextField->text );
        }
        return false;
    }
    virtual void elementExport( SVGExport* pSVGExport ) const
    {
        TextField::elementExport( pSVGExport );
        SvXMLElementExport aExp( *pSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
        pSVGExport->GetDocHandler()->characters( text );
    }
    virtual ~FixedTextField() {}
};

class FixedDateTimeField : public FixedTextField
{
public:
    FixedDateTimeField() {}
    TEXT_FIELD_GET_CLASS_NAME_METHOD( FixedDateTimeField )
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const
    {
        static const ::rtl::OUString sFieldId = B2UCONST( aOOOAttrDateTimeField );
        implGrowCharSet( aTextFieldCharSets, text, sFieldId );
    }
    virtual ~FixedDateTimeField() {}
};

class FooterField : public FixedTextField
{
public:
    FooterField() {}
    TEXT_FIELD_GET_CLASS_NAME_METHOD( FooterField )
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const
    {
        static const ::rtl::OUString sFieldId = B2UCONST( aOOOAttrFooterField );
        implGrowCharSet( aTextFieldCharSets, text, sFieldId );
    }
    virtual ~FooterField() {}
};

class HeaderField : public FixedTextField
{
public:
    HeaderField() {}
    TEXT_FIELD_GET_CLASS_NAME_METHOD( HeaderField )
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const
    {
        static const ::rtl::OUString sFieldId = B2UCONST( aOOOAttrHeaderField );
        implGrowCharSet( aTextFieldCharSets, text, sFieldId );
    }
    virtual ~HeaderField() {}
};

class VariableTextField : public TextField
{
public:
    TEXT_FIELD_GET_CLASS_NAME_METHOD( VariableTextField )
    virtual ~VariableTextField() {}
};

class VariableDateTimeField : public VariableTextField
{
public:
    sal_Int32 format;

    VariableDateTimeField() {}
    TEXT_FIELD_GET_CLASS_NAME_METHOD( VariableDateTimeField )
    virtual sal_Bool equalTo( const TextField & aTextField ) const
    {
        if( const VariableDateTimeField* aField = dynamic_cast< const VariableDateTimeField* >( &aTextField ) )
        {
            return ( format == aField->format );
        }
        return false;
    }
    virtual void elementExport( SVGExport* pSVGExport ) const
    {
        VariableTextField::elementExport( pSVGExport );
        OUString sDateFormat, sTimeFormat;
        SvxDateFormat eDateFormat = (SvxDateFormat)( format & 0x0f );
        if( eDateFormat )
        {
            switch( eDateFormat )
            {
                case SVXDATEFORMAT_STDSMALL: ;
                case SVXDATEFORMAT_A:       // 13.02.96
                    sDateFormat = B2UCONST( "" );
                    break;
                case SVXDATEFORMAT_C:       // 13.Feb 1996
                    sDateFormat = B2UCONST( "" );
                    break;
                case SVXDATEFORMAT_D:       // 13.February 1996
                    sDateFormat = B2UCONST( "" );
                    break;
                case SVXDATEFORMAT_E:       // Tue, 13.February 1996
                    sDateFormat = B2UCONST( "" );
                    break;
                case SVXDATEFORMAT_STDBIG: ;
                case SVXDATEFORMAT_F:       // Tuesday, 13.February 1996
                    sDateFormat = B2UCONST( "" );
                    break;
                // default case
                case SVXDATEFORMAT_B: ;    // 13.02.1996
                default:
                    sDateFormat = B2UCONST( "" );
                    break;
            }
        }

        SvxTimeFormat eTimeFormat = (SvxTimeFormat)( ( format >> 4 ) & 0x0f );
        if( eTimeFormat )
        {
            switch( eTimeFormat )
            {
                case SVXTIMEFORMAT_24_HMS:      // 13:49:38
                    sTimeFormat = B2UCONST( "" );
                    break;
                case SVXTIMEFORMAT_AM_HM: ;     // 01:49 PM
                case SVXTIMEFORMAT_12_HM:
                    sTimeFormat = B2UCONST( "" );
                    break;
                case SVXTIMEFORMAT_AM_HMS: ;    // 01:49:38 PM
                case SVXTIMEFORMAT_12_HMS:
                    sTimeFormat = B2UCONST( "" );
                    break;
                // default case
                case SVXTIMEFORMAT_24_HM: ;      // 13:49
                default:
                    sTimeFormat = B2UCONST( "" );
                    break;
            }
        }

        OUString sDateTimeFormat = sDateFormat;
        sDateTimeFormat += OUString::valueOf( sal_Unicode(' ') );
        sDateTimeFormat += sTimeFormat;

        pSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeFormat, sDateTimeFormat );
        SvXMLElementExport aExp( *pSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
    }
    virtual void growCharSet( SVGFilter::UCharSetMapMap & aTextFieldCharSets ) const
    {
        // we use the unicode char set in an improper way: we put in the date/time fortat
        // in order to pass it to the CalcFieldValue method
        static const ::rtl::OUString sFieldId = B2UCONST( aOOOAttrDateTimeField ) + B2UCONST( "-variable" );
        SVGFilter::ObjectSet::const_iterator aMasterPageIt = mMasterPageSet.begin();
        for( ; aMasterPageIt != mMasterPageSet.end(); ++aMasterPageIt )
        {
            aTextFieldCharSets[ *aMasterPageIt ][ sFieldId ].insert( (sal_Unicode)( format ) );
        }
    }
    virtual ~VariableDateTimeField() {}
};

sal_Bool operator==( const TextField & aLhsTextField, const TextField & aRhsTextField )
{
    return aLhsTextField.equalTo( aRhsTextField );
}



// -------------
// - SVGExport -
// -------------

SVGExport::SVGExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const Reference< XDocumentHandler >& rxHandler,
    const Sequence< PropertyValue >& rFilterData )
    : SvXMLExport( util::MeasureUnit::MM_100TH,
                   xServiceFactory,
                   xmloff::token::XML_TOKEN_INVALID,
                   EXPORT_META|EXPORT_PRETTY )
        , mrFilterData( rFilterData )
{
    SetDocHandler( rxHandler );
    GetDocHandler()->startDocument();
}

// -----------------------------------------------------------------------------

SVGExport::~SVGExport()
{
    GetDocHandler()->endDocument();
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsUseTinyProfile() const
{
    sal_Bool bRet = sal_False;

    if( IsUsePositionedCharacters() && mrFilterData.getLength() > 0 )
        mrFilterData[ 0 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsEmbedFonts() const
{
    sal_Bool bRet = sal_False;

    if( IsUsePositionedCharacters() && mrFilterData.getLength() > 1 )
        mrFilterData[ 1 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsUseNativeTextDecoration() const
{
    sal_Bool bRet = !IsUseTinyProfile();

    if( bRet && ( mrFilterData.getLength() > 2 ) )
        mrFilterData[ 2 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsUseOpacity() const
{
    sal_Bool bRet = !IsUseTinyProfile();

    if( !bRet && ( mrFilterData.getLength() > 4 ) )
        mrFilterData[ 4 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsUsePositionedCharacters() const
{
    sal_Bool bRet = sal_False;
    if( mrFilterData.getLength() > 6 )
        mrFilterData[ 6 ].Value >>= bRet;

    return bRet;
}


// ------------------------
// - ObjectRepresentation -
// ------------------------

ObjectRepresentation::ObjectRepresentation() :
    mpMtf( NULL )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::ObjectRepresentation( const Reference< XInterface >& rxObject,
                                            const GDIMetaFile& rMtf ) :
    mxObject( rxObject ),
    mpMtf( new GDIMetaFile( rMtf ) )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::ObjectRepresentation( const ObjectRepresentation& rPresentation ) :
    mxObject( rPresentation.mxObject ),
    mpMtf( rPresentation.mpMtf ? new GDIMetaFile( *rPresentation.mpMtf ) : NULL )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::~ObjectRepresentation()
{
    delete mpMtf;
}

// -----------------------------------------------------------------------------

ObjectRepresentation& ObjectRepresentation::operator=( const ObjectRepresentation& rPresentation )
{
    // Check for self-assignment
    if (this == &rPresentation)
        return *this;
    mxObject = rPresentation.mxObject;
    delete mpMtf, ( mpMtf = rPresentation.mpMtf ? new GDIMetaFile( *rPresentation.mpMtf ) : NULL );

    return *this;
}

// -----------------------------------------------------------------------------

sal_Bool ObjectRepresentation::operator==( const ObjectRepresentation& rPresentation ) const
{
    return( ( mxObject == rPresentation.mxObject ) &&
            ( *mpMtf == *rPresentation.mpMtf ) );
}

// -----------------------------------------------------------------------------

sal_uLong GetBitmapChecksum( const MetaAction* pAction )
{
    sal_uLong nChecksum = 0;
    const sal_uInt16 nType = pAction->GetType();

    switch( nType )
    {
        case( META_BMPSCALE_ACTION ):
        {
            const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
            if( pA  )
                nChecksum = pA->GetBitmap().GetChecksum();
            else
                OSL_FAIL( "GetBitmapChecksum: MetaBmpScaleAction pointer is null." );
        }
        break;
        case( META_BMPEXSCALE_ACTION ):
        {
            const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
            if( pA )
                nChecksum = pA->GetBitmapEx().GetChecksum();
            else
                OSL_FAIL( "GetBitmapChecksum: MetaBmpExScaleAction pointer is null." );
        }
        break;
    }
    return nChecksum;
}
// -----------------------------------------------------------------------------

void MetaBitmapActionGetPoint( const MetaAction* pAction, Point& rPt )
{
    const sal_uInt16 nType = pAction->GetType();
    switch( nType )
    {
        case( META_BMPSCALE_ACTION ):
        {
            const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
            if( pA  )
                rPt = pA->GetPoint();
            else
                OSL_FAIL( "MetaBitmapActionGetPoint: MetaBmpScaleAction pointer is null." );
        }
        break;
        case( META_BMPEXSCALE_ACTION ):
        {
            const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
            if( pA )
                rPt = pA->GetPoint();
            else
                OSL_FAIL( "MetaBitmapActionGetPoint: MetaBmpExScaleAction pointer is null." );
        }
        break;
    }

}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

bool EqualityBitmap::operator()( const ObjectRepresentation& rObjRep1,
                                     const ObjectRepresentation& rObjRep2 ) const
{
    const GDIMetaFile& aMtf1 = rObjRep1.GetRepresentation();
    const GDIMetaFile& aMtf2 = rObjRep2.GetRepresentation();
    if( aMtf1.GetActionSize() == 1 && aMtf2.GetActionSize() == 1 )
    {
        sal_uLong nChecksum1 = GetBitmapChecksum( aMtf1.GetAction( 0 ) );
        sal_uLong nChecksum2 = GetBitmapChecksum( aMtf2.GetAction( 0 ) );
        return ( nChecksum1 == nChecksum2 );
    }
    else
    {
        OSL_FAIL( "EqualityBitmap: metafile should have a single action." );
        return false;
    }
}


// -------------
// - SVGFilter -
// -------------

sal_Bool SVGFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Reference< XMultiServiceFactory >    xServiceFactory( ::comphelper::getProcessServiceFactory() ) ;
    Reference< XOutputStream >          xOStm;
    SvStream*                           pOStm = NULL;
    sal_Int32                            nLength = rDescriptor.getLength();
    const PropertyValue*                pValue = rDescriptor.getConstArray();
    sal_Bool                            bRet = sal_False;

    maFilterData.realloc( 0 );

    for ( sal_Int32 i = 0 ; i < nLength; ++i)
    {
        if ( pValue[ i ].Name == "OutputStream" )
            pValue[ i ].Value >>= xOStm;
        else if ( pValue[ i ].Name == "FileName" )
        {
            ::rtl::OUString aFileName;

            pValue[ i ].Value >>= aFileName;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC );

            if( pOStm )
                xOStm = Reference< XOutputStream >( new ::utl::OOutputStreamWrapper ( *pOStm ) );
        }
        else if ( pValue[ i ].Name == "FilterData" )
        {
            pValue[ i ].Value >>= maFilterData;
        }
    }

    // if no filter data is given use stored/prepared ones
    if( !maFilterData.getLength() )
    {
        maFilterData.realloc( 6 );

        maFilterData[ 0 ].Name = B2UCONST( SVG_PROP_TINYPROFILE );
        maFilterData[ 0 ].Value <<= (sal_Bool) sal_True;

        // font embedding
        const char* pSVGDisableFontEmbedding = getenv( "SVG_DISABLE_FONT_EMBEDDING" );
        rtl::OUString aEmbedFontEnv(
            RTL_CONSTASCII_USTRINGPARAM("${SVG_DISABLE_FONT_EMBEDDING}"));
        rtl::Bootstrap::expandMacros(aEmbedFontEnv);
        const bool bEmbedFonts=pSVGDisableFontEmbedding ? false : (
            aEmbedFontEnv.getLength() ? false : true);

        maFilterData[ 1 ].Name = B2UCONST( SVG_PROP_EMBEDFONTS );
        maFilterData[ 1 ].Value <<= (sal_Bool) (bEmbedFonts);

        // Native decoration
        maFilterData[ 2 ].Name = B2UCONST( SVG_PROP_NATIVEDECORATION );
        maFilterData[ 2 ].Value <<= (sal_Bool) sal_False;

        // glyph placement
        const char* pSVGGlyphPlacement = getenv( "SVG_GLYPH_PLACEMENT" );

        maFilterData[ 3 ].Name = B2UCONST( SVG_PROP_GLYPHPLACEMENT );

        if( pSVGGlyphPlacement )
            maFilterData[ 3 ].Value <<= ::rtl::OUString::createFromAscii( pSVGGlyphPlacement );
        else
            maFilterData[ 3 ].Value <<= B2UCONST( "xlist" );

        // Tiny Opacity
        maFilterData[ 4 ].Name = B2UCONST( SVG_PROP_OPACITY );
        maFilterData[ 4 ].Value <<= (sal_Bool) sal_True;

        // Tiny Gradient
        maFilterData[ 5 ].Name = B2UCONST( SVG_PROP_GRADIENT );
        maFilterData[ 5 ].Value <<= (sal_Bool) sal_False;
    }

    if( xOStm.is() && xServiceFactory.is() )
    {
        if( mSelectedPages.hasElements() && mMasterPageTargets.hasElements() )
        {
            Reference< XDocumentHandler > xDocHandler( implCreateExportDocumentHandler( xOStm ) );

            if( xDocHandler.is() )
            {
                mbPresentation = Reference< XPresentationSupplier >( mxSrcDoc, UNO_QUERY ).is();
                mpObjects = new ObjectMap;

                // #110680#
                // mpSVGExport = new SVGExport( xDocHandler );
                mpSVGExport = new SVGExport( xServiceFactory, xDocHandler, maFilterData );

                if( mpSVGExport != NULL )
                {
                    // xSVGExport is set up only to manage the life-time of the object pointed by mpSVGExport,
                    // and in order to prevent that it is destroyed when passed to AnimationExporter.
                    Reference< XInterface > xSVGExport = static_cast< ::com::sun::star::document::XFilter* >( mpSVGExport );

                    // create an id for each draw page
                    for( sal_Int32 i = 0; i < mSelectedPages.getLength(); ++i )
                        implRegisterInterface( mSelectedPages[i] );

                    // create an id for each master page
                    for( sal_Int32 i = 0; i < mMasterPageTargets.getLength(); ++i )
                        implRegisterInterface( mMasterPageTargets[i] );

                    try
                    {
                        mxDefaultPage = mSelectedPages[0];

                        if( mxDefaultPage.is() )
                        {
                            SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( mxDefaultPage );

                            if( pSvxDrawPage )
                            {
                                mpDefaultSdrPage = pSvxDrawPage->GetSdrPage();
                                mpSdrModel = mpDefaultSdrPage->GetModel();

                                if( mpSdrModel )
                                {
                                    SdrOutliner& rOutl = mpSdrModel->GetDrawOutliner(NULL);

                                    maOldFieldHdl = rOutl.GetCalcFieldValueHdl();
                                    rOutl.SetCalcFieldValueHdl( LINK( this, SVGFilter, CalcFieldHdl) );
                                }
                            }
                            bRet = implExportDocument();
                        }
                    }
                    catch( ... )
                    {
                        delete mpSVGDoc, mpSVGDoc = NULL;
                        OSL_FAIL( "Exception caught" );
                    }

                    if( mpSdrModel )
                        mpSdrModel->GetDrawOutliner( NULL ).SetCalcFieldValueHdl( maOldFieldHdl );

                    delete mpSVGWriter, mpSVGWriter = NULL;
                    mpSVGExport = NULL; // pointed object is released by xSVGExport dtor at the end of this scope
                    delete mpSVGFontExport, mpSVGFontExport = NULL;
                    delete mpObjects, mpObjects = NULL;
                    mbPresentation = sal_False;
                }
            }
        }
    }

    delete pOStm;

    return bRet;
}

// -----------------------------------------------------------------------------

Reference< XDocumentHandler > SVGFilter::implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm )
{
    Reference< XMultiServiceFactory >   xMgr( ::comphelper::getProcessServiceFactory() );
    Reference< XDocumentHandler >       xSaxWriter;

    if( xMgr.is() && rxOStm.is() )
    {
        xSaxWriter = Reference< XDocumentHandler >( xMgr->createInstance( B2UCONST( "com.sun.star.xml.sax.Writer" ) ), UNO_QUERY );

        if( xSaxWriter.is() )
        {
            Reference< XActiveDataSource > xActiveDataSource( xSaxWriter, UNO_QUERY );

            if( xActiveDataSource.is() )
                xActiveDataSource->setOutputStream( rxOStm );
            else
                xSaxWriter = NULL;
        }
    }

    return xSaxWriter;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implLookForFirstVisiblePage()
{
    sal_Int32 nCurPage = 0, nLastPage = mSelectedPages.getLength() - 1;

    while( ( nCurPage <= nLastPage ) && ( -1 == mnVisiblePage ) )
    {
        const Reference< XDrawPage > & xDrawPage = mSelectedPages[nCurPage];

        if( xDrawPage.is() )
        {
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

            if( xPropSet.is() )
            {
                sal_Bool bVisible = sal_False;

                if( !mbPresentation || mbSinglePage ||
                    ( ( xPropSet->getPropertyValue( B2UCONST( "Visible" ) ) >>= bVisible ) && bVisible ) )
                {
                    mnVisiblePage = nCurPage;
                }
            }
        }
        ++nCurPage;
    }

    return ( mnVisiblePage != -1 );
}

// -----------------------------------------------------------------------------
sal_Bool SVGFilter::implExportDocument()
{
    OUString         aAttr;
    sal_Int32        nDocWidth = 0, nDocHeight = 0;
    sal_Bool         bRet = sal_False;
    sal_Int32        nLastPage = mSelectedPages.getLength() - 1;

    SvtMiscOptions aMiscOptions;
    const bool bExperimentalMode = aMiscOptions.IsExperimentalMode();

    mbSinglePage = (nLastPage == 0) || !bExperimentalMode;
    mnVisiblePage = -1;

    const Reference< XPropertySet >             xDefaultPagePropertySet( mxDefaultPage, UNO_QUERY );
    const Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

    if( xDefaultPagePropertySet.is() )
    {
        xDefaultPagePropertySet->getPropertyValue( B2UCONST( "Width" ) ) >>= nDocWidth;
        xDefaultPagePropertySet->getPropertyValue( B2UCONST( "Height" ) ) >>= nDocHeight;
    }

    if( xExtDocHandler.is() && !mpSVGExport->IsUseTinyProfile() )
    {
        xExtDocHandler->unknown( SVG_DTD_STRING );
    }

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "version", B2UCONST( "1.2" ) );

    if( mpSVGExport->IsUseTinyProfile() )
         mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "baseProfile", B2UCONST( "tiny" ) );

    // enabling _SVG_WRITE_EXTENTS means that the slide size is not adapted
    // to the size of the browser window, moreover the slide is top left aligned
    // instead of centered.
    #define _SVG_WRITE_EXTENTS
    #ifdef _SVG_WRITE_EXTENTS
    if( mbSinglePage )
    {
        aAttr = OUString::valueOf( nDocWidth * 0.01 );
        aAttr += B2UCONST( "mm" );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

        aAttr = OUString::valueOf( nDocHeight * 0.01 );
        aAttr += B2UCONST( "mm" );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );
    }
    #endif


    aAttr = B2UCONST( "0 0 " );
    aAttr += OUString::valueOf( nDocWidth );
    aAttr += B2UCONST( " " );
    aAttr += OUString::valueOf( nDocHeight );

    msClipPathId = B2UCONST( "presentation_clip_path" );
    OUString sClipPathAttrValue = B2UCONST( "url(#" ) + msClipPathId + B2UCONST( ")" );

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", B2UCONST( "xMidYMid" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill-rule", B2UCONST( "evenodd" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clip-path", sClipPathAttrValue );

    // standard line width is based on 1 pixel on a 90 DPI device (0.28222mmm)
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-width", OUString::valueOf( 28.222 ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "stroke-linejoin", B2UCONST( "round" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns", B2UCONST( "http://www.w3.org/2000/svg" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", B2UCONST( "http://xml.openoffice.org/svg/export" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:xlink", B2UCONST( "http://www.w3.org/1999/xlink" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xml:space", B2UCONST( "preserve" ) );

    mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", sal_True, sal_True );

    // Create a ClipPath element that will be used for cutting bitmaps and other elements that could exceed the page margins.
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "ClipPathGroup" ) );
        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );
        {
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", msClipPathId );
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clipPathUnits", B2UCONST( "userSpaceOnUse" ) );
            SvXMLElementExport aClipPathElem( *mpSVGExport, XML_NAMESPACE_NONE, "clipPath", sal_True, sal_True );
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "x", OUString::valueOf( sal_Int32(0) ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "y", OUString::valueOf( sal_Int32(0) ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", OUString::valueOf( nDocWidth ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", OUString::valueOf( nDocHeight ) );
                SvXMLElementExport aRectElem( *mpSVGExport, XML_NAMESPACE_NONE, "rect", sal_True, sal_True );
            }
        }
    }

    if( implLookForFirstVisiblePage() )  // OK! We found at least one visible page.
    {
        if( !mbSinglePage )
        {
            implGenerateMetaData();
            if( bExperimentalMode )
                implExportAnimations();
        }
        else
        {
            implGetPagePropSet( mSelectedPages[0] );
        }

        // Create the (Shape, GDIMetaFile) map
        if( implCreateObjects() )
        {
            ObjectMap::const_iterator                aIter( mpObjects->begin() );
            ::std::vector< ObjectRepresentation >    aObjects( mpObjects->size() );
            sal_uInt32                               nPos = 0;

            while( aIter != mpObjects->end() )
            {
                aObjects[ nPos++ ] = (*aIter).second;
                ++aIter;
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
            implExportMasterPages( mMasterPageTargets, 0, mMasterPageTargets.getLength() - 1 );
            implExportDrawPages( mSelectedPages, 0, nLastPage );

            if( !mbSinglePage )
            {
                implGenerateScript();
            }

            delete mpSVGDoc, mpSVGDoc = NULL;
            bRet = sal_True;
        }
    }

    return bRet;
}


//------------------------------------------------------------------------------
// Append aField to aFieldSet if it is not already present in the set
// and create the field id sFieldId
//

template< typename TextFieldType >
OUString implGenerateFieldId( std::vector< TextField* > & aFieldSet,
                              const TextFieldType & aField,
                              const OUString & sOOOElemField,
                              Reference< XDrawPage > xMasterPage )
{
    sal_Bool bFound = sal_False;
    sal_Int32 i;
    sal_Int32 nSize = aFieldSet.size();
    for( i = 0; i < nSize; ++i )
    {
        if( *(aFieldSet[i]) == aField )
        {
            bFound = sal_True;
            break;
        }
    }
    OUString sFieldId( sOOOElemField );
    sFieldId += OUString::valueOf( sal_Unicode('_') );
    if( !bFound )
    {
        aFieldSet.push_back( new TextFieldType( aField ) );
    }
    aFieldSet[i]->insertMasterPage( xMasterPage );
    sFieldId += OUString::valueOf( i );
    return sFieldId;
}

//------------------------------------------------------------------------------

sal_Bool SVGFilter::implGenerateMetaData()
{
    sal_Bool bRet = sal_False;
    sal_Int32 nCount = mSelectedPages.getLength();
    if( nCount != 0 )
    {
        // we wrap all meta presentation info into a svg:defs element
        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", B2UCONST( aOOOElemMetaSlides ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrNumberOfSlides, OUString::valueOf( nCount ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrStartSlideNumber, OUString::valueOf( mnVisiblePage ) );

        /*
         *  Add a (global) Page Numbering Type attribute for the document
         */
        // NOTE:
        // at present pSdrModel->GetPageNumType() returns always SVX_ARABIC
        // so the following code fragment is pretty unuseful
        sal_Int32 nPageNumberingType =  SVX_ARABIC;
        SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( mSelectedPages[0] );
        if( pSvxDrawPage )
        {
            SdrPage* pSdrPage = pSvxDrawPage->GetSdrPage();
            SdrModel* pSdrModel = pSdrPage->GetModel();
            nPageNumberingType = pSdrModel->GetPageNumType();

            // That is used by CalcFieldHdl method.
            mVisiblePagePropSet.nPageNumberingType = nPageNumberingType;
        }
        if( nPageNumberingType != SVX_NUMBER_NONE )
        {
            OUString sNumberingType;
            switch( nPageNumberingType )
            {
                case SVX_CHARS_UPPER_LETTER:
                    sNumberingType = B2UCONST( "alpha-upper" );
                    break;
                case SVX_CHARS_LOWER_LETTER:
                    sNumberingType = B2UCONST( "alpha-lower" );
                    break;
                case SVX_ROMAN_UPPER:
                    sNumberingType = B2UCONST( "roman-upper" );
                    break;
                case SVX_ROMAN_LOWER:
                    sNumberingType = B2UCONST( "roman-lower" );
                    break;
                // arabic numbering type is the default, so we do not append any attribute for it
                case SVX_ARABIC: ;
                // in case the numbering type is not handled we fall back on arabic numbering
                default: ;
                    break;
            }
            if( !sNumberingType.isEmpty() )
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrNumberingType, sNumberingType );
        }


        {
            SvXMLElementExport    aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
            const OUString                aId( B2UCONST( aOOOElemMetaSlide ) );
            const OUString                aElemTextFieldId( B2UCONST( aOOOElemTextField ) );
            std::vector< TextField* >     aFieldSet;

            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                const Reference< XDrawPage > &    xDrawPage = mSelectedPages[i];
                Reference< XMasterPageTarget >    xMasterPageTarget( xDrawPage, UNO_QUERY );
                Reference< XDrawPage >            xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
                OUString                          aSlideId( aId );
                aSlideId += OUString::valueOf( sal_Unicode('_') );
                aSlideId += OUString::valueOf( i );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", aSlideId );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, implGetValidIDFromInterface( xDrawPage ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMaster, implGetValidIDFromInterface( xMasterPage ) );


                if( mbPresentation )
                {
                    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

                    if( xPropSet.is() )
                    {
                        sal_Bool bBackgroundVisibility                = sal_True;     // default: visible
                        sal_Bool bBackgroundObjectsVisibility         = sal_True;     // default: visible
                        sal_Bool bPageNumberVisibility                = sal_False;    // default: hidden
                        sal_Bool bDateTimeVisibility                  = sal_True;     // default: visible
                        sal_Bool bFooterVisibility                    = sal_True;     // default: visible
                        sal_Bool bDateTimeFixed                       = sal_True;     // default: fixed

                        FixedDateTimeField            aFixedDateTimeField;
                        VariableDateTimeField         aVariableDateTimeField;
                        FooterField                   aFooterField;

                        xPropSet->getPropertyValue( B2UCONST( "IsBackgroundVisible" ) )  >>= bBackgroundVisibility;
                        // in case the attribute is set to its default value it is not appended to the meta-slide element
                        if( !bBackgroundVisibility ) // visibility default value: 'visible'
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrBackgroundVisibility, B2UCONST( "hidden" ) );

                        // Page Number, Date/Time, Footer and Header Fields are regarded as background objects.
                        // So bBackgroundObjectsVisibility overrides visibility of master page text fields.
                        xPropSet->getPropertyValue( B2UCONST( "IsBackgroundObjectsVisible" ) )  >>= bBackgroundObjectsVisibility;
                        if( bBackgroundObjectsVisibility ) // visibility default value: 'visible'
                        {
                            /*
                             *  Page Number Field
                             */
                            xPropSet->getPropertyValue( B2UCONST( "IsPageNumberVisible" ) )  >>= bPageNumberVisibility;
                            bPageNumberVisibility = bPageNumberVisibility && ( nPageNumberingType != SVX_NUMBER_NONE );
                            if( bPageNumberVisibility ) // visibility default value: 'hidden'
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrPageNumberVisibility, B2UCONST( "visible" ) );
                            }
                            /*
                             *  Date/Time Field
                             */
                            xPropSet->getPropertyValue( B2UCONST( "IsDateTimeVisible" ) ) >>= bDateTimeVisibility;
                            if( bDateTimeVisibility ) // visibility default value: 'visible'
                            {
                                xPropSet->getPropertyValue( B2UCONST( "IsDateTimeFixed" ) ) >>= bDateTimeFixed;
                                if( bDateTimeFixed ) // we are interested only in the field text not in the date/time format
                                {
                                    xPropSet->getPropertyValue( B2UCONST( "DateTimeText" ) ) >>= aFixedDateTimeField.text;
                                    if( !aFixedDateTimeField.text.isEmpty() )
                                    {
                                        OUString sFieldId = implGenerateFieldId( aFieldSet, aFixedDateTimeField, aElemTextFieldId, xMasterPage );
                                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeField, sFieldId );
                                    }
                                }
                                else // the inverse applies: we are interested only in the date/time format not in the field text
                                {
                                    xPropSet->getPropertyValue( B2UCONST( "DateTimeFormat" ) ) >>= aVariableDateTimeField.format;
                                    OUString sFieldId = implGenerateFieldId( aFieldSet, aVariableDateTimeField, aElemTextFieldId, xMasterPage );
                                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeField, sFieldId );
                                }
                            }
                            else
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrDateTimeVisibility, B2UCONST( "hidden" ) );
                            }
                            /*
                             *  Footer Field
                             */
                            xPropSet->getPropertyValue( B2UCONST( "IsFooterVisible" ) )  >>= bFooterVisibility;
                            if( bFooterVisibility ) // visibility default value: 'visible'
                            {
                                xPropSet->getPropertyValue( B2UCONST( "FooterText" ) ) >>= aFooterField.text;
                                if( !aFooterField.text.isEmpty() )
                                {
                                    OUString sFieldId = implGenerateFieldId( aFieldSet, aFooterField, aElemTextFieldId, xMasterPage );
                                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrFooterField, sFieldId );
                                }
                            }
                            else
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrFooterVisibility, B2UCONST( "hidden" ) );
                            }
                        }
                        else
                        {
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrMasterObjectsVisibility, B2UCONST( "hidden" ) );
                        }

                        // We look for a slide transition.
                        // Transition properties are exported together with animations.
                        sal_Int16 nTransitionType(0);
                        if( xPropSet->getPropertySetInfo()->hasPropertyByName( "TransitionType" ) &&
                            (xPropSet->getPropertyValue( "TransitionType" ) >>= nTransitionType) )
                        {
                            sal_Int16 nTransitionSubType(0);
                            if( xPropSet->getPropertyValue( B2UCONST( "TransitionSubtype" ) )  >>= nTransitionSubType )
                            {
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrHasTransition, B2UCONST( "true" ) );
                            }
                        }

                    }
                }

                {
                    SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
                }  // when the aExp2 destructor is called the </g> tag is appended to the output file
            }

            // export text field elements
            if( mbPresentation )
            {
                for( sal_Int32 i = 0, nSize = aFieldSet.size(); i < nSize; ++i )
                {
                    OUString sElemId = B2UCONST( aOOOElemTextField );
                    sElemId += OUString::valueOf( sal_Unicode('_') );
                    sElemId += OUString::valueOf( i );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sElemId );
                    aFieldSet[i]->elementExport( mpSVGExport );
                }
                if( mpSVGExport->IsEmbedFonts() && mpSVGExport->IsUsePositionedCharacters() )
                {
                    for( sal_Int32 i = 0, nSize = aFieldSet.size(); i < nSize; ++i )
                    {
                        aFieldSet[i]->growCharSet( mTextFieldCharSets );
                    }
                }
            }
            // text fields are used only for generating meta info so we don't need them anymore
            for( sal_uInt32 i = 0; i < aFieldSet.size(); ++i )
            {
                if( aFieldSet[i] != NULL )
                {
                    delete aFieldSet[i];
                }
            }
        }
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportAnimations()
{
    sal_Bool bRet = sal_False;

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", B2UCONST( "presentation-animations" )  );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

    for( sal_Int32 i = 0; i < mSelectedPages.getLength(); ++i )
    {
        Reference< XPropertySet > xProps( mSelectedPages[i], UNO_QUERY );

        if( xProps.is() && xProps->getPropertySetInfo()->hasPropertyByName( "TransitionType" ) )
        {
            sal_Int16 nTransition = 0;
            xProps->getPropertyValue(  B2UCONST( "TransitionType" ) )  >>= nTransition;
            // we have a slide transition ?
            sal_Bool bHasEffects = ( nTransition != 0 );

            Reference< XAnimationNodeSupplier > xAnimNodeSupplier( mSelectedPages[i], UNO_QUERY );
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
                        OUString sId = implGetValidIDFromInterface( mSelectedPages[i] );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, sId  );
                        sId += B2UCONST( "-animations" );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId  );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "Animations" ) );
                        SvXMLElementExport aDefsElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

                        UniReference< xmloff::AnimationsExporter > xAnimationsExporter;
                        xAnimationsExporter = new xmloff::AnimationsExporter( *mpSVGExport, xProps );
                        xAnimationsExporter->prepare( xRootNode );
                        xAnimationsExporter->exportAnimations( xRootNode );
                    }
                }
            }
        }
    }

    bRet = sal_True;
    return bRet;
}

// -----------------------------------------------------------------------------

void SVGFilter::implExportTextShapeIndex()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextShapeIndex" )  );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

    sal_Int32 nCount = mSelectedPages.getLength();
    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        const Reference< XDrawPage > & xDrawPage = mSelectedPages[i];
        if( mTextShapeIdListMap.find( xDrawPage ) != mTextShapeIdListMap.end() )
        {
            OUString sTextShapeIdList = mTextShapeIdListMap[xDrawPage].trim();

            Reference< XInterface > xRef( xDrawPage, UNO_QUERY );
            const OUString& rPageId = implGetValidIDFromInterface( xRef );
            if( !rPageId.isEmpty() && !sTextShapeIdList.isEmpty() )
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrSlide, rPageId  );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrIdList, sTextShapeIdList );
                SvXMLElementExport aGElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGFilter::implEmbedBulletGlyphs()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "EmbeddedBulletChars" )  );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

    OUString sPathData =  B2UCONST( "M 580,1141 L 1163,571 580,0 -4,571 580,1141 Z" );
    implEmbedBulletGlyph( 57356, sPathData );
    sPathData =  B2UCONST( "M 8,1128 L 1137,1128 1137,0 8,0 8,1128 Z" );
    implEmbedBulletGlyph( 57354, sPathData );
    sPathData =  B2UCONST( "M 174,0 L 602,739 174,1481 1456,739 174,0 Z M 1358,739 L 309,1346 659,739 1358,739 Z" );
    implEmbedBulletGlyph( 10146, sPathData );
    sPathData =  B2UCONST( "M 2015,739 L 1276,0 717,0 1260,543 174,543 174,936 1260,936 717,1481 1274,1481 2015,739 Z" );
    implEmbedBulletGlyph( 10132, sPathData );
    sPathData =  B2UCONST( "M 0,-2 C -7,14 -16,27 -25,37 L 356,567 C 262,823 215,952 215,954 215,979 228,992 255,992 264,992 276,990 289,987 310,991 331,999 354,1012 L 381,999 492,748 772,1049 836,1024 860,1049 C 881,1039 901,1025 922,1006 886,937 835,863 770,784 769,783 710,716 594,584 L 774,223 C 774,196 753,168 711,139 L 727,119 C 717,90 699,76 672,76 641,76 570,178 457,381 L 164,-76 C 142,-110 111,-127 72,-127 30,-127 9,-110 8,-76 1,-67 -2,-52 -2,-32 -2,-23 -1,-13 0,-2 Z" );
    implEmbedBulletGlyph( 10007, sPathData );
    sPathData =  B2UCONST( "M 285,-33 C 182,-33 111,30 74,156 52,228 41,333 41,471 41,549 55,616 82,672 116,743 169,778 240,778 293,778 328,747 346,684 L 369,508 C 377,444 397,411 428,410 L 1163,1116 C 1174,1127 1196,1133 1229,1133 1271,1133 1292,1118 1292,1087 L 1292,965 C 1292,929 1282,901 1262,881 L 442,47 C 390,-6 338,-33 285,-33 Z" );
    implEmbedBulletGlyph( 10004, sPathData );
    sPathData =  B2UCONST( "M 813,0 C 632,0 489,54 383,161 276,268 223,411 223,592 223,773 276,916 383,1023 489,1130 632,1184 813,1184 992,1184 1136,1130 1245,1023 1353,916 1407,772 1407,592 1407,412 1353,268 1245,161 1136,54 992,0 813,0 Z" );
    implEmbedBulletGlyph( 9679, sPathData );
    sPathData =  B2UCONST( "M 346,457 C 273,457 209,483 155,535 101,586 74,649 74,723 74,796 101,859 155,911 209,963 273,989 346,989 419,989 480,963 531,910 582,859 608,796 608,723 608,648 583,586 532,535 482,483 420,457 346,457 Z" );
    implEmbedBulletGlyph( 8226, sPathData );
    sPathData =  B2UCONST( "M -4,459 L 1135,459 1135,606 -4,606 -4,459 Z" );
    implEmbedBulletGlyph( 8211, sPathData );
}

// -----------------------------------------------------------------------------

void SVGFilter::implEmbedBulletGlyph( sal_Unicode cBullet, const ::rtl::OUString & sPathData )
{
    OUString sId = B2UCONST( "bullet-char-template(" );
    sId += OUString::valueOf( (sal_Int32)cBullet );
    sId += B2UCONST( ")" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId );

    double fFactor = 1.0 / 2048;
    OUString sFactor = OUString::valueOf( fFactor );
    OUString sTransform = B2UCONST( "scale(" );
    sTransform += sFactor; sTransform += B2UCONST( ",-" ); sTransform += sFactor;
    sTransform += B2UCONST( ")" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "transform", sTransform );

    SvXMLElementExport aGElem( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "d", sPathData );
    SvXMLElementExport aPathElem( *mpSVGExport, XML_NAMESPACE_NONE, "path", sal_True, sal_True );

}

// -----------------------------------------------------------------------------

/** SVGFilter::implExportTextEmbeddedBitmaps
 *  We export bitmaps embedded into text shapes, such as those used by list
 *  items with image style, only once in a specic <defs> element.
 */
sal_Bool SVGFilter::implExportTextEmbeddedBitmaps()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextEmbeddedBitmaps" )  );
    SvXMLElementExport aDefsContainerElem( *mpSVGExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );

    OUString sId;

    MetaBitmapActionSet::const_iterator it = mEmbeddedBitmapActionSet.begin();
    MetaBitmapActionSet::const_iterator end = mEmbeddedBitmapActionSet.end();
    for( ; it != end; ++it)
    {
        const GDIMetaFile& aMtf = it->GetRepresentation();

        if( aMtf.GetActionSize() == 1 )
        {
            MetaAction* pAction = aMtf.GetAction( 0 );
            if( pAction )
            {
                sal_uLong nId = GetBitmapChecksum( pAction );
                sId = B2UCONST( "bitmap(" );
                sId += OUString::valueOf( (sal_Int64)nId );
                sId += B2UCONST( ")" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sId );

                const Reference< XShape >& rxShape = (const Reference< XShape >&)( it->GetObject() );
                Reference< XPropertySet > xShapePropSet( rxShape, UNO_QUERY );
                ::com::sun::star::awt::Rectangle    aBoundRect;
                if( xShapePropSet.is() && ( xShapePropSet->getPropertyValue( B2UCONST( "BoundRect" ) ) >>= aBoundRect ) )
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
                    mpSVGWriter->WriteMetaFile( aTopLeft, aSize, aMtf, SVGWRITER_WRITE_ALL, NULL );
                    // We reset to the original values so that when the <use>
                    // element is created the x, y attributes are correct.
                    pAction->Move( aPt.X(), aPt.Y() );
                }
                else
                {
                    OSL_FAIL( "implExportTextEmbeddedBitmaps: no shape bounding box." );
                    return sal_False;
                }
            }
            else
            {
                OSL_FAIL( "implExportTextEmbeddedBitmaps: metafile should have MetaBmpExScaleAction only." );
                return sal_False;
            }
        }
        else
        {
            OSL_FAIL( "implExportTextEmbeddedBitmaps: metafile should have a single action." );
            return sal_False;
        }

    }
    return sal_True;
}

// -----------------------------------------------------------------------------

#define SVGFILTER_EXPORT_SVGSCRIPT( z, n, aFragment ) \
        xExtDocHandler->unknown( OUString::createFromAscii( aFragment ## n ) );

sal_Bool SVGFilter::implGenerateScript()
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "type", B2UCONST( "text/ecmascript" ) );

    {
        SvXMLElementExport                       aExp( *mpSVGExport, XML_NAMESPACE_NONE, "script", sal_True, sal_True );
        Reference< XExtendedDocumentHandler >    xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

        if( xExtDocHandler.is() )
        {
            BOOST_PP_REPEAT( N_SVGSCRIPT_FRAGMENTS, SVGFILTER_EXPORT_SVGSCRIPT, aSVGScript )
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

/*  SVGFilter::implGetPagePropSet
 *
 *  We collect info on master page elements visibility,
 *  and placeholder text shape content.
 *  This method is used when exporting a single page
 *  as implGenerateMetaData is not invoked.
 */
sal_Bool SVGFilter::implGetPagePropSet( const Reference< XDrawPage > & rxPage )
{
    sal_Bool bRet = sal_False;

    mVisiblePagePropSet.bIsBackgroundVisible                = true;
    mVisiblePagePropSet.bAreBackgroundObjectsVisible        = true;
    mVisiblePagePropSet.bIsPageNumberFieldVisible           = false;;
    mVisiblePagePropSet.bIsHeaderFieldVisible               = false;
    mVisiblePagePropSet.bIsFooterFieldVisible               = true;
    mVisiblePagePropSet.bIsDateTimeFieldVisible             = true;
    mVisiblePagePropSet.bIsDateTimeFieldFixed               = true;
    mVisiblePagePropSet.nDateTimeFormat                     = SVXDATEFORMAT_B;
    mVisiblePagePropSet.nPageNumberingType                  = SVX_ARABIC;

    /*  We collect info on master page elements visibility,
     *  and placeholder text shape content.
     */
    Any result;
    Reference< XPropertySet > xPropSet( rxPage, UNO_QUERY );
    if( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        if( xPropSetInfo.is() )
        {
            implSafeGetPagePropSet( B2UCONST( "IsBackgroundVisible" ), xPropSet, xPropSetInfo )          >>= mVisiblePagePropSet.bIsBackgroundVisible;
            implSafeGetPagePropSet( B2UCONST( "IsBackgroundObjectsVisible" ), xPropSet, xPropSetInfo )   >>= mVisiblePagePropSet.bAreBackgroundObjectsVisible;
            implSafeGetPagePropSet( B2UCONST( "IsPageNumberVisible" ), xPropSet, xPropSetInfo )          >>= mVisiblePagePropSet.bIsPageNumberFieldVisible;
            implSafeGetPagePropSet( B2UCONST( "IsHeaderVisible" ), xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsHeaderFieldVisible;
            implSafeGetPagePropSet( B2UCONST( "IsFooterVisible" ), xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsFooterFieldVisible;
            implSafeGetPagePropSet( B2UCONST( "IsDateTimeVisible" ), xPropSet, xPropSetInfo )            >>= mVisiblePagePropSet.bIsDateTimeFieldVisible;

            implSafeGetPagePropSet( B2UCONST( "IsDateTimeFixed" ), xPropSet, xPropSetInfo )              >>= mVisiblePagePropSet.bIsDateTimeFieldFixed;
            implSafeGetPagePropSet( B2UCONST( "DateTimeFormat" ), xPropSet, xPropSetInfo )               >>= mVisiblePagePropSet.nDateTimeFormat;
            implSafeGetPagePropSet( B2UCONST( "Number" ), xPropSet, xPropSetInfo )                       >>= mVisiblePagePropSet.nPageNumber;
            implSafeGetPagePropSet( B2UCONST( "DateTimeText" ), xPropSet, xPropSetInfo )                 >>= mVisiblePagePropSet.sDateTimeText;
            implSafeGetPagePropSet( B2UCONST( "FooterText" ), xPropSet, xPropSetInfo )                   >>= mVisiblePagePropSet.sFooterText;
            implSafeGetPagePropSet( B2UCONST( "HeaderText" ), xPropSet, xPropSetInfo )                   >>= mVisiblePagePropSet.sHeaderText;

            if( mVisiblePagePropSet.bIsPageNumberFieldVisible )
            {
                SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( rxPage );
                if( pSvxDrawPage )
                {
                    SdrPage* pSdrPage = pSvxDrawPage->GetSdrPage();
                    SdrModel* pSdrModel = pSdrPage->GetModel();
                    mVisiblePagePropSet.nPageNumberingType = pSdrModel->GetPageNumType();
                }
            }

            bRet = sal_True;
        }
    }

    return bRet;
}


// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportMasterPages( const SVGFilter::XDrawPageSequence & rxPages,
                                           sal_Int32 nFirstPage, sal_Int32 nLastPage )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportPages: nFirstPage > nLastPage" );

    // When the exported slides are more than one we wrap master page elements
    // with a svg <defs> element.
    OUString aContainerTag = (mbSinglePage) ? B2UCONST( "g" ) : B2UCONST( "defs" );
    SvXMLElementExport aContainerElement( *mpSVGExport, XML_NAMESPACE_NONE, aContainerTag, sal_True, sal_True );

    sal_Bool bRet = sal_False;
    for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
    {
        if( rxPages[i].is() )
        {
            Reference< XShapes > xShapes( rxPages[i], UNO_QUERY );

            if( xShapes.is() )
            {
                // add id attribute
                const OUString & sPageId = implGetValidIDFromInterface( rxPages[i] );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sPageId );

                bRet = implExportPage( sPageId, rxPages[i], xShapes, sal_True /* is a master page */ ) || bRet;
            }
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportDrawPages( const SVGFilter::XDrawPageSequence & rxPages,
                                           sal_Int32 nFirstPage, sal_Int32 nLastPage )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportPages: nFirstPage > nLastPage" );

    // We wrap all slide in a group element with class name "SlideGroup".
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "SlideGroup" ) );
    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

    sal_Bool bRet = sal_False;
    for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
    {
        Reference< XShapes > xShapes( rxPages[i], UNO_QUERY );

        if( xShapes.is() )
        {
            // Insert the <g> open tag related to the svg element for
            // handling a slide visibility.
            // In case the exported slides are more than one the initial
            // visibility of each slide is set to 'hidden'.
            if( !mbSinglePage )
            {
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", B2UCONST( "hidden" ) );
            }
            SvXMLElementExport aGElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

            {
                // add id attribute
                const OUString & sPageId = implGetValidIDFromInterface( rxPages[i] );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sPageId );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "Slide" ) );

                // Adding a clip path to each exported slide , so in case
                // bitmaps or other elements exceed the slide margins, they are
                // trimmed, even when they are shown inside a thumbnail view.
                OUString sClipPathAttrValue = B2UCONST( "url(#" ) + msClipPathId + B2UCONST( ")" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clip-path", sClipPathAttrValue );

                SvXMLElementExport aSlideElement( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

                bRet = implExportPage( sPageId, rxPages[i], xShapes, sal_False /* is not a master page */ ) || bRet;
            }
        } // append the </g> closing tag related to the svg element handling the slide visibility
    }

    return bRet;
}

// -----------------------------------------------------------------------------
sal_Bool SVGFilter::implExportPage( const ::rtl::OUString & sPageId,
                                    const Reference< XDrawPage > & rxPage,
                                    const Reference< XShapes > & xShapes,
                                    sal_Bool bMaster )
{
    sal_Bool bRet = sal_False;

    {
        OUString sPageName = implGetInterfaceName( rxPage );
        if( !(sPageName.isEmpty() || mbSinglePage ))
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrName, sPageName );

        {
            Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

            if( xExtDocHandler.is() )
            {
                OUString aDesc;

                if( bMaster )
                    aDesc = B2UCONST( "Master_Slide" );
                else
                    aDesc = B2UCONST( "Page" );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aDesc );
            }
        }

        // insert the <g> open tag related to the DrawPage/MasterPage
        SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

        // In case the page has a background object we append it .
        if( (mpObjects->find( rxPage ) != mpObjects->end()) )
        {
            const GDIMetaFile& rMtf = (*mpObjects)[ rxPage ].GetRepresentation();
            if( rMtf.GetActionSize() )
            {
                // background id = "bg-" + page id
                OUString sBackgroundId = B2UCONST( "bg-" );
                sBackgroundId += sPageId;
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sBackgroundId );

                // At present (LibreOffice 3.4.0) the 'IsBackgroundVisible' property is not handled
                // by Impress; anyway we handle this property as referring only to the visibility
                // of the master page background. So if a slide has its own background object,
                // the visibility of such a background object is always inherited from the visibility
                // of the parent slide regardless of the value of the 'IsBackgroundVisible' property.
                // This means that we need to set up the visibility attribute only for the background
                // element of a master page.
                if( mbSinglePage && bMaster )
                {
                    if( !mVisiblePagePropSet.bIsBackgroundVisible )
                    {
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", B2UCONST( "hidden" ) );
                    }
                }

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class",  B2UCONST( "Background" ) );

                // insert the <g> open tag related to the Background
                SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

                // append all elements that make up the Background
                const Point aNullPt;
                mpSVGWriter->WriteMetaFile( aNullPt, rMtf.GetPrefSize(), rMtf, SVGWRITER_WRITE_FILL );
            }   // insert the </g> closing tag related to the Background
        }

        // In case we are dealing with a master page we need to to group all its shapes
        // into a group element, this group will make up the so named "background objects"
        if( bMaster )
        {
            // background objects id = "bo-" + page id
            OUString sBackgroundObjectsId = B2UCONST( "bo-" );
            sBackgroundObjectsId += sPageId;
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sBackgroundObjectsId );
            if( mbSinglePage )
            {
                if( !mVisiblePagePropSet.bAreBackgroundObjectsVisible )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", B2UCONST( "hidden" ) );
                }
            }
            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class",  B2UCONST( "BackgroundObjects" ) );

            // insert the <g> open tag related to the Background Objects
            SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

            // append all shapes that make up the Master Slide
            bRet = implExportShapes( xShapes ) || bRet;
        }   // append the </g> closing tag related to the Background Objects
        else
        {
            // append all shapes that make up the Slide
            bRet = implExportShapes( xShapes ) || bRet;
        }
    }  // append the </g> closing tag related to the Slide/Master_Slide

    return bRet;
}


// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportShapes( const Reference< XShapes >& rxShapes )
{
    Reference< XShape > xShape;
    sal_Bool            bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implExportShape( xShape ) || bRet;

        xShape = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportShape( const Reference< XShape >& rxShape )
{
    Reference< XPropertySet >   xShapePropSet( rxShape, UNO_QUERY );
    sal_Bool                    bRet = sal_False;

    if( xShapePropSet.is() )
    {
        const ::rtl::OUString   aShapeType( rxShape->getShapeType() );
        sal_Bool                    bHideObj = sal_False;

        if( mbPresentation )
        {
            xShapePropSet->getPropertyValue( B2UCONST( "IsEmptyPresentationObject" ) )  >>= bHideObj;
        }

        if( !bHideObj )
        {
            if( aShapeType.lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
            {
                Reference< XShapes > xShapes( rxShape, UNO_QUERY );

                if( xShapes.is() )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "Group" ) );
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

                    bRet = implExportShapes( xShapes );
                }
            }

            if( !bRet && mpObjects->find( rxShape ) !=  mpObjects->end() )
            {
                const ::rtl::OUString*                    pElementId = NULL;

                ::com::sun::star::awt::Rectangle    aBoundRect;
                const GDIMetaFile&                  rMtf = (*mpObjects)[ rxShape ].GetRepresentation();

                xShapePropSet->getPropertyValue( B2UCONST( "BoundRect" ) ) >>= aBoundRect;

                const Point aTopLeft( aBoundRect.X, aBoundRect.Y );
                const Size  aSize( aBoundRect.Width, aBoundRect.Height );

                if( rMtf.GetActionSize() )
                {   // for text field shapes we set up text-adjust attributes
                    // and set visibility to hidden
                    OUString aShapeClass = implGetClassFromShape( rxShape );
                    if( mbPresentation )
                    {
                        sal_Bool bIsPageNumber  = ( aShapeClass == "Slide_Number" );
                        sal_Bool bIsFooter      = ( aShapeClass == "Footer" );
                        sal_Bool bIsDateTime    = ( aShapeClass == "Date/Time" );
                        if( bIsPageNumber || bIsDateTime || bIsFooter )
                        {
                            if( !mbSinglePage )
                            {
                                // to notify to the SVGActionWriter::ImplWriteActions method
                                // that we are dealing with a placeholder shape
                                pElementId = &sPlaceholderTag;

                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", B2UCONST( "hidden" ) );

                                sal_uInt16 nTextAdjust = ParagraphAdjust_LEFT;
                                OUString sTextAdjust;
                                xShapePropSet->getPropertyValue( B2UCONST( "ParaAdjust" ) ) >>= nTextAdjust;

                                switch( nTextAdjust )
                                {
                                    case ParagraphAdjust_LEFT:
                                            sTextAdjust = B2UCONST( "left" );
                                            break;
                                    case ParagraphAdjust_CENTER:
                                            sTextAdjust = B2UCONST( "center" );
                                            break;
                                    case ParagraphAdjust_RIGHT:
                                            sTextAdjust = B2UCONST( "right" );
                                            break;
                                    default:
                                        break;
                                }
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrTextAdjust, sTextAdjust );
                            }
                            else // single page case
                            {
                                if( !mVisiblePagePropSet.bAreBackgroundObjectsVisible || (
                                    ( bIsPageNumber && !mVisiblePagePropSet.bIsPageNumberFieldVisible ) ||
                                    ( bIsDateTime && !mVisiblePagePropSet.bIsDateTimeFieldVisible ) ||
                                    ( bIsFooter && !mVisiblePagePropSet.bIsFooterFieldVisible ) ) )
                                {
                                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", B2UCONST( "hidden" ) );
                                }
                            }
                        }
                    }
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aShapeClass );
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

                    Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                    OUString aTitle;
                    xShapePropSet->getPropertyValue( B2UCONST( "Title" ) ) >>= aTitle;
                    if( !aTitle.isEmpty() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "title", sal_True, sal_True );
                        xExtDocHandler->characters( aTitle );
                    }

                    OUString aDescription;
                    xShapePropSet->getPropertyValue( B2UCONST( "Description" ) ) >>= aDescription;
                    if( !aDescription.isEmpty() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", sal_True, sal_True );
                        xExtDocHandler->characters( aDescription );
                    }


                    Reference< XInterface > xRef( rxShape, UNO_QUERY );
                    const OUString& rShapeId = implGetValidIDFromInterface( xRef );
                    if( !rShapeId.isEmpty() )
                    {
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", rShapeId );
                    }

                    const GDIMetaFile* pEmbeddedBitmapsMtf = NULL;
                    if( mEmbeddedBitmapActionMap.find( rxShape ) !=  mEmbeddedBitmapActionMap.end() )
                    {
                        pEmbeddedBitmapsMtf = &( mEmbeddedBitmapActionMap[ rxShape ].GetRepresentation() );
                    }

                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
                        mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf,
                                                    SVGWRITER_WRITE_ALL,
                                                    pElementId,
                                                    &rxShape,
                                                    pEmbeddedBitmapsMtf );
                    }
                }

                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjects()
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = mMasterPageTargets.getLength(); i < nCount; ++i )
    {
        const Reference< XDrawPage > & xMasterPage = mMasterPageTargets[i];

        if( xMasterPage.is() )
        {
            mCreateOjectsCurrentMasterPage = xMasterPage;
            implCreateObjectsFromBackground( xMasterPage );

            Reference< XShapes > xShapes( xMasterPage, UNO_QUERY );

            if( xShapes.is() )
                implCreateObjectsFromShapes( xMasterPage, xShapes );
        }
    }

    for( i = 0, nCount = mSelectedPages.getLength(); i < nCount; ++i )
    {
        const Reference< XDrawPage > & xDrawPage = mSelectedPages[i];

        if( xDrawPage.is() )
        {
#if ENABLE_EXPORT_CUSTOM_SLIDE_BACKGROUND
            // TODO complete the implementation for exporting custom background for each slide
            // implementation status:
            // - hatch stroke color is set to 'none' so the hatch is not visible, why ?
            // - gradient look is not really awesome, too few colors are used;
            // - stretched bitmap, gradient and hatch are not exported only once
            //   and then referenced in case more than one slide uses them.
            // - tiled bitmap: an image element is exported for each tile,
            //   this is really too expensive!
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
            Reference< XPropertySet > xBackground;
            xPropSet->getPropertyValue( B2UCONST( "Background" ) ) >>= xBackground;
            if( xBackground.is() )
            {
                drawing::FillStyle aFillStyle;
                sal_Bool assigned = ( xBackground->getPropertyValue( B2UCONST( "FillStyle" ) ) >>= aFillStyle );
                if( assigned && aFillStyle != drawing::FillStyle_NONE )
                {
                    implCreateObjectsFromBackground( xDrawPage );
                }
            }
#endif
            Reference< XShapes > xShapes( xDrawPage, UNO_QUERY );

            if( xShapes.is() )
                implCreateObjectsFromShapes( xDrawPage, xShapes );
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShapes( const Reference< XDrawPage > & rxPage, const Reference< XShapes >& rxShapes )
{
    Reference< XShape > xShape;
    sal_Bool            bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implCreateObjectsFromShape( rxPage, xShape ) || bRet;

        xShape = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShape( const Reference< XDrawPage > & rxPage, const Reference< XShape >& rxShape )
{
    sal_Bool bRet = sal_False;
    if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
    {
        Reference< XShapes > xShapes( rxShape, UNO_QUERY );

        if( xShapes.is() )
            bRet = implCreateObjectsFromShapes( rxPage, xShapes );
    }
    else
    {
        SdrObject*  pObj = GetSdrObjectFromXShape( rxShape );

        if( pObj )
        {
            Graphic aGraphic( SdrExchangeView::GetObjGraphic( pObj->GetModel(), pObj ) );

            if( aGraphic.GetType() != GRAPHIC_NONE )
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GDIMetaFile    aMtf;
                    const Point    aNullPt;
                    const Size    aSize( pObj->GetCurrentBoundRect().GetSize() );

                    aMtf.AddAction( new MetaBmpExScaleAction( aNullPt, aSize, aGraphic.GetBitmapEx() ) );
                    aMtf.SetPrefSize( aSize );
                    aMtf.SetPrefMapMode( MAP_100TH_MM );

                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aMtf );
                }
                else
                {
                    if( aGraphic.GetGDIMetaFile().GetActionSize() )
                    {
                        Reference< XText > xText( rxShape, UNO_QUERY );
                        sal_Bool bIsTextShape = xText.is();

                        if( !mpSVGExport->IsUsePositionedCharacters() && bIsTextShape )
                        {
                            Reference< XPropertySet >   xShapePropSet( rxShape, UNO_QUERY );

                            if( xShapePropSet.is() )
                            {
                                sal_Bool bHideObj = sal_False;

                                if( mbPresentation )
                                {
                                    xShapePropSet->getPropertyValue( B2UCONST( "IsEmptyPresentationObject" ) )  >>= bHideObj;
                                }

                                if( !bHideObj )
                                {
                                    // We create a map of text shape ids.
                                    implRegisterInterface( rxShape );
                                    Reference< XInterface > xRef( rxShape, UNO_QUERY );
                                    const OUString& rShapeId = implGetValidIDFromInterface( xRef );
                                    if( !rShapeId.isEmpty() )
                                    {
                                        mTextShapeIdListMap[rxPage] += rShapeId;
                                        mTextShapeIdListMap[rxPage] += B2UCONST( " " );
                                    }

                                    // We create a set of bitmaps embedded into text shape.
                                    GDIMetaFile   aMtf;
                                    const Size    aSize( pObj->GetCurrentBoundRect().GetSize() );
                                    MetaAction*   pAction;
                                    sal_Bool bIsTextShapeStarted = sal_False;
                                    const GDIMetaFile& rMtf = aGraphic.GetGDIMetaFile();
                                    sal_uLong nCount = rMtf.GetActionSize();
                                    for( sal_uLong nCurAction = 0; nCurAction < nCount; ++nCurAction )
                                    {
                                        pAction = rMtf.GetAction( nCurAction );
                                        const sal_uInt16    nType = pAction->GetType();

                                        if( nType == META_COMMENT_ACTION )
                                        {
                                            const MetaCommentAction* pA = (const MetaCommentAction*) pAction;
                                            if( ( pA->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XTEXT_PAINTSHAPE_BEGIN")) ) )
                                            {
                                                bIsTextShapeStarted = sal_True;
                                            }
                                            else if( ( pA->GetComment().equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_PAINTSHAPE_END" ) ) ) )
                                            {
                                                bIsTextShapeStarted = sal_False;
                                            }
                                        }
                                        if( bIsTextShapeStarted && ( nType == META_BMPSCALE_ACTION  || nType == META_BMPEXSCALE_ACTION ) )
                                        {
                                            GDIMetaFile aEmbeddedBitmapMtf;
                                            pAction->Duplicate();
                                            aEmbeddedBitmapMtf.AddAction( pAction );
                                            aEmbeddedBitmapMtf.SetPrefSize( aSize );
                                            aEmbeddedBitmapMtf.SetPrefMapMode( MAP_100TH_MM );
                                            mEmbeddedBitmapActionSet.insert( ObjectRepresentation( rxShape, aEmbeddedBitmapMtf ) );
                                            pAction->Duplicate();
                                            aMtf.AddAction( pAction );
                                        }
                                    }
                                    aMtf.SetPrefSize( aSize );
                                    aMtf.SetPrefMapMode( MAP_100TH_MM );
                                    mEmbeddedBitmapActionMap[ rxShape ] = ObjectRepresentation( rxShape, aMtf );
                                }
                            }
                        }
                    }
                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aGraphic.GetGDIMetaFile() );
                }
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromBackground( const Reference< XDrawPage >& rxDrawPage )
{
    Reference< XExporter >    xExporter( mxMSF->createInstance( B2UCONST( "com.sun.star.drawing.GraphicExportFilter" ) ), UNO_QUERY );
    sal_Bool                bRet = sal_False;

    if( xExporter.is() )
    {
        GDIMetaFile                aMtf;
        Reference< XFilter >    xFilter( xExporter, UNO_QUERY );

        utl::TempFile aFile;
        aFile.EnableKillingFile();

        Sequence< PropertyValue > aDescriptor( 3 );
        aDescriptor[0].Name = B2UCONST( "FilterName" );
        aDescriptor[0].Value <<= B2UCONST( "SVM" );
        aDescriptor[1].Name = B2UCONST( "URL" );
        aDescriptor[1].Value <<= OUString( aFile.GetURL() );
        aDescriptor[2].Name = B2UCONST( "ExportOnlyBackground" );
        aDescriptor[2].Value <<= (sal_Bool) sal_True;

        xExporter->setSourceDocument( Reference< XComponent >( rxDrawPage, UNO_QUERY ) );
        xFilter->filter( aDescriptor );
        aMtf.Read( *aFile.GetStream( STREAM_READ ) );

        (*mpObjects)[ rxDrawPage ] = ObjectRepresentation( rxDrawPage, aMtf );

        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

OUString SVGFilter::implGetClassFromShape( const Reference< XShape >& rxShape )
{
    OUString            aRet;
    const OUString      aShapeType( rxShape->getShapeType() );

    if( aShapeType.lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
        aRet = B2UCONST( "Group" );
    else if( aShapeType.lastIndexOf( B2UCONST( "drawing.GraphicObjectShape" ) ) != -1 )
        aRet = B2UCONST( "Graphic" );
    else if( aShapeType.lastIndexOf( B2UCONST( "drawing.OLE2Shape" ) ) != -1 )
        aRet = B2UCONST( "OLE2" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.HeaderShape" ) ) != -1 )
        aRet = B2UCONST( "Header" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.FooterShape" ) ) != -1 )
        aRet = B2UCONST( "Footer" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.DateTimeShape" ) ) != -1 )
        aRet = B2UCONST( "Date/Time" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.SlideNumberShape" ) ) != -1 )
        aRet = B2UCONST( "Slide_Number" );
    else
        aRet = aShapeType;
        //aRet = B2UCONST( "Drawing" );

    return aRet;
}

// -----------------------------------------------------------------------------

void SVGFilter::implRegisterInterface( const Reference< XInterface >& rxIf )
{
    if( rxIf.is() )
        (mpSVGExport->getInterfaceToIdentifierMapper()).registerReference( rxIf );
}

// -----------------------------------------------------------------------------

const ::rtl::OUString & SVGFilter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
   return (mpSVGExport->getInterfaceToIdentifierMapper()).getIdentifier( rxIf );
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

IMPL_LINK( SVGFilter, CalcFieldHdl, EditFieldInfo*, pInfo )
{
    sal_Bool bFieldProcessed = sal_False;
    if( pInfo && mbPresentation )
    {
        bFieldProcessed = true;
        OUString   aRepresentation = B2UCONST("");
        if( !mbSinglePage )
        {
            if( mpSVGExport->IsEmbedFonts() && mpSVGExport->IsUsePositionedCharacters() )
            {
                // to notify to the SVGActionWriter::ImplWriteText method
                // that we are dealing with a placeholder shape
                aRepresentation = sPlaceholderTag;

                if( !mCreateOjectsCurrentMasterPage.is() )
                {
                    OSL_FAIL( "error: !mCreateOjectsCurrentMasterPage.is()" );
                    return 0;
                }
                sal_Bool bHasCharSetMap = !( mTextFieldCharSets.find( mCreateOjectsCurrentMasterPage ) == mTextFieldCharSets.end() );

                static const ::rtl::OUString aHeaderId( B2UCONST( aOOOAttrHeaderField ) );
                static const ::rtl::OUString aFooterId( B2UCONST( aOOOAttrFooterField ) );
                static const ::rtl::OUString aDateTimeId( B2UCONST( aOOOAttrDateTimeField ) );
                static const ::rtl::OUString aVariableDateTimeId( B2UCONST( aOOOAttrDateTimeField ) + B2UCONST( "-variable" ) );

                const UCharSet * pCharSet = NULL;
                UCharSetMap * pCharSetMap = NULL;
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
                        SvxDateFormat eDateFormat = SVXDATEFORMAT_B, eCurDateFormat;
                        const UCharSet & aCharSet = (*pCharSetMap)[ aVariableDateTimeId ];
                        UCharSet::const_iterator aChar = aCharSet.begin();
                        // we look for the most verbose date format
                        for( ; aChar != aCharSet.end(); ++aChar )
                        {
                            eCurDateFormat = (SvxDateFormat)( (int)( *aChar ) & 0x0f );
                            switch( eDateFormat )
                            {
                                case SVXDATEFORMAT_STDSMALL: ;
                                case SVXDATEFORMAT_A: ;     // 13.02.96
                                case SVXDATEFORMAT_B:       // 13.02.1996
                                    switch( eCurDateFormat )
                                    {
                                        case SVXDATEFORMAT_C: ;     // 13.Feb 1996
                                        case SVXDATEFORMAT_D:       // 13.February 1996
                                        case SVXDATEFORMAT_E: ;     // Tue, 13.February 1996
                                        case SVXDATEFORMAT_STDBIG: ;
                                        case SVXDATEFORMAT_F:       // Tuesday, 13.February 1996
                                            eDateFormat = eCurDateFormat;
                                            break;
                                        default:
                                            break;
                                    }
                                case SVXDATEFORMAT_C: ;     // 13.Feb 1996
                                case SVXDATEFORMAT_D:       // 13.February 1996
                                    switch( eCurDateFormat )
                                    {
                                        case SVXDATEFORMAT_E: ;     // Tue, 13.February 1996
                                        case SVXDATEFORMAT_STDBIG: ;
                                        case SVXDATEFORMAT_F:       // Tuesday, 13.February 1996
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
                        aRepresentation += B2UCONST( "0123456789.:/-APM" );

                        if( eDateFormat )
                        {
                            String sDate;
                            LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
                            SvNumberFormatter * pNumberFormatter = new SvNumberFormatter( ::comphelper::getProcessServiceFactory(), LANGUAGE_SYSTEM );
                            // We always collect the characters obtained by using the SVXDATEFORMAT_B (as: 13.02.1996)
                            // so we are sure to include any unusual day|month|year separator.
                            Date aDate( 1, 1, 1996 );
                            sDate += SvxDateField::GetFormatted( aDate, SVXDATEFORMAT_B, *pNumberFormatter, eLang );
                            switch( eDateFormat )
                            {
                                case SVXDATEFORMAT_E: ;     // Tue, 13.February 1996
                                case SVXDATEFORMAT_STDBIG: ;
                                case SVXDATEFORMAT_F:       // Tuesday, 13.February 1996
                                    for( sal_uInt16 i = 1; i <= 7; ++i )  // we get all days in a week
                                    {
                                        aDate.SetDay( i );
                                        sDate += SvxDateField::GetFormatted( aDate, eDateFormat, *pNumberFormatter, eLang );
                                    }
                                    // No break here! We need months too!
                                case SVXDATEFORMAT_C: ;     // 13.Feb 1996
                                case SVXDATEFORMAT_D:       // 13.February 1996
                                    for( sal_uInt16 i = 1; i <= 12; ++i ) // we get all months in a year
                                    {
                                        aDate.SetMonth( i );
                                        sDate += SvxDateField::GetFormatted( aDate, eDateFormat, *pNumberFormatter, eLang );
                                    }
                                    break;
                                case SVXDATEFORMAT_STDSMALL: ;
                                case SVXDATEFORMAT_A: ;     // 13.02.96
                                case SVXDATEFORMAT_B: ;     // 13.02.1996
                                default:
                                    // nothing to do here, we always collect the characters needed for these cases.
                                    break;
                            }
                            aRepresentation += sDate;
                        }
                    }
                }
                else if( pField->GetClassId() == text::textfield::Type::PAGE )
                {
                    switch( mVisiblePagePropSet.nPageNumberingType )
                    {
                        case SVX_CHARS_UPPER_LETTER:
                            aRepresentation += B2UCONST( "QWERTYUIOPASDFGHJKLZXCVBNM" );
                            break;
                        case SVX_CHARS_LOWER_LETTER:
                            aRepresentation += B2UCONST( "qwertyuiopasdfghjklzxcvbnm" );
                            break;
                        case SVX_ROMAN_UPPER:
                            aRepresentation += B2UCONST( "IVXLCDM" );
                            break;
                        case SVX_ROMAN_LOWER:
                            aRepresentation += B2UCONST( "ivxlcdm" );
                            break;
                        // arabic numbering type is the default
                        case SVX_ARABIC: ;
                        // in case the numbering type is not handled we fall back on arabic numbering
                        default:
                            aRepresentation += B2UCONST( "0123456789" );
                            break;
                    }
                }
                else
                {
                    bFieldProcessed = sal_False;
                }
                if( bFieldProcessed )
                {
                    if( pCharSet != NULL )
                    {
                        UCharSet::const_iterator aChar = pCharSet->begin();
                        for( ; aChar != pCharSet->end(); ++aChar )
                        {
                            aRepresentation += OUString::valueOf( *aChar );
                        }
                    }
                    pInfo->SetRepresentation( aRepresentation );
                }
            }
            else
            {
                bFieldProcessed = sal_False;
            }
        }
        else  // single page case
        {
            if( mVisiblePagePropSet.bAreBackgroundObjectsVisible )
            {
                const SvxFieldData* pField = pInfo->GetField().GetField();
                if( ( pField->GetClassId() == text::textfield::Type::PRESENTATION_HEADER ) && mVisiblePagePropSet.bIsHeaderFieldVisible )
                {
                    aRepresentation += mVisiblePagePropSet.sHeaderText;
                }
                else if( ( pField->GetClassId() == text::textfield::Type::PRESENTATION_FOOTER ) && mVisiblePagePropSet.bIsFooterFieldVisible )
                {
                    aRepresentation += mVisiblePagePropSet.sFooterText;
                }
                else if( ( pField->GetClassId() == text::textfield::Type::PRESENTATION_DATE_TIME ) && mVisiblePagePropSet.bIsDateTimeFieldVisible )
                {
                    // TODO: implement the variable case
                    aRepresentation += mVisiblePagePropSet.sDateTimeText;
                }
                else if( ( pField->GetClassId() == text::textfield::Type::PAGE ) && mVisiblePagePropSet.bIsPageNumberFieldVisible )
                {
                    sal_Int16 nPageNumber = mVisiblePagePropSet.nPageNumber;
                    switch( mVisiblePagePropSet.nPageNumberingType )
                    {
                        case SVX_CHARS_UPPER_LETTER:
                            aRepresentation += OUString::valueOf( (sal_Unicode)(char)( ( nPageNumber - 1 ) % 26 + 'A' ) );
                            break;
                        case SVX_CHARS_LOWER_LETTER:
                            aRepresentation += OUString::valueOf( (sal_Unicode)(char)( ( nPageNumber - 1 ) % 26 + 'a' ) );
                            break;
                        case SVX_ROMAN_UPPER:
                            aRepresentation += SvxNumberFormat::CreateRomanString( nPageNumber, true /* upper */ );
                            break;
                        case SVX_ROMAN_LOWER:
                            aRepresentation += SvxNumberFormat::CreateRomanString( nPageNumber, false /* lower */ );
                            break;
                        // arabic numbering type is the default
                        case SVX_ARABIC: ;
                        // in case the numbering type is not handled we fall back on arabic numbering
                        default:
                            aRepresentation += OUString::valueOf( sal_Int32(nPageNumber) );
                            break;
                    }
                }
                else
                {
                    bFieldProcessed = sal_False;
                }
                if( bFieldProcessed )
                {
                    pInfo->SetRepresentation( aRepresentation );
                }
            }

        }
    }
    return ( bFieldProcessed ? 0 : maOldFieldHdl.Call( pInfo ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
