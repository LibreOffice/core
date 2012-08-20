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

#include <comphelper/TypeGeneration.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/XModule.hpp>
#include <com/sun/star/text/XTextRangeMover.hpp>
#include <com/sun/star/text/XFootnotesSettingsSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientationFormat.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
//undef to prevent error (from sfx2/docfile.cxx)
#undef SEQUENCE
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/text/UserFieldFormat.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextColumnSequence.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModel.hpp>
// #i28749#
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using namespace ::comphelper;

namespace comphelper
{
    void GenerateCppuType (
        CppuTypes eType, const com::sun::star::uno::Type*& pType )
    {
        switch( eType )
        {
            case CPPUTYPE_BOOLEAN:      pType = &::getBooleanCppuType();    break;
            case CPPUTYPE_INT8:         pType = &::getCppuType( (sal_Int8*)0 ); break;
            case CPPUTYPE_INT16:        pType = &::getCppuType( (sal_Int16*)0 );    break;
            case CPPUTYPE_INT32:        pType = &::getCppuType( (sal_Int32*)0 );    break;

            case CPPUTYPE_DOUBLE:       pType = &::getCppuType( (double*)0 ); break;
            case CPPUTYPE_FLOAT:        pType = &::getCppuType( (float*)0 );    break;
            case CPPUTYPE_OUSTRING:     pType = &::getCppuType( (OUString*)0 ); break;
            case CPPUTYPE_FONTSLANT:    pType = &::getCppuType( (FontSlant*)0 );    break;
            case CPPUTYPE_LOCALE:       pType = &::getCppuType( (Locale*)0 );   break;
            case CPPUTYPE_PROPERTYVALUE:pType = &::getCppuType( (Sequence<PropertyValue>*)0 );  break;
            case CPPUTYPE_PROPERTYVALUES:   pType = &::getCppuType( (Sequence<PropertyValues>*)0 ); break;
            case CPPUTYPE_BORDERLINE:   pType = &::getCppuType( (table::BorderLine*)0 );    break;
            case CPPUTYPE_BREAK:        pType = &::getCppuType( (style::BreakType*)0 ); break;
            case CPPUTYPE_GRAPHICLOC:   pType = &::getCppuType( (style::GraphicLocation*)0 );   break;
            case CPPUTYPE_DROPCAPFMT:   pType = &::getCppuType( (style::DropCapFormat*)0 ); break;
            case CPPUTYPE_LINESPACE:    pType = &::getCppuType( (style::LineSpacing*)0 );   break;

            case CPPUTYPE_AWTSIZE:      pType = &::getCppuType( (awt::Size*)0 );    break;
            case CPPUTYPE_SHADOWFMT:    pType = &::getCppuType( (table::ShadowFormat*)0 );  break;
            case CPPUTYPE_TBLCOLSEP:    pType = &::getCppuType( (Sequence<text::TableColumnSeparator>*)0 ); break;
            case CPPUTYPE_PNTSEQSEQ:    pType = &::getCppuType( (PointSequenceSequence*)0 );    break;
            case CPPUTYPE_DOCIDXMRK:    pType = &::getCppuType( (Sequence< Reference< XDocumentIndexMark > >*)0 );  break;
            case CPPUTYPE_SEQINT8:      pType = &::getCppuType( (Sequence<sal_Int8>*)0 );   break;
            case CPPUTYPE_SEQTABSTOP:   pType = &::getCppuType( (Sequence<style::TabStop>*)0 ); break;
            case CPPUTYPE_SEQANCHORTYPE:pType = &::getCppuType( (Sequence<text::TextContentAnchorType>*)0 ); break;
            case CPPUTYPE_SEQDEPTXTFLD: pType = &::getCppuType( (Sequence<Reference<XDependentTextField> >*)0); break;
            case CPPUTYPE_TXTCNTANCHOR: pType = &::getCppuType( (text::TextContentAnchorType*)0 );  break;
            case CPPUTYPE_WRAPTXTMODE:  pType = &::getCppuType( (text::WrapTextMode*)0 );   break;

            case CPPUTYPE_COLORMODE:    pType = &::getCppuType( (drawing::ColorMode*)0 ); break;
            case CPPUTYPE_PAGESTYLELAY: pType = &::getCppuType( (style::PageStyleLayout*)0 ); break;
            case CPPUTYPE_VERTALIGN:    pType = &::getCppuType( (style::VerticalAlignment*)0 ); break;
            case CPPUTYPE_TABLEBORDER:  pType = &::getCppuType( (table::TableBorder*)0 ); break;
            case CPPUTYPE_TABLEBORDER2: pType = &::getCppuType( (table::TableBorder2*)0 ); break;
            case CPPUTYPE_GRFCROP:      pType = &::getCppuType( (text::GraphicCrop*)0 ); break;
            case CPPUTYPE_SECTFILELNK:  pType = &::getCppuType( (text::SectionFileLink*)0 ); break;
            case CPPUTYPE_PAGENUMTYPE:  pType = &::getCppuType( (text::PageNumberType*)0); break;
            case CPPUTYPE_DATETIME:     pType = &::getCppuType( (util::DateTime*)0 ); break;
            case CPPUTYPE_DATE:         pType = &::getCppuType( (util::Date*)0 ); break;

            case CPPUTYPE_REFINTERFACE: pType = &::getCppuType( (Reference<XInterface>*)0 );    break;
            case CPPUTYPE_REFIDXREPL:   pType = &::getCppuType( (Reference<container::XIndexReplace>*)0 );  break;
            case CPPUTYPE_REFNAMECNT:   pType = &::getCppuType( (Reference<container::XNameContainer>*)0 ); break;
            case CPPUTYPE_REFTEXTFRAME: pType = &::getCppuType( (Reference<text::XTextFrame>*)0 );  break;
            case CPPUTYPE_REFTEXTSECTION:   pType = &::getCppuType( (Reference<text::XTextSection>*)0 );    break;
            case CPPUTYPE_REFFOOTNOTE:  pType = &::getCppuType( (Reference<text::XFootnote>*)0 );   break;
            case CPPUTYPE_REFTEXT:      pType = &::getCppuType( (Reference<text::XText>*)0 );   break;
            case CPPUTYPE_REFTEXTCOL:   pType = &::getCppuType( (Reference<text::XTextColumns>*)0 );    break;
            case CPPUTYPE_REFFORBCHARS: pType = &::getCppuType( (Reference<XForbiddenCharacters>*)0 ); break;
            case CPPUTYPE_REFIDXCNTNR:  pType = &::getCppuType( (Reference<XIndexContainer>*)0 ); break;
            case CPPUTYPE_REFTEXTCNTNT: pType = &::getCppuType( (Reference<XTextContent>*)0 ); break;
            case CPPUTYPE_REFBITMAP:    pType = &::getCppuType( (Reference<awt::XBitmap>*)0 ); break;
            case CPPUTYPE_REFNMREPLACE: pType = &::getCppuType( (Reference<container::XNameReplace>*)0 ); break;
            case CPPUTYPE_REFCELL:      pType = &::getCppuType( (Reference<table::XCell>*)0 ); break;
            case CPPUTYPE_REFDOCINDEX:  pType = &::getCppuType( (Reference<text::XDocumentIndex>*)0 ); break;
            case CPPUTYPE_REFDOCIDXMRK: pType = &::getCppuType( (Reference<text::XDocumentIndexMark>*)0 ); break;
            case CPPUTYPE_REFTXTFIELD:  pType = &::getCppuType( (Reference<text::XTextField>*)0 ); break;
            case CPPUTYPE_REFTXTRANGE:  pType = &::getCppuType( (Reference<text::XTextRange>*)0 ); break;
            case CPPUTYPE_REFTXTTABLE:  pType = &::getCppuType( (Reference<text::XTextTable>*)0 ); break;
            case CPPUTYPE_AWTPOINT:     pType = &::getCppuType( (awt::Point*)0 );    break;
            case CPPUTYPE_REFLIBCONTAINER:  pType = &::getCppuType( (Reference< script::XLibraryContainer >*)0);    break;
            case CPPUTYPE_OUSTRINGS:    pType = &::getCppuType( (Sequence< ::rtl::OUString >*)0);    break;
            case CPPUTYPE_SEQANY:           pType = &::getCppuType( (Sequence< uno::Any >*)0);    break;
            case CPPUTYPE_REFRESULTSET:     pType = &::getCppuType( (Reference< sdbc::XResultSet >*)0);    break;
            case CPPUTYPE_REFCONNECTION:    pType = &::getCppuType( (Reference< sdbc::XConnection >*)0);    break;
            case CPPUTYPE_REFMODEL:         pType = &::getCppuType( (Reference< frame::XModel >*)0);    break;
            case CPPUTYPE_REFCOMPONENT:     pType = &::getCppuType( (Reference< lang::XComponent >*)0 ); break;
            // #i28749#
            case CPPUTYPE_TRANSFORMATIONINHORIL2R:
            {
                pType = &::getCppuType( (drawing::HomogenMatrix3*)0 );
            }
            break;
            case CPPUTYPE_SEQNAMEDVALUE:    pType = &::getCppuType( (Sequence<beans::NamedValue>*)0 ); break;
            case CPPUTYPE_REFXGRAPHIC:      pType = &::getCppuType( (Reference< graphic::XGraphic >*)0); break;
            case CPPUTYPE_TABLEBORDERDISTANCES:     pType = &::getCppuType( (table::TableBorderDistances*)0 ); break;
            case CPPUTPYE_REFEMBEDDEDOBJECT:        pType = &embed::XEmbeddedObject::static_type(); break;
            case CPPUTYPE_LINESTYLE:        pType = &::getCppuType( (drawing::LineStyle*)0 ); break;
            default:
                OSL_FAIL( "Unknown CPPU type" );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
