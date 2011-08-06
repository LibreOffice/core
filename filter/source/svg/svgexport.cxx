/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svgexport.cxx,v $
 * $Revision: 1.12.62.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#define ITEMID_FIELD 0

#include "svgwriter.hxx"
#include "svgfontexport.hxx"
#include "svgfilter.hxx"
#include "svgscript.hxx"
#include "impsvgdialog.hxx"

#include <svtools/FilterConfigItem.hxx>
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

#include <boost/preprocessor/repetition/repeat.hpp>


using ::rtl::OUString;




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
    const Sequence< PropertyValue >& rFilterData ) :
        SvXMLExport( xServiceFactory, MAP_100TH_MM ),
        mrFilterData( rFilterData )
{
    SetDocHandler( rxHandler );
    sal_uInt16 nExportFlags = getExportFlags() | EXPORT_PRETTY;
    setExportFlags( nExportFlags );
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

    if( mrFilterData.getLength() > 0 )
        mrFilterData[ 0 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::IsEmbedFonts() const
{
    sal_Bool bRet = sal_False;

    if( mrFilterData.getLength() > 1 )
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

::rtl::OUString SVGExport::GetGlyphPlacement() const
{
    ::rtl::OUString aRet;

    if( mrFilterData.getLength() > 3 )
        mrFilterData[ 3 ].Value >>= aRet;
    else
        aRet = B2UCONST( "abs" );

    return aRet;
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

sal_Bool SVGExport::IsUseGradient() const
{
    sal_Bool bRet = !IsUseTinyProfile();

    if( !bRet && ( mrFilterData.getLength() > 5 ) )
        mrFilterData[ 5 ].Value >>= bRet;

    return bRet;
}

// -----------------------------------------------------------------------------

void SVGExport::pushClip( const ::basegfx::B2DPolyPolygon& rPolyPoly )
{
    maClipList.push_front( ::basegfx::tools::correctOrientations( rPolyPoly ) );
}

// -----------------------------------------------------------------------------

void SVGExport::popClip()
{
    if( !maClipList.empty() )
        maClipList.pop_front();
}

// -----------------------------------------------------------------------------

sal_Bool SVGExport::hasClip() const
{
    return( !maClipList.empty() );
}

// -----------------------------------------------------------------------------

const ::basegfx::B2DPolyPolygon* SVGExport::getCurClip() const
{
    return( maClipList.empty() ? NULL : &( *maClipList.begin() ) );
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

    mnMasterSlideId = mnSlideId = mnDrawingGroupId = mnDrawingId = 0;
    maFilterData.realloc( 0 );

    for ( sal_Int32 i = 0 ; i < nLength; ++i)
    {
        if( pValue[ i ].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OutputStream" ) ) )
            pValue[ i ].Value >>= xOStm;
        else if( pValue[ i ].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FileName" ) ) )
        {
            ::rtl::OUString aFileName;

            pValue[ i ].Value >>= aFileName;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC );

            if( pOStm )
                xOStm = Reference< XOutputStream >( new ::utl::OOutputStreamWrapper ( *pOStm ) );
        }
        else if( pValue[ i ].Name.equalsAscii( "FilterData" ) )
        {
            pValue[ i ].Value >>= maFilterData;
        }
    }

    // if no filter data is given use stored/prepared ones
    if( !maFilterData.getLength() )
    {
#ifdef _SVG_USE_CONFIG
        FilterConfigItem aCfgItem( String( RTL_CONSTASCII_USTRINGPARAM( SVG_EXPORTFILTER_CONFIGPATH ) ) );

        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_TINYPROFILE ) ), sal_True );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_EMBEDFONTS ) ), sal_True );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_NATIVEDECORATION ) ), sal_False );
        aCfgItem.ReadString( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_NATIVEDECORATION ) ), B2UCONST( "xlist" ) );
        aCfgItem.ReadString( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_OPACITY ) ), sal_True );
        aCfgItem.ReadString( String( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_GRADIENT ) ), sal_True );

        maFilterData = aCfgItem.GetFilterData();
