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

#ifndef _OOX_EXPORT_DRAWINGML_HXX_
#define _OOX_EXPORT_DRAWINGML_HXX_

#include <oox/dllapi.h>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/uno/XReference.hpp>
#include <tools/poly.hxx>
#include <filter/msfilter/escherex.hxx>
#ifndef PPTX_EXPORT_ROTATE_CLOCKWISIFY
#define PPTX_EXPORT_ROTATE_CLOCKWISIFY(input) ((21600000-input*600)%21600000)
#endif

class Graphic;
class String;

namespace com { namespace sun { namespace star {
namespace beans {
    class XPropertySet;
    class XPropertyState;
}
namespace drawing {
    class XShape;
}
namespace style {
    struct LineSpacing;
}
namespace text {
    class XTextContent;
    class XTextRange;
}
namespace io {
    class XOutputStream;
}
}}}

namespace oox {
namespace core {
    class XmlFilterBase;
}

namespace drawingml {

class OOX_DLLPUBLIC DrawingML {
public:
    enum DocumentType { DOCUMENT_DOCX, DOCUMENT_PPTX, DOCUMENT_XLSX };

private:
    static int mnImageCounter;

    /// To specify where write eg. the images to (like 'ppt', or 'word' - according to the OPC).
    DocumentType meDocumentType;

protected:
    ::com::sun::star::uno::Any mAny;
    ::sax_fastparser::FSHelperPtr mpFS;
    ::oox::core::XmlFilterBase* mpFB;

    bool GetProperty( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, OUString aName );
    bool GetPropertyAndState( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet,
                  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState > rXPropState,
                  String aName, ::com::sun::star::beans::PropertyState& eState );
    const char* GetFieldType( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun, sal_Bool& bIsField );

    rtl::OUString WriteImage( const rtl::OUString& rURL );

    const char* GetComponentDir();
    const char* GetRelationCompPrefix();

public:
    DrawingML( ::sax_fastparser::FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFB = NULL, DocumentType eDocumentType = DOCUMENT_PPTX ) : meDocumentType( eDocumentType ), mpFS( pFS ), mpFB( pFB ) {}
    void SetFS( ::sax_fastparser::FSHelperPtr pFS ) { mpFS = pFS; }
    ::sax_fastparser::FSHelperPtr GetFS() { return mpFS; }
    ::oox::core::XmlFilterBase* GetFB() { return mpFB; }
    DocumentType GetDocumentType() { return meDocumentType; }

    rtl::OUString WriteImage( const Graphic &rGraphic );

    void WriteColor( sal_uInt32 nColor );
    void WriteGradientStop( sal_uInt16 nStop, sal_uInt32 nColor );
    void WriteLineArrow( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, sal_Bool bLineStart );
    void WriteConnectorConnections( EscherConnectorListEntry& rConnectorEntry, sal_Int32 nStartID, sal_Int32 nEndID );

    void WriteSolidFill( sal_uInt32 nColor );
    void WriteSolidFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteGradientFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteBlipFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, OUString sURLPropName, sal_Int32 nXmlNamespace );
    void WriteBlipFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, OUString sURLPropName );
    void WriteSrcRect( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, const OUString& );
    void WriteOutline( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteStretch();
    void WriteLinespacing( ::com::sun::star::style::LineSpacing& rLineSpacing );

    ::rtl::OUString WriteBlip( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, ::rtl::OUString& rURL, const Graphic *pGraphic=NULL );
    void WriteBlipMode( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );

    void WriteShapeTransformation( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > rXShape,
                  sal_Int32 nXmlNamespace, sal_Bool bFlipH = false, sal_Bool bFlipV = false, sal_Bool bSuppressRotation = false );
    void WriteTransformation( const Rectangle& rRectangle,
                  sal_Int32 nXmlNamespace, sal_Bool bFlipH = false, sal_Bool bFlipV = false, sal_Int32 nRotation = 0 );

    void WriteText( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > rXIface );
    void WriteParagraph( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > rParagraph );
    void WriteParagraphProperties( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > rParagraph );
    void WriteParagraphNumbering( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet,
                                  sal_Int16 nLevel );
    void WriteRun( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun );
    void WriteRunProperties( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rRun, sal_Bool bIsField );

    void WritePresetShape( const char* pShape );
    void WritePresetShape( const char* pShape, MSO_SPT eShapeType, sal_Bool bPredefinedHandlesUsed, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, const ::com::sun::star::beans::PropertyValue& rProp );
    void WritePolyPolygon( const PolyPolygon& rPolyPolygon );
    void WriteFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet );

    static void ResetCounters();

    void GetUUID( ::rtl::OStringBuffer& rBuffer );

    static sal_Unicode SubstituteBullet( sal_Unicode cBulletId, ::com::sun::star::awt::FontDescriptor& rFontDesc );

    sal_uInt32 ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity );

    static const char* GetAlignment( sal_Int32 nAlignment );

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const ::rtl::OUString& sFullStream,
                                        const ::rtl::OUString& sRelativeStream,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xParentRelation,
                                        const char* sContentType,
                                        const char* sRelationshipType,
                                        ::rtl::OUString* pRelationshipId = NULL );

};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
