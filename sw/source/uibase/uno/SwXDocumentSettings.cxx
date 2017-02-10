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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <SwXDocumentSettings.hxx>
#include <comphelper/MasterPropertySetInfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docsh.hxx>
#include <fldupde.hxx>
#include <linkenum.hxx>
#include <sfx2/printer.hxx>
#include <editsh.hxx>
#include <drawdoc.hxx>
#include <svl/zforlist.hxx>
#include <unotxdoc.hxx>
#include <cmdid.h>
#include <sfx2/zoomitem.hxx>
#include <unomod.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>

#include "swmodule.hxx"
#include "cfgitems.hxx"
#include "prtopt.hxx"
#include "dbmgr.hxx"

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

enum SwDocumentSettingsPropertyHandles
{
    HANDLE_FORBIDDEN_CHARS,
    HANDLE_LINK_UPDATE_MODE,
    HANDLE_FIELD_AUTO_UPDATE,
    HANDLE_CHART_AUTO_UPDATE,
    HANDLE_ADD_PARA_TABLE_SPACING,
    HANDLE_ADD_PARA_TABLE_SPACING_AT_START,
    HANDLE_ALIGN_TAB_STOP_POSITION,
    HANDLE_PRINTER_NAME,
    HANDLE_PRINTER_SETUP,
    HANDLE_IS_KERN_ASIAN_PUNCTUATION,
    HANDLE_CHARACTER_COMPRESSION_TYPE,
    HANDLE_APPLY_USER_DATA,
    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,
    HANDLE_CURRENT_DATABASE_DATA_SOURCE,
    HANDLE_CURRENT_DATABASE_COMMAND,
    HANDLE_CURRENT_DATABASE_COMMAND_TYPE,
    HANDLE_EMBEDDED_DATABASE_NAME,
    HANDLE_SAVE_VERSION_ON_CLOSE,
    HANDLE_UPDATE_FROM_TEMPLATE,
    HANDLE_PRINTER_INDEPENDENT_LAYOUT,
    HANDLE_IS_LABEL_DOC,
    HANDLE_IS_ADD_FLY_OFFSET,
    HANDLE_IS_ADD_VERTICAL_FLY_OFFSET,
    HANDLE_IS_ADD_EXTERNAL_LEADING,
    HANDLE_OLD_NUMBERING,
    HANDLE_OUTLINELEVEL_YIELDS_NUMBERING,
    /* Stampit It disable the print cancel button of the shown progress dialog. */
    HANDLE_ALLOW_PRINTJOB_CANCEL,
    HANDLE_USE_FORMER_LINE_SPACING,
    HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS,
    HANDLE_USE_FORMER_OBJECT_POSITIONING,
    HANDLE_USE_FORMER_TEXT_WRAPPING,
    HANDLE_CHANGES_PASSWORD,
    HANDLE_CONSIDER_WRAP_ON_OBJPOS,
    HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING,
    HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK,
    HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT,
    HANDLE_TABLE_ROW_KEEP,
    HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION,
    HANDLE_LOAD_READONLY,
    HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE,
    HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES,
    HANDLE_UNIX_FORCE_ZERO_EXT_LEADING,
    HANDLE_USE_OLD_PRINTER_METRICS,
    HANDLE_PROTECT_FORM,
    HANDLE_MS_WORD_COMP_TRAILING_BLANKS,
    HANDLE_TABS_RELATIVE_TO_INDENT,
    HANDLE_RSID,
    HANDLE_RSID_ROOT,
    HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST,
    HANDLE_MODIFYPASSWORDINFO,
    HANDLE_MATH_BASELINE_ALIGNMENT,
    HANDLE_INVERT_BORDER_SPACING,
    HANDLE_COLLAPSE_EMPTY_CELL_PARA,
    HANDLE_SMALL_CAPS_PERCENTAGE_66,
    HANDLE_TAB_OVERFLOW,
    HANDLE_UNBREAKABLE_NUMBERINGS,
    HANDLE_STYLES_NODEFAULT,
    HANDLE_FLOATTABLE_NOMARGINS,
    HANDLE_CLIPPED_PICTURES,
    HANDLE_BACKGROUND_PARA_OVER_DRAWINGS,
    HANDLE_EMBED_FONTS,
    HANDLE_EMBED_SYSTEM_FONTS,
    HANDLE_TAB_OVER_MARGIN,
    HANDLE_TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK,
    HANDLE_SURROUND_TEXT_WRAP_SMALL,
    HANDLE_APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING,
    HANDLE_PROP_LINE_SPACING_SHRINKS_FIRST_LINE,
    HANDLE_SUBTRACT_FLYS,
};