#else
        maFilterData.realloc( 6 );

        maFilterData[ 0 ].Name = B2UCONST( SVG_PROP_TINYPROFILE );
        maFilterData[ 0 ].Value <<= (sal_Bool) sal_True;

        // font embedding
        const char* pSVGDisableFontEmbedding = getenv( "SVG_DISABLE_FONT_EMBEDDING" );

        maFilterData[ 1 ].Name = B2UCONST( SVG_PROP_EMBEDFONTS );
        maFilterData[ 1 ].Value <<= (sal_Bool) ( pSVGDisableFontEmbedding ? sal_False : sal_True );

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
#endif
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
                delete mpSVGExport, mpSVGExport = NULL;
                delete mpSVGFontExport, mpSVGFontExport = NULL;
                delete mpObjects, mpObjects = NULL;
                mbPresentation = sal_False;
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

inline
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

                    Reference< XMasterPageTarget > xMasterTarget( xDrawPage, UNO_QUERY );
                    if( xMasterTarget.is() )
                    {
                        Reference< XDrawPage > xMasterPage( xMasterTarget->getMasterPage() );

                        for( sal_Int32 nMaster = 0, nMasterCount = mMasterPageTargets.getLength();
                             ( nMaster < nMasterCount ) && ( -1 == mnVisibleMasterPage );
                             ++nMaster )
                        {
                            const Reference< XDrawPage > & xMasterTestPage = mMasterPageTargets[nMaster];

                            if( xMasterTestPage.is() && xMasterTestPage == xMasterPage )
                                mnVisibleMasterPage = nMaster;
                        }
                    }
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

    mbSinglePage = (nLastPage == 0);
    mnVisiblePage = -1;
    mnVisibleMasterPage = -1;

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
#undef _SVG_WRITE_EXTENTS
#ifdef _SVG_WRITE_EXTENTS
    aAttr = OUString::valueOf( nDocWidth * 0.01 );
    aAttr += B2UCONST( "mm" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

    aAttr = OUString::valueOf( nDocHeight * 0.01 );
    aAttr += B2UCONST( "mm" );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );
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

    if( !mbSinglePage )
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", B2UCONST( "http://xml.openoffice.org/svg/export" ) );
    }


    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns", B2UCONST( "http://www.w3.org/2000/svg" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:xlink", B2UCONST( "http://www.w3.org/1999/xlink" ) );

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xml:space", B2UCONST( "preserve" ) );

    mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", sal_True, sal_True );

    // Create a ClipPath element that will be used for cutting bitmaps and other elements that could exceed the page margins.
    {
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

            implExportPages( mMasterPageTargets, 0, mMasterPageTargets.getLength() - 1, mnVisibleMasterPage, sal_True /* is a master page */ );
            implExportPages( mSelectedPages, 0, nLastPage, mnVisiblePage, sal_False /* is not a master page */ );

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
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", B2UCONST( aOOOElemMetaSlides ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, aOOOAttrNumberOfSlides, OUString::valueOf( nCount ) );

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
                        sal_Bool bHeaderVisibility                    = sal_True;     // default: visible
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
                        if( i == mnVisiblePage )
                        {
                            mVisiblePagePropSet.bIsBackgroundVisible = bBackgroundVisibility;
                            mVisiblePagePropSet.bAreBackgroundObjectsVisible = bBackgroundObjectsVisibility;
                            mVisiblePagePropSet.bIsPageNumberFieldVisible = bPageNumberVisibility;
                            mVisiblePagePropSet.bIsDateTimeFieldVisible = bDateTimeVisibility;
                            mVisiblePagePropSet.bIsFooterFieldVisible = bFooterVisibility;
                            mVisiblePagePropSet.bIsHeaderFieldVisible = bHeaderVisibility;
                            mVisiblePagePropSet.nPageNumberingType = nPageNumberingType;
                            mVisiblePagePropSet.bIsDateTimeFieldFixed = bDateTimeFixed;
                            mVisiblePagePropSet.nDateTimeFormat = aVariableDateTimeField.format;
                            mVisiblePagePropSet.sDateTimeText = aFixedDateTimeField.text;
                            mVisiblePagePropSet.sFooterText = aFooterField.text;
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

                    aFieldSet[i]->growCharSet( mTextFieldCharSets );
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

sal_Bool SVGFilter::implExportPages( const SVGFilter::XDrawPageSequence & rxPages,
                                     sal_Int32 nFirstPage, sal_Int32 nLastPage,
                                     sal_Int32 nVisiblePage, sal_Bool bMaster )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportPages: nFirstPage > nLastPage" );

    sal_Bool bRet = sal_False;

    for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
    {
        if( rxPages[i].is() )
        {
            Reference< XShapes > xShapes( rxPages[i], UNO_QUERY );

            if( xShapes.is() )
            {
                // add id attribute
                OUString sPageId = implGetValidIDFromInterface( xShapes );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", sPageId );

                {
                    {
                        Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        if( xExtDocHandler.is() )
                        {
                            OUString aDesc;

                            if( bMaster )
                                aDesc = B2UCONST( "Master_Slide" );
                            else
                                aDesc = B2UCONST( "Slide" );

                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aDesc );
                        }
                    }

                    // We don't set a visibility attribute for a master page element
                    // as the visibility of each master page sub element (background,
                    // placeholder shapes, background objects) is managed separately.
                    OUString aAttrVisibilityValue;
                    if( !bMaster )
                    {
                        if( i == nVisiblePage )
                            aAttrVisibilityValue = B2UCONST( "visible" );
                        else
                            aAttrVisibilityValue = B2UCONST( "hidden" );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", aAttrVisibilityValue );
                    }
                    else
                    {   // when we export the shapes of a master page (implExportShape) we need
                        // to know if it is the master page targeted by the initially visible slide
                        mbIsPageVisible = ( i == nVisiblePage );
                    }

                    // Adding a clip path to each exported slide and master page,
                    // so in case bitmaps or other elements exceed the slide margins
                    // they are trimmed, even when they are shown inside a thumbnail view.
                    OUString sClipPathAttrValue = B2UCONST( "url(#" ) + msClipPathId + B2UCONST( ")" );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "clip-path", sClipPathAttrValue );

                    // insert the <g> open tag related to the Slide/Master_Slide
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );

                    // In case the page has a background object we append it .
                    if( (mpObjects->find( rxPages[i] ) != mpObjects->end()) )
                    {
                        const GDIMetaFile& rMtf = (*mpObjects)[ rxPages[i] ].GetRepresentation();
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
                            if( bMaster )
                            {
                                if( i == nVisiblePage && mVisiblePagePropSet.bIsBackgroundVisible )
                                    aAttrVisibilityValue = B2UCONST( "visible" );
                                else
                                    aAttrVisibilityValue = B2UCONST( "hidden" );
                                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", aAttrVisibilityValue );
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

                        if( i == nVisiblePage && mVisiblePagePropSet.bAreBackgroundObjectsVisible )
                            aAttrVisibilityValue = B2UCONST( "visible" );
                        else
                            aAttrVisibilityValue = B2UCONST( "hidden" );
                        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", aAttrVisibilityValue );
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
            }
        }
    }

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
                {   // for text field shapes we set up visibility and text-adjust attributes
                    // TODO should we set up visibility for all text field shapes to hidden at start ?
                    OUString aShapeClass = implGetClassFromShape( rxShape );
                    if( mbPresentation )
                    {
                        sal_Bool bIsPageNumber = aShapeClass.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Slide_Number" ) );
                        sal_Bool bIsFooter = aShapeClass.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Footer" ) );
                        sal_Bool bIsDateTime = aShapeClass.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Date/Time" ) );
                        if( bIsPageNumber || bIsDateTime || bIsFooter )
                        {
                            // to notify to the SVGActionWriter::ImplWriteActions method
                            // that we are dealing with a placeholder shape
                            pElementId = &sPlaceholderTag;

                            // if the text field does not belong to the visible page its svg:visibility
                            // attribute is set to 'hidden'; else it depends on the related property of the visible page
                            OUString aAttrVisibilityValue( B2UCONST( "hidden" ) );
                            if(mbIsPageVisible && mVisiblePagePropSet.bAreBackgroundObjectsVisible && (
                                    ( bIsPageNumber && mVisiblePagePropSet.bIsPageNumberFieldVisible ) ||
                                    ( bIsDateTime && mVisiblePagePropSet.bIsDateTimeFieldVisible ) ||
                                    ( bIsFooter && mVisiblePagePropSet.bIsFooterFieldVisible ) ) )
                            {
                                aAttrVisibilityValue = B2UCONST( "visible" );
                            }
                            mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", aAttrVisibilityValue );

                            if( !mbSinglePage )
                            {
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
                        }
                    }
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "class", aShapeClass );
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
                    OUString    aId( B2UCONST( "DrawingGroup_" ) );
                    OUString    aObjName( implGetValidIDFromInterface( rxShape, true ) ), aObjDesc;

                    Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                    OUString aTitle;
                    xShapePropSet->getPropertyValue( B2UCONST( "Title" ) ) >>= aTitle;
                    if( aTitle.getLength() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "title", sal_True, sal_True );
                        xExtDocHandler->characters( aTitle );
                    }

                    OUString aDescription;
                    xShapePropSet->getPropertyValue( B2UCONST( "Description" ) ) >>= aDescription;
                    if( aDescription.getLength() )
                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", sal_True, sal_True );
                        xExtDocHandler->characters( aDescription );
                    }

                    if( aObjName.getLength() )
                        ( ( aId += B2UCONST( "(" ) ) += aObjName ) += B2UCONST( ")" );

                    {
                        SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", sal_True, sal_True );
                        mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_ALL, pElementId );
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
                implCreateObjectsFromShapes( xShapes );
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
                implCreateObjectsFromShapes( xShapes );
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShapes( const Reference< XShapes >& rxShapes )
{
    Reference< XShape > xShape;
    sal_Bool            bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implCreateObjectsFromShape( xShape ) || bRet;

        xShape = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShape( const Reference< XShape >& rxShape )
{
    sal_Bool bRet = sal_False;
    if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
    {
        Reference< XShapes > xShapes( rxShape, UNO_QUERY );

        if( xShapes.is() )
            bRet = implCreateObjectsFromShapes( xShapes );
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
                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aGraphic.GetGDIMetaFile() );

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

OUString SVGFilter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf, sal_Bool bUnique )
{
    Reference< XNamed > xNamed( rxIf, UNO_QUERY );
    OUString            aRet;

    if( xNamed.is() )
    {
        aRet = xNamed->getName().replace( ' ', '_' ).
               replace( ':', '_' ).
               replace( ',', '_' ).
               replace( ';', '_' ).
               replace( '&', '_' ).
               replace( '!', '_' ).
               replace( '|', '_' );
    }

    if( ( aRet.getLength() > 0 ) && bUnique )
    {
        while( ::std::find( maUniqueIdVector.begin(), maUniqueIdVector.end(), aRet ) != maUniqueIdVector.end() )
        {
            aRet += B2UCONST( "_" );
        }

        maUniqueIdVector.push_back( aRet );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SVGFilter, CalcFieldHdl, EditFieldInfo*, pInfo )
{
    sal_Bool       bFieldProcessed = sal_False;
    if( pInfo && mbPresentation )
    {
        bFieldProcessed = true;
        // to notify to the SVGActionWriter::ImplWriteText method
        // that we are dealing with a placeholder shape
        OUString   aRepresentation = sPlaceholderTag;

        if( !mbSinglePage )
        {
            if( !mCreateOjectsCurrentMasterPage.is() )
            {
                OSL_FAIL( "error: !mCreateOjectsCurrentMasterPage.is()" );
                return 0;
            }
            if( mTextFieldCharSets.find( mCreateOjectsCurrentMasterPage ) == mTextFieldCharSets.end() )
            {
                OSL_FAIL( "error: mTextFieldCharSets.find( mCreateOjectsCurrentMasterPage ) == mTextFieldCharSets.end()" );
                return 0;
            }

            static const ::rtl::OUString aHeaderId( B2UCONST( aOOOAttrHeaderField ) );
            static const ::rtl::OUString aFooterId( B2UCONST( aOOOAttrFooterField ) );
            static const ::rtl::OUString aDateTimeId( B2UCONST( aOOOAttrDateTimeField ) );
            static const ::rtl::OUString aVariableDateTimeId( B2UCONST( aOOOAttrDateTimeField ) + B2UCONST( "-variable" ) );

            const UCharSet * pCharSet = NULL;
            UCharSetMap & aCharSetMap = mTextFieldCharSets[ mCreateOjectsCurrentMasterPage ];
            const SvxFieldData* pField = pInfo->GetField().GetField();
            if( ( pField->GetClassId() == SVX_HEADERFIELD ) && ( aCharSetMap.find( aHeaderId ) != aCharSetMap.end() ) )
            {
                pCharSet = &( aCharSetMap[ aHeaderId ] );
            }
            else if( ( pField->GetClassId() == SVX_FOOTERFIELD ) && ( aCharSetMap.find( aFooterId ) != aCharSetMap.end() ) )
            {
                pCharSet = &( aCharSetMap[ aFooterId ] );
            }
            else if( pField->GetClassId() == SVX_DATEFIMEFIELD )
            {
                if( aCharSetMap.find( aDateTimeId ) != aCharSetMap.end() )
                {
                    pCharSet = &( aCharSetMap[ aDateTimeId ] );
                }
                if( ( aCharSetMap.find( aVariableDateTimeId ) != aCharSetMap.end() ) && !aCharSetMap[ aVariableDateTimeId ].empty() )
                {
                    SvxDateFormat eDateFormat = SVXDATEFORMAT_B, eCurDateFormat;
                    const UCharSet & aCharSet = aCharSetMap[ aVariableDateTimeId ];
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
            else if( pField->GetClassId() == SVX_PAGEFIELD )
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
        else
        {
            if( mVisiblePagePropSet.bAreBackgroundObjectsVisible )
            {
                const SvxFieldData* pField = pInfo->GetField().GetField();
                if( ( pField->GetClassId() == SVX_HEADERFIELD ) && mVisiblePagePropSet.bIsHeaderFieldVisible )
                {
                    aRepresentation += mVisiblePagePropSet.sHeaderText;
                }
                else if( ( pField->GetClassId() == SVX_FOOTERFIELD ) && mVisiblePagePropSet.bIsFooterFieldVisible )
                {
                    aRepresentation += mVisiblePagePropSet.sFooterText;
                }
                else if( ( pField->GetClassId() == SVX_DATEFIMEFIELD ) && mVisiblePagePropSet.bIsDateTimeFieldVisible )
                {
                    // TODO: implement the variable case
                    aRepresentation += mVisiblePagePropSet.sDateTimeText;
                }
                else if( ( pField->GetClassId() == SVX_PAGEFIELD ) && mVisiblePagePropSet.bIsPageNumberFieldVisible )
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
            }

            pInfo->SetRepresentation( aRepresentation );
        }
    }
    return ( bFieldProcessed ? 0 : maOldFieldHdl.Call( pInfo ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