static MasterPropertySetInfo * lcl_createSettingsInfo()
{
    static PropertyInfo const aWriterSettingsInfoMap[] =
    {
        { OUString("ForbiddenCharacters"),        HANDLE_FORBIDDEN_CHARS,                 cppu::UnoType<css::i18n::XForbiddenCharacters>::get(),      0},
        { OUString("LinkUpdateMode"),             HANDLE_LINK_UPDATE_MODE,                cppu::UnoType<sal_Int16>::get(),             0},
        { OUString("FieldAutoUpdate"),            HANDLE_FIELD_AUTO_UPDATE,               cppu::UnoType<bool>::get(),           0},
        { OUString("ChartAutoUpdate"),            HANDLE_CHART_AUTO_UPDATE,               cppu::UnoType<bool>::get(),           0},
        { OUString("AddParaTableSpacing"),        HANDLE_ADD_PARA_TABLE_SPACING,          cppu::UnoType<bool>::get(),           0},
        { OUString("AddParaTableSpacingAtStart"), HANDLE_ADD_PARA_TABLE_SPACING_AT_START, cppu::UnoType<bool>::get(),           0},
        { OUString("AlignTabStopPosition"),       HANDLE_ALIGN_TAB_STOP_POSITION,         cppu::UnoType<bool>::get(),           0},
        { OUString("PrinterName"),                HANDLE_PRINTER_NAME,                    cppu::UnoType<OUString>::get(),          0},
        { OUString("PrinterSetup"),               HANDLE_PRINTER_SETUP,                   cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(),           0},
        { OUString("IsKernAsianPunctuation"),     HANDLE_IS_KERN_ASIAN_PUNCTUATION,       cppu::UnoType<bool>::get(),           0},
        { OUString("CharacterCompressionType"),   HANDLE_CHARACTER_COMPRESSION_TYPE,      cppu::UnoType<sal_Int16>::get(),             0},
        { OUString("ApplyUserData"),              HANDLE_APPLY_USER_DATA,                 cppu::UnoType<bool>::get(),           0},
        { OUString("SaveGlobalDocumentLinks"),    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,      cppu::UnoType<bool>::get(),           0},
        { OUString("CurrentDatabaseDataSource"),  HANDLE_CURRENT_DATABASE_DATA_SOURCE,    cppu::UnoType<OUString>::get(),          0},
        { OUString("CurrentDatabaseCommand"),     HANDLE_CURRENT_DATABASE_COMMAND,        cppu::UnoType<OUString>::get(),          0},
        { OUString("CurrentDatabaseCommandType"), HANDLE_CURRENT_DATABASE_COMMAND_TYPE,   cppu::UnoType<sal_Int32>::get(),             0},
        { OUString("EmbeddedDatabaseName"),       HANDLE_EMBEDDED_DATABASE_NAME,          cppu::UnoType<OUString>::get(),              0},
        { OUString("SaveVersionOnClose"),         HANDLE_SAVE_VERSION_ON_CLOSE,           cppu::UnoType<bool>::get(),           0},
        { OUString("UpdateFromTemplate"),         HANDLE_UPDATE_FROM_TEMPLATE,            cppu::UnoType<bool>::get(),           0},

        { OUString("PrinterIndependentLayout"),   HANDLE_PRINTER_INDEPENDENT_LAYOUT,      cppu::UnoType<sal_Int16>::get(),             0},
        { OUString("IsLabelDocument"),            HANDLE_IS_LABEL_DOC,                    cppu::UnoType<bool>::get(),           0},
        { OUString("AddFrameOffsets"),            HANDLE_IS_ADD_FLY_OFFSET,               cppu::UnoType<bool>::get(),           0},
        { OUString("AddVerticalFrameOffsets"),    HANDLE_IS_ADD_VERTICAL_FLY_OFFSET,      cppu::UnoType<bool>::get(),           0},
        { OUString("AddExternalLeading"),         HANDLE_IS_ADD_EXTERNAL_LEADING,         cppu::UnoType<bool>::get(),           0},
        { OUString("UseOldNumbering"),            HANDLE_OLD_NUMBERING,                   cppu::UnoType<bool>::get(),           0},
        { OUString("OutlineLevelYieldsNumbering"), HANDLE_OUTLINELEVEL_YIELDS_NUMBERING, cppu::UnoType<bool>::get(),           0},
        /* Stampit It disable the print cancel button of the shown progress dialog. */
        { OUString("AllowPrintJobCancel"),        HANDLE_ALLOW_PRINTJOB_CANCEL,           cppu::UnoType<bool>::get(),           0},
        { OUString("UseFormerLineSpacing"),       HANDLE_USE_FORMER_LINE_SPACING,         cppu::UnoType<bool>::get(),           0},
        { OUString("AddParaSpacingToTableCells"), HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS, cppu::UnoType<bool>::get(),           0},
        { OUString("UseFormerObjectPositioning"), HANDLE_USE_FORMER_OBJECT_POSITIONING,   cppu::UnoType<bool>::get(),           0},
        { OUString("UseFormerTextWrapping"),      HANDLE_USE_FORMER_TEXT_WRAPPING,        cppu::UnoType<bool>::get(),           0},
        { OUString("RedlineProtectionKey"),       HANDLE_CHANGES_PASSWORD,                cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(),           0},
        { OUString("ConsiderTextWrapOnObjPos"),   HANDLE_CONSIDER_WRAP_ON_OBJPOS,         cppu::UnoType<bool>::get(),           0},
        { OUString("IgnoreFirstLineIndentInNumbering"),   HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING,         cppu::UnoType<bool>::get(),           0},
        { OUString("DoNotJustifyLinesWithManualBreak"),   HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK,         cppu::UnoType<bool>::get(),           0},
        { OUString("DoNotResetParaAttrsForNumFont"),   HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT,         cppu::UnoType<bool>::get(),           0},
        { OUString("TableRowKeep"),               HANDLE_TABLE_ROW_KEEP,         cppu::UnoType<bool>::get(),           0},
        { OUString("IgnoreTabsAndBlanksForLineCalculation"),   HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION,         cppu::UnoType<bool>::get(),           0},
        { OUString("LoadReadonly"),               HANDLE_LOAD_READONLY,                   cppu::UnoType<bool>::get(),           0},
        { OUString("DoNotCaptureDrawObjsOnPage"),   HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, cppu::UnoType<bool>::get(), 0},
        { OUString("ClipAsCharacterAnchoredWriterFlyFrames"), HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES, cppu::UnoType<bool>::get(), 0},
        { OUString("UnxForceZeroExtLeading"), HANDLE_UNIX_FORCE_ZERO_EXT_LEADING, cppu::UnoType<bool>::get(), 0},
        { OUString("UseOldPrinterMetrics"), HANDLE_USE_OLD_PRINTER_METRICS, cppu::UnoType<bool>::get(), 0},
        { OUString("TabsRelativeToIndent"), HANDLE_TABS_RELATIVE_TO_INDENT, cppu::UnoType<bool>::get(), 0},
        { OUString("Rsid"), HANDLE_RSID, cppu::UnoType<sal_Int32>::get(), 0},
        { OUString("RsidRoot"), HANDLE_RSID_ROOT, cppu::UnoType<sal_Int32>::get(), 0},
        { OUString("ProtectForm"), HANDLE_PROTECT_FORM, cppu::UnoType<bool>::get(), 0},
        { OUString("MsWordCompTrailingBlanks"), HANDLE_MS_WORD_COMP_TRAILING_BLANKS, cppu::UnoType<bool>::get(), 0 },
        { OUString("TabAtLeftIndentForParagraphsInList"), HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST, cppu::UnoType<bool>::get(), 0},
        { OUString("ModifyPasswordInfo"), HANDLE_MODIFYPASSWORDINFO, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), 0},
        { OUString("MathBaselineAlignment"), HANDLE_MATH_BASELINE_ALIGNMENT, cppu::UnoType<bool>::get(), 0},
        { OUString("InvertBorderSpacing"), HANDLE_INVERT_BORDER_SPACING, cppu::UnoType<bool>::get(), 0},
        { OUString("CollapseEmptyCellPara"), HANDLE_COLLAPSE_EMPTY_CELL_PARA, cppu::UnoType<bool>::get(), 0},
        { OUString("SmallCapsPercentage66"), HANDLE_SMALL_CAPS_PERCENTAGE_66, cppu::UnoType<bool>::get(), 0},
        { OUString("TabOverflow"), HANDLE_TAB_OVERFLOW, cppu::UnoType<bool>::get(), 0},
        { OUString("UnbreakableNumberings"), HANDLE_UNBREAKABLE_NUMBERINGS, cppu::UnoType<bool>::get(), 0},
        { OUString("StylesNoDefault"), HANDLE_STYLES_NODEFAULT, cppu::UnoType<bool>::get(), 0},
        { OUString("FloattableNomargins"), HANDLE_FLOATTABLE_NOMARGINS, cppu::UnoType<bool>::get(), 0},
        { OUString("ClippedPictures"), HANDLE_CLIPPED_PICTURES, cppu::UnoType<bool>::get(), 0},
        { OUString("BackgroundParaOverDrawings"), HANDLE_BACKGROUND_PARA_OVER_DRAWINGS, cppu::UnoType<bool>::get(), 0},
        { OUString("EmbedFonts"), HANDLE_EMBED_FONTS, cppu::UnoType<bool>::get(), 0},
        { OUString("EmbedSystemFonts"), HANDLE_EMBED_SYSTEM_FONTS, cppu::UnoType<bool>::get(), 0},
        { OUString("TabOverMargin"), HANDLE_TAB_OVER_MARGIN, cppu::UnoType<bool>::get(), 0},
        { OUString("TreatSingleColumnBreakAsPageBreak"), HANDLE_TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK, cppu::UnoType<bool>::get(), 0},
        { OUString("SurroundTextWrapSmall"), HANDLE_SURROUND_TEXT_WRAP_SMALL, cppu::UnoType<bool>::get(), 0},
        { OUString("ApplyParagraphMarkFormatToNumbering"), HANDLE_APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING, cppu::UnoType<bool>::get(), 0},
        { OUString("PropLineSpacingShrinksFirstLine"),       HANDLE_PROP_LINE_SPACING_SHRINKS_FIRST_LINE,         cppu::UnoType<bool>::get(),           0},
        { OUString("SubtractFlysAnchoredAtFlys"),       HANDLE_SUBTRACT_FLYS,         cppu::UnoType<bool>::get(),           0},
/*
 * As OS said, we don't have a view when we need to set this, so I have to
 * find another solution before adding them to this property set - MTG
        { OUString("IsGridVisible"),              HANDLE_IS_GRID_VISIBLE,                 cppu::UnoType<bool>::get(),           0,   0},
        { OUString("IsSnapToGrid"),               HANDLE_IS_SNAP_TO_GRID,                 cppu::UnoType<bool>::get(),           0,   0},
        { OUString("IsSynchroniseAxes"),          HANDLE_IS_SYNCHRONISE_AXES,             cppu::UnoType<bool>::get(),           0,   0},
        { OUString("HorizontalGridResolution"),   HANDLE_HORIZONTAL_GRID_RESOLUTION,      cppu::UnoType<sal_Int32>::get(),             0,   0},
        { OUString("HorizontalGridSubdivision"),  HANDLE_HORIZONTAL_GRID_SUBDIVISION,     cppu::UnoType<sal_Int16>::get(),             0,   0},
        { OUString("VerticalGridResolution"),     HANDLE_VERTICAL_GRID_RESOLUTION,        cppu::UnoType<sal_Int32>::get(),             0,   0},
        { OUString("VerticalGridSubdivision"),    HANDLE_VERTICAL_GRID_SUBDIVISION,       cppu::UnoType<sal_Int16>::get(),             0,   0},

        { OUString("ShowParagraphEnd"),           HANDLE_SHOW_PARAGRAPH_END,              cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowOptionalHyphens"),        HANDLE_SHOW_OPTIONAL_HYPHENS,           cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowSpaces"),                 HANDLE_SHOW_SPACES,                     cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowTabs"),                   HANDLE_SHOW_TABS,                       cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowBreaks"),                 HANDLE_SHOW_BREAKS,                     cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowHiddenText"),             HANDLE_SHOW_HIDDEN_TEXT,                cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowHiddenParagraphs"),       HANDLE_SHOW_HIDDEN_PARAGRAPHS,          cppu::UnoType<bool>::get(),           0,   0},

        { OUString("ShowTextLimitGuide"),         HANDLE_SHOW_TEXT_LIMIT_GUIDE,           cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowTableLimitGuide"),        HANDLE_SHOW_TABLE_LIMIT_GUIDE,          cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowSectionLimitGuide"),      HANDLE_SHOW_SECTION_LIMIT_GUIDE,        cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ShowGuidesWhileMoving"),      HANDLE_SHOW_GUIDES_WHILE_MOVING,        cppu::UnoType<bool>::get(),           0,   0},
*/
        { OUString(), 0, css::uno::Type(), 0}
    };
    return new MasterPropertySetInfo ( aWriterSettingsInfoMap );
}

SwXDocumentSettings::SwXDocumentSettings ( SwXTextDocument * pModel )
: MasterPropertySet ( lcl_createSettingsInfo (),
                      &Application::GetSolarMutex () )
, mpModel ( pModel )
, mpDocSh ( nullptr )
, mpDoc ( nullptr )
, mpPrinter( nullptr )
{
    registerSlave ( new SwXPrintSettings ( SwXPrintSettingsType::Document, mpModel->GetDocShell()->GetDoc() ) );
}

SwXDocumentSettings::~SwXDocumentSettings()
    throw()
{
}

Any SAL_CALL SwXDocumentSettings::queryInterface( const Type& rType )
{
        return ::cppu::queryInterface ( rType,
                                        // OWeakObject interfaces
                                        dynamic_cast< XInterface* > ( dynamic_cast< OWeakObject*  >(this) ),
                                        dynamic_cast< XWeak* > ( this ),
                                        // my own interfaces
                                        dynamic_cast< XPropertySet*  > ( this ),
                                        dynamic_cast< XPropertyState* > ( this ),
                                        dynamic_cast< XMultiPropertySet* > ( this ),
                                        dynamic_cast< XServiceInfo* > ( this ),
                                        dynamic_cast< XTypeProvider* > ( this ) );
}
void SwXDocumentSettings::acquire ()
    throw ()
{
    OWeakObject::acquire();
}
void SwXDocumentSettings::release ()
    throw ()
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SwXDocumentSettings::getTypes(  )
{
    SolarMutexGuard aGuard;

    uno::Sequence< uno::Type > aBaseTypes( 5 );
    uno::Type* pBaseTypes = aBaseTypes.getArray();

    // from MasterPropertySet
    pBaseTypes[0] = cppu::UnoType<XPropertySet>::get();
    pBaseTypes[1] = cppu::UnoType<XPropertyState>::get();
    pBaseTypes[2] = cppu::UnoType<XMultiPropertySet>::get();

    pBaseTypes[3] = cppu::UnoType<XServiceInfo>::get();
    pBaseTypes[4] = cppu::UnoType<XTypeProvider>::get();

    return aBaseTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXDocumentSettings::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

void SwXDocumentSettings::_preSetValues ()
{
    mpDocSh = mpModel->GetDocShell();
    if (nullptr == mpDocSh)
        throw UnknownPropertyException();

    mpDoc = mpDocSh->GetDoc();
    if (nullptr == mpDoc)
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
{
    if (rInfo.mnAttributes & PropertyAttribute::READONLY)
        throw PropertyVetoException ("Property is read-only: " + rInfo.maName, static_cast < cppu::OWeakObject * > ( nullptr ) );

    switch( rInfo.mnHandle )
    {
        case HANDLE_FORBIDDEN_CHARS:
            break;
        case HANDLE_LINK_UPDATE_MODE:
        {
            sal_Int16 nMode = 0;
            rValue >>= nMode;
            switch (nMode)
            {
                case NEVER:
                case MANUAL:
                case AUTOMATIC:
                case GLOBALSETTING:
                    break;
                default:
                    throw IllegalArgumentException();
            }
            mpDoc->getIDocumentSettingAccess().setLinkUpdateMode(nMode);
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            bool bUpdateField = *o3tl::doAccess<bool>(rValue);
            SwFieldUpdateFlags nFlag = mpDoc->getIDocumentSettingAccess().getFieldUpdateFlags(true);
            mpDoc->getIDocumentSettingAccess().setFieldUpdateFlags( bUpdateField ?
                                        nFlag == AUTOUPD_FIELD_AND_CHARTS ?
                                        AUTOUPD_FIELD_AND_CHARTS :
                                        AUTOUPD_FIELD_ONLY :
                                        AUTOUPD_OFF );
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            bool bUpdateChart = *o3tl::doAccess<bool>(rValue);
            SwFieldUpdateFlags nFlag = mpDoc->getIDocumentSettingAccess().getFieldUpdateFlags(true);
            mpDoc->getIDocumentSettingAccess().setFieldUpdateFlags( (nFlag == AUTOUPD_FIELD_ONLY || nFlag == AUTOUPD_FIELD_AND_CHARTS ) ?
                                        bUpdateChart ?
                                        AUTOUPD_FIELD_AND_CHARTS :
                                        AUTOUPD_FIELD_ONLY :
                                        AUTOUPD_OFF );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            bool bParaSpace = false;
            rValue >>= bParaSpace;
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::PARA_SPACE_MAX, bParaSpace );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            bool bParaSpacePage = false;
            rValue >>= bParaSpacePage;
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES, bParaSpacePage );
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            bool bAlignTab = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TAB_COMPAT, bAlignTab);
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            //the printer must be created
            OUString sPrinterName;
            if( rValue >>= sPrinterName  )
            {
                if( !mpPrinter && !sPrinterName.isEmpty() && mpDocSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
                {
                    SfxPrinter* pPrinter = mpDoc->getIDocumentDeviceAccess().getPrinter( true );
                    if ( OUString ( pPrinter->GetName()) != sPrinterName )
                    {
                        VclPtrInstance<SfxPrinter> pNewPrinter( pPrinter->GetOptions().Clone(), sPrinterName );
                        assert (! pNewPrinter->isDisposed() );
                        if( pNewPrinter->IsKnown() )
                        {
                            // set printer only once; in _postSetValues
                            mpPrinter = pNewPrinter;
                        }
                        else
                        {
                            pNewPrinter.disposeAndClear();
                        }
                    }
                }
            }
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            Sequence < sal_Int8 > aSequence;
            if ( rValue >>= aSequence )
            {
                sal_uInt32 nSize = aSequence.getLength();
                if( nSize > 0 )
                {
                    SvMemoryStream aStream (aSequence.getArray(), nSize,
                                            StreamMode::READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    static sal_uInt16 const nRange[] =
                    {
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_HTML_MODE,  SID_HTML_MODE,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0
                    };
                    SfxItemSet *pItemSet = new SfxItemSet( mpDoc->GetAttrPool(), nRange );
                    VclPtr<SfxPrinter> pPrinter = SfxPrinter::Create ( aStream, pItemSet );
                    assert (! pPrinter->isDisposed() );
                    // set printer only once; in _postSetValues
                    mpPrinter.disposeAndClear();
                    mpPrinter = pPrinter;
                }
            }
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
        {
            bool bIsKern = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::KERN_ASIAN_PUNCTUATION, bIsKern);
            SwEditShell* pEditSh = mpDoc->GetEditShell();
            if(pEditSh)
                pEditSh->ChgHyphenation();
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            sal_Int16 nMode = 0;
            rValue >>= nMode;
            switch (nMode)
            {
                case CHARCOMPRESS_NONE:
                case CHARCOMPRESS_PUNCTUATION:
                case CHARCOMPRESS_PUNCTUATION_KANA:
                    break;
                default:
                    throw IllegalArgumentException();
            }
            mpDoc->getIDocumentSettingAccess().setCharacterCompressionType(static_cast < SwCharCompressType > (nMode) );
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            mpDocSh->SetUseUserData( *o3tl::doAccess<bool>(rValue) );
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            bool bSaveGlobal = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS, bSaveGlobal );
        }
        break;
        case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.sDataSource )
                mpDoc->ChgDBData( aData );
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND:
        {
            SwDBData aData = mpDoc->GetDBData();

            if ( rValue >>= aData.sCommand )
                mpDoc->ChgDBData( aData );

            SAL_WARN_IF( aData.sDataSource.isEmpty() && !aData.sCommand.isEmpty(), "sw.uno",
                "\"CurrentDatabaseCommand\" property possibly set before \"CurrentDatabaseDataSource\"" );
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.nCommandType )
                mpDoc->ChgDBData( aData );
            SAL_WARN_IF( aData.nCommandType && aData.sDataSource.isEmpty(), "sw.uno",
                "\"CurrentDatabaseCommandType\" property possibly set before \"CurrentDatabaseDataSource\"" );
            SAL_WARN_IF( aData.nCommandType && aData.sCommand.isEmpty(), "sw.uno",
                "\"CurrentDatabaseCommandType\" property possibly set before \"CurrentDatabaseCommand\"" );
        }
        break;
        case HANDLE_EMBEDDED_DATABASE_NAME:
        {
            OUString sEmbeddedName;
            if (rValue >>= sEmbeddedName)
                mpDoc->GetDBManager()->setEmbeddedName(sEmbeddedName, *mpDocSh);
        }
        break;
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            mpDocSh->SetSaveVersionOnClose( *o3tl::doAccess<bool>(rValue) );
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            mpDocSh->SetQueryLoadTemplate( *o3tl::doAccess<bool>(rValue) );
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            sal_Int16 nTmp = 0;
            rValue >>= nTmp;

            bool bUseVirDev = true;
            bool bHiResVirDev = true;
            if( nTmp == document::PrinterIndependentLayout::DISABLED )
                bUseVirDev = false;
            else if ( nTmp == document::PrinterIndependentLayout::LOW_RESOLUTION )
                bHiResVirDev = false;
            else if ( nTmp != document::PrinterIndependentLayout::HIGH_RESOLUTION )
                throw IllegalArgumentException();

            mpDoc->getIDocumentDeviceAccess().setReferenceDeviceType( bUseVirDev, bHiResVirDev );
        }
        break;
        case HANDLE_IS_LABEL_DOC :
        {
            bool bSet = false;
            if(!(rValue >>= bSet))
                throw IllegalArgumentException();
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::LABEL_DOCUMENT, bSet);
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::ADD_FLY_OFFSETS, bTmp);
        }
        break;
        case HANDLE_IS_ADD_VERTICAL_FLY_OFFSET:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS, bTmp);
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::ADD_EXT_LEADING, bTmp);
        }
        break;
        case HANDLE_OLD_NUMBERING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::OLD_NUMBERING, bTmp);
        }
        break;
        case HANDLE_OUTLINELEVEL_YIELDS_NUMBERING:
        {
            // ignore - this is a dead property
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            bool bState = false;
            if (!(rValue >>= bState))
                throw IllegalArgumentException();
            mpDocSh->Stamp_SetPrintCancelState(bState);
        }
        break;
        case HANDLE_USE_FORMER_LINE_SPACING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::OLD_LINE_SPACING, bTmp);
        }
        break;
        case HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS, bTmp);
        }
        break;
        case HANDLE_USE_FORMER_OBJECT_POSITIONING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::USE_FORMER_OBJECT_POS, bTmp);
        }
        break;
        case HANDLE_USE_FORMER_TEXT_WRAPPING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::USE_FORMER_TEXT_WRAPPING, bTmp);
        }
        break;
        case HANDLE_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(rValue >>= aNew)
            {
                mpDoc->getIDocumentRedlineAccess().SetRedlinePassword(aNew);
                if(aNew.getLength())
                {
                    RedlineFlags eMode = mpDoc->getIDocumentRedlineAccess().GetRedlineFlags();
                    eMode |= RedlineFlags::On;
                    mpDoc->getIDocumentRedlineAccess().SetRedlineFlags( eMode );
                }
            }
        }
        break;
        case HANDLE_CONSIDER_WRAP_ON_OBJPOS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION, bTmp );
        }
        break;
        case HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, bTmp);
        }
        break;
        case HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, bTmp);
        }
        break;
        case HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, bTmp);
        }
        break;
        case HANDLE_TABLE_ROW_KEEP:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TABLE_ROW_KEEP, bTmp);
        }
        break;
        case HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, bTmp);
        }
        break;
        case HANDLE_LOAD_READONLY:
        {
            mpDocSh->SetLoadReadonly( *o3tl::doAccess<bool>(rValue) );
        }
        break;
        case HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, bTmp);
        }
        break;
        case HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME, bTmp);
        }
        break;
        case HANDLE_UNIX_FORCE_ZERO_EXT_LEADING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::UNIX_FORCE_ZERO_EXT_LEADING, bTmp);
        }
        break;
        case HANDLE_USE_OLD_PRINTER_METRICS:
            // ignore - this is a dead property
        break;
        case HANDLE_TABS_RELATIVE_TO_INDENT:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TABS_RELATIVE_TO_INDENT, bTmp);
        }
        break;
        case HANDLE_RSID:
        {
            sal_uInt32 nTmp = 0;
            rValue >>= nTmp;
            mpDoc->setRsid( nTmp );
        }
        break;
        case HANDLE_RSID_ROOT:
        {
            sal_uInt32 nTmp = 0;
            rValue >>= nTmp;
            mpDoc->setRsidRoot( nTmp );
        }
        break;
        case HANDLE_PROTECT_FORM:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::PROTECT_FORM, bTmp);
        }
        break;
        case HANDLE_MS_WORD_COMP_TRAILING_BLANKS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS, bTmp);
        }
        break;
        case HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST, bTmp);
        }
        break;
        case HANDLE_MODIFYPASSWORDINFO:
        {
            uno::Sequence< beans::PropertyValue > aInfo;
            if ( !( rValue >>= aInfo ) )
                throw lang::IllegalArgumentException(
                    "Value of type Sequence<PropertyValue> expected!",
                    uno::Reference< uno::XInterface >(),
                    2 );

            if ( !mpDocSh->SetModifyPasswordInfo( aInfo ) )
                throw beans::PropertyVetoException(
                    "The hash is not allowed to be changed now!",
                    uno::Reference< uno::XInterface >() );
        }
        break;
        case HANDLE_MATH_BASELINE_ALIGNMENT:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set( DocumentSettingId::MATH_BASELINE_ALIGNMENT, bTmp );
        }
        break;
        case HANDLE_INVERT_BORDER_SPACING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::INVERT_BORDER_SPACING, bTmp);
        }
        break;
        case HANDLE_COLLAPSE_EMPTY_CELL_PARA:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA, bTmp);
        }
        break;
        case HANDLE_SMALL_CAPS_PERCENTAGE_66:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::SMALL_CAPS_PERCENTAGE_66, bTmp);
        }
        break;
        case HANDLE_TAB_OVERFLOW:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TAB_OVERFLOW, bTmp);
        }
        break;
        case HANDLE_UNBREAKABLE_NUMBERINGS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::UNBREAKABLE_NUMBERINGS, bTmp);
        }
        break;
        case HANDLE_STYLES_NODEFAULT:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::STYLES_NODEFAULT, bTmp);
        }
        break;
        case HANDLE_FLOATTABLE_NOMARGINS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::FLOATTABLE_NOMARGINS, bTmp);
        }
        break;
        case HANDLE_CLIPPED_PICTURES:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::CLIPPED_PICTURES, bTmp);
        }
        break;
        case HANDLE_BACKGROUND_PARA_OVER_DRAWINGS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS, bTmp);
        }
        break;
        case HANDLE_EMBED_FONTS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::EMBED_FONTS, bTmp);
        }
        break;
        case HANDLE_EMBED_SYSTEM_FONTS:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::EMBED_SYSTEM_FONTS, bTmp);
        }
        break;
        case HANDLE_TAB_OVER_MARGIN:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TAB_OVER_MARGIN, bTmp);
        }
        break;
        case HANDLE_TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK, bTmp);
        }
        break;
        case HANDLE_SURROUND_TEXT_WRAP_SMALL:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::SURROUND_TEXT_WRAP_SMALL, bTmp);
        }
        break;
        case HANDLE_APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING:
        {
            bool bTmp = *o3tl::doAccess<bool>(rValue);
            mpDoc->getIDocumentSettingAccess().set(DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING, bTmp);
        }
        break;
        case HANDLE_PROP_LINE_SPACING_SHRINKS_FIRST_LINE:
        {
            bool bTmp;
            if (rValue >>= bTmp)
            {
                mpDoc->getIDocumentSettingAccess().set(
                    DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE, bTmp);
            }
        }
        break;
        case HANDLE_SUBTRACT_FLYS:
        {
            bool bTmp;
            if (rValue >>= bTmp)
            {
                mpDoc->getIDocumentSettingAccess().set(
                    DocumentSettingId::SUBTRACT_FLYS, bTmp);
            }
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postSetValues ()
{
    // set printer only once, namely here!
    if( mpPrinter != nullptr )
    {
        // #i86352# the printer is also used as container for options by sfx
        // when setting a printer it should have decent default options
        SfxItemSet aOptions( mpPrinter->GetOptions() );
        SwPrintData aPrtData( mpDoc->getIDocumentDeviceAccess().getPrintData() );
        SwAddPrinterItem aAddPrinterItem (aPrtData);
        aOptions.Put(aAddPrinterItem);
        mpPrinter->SetOptions( aOptions );

        mpDoc->getIDocumentDeviceAccess().setPrinter( mpPrinter, true, true );
    }

    mpPrinter = nullptr;
    mpDocSh = nullptr;
    mpDoc = nullptr;
}

void SwXDocumentSettings::_preGetValues ()
{
    mpDocSh = mpModel->GetDocShell();
    if (nullptr == mpDocSh)
        throw UnknownPropertyException();
    mpDoc = mpDocSh->GetDoc();
    if (nullptr == mpDoc)
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, uno::Any & rValue )
{
    switch( rInfo.mnHandle )
    {
        case HANDLE_FORBIDDEN_CHARS:
        {
            Reference<XForbiddenCharacters> xRet(*mpModel->GetPropertyHelper(), UNO_QUERY);
            rValue <<= xRet;
        }
        break;
        case HANDLE_LINK_UPDATE_MODE:
        {
            rValue <<= static_cast < sal_Int16 > ( mpDoc->getIDocumentSettingAccess().getLinkUpdateMode(true) );
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            SwFieldUpdateFlags nFlags = mpDoc->getIDocumentSettingAccess().getFieldUpdateFlags(true);
            rValue <<= nFlags == AUTOUPD_FIELD_ONLY || nFlags == AUTOUPD_FIELD_AND_CHARTS;
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            SwFieldUpdateFlags nFlags = mpDoc->getIDocumentSettingAccess().getFieldUpdateFlags(true);
            rValue <<= nFlags == AUTOUPD_FIELD_AND_CHARTS;
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::PARA_SPACE_MAX);
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES);
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::TAB_COMPAT);
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            SfxPrinter *pPrinter = mpDoc->getIDocumentDeviceAccess().getPrinter( false );
            rValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            SfxPrinter *pPrinter = mpDoc->getIDocumentDeviceAccess().getPrinter( false );
            if (pPrinter)
            {
                SvMemoryStream aStream;
                pPrinter->Store( aStream );
                aStream.Seek ( STREAM_SEEK_TO_END );
                sal_uInt32 nSize = aStream.Tell();
                aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                Sequence < sal_Int8 > aSequence( nSize );
                aStream.ReadBytes(aSequence.getArray(), nSize);
                rValue <<= aSequence;
            }
            else
            {
                Sequence < sal_Int8 > aSequence ( 0 );
                rValue <<= aSequence;
            }
        }
        break;
        case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::KERN_ASIAN_PUNCTUATION);
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            rValue <<= mpDocSh->IsUseUserData();
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            rValue <<= static_cast < sal_Int16 > (mpDoc->getIDocumentSettingAccess().getCharacterCompressionType());
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS);
        }
        break;
        case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.sDataSource;
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.sCommand;
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.nCommandType;
        }
        break;
        case HANDLE_EMBEDDED_DATABASE_NAME:
        {
            rValue <<= mpDoc->GetDBManager()->getEmbeddedName();
        }
        break;
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            rValue <<= mpDocSh->IsSaveVersionOnClose();
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            rValue <<= mpDocSh->IsQueryLoadTemplate();
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            // returns short (see css.document.PrinterIndependentLayout)
            sal_Int16 nVirDevType = mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::USE_VIRTUAL_DEVICE) ?
                                    ( mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::USE_HIRES_VIRTUAL_DEVICE) ?
                                      document::PrinterIndependentLayout::HIGH_RESOLUTION :
                                      document::PrinterIndependentLayout::LOW_RESOLUTION ) :
                                    document::PrinterIndependentLayout::DISABLED;
            rValue <<= nVirDevType;
        }
        break;
        case HANDLE_IS_LABEL_DOC:
        {
            bool bLabel = mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::LABEL_DOCUMENT);
            rValue <<= bLabel;
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::ADD_FLY_OFFSETS);
        }
        break;
        case HANDLE_IS_ADD_VERTICAL_FLY_OFFSET:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS);
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::ADD_EXT_LEADING);
        }
        break;
        case HANDLE_OLD_NUMBERING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::OLD_NUMBERING);
        }
        break;
        case HANDLE_OUTLINELEVEL_YIELDS_NUMBERING:
        {
            rValue <<= false;
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            rValue <<= mpDocSh->Stamp_GetPrintCancelState();
        }
        break;
        case HANDLE_USE_FORMER_LINE_SPACING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::OLD_LINE_SPACING);
        }
        break;
        case HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS);
        }
        break;
        case HANDLE_USE_FORMER_OBJECT_POSITIONING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::USE_FORMER_OBJECT_POS);
        }
        break;
        case HANDLE_USE_FORMER_TEXT_WRAPPING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING);
        }
        break;
        case HANDLE_CHANGES_PASSWORD:
        {
            rValue <<= mpDoc->getIDocumentRedlineAccess().GetRedlinePassword();
        }
        break;
        case HANDLE_CONSIDER_WRAP_ON_OBJPOS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        }
        break;
        case HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING);
        }
        break;
        case HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);
        }
        break;
        case HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT);
        }
        break;
        case HANDLE_TABLE_ROW_KEEP :
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::TABLE_ROW_KEEP);
        }
        break;
        case HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION);
        }
        break;
        case HANDLE_LOAD_READONLY:
        {
            rValue <<= mpDocSh->IsLoadReadonly();
        }
        break;
        case HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE);
        }
        break;
        case HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME);
        }
        break;
        case HANDLE_UNIX_FORCE_ZERO_EXT_LEADING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::UNIX_FORCE_ZERO_EXT_LEADING);
        }
        break;
        case HANDLE_USE_OLD_PRINTER_METRICS:
        {
            rValue <<= false;
        }
        break;
        case HANDLE_TABS_RELATIVE_TO_INDENT:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::TABS_RELATIVE_TO_INDENT);
        }
        break;
        case HANDLE_RSID:
        {
            rValue <<= static_cast < sal_Int32 > ( mpDoc->getRsid() );
        }
        break;
        case HANDLE_RSID_ROOT:
        {
            rValue <<= static_cast < sal_Int32 > ( mpDoc->getRsidRoot() );
        }
        break;
        case HANDLE_PROTECT_FORM:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_FORM);
        }
        break;
        case HANDLE_MS_WORD_COMP_TRAILING_BLANKS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS);
        }
        break;
        case HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
        }
        break;
        case HANDLE_MODIFYPASSWORDINFO:
        {
            rValue <<= mpDocSh->GetModifyPasswordInfo();
        }
        break;
        case HANDLE_MATH_BASELINE_ALIGNMENT:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
        }
        break;
        case HANDLE_INVERT_BORDER_SPACING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::INVERT_BORDER_SPACING);
        }
        break;
        case HANDLE_COLLAPSE_EMPTY_CELL_PARA:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA );
        }
        break;
        case HANDLE_SMALL_CAPS_PERCENTAGE_66:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::SMALL_CAPS_PERCENTAGE_66 );
        }
        break;
        case HANDLE_TAB_OVERFLOW:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::TAB_OVERFLOW );
        }
        break;
        case HANDLE_UNBREAKABLE_NUMBERINGS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::UNBREAKABLE_NUMBERINGS );
        }
        break;
        case HANDLE_STYLES_NODEFAULT:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::STYLES_NODEFAULT );
        }
        break;
        case HANDLE_FLOATTABLE_NOMARGINS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::FLOATTABLE_NOMARGINS );
        }
        break;
        case HANDLE_CLIPPED_PICTURES:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::CLIPPED_PICTURES );
        }
        break;
        case HANDLE_BACKGROUND_PARA_OVER_DRAWINGS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS );
        }
        break;
        case HANDLE_EMBED_FONTS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS );
        }
        break;
        case HANDLE_EMBED_SYSTEM_FONTS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_SYSTEM_FONTS );
        }
        break;
        case HANDLE_TAB_OVER_MARGIN:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::TAB_OVER_MARGIN );
        }
        break;
        case HANDLE_TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK );
        }
        break;
        case HANDLE_SURROUND_TEXT_WRAP_SMALL:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::SURROUND_TEXT_WRAP_SMALL );
        }
        break;
        case HANDLE_APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get( DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING );
        }
        break;
        case HANDLE_PROP_LINE_SPACING_SHRINKS_FIRST_LINE:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE);
        }
        break;
        case HANDLE_SUBTRACT_FLYS:
        {
            rValue <<= mpDoc->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS);
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postGetValues ()
{
    mpDocSh = nullptr;
    mpDoc = nullptr;
}

// XServiceInfo
OUString SAL_CALL SwXDocumentSettings::getImplementationName(  )
{
    return OUString("com.sun.star.comp.Writer.DocumentSettings");
}

sal_Bool SAL_CALL SwXDocumentSettings::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SwXDocumentSettings::getSupportedServiceNames(  )
{
    Sequence< OUString > aSeq( 3 );
    aSeq[0] = "com.sun.star.document.Settings";
    aSeq[1] = "com.sun.star.text.DocumentSettings";
    aSeq[2] = "com.sun.star.text.PrintSettings";
    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
