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

#include <osl/mutex.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <SwXDocumentSettings.hxx>
#include <comphelper/MasterPropertySetInfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <doc.hxx>
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
    HANDLE_SAVE_VERSION_ON_CLOSE,
    HANDLE_IS_GRID_VISIBLE,
    HANDLE_IS_SNAP_TO_GRID,
    HANDLE_IS_SYNCHRONISE_AXES,
    HANDLE_HORIZONTAL_GRID_RESOLUTION,
    HANDLE_HORIZONTAL_GRID_SUBDIVISION,
    HANDLE_VERTICAL_GRID_RESOLUTION,
    HANDLE_VERTICAL_GRID_SUBDIVISION,
    HANDLE_UPDATE_FROM_TEMPLATE,
    HANDLE_PRINTER_INDEPENDENT_LAYOUT,
    HANDLE_IS_LABEL_DOC,
    HANDLE_IS_ADD_FLY_OFFSET,
    HANDLE_IS_ADD_EXTERNAL_LEADING,
    HANDLE_OLD_NUMBERING, // #111955#
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
    HANDLE_TABS_RELATIVE_TO_INDENT,
    HANDLE_RSID,
    HANDLE_RSID_ROOT,
    // #i89181#
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
    HANDLE_SURROUND_TEXT_WRAP_SMALL
};

static MasterPropertySetInfo * lcl_createSettingsInfo()
{
    static PropertyInfo const aWriterSettingsInfoMap[] =
    {
        { OUString("ForbiddenCharacters"),        HANDLE_FORBIDDEN_CHARS,                 cppu::UnoType<css::i18n::XForbiddenCharacters>::get(),      0,   0},
        { OUString("LinkUpdateMode"),             HANDLE_LINK_UPDATE_MODE,                cppu::UnoType<sal_Int16>::get(),             0,   0},
        { OUString("FieldAutoUpdate"),            HANDLE_FIELD_AUTO_UPDATE,               cppu::UnoType<bool>::get(),           0,   0},
        { OUString("ChartAutoUpdate"),            HANDLE_CHART_AUTO_UPDATE,               cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AddParaTableSpacing"),        HANDLE_ADD_PARA_TABLE_SPACING,          cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AddParaTableSpacingAtStart"), HANDLE_ADD_PARA_TABLE_SPACING_AT_START, cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AlignTabStopPosition"),       HANDLE_ALIGN_TAB_STOP_POSITION,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("PrinterName"),                HANDLE_PRINTER_NAME,                    cppu::UnoType<OUString>::get(),          0,   0},
        { OUString("PrinterSetup"),               HANDLE_PRINTER_SETUP,                   cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(),           0,   0},
        { OUString("IsKernAsianPunctuation"),     HANDLE_IS_KERN_ASIAN_PUNCTUATION,       cppu::UnoType<bool>::get(),           0,   0},
        { OUString("CharacterCompressionType"),   HANDLE_CHARACTER_COMPRESSION_TYPE,      cppu::UnoType<sal_Int16>::get(),             0,   0},
        { OUString("ApplyUserData"),              HANDLE_APPLY_USER_DATA,                 cppu::UnoType<bool>::get(),           0,   0},
        { OUString("SaveGlobalDocumentLinks"),    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,      cppu::UnoType<bool>::get(),           0,   0},
        { OUString("CurrentDatabaseDataSource"),  HANDLE_CURRENT_DATABASE_DATA_SOURCE,    cppu::UnoType<OUString>::get(),          0,   0},
        { OUString("CurrentDatabaseCommand"),     HANDLE_CURRENT_DATABASE_COMMAND,        cppu::UnoType<OUString>::get(),          0,   0},
        { OUString("CurrentDatabaseCommandType"), HANDLE_CURRENT_DATABASE_COMMAND_TYPE,   cppu::UnoType<sal_Int32>::get(),             0,   0},
        { OUString("SaveVersionOnClose"),         HANDLE_SAVE_VERSION_ON_CLOSE,           cppu::UnoType<bool>::get(),           0,   0},
        { OUString("UpdateFromTemplate"),         HANDLE_UPDATE_FROM_TEMPLATE,            cppu::UnoType<bool>::get(),           0,   0},

        { OUString("PrinterIndependentLayout"),   HANDLE_PRINTER_INDEPENDENT_LAYOUT,      cppu::UnoType<sal_Int16>::get(),             0,   0},
        { OUString("IsLabelDocument"),            HANDLE_IS_LABEL_DOC,                    cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AddFrameOffsets"),            HANDLE_IS_ADD_FLY_OFFSET,               cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AddExternalLeading"),         HANDLE_IS_ADD_EXTERNAL_LEADING,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("UseOldNumbering"),            HANDLE_OLD_NUMBERING,                   cppu::UnoType<bool>::get(),           0,   0}, // #111955#
        { OUString("OutlineLevelYieldsNumbering"), HANDLE_OUTLINELEVEL_YIELDS_NUMBERING, cppu::UnoType<bool>::get(),           0,   0},
        /* Stampit It disable the print cancel button of the shown progress dialog. */
        { OUString("AllowPrintJobCancel"),        HANDLE_ALLOW_PRINTJOB_CANCEL,           cppu::UnoType<bool>::get(),           0,   0},
        { OUString("UseFormerLineSpacing"),       HANDLE_USE_FORMER_LINE_SPACING,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("AddParaSpacingToTableCells"), HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS, cppu::UnoType<bool>::get(),           0,   0},
        { OUString("UseFormerObjectPositioning"), HANDLE_USE_FORMER_OBJECT_POSITIONING,   cppu::UnoType<bool>::get(),           0,   0},
        { OUString("UseFormerTextWrapping"),      HANDLE_USE_FORMER_TEXT_WRAPPING,        cppu::UnoType<bool>::get(),           0,   0},
        { OUString("RedlineProtectionKey"),       HANDLE_CHANGES_PASSWORD,                cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(),           0,   0},
        { OUString("ConsiderTextWrapOnObjPos"),   HANDLE_CONSIDER_WRAP_ON_OBJPOS,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("IgnoreFirstLineIndentInNumbering"),   HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("DoNotJustifyLinesWithManualBreak"),   HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("DoNotResetParaAttrsForNumFont"),   HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("TableRowKeep"),               HANDLE_TABLE_ROW_KEEP,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("IgnoreTabsAndBlanksForLineCalculation"),   HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION,         cppu::UnoType<bool>::get(),           0,   0},
        { OUString("LoadReadonly"),               HANDLE_LOAD_READONLY,                   cppu::UnoType<bool>::get(),           0,   0},
        { OUString("DoNotCaptureDrawObjsOnPage"),   HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("ClipAsCharacterAnchoredWriterFlyFrames"), HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("UnxForceZeroExtLeading"), HANDLE_UNIX_FORCE_ZERO_EXT_LEADING, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("UseOldPrinterMetrics"), HANDLE_USE_OLD_PRINTER_METRICS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("TabsRelativeToIndent"), HANDLE_TABS_RELATIVE_TO_INDENT, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("Rsid"), HANDLE_RSID, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { OUString("RsidRoot"), HANDLE_RSID_ROOT, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { OUString("ProtectForm"), HANDLE_PROTECT_FORM, cppu::UnoType<bool>::get(), 0, 0},
        // #i89181#
        { OUString("TabAtLeftIndentForParagraphsInList"), HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("ModifyPasswordInfo"), HANDLE_MODIFYPASSWORDINFO, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), 0,   0},
        { OUString("MathBaselineAlignment"), HANDLE_MATH_BASELINE_ALIGNMENT, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("InvertBorderSpacing"), HANDLE_INVERT_BORDER_SPACING, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("CollapseEmptyCellPara"), HANDLE_COLLAPSE_EMPTY_CELL_PARA, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("SmallCapsPercentage66"), HANDLE_SMALL_CAPS_PERCENTAGE_66, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("TabOverflow"), HANDLE_TAB_OVERFLOW, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("UnbreakableNumberings"), HANDLE_UNBREAKABLE_NUMBERINGS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("StylesNoDefault"), HANDLE_STYLES_NODEFAULT, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("FloattableNomargins"), HANDLE_FLOATTABLE_NOMARGINS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("ClippedPictures"), HANDLE_CLIPPED_PICTURES, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("BackgroundParaOverDrawings"), HANDLE_BACKGROUND_PARA_OVER_DRAWINGS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("EmbedFonts"), HANDLE_EMBED_FONTS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("EmbedSystemFonts"), HANDLE_EMBED_SYSTEM_FONTS, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("TabOverMargin"), HANDLE_TAB_OVER_MARGIN, cppu::UnoType<bool>::get(), 0, 0},
        { OUString("SurroundTextWrapSmall"), HANDLE_SURROUND_TEXT_WRAP_SMALL, cppu::UnoType<bool>::get(), 0, 0},
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
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return new MasterPropertySetInfo ( aWriterSettingsInfoMap );
}

SwXDocumentSettings::SwXDocumentSettings ( SwXTextDocument * pModel )
: MasterPropertySet ( lcl_createSettingsInfo (),
                      &Application::GetSolarMutex () )
, mxModel ( pModel )
, mpModel ( pModel )
, mpDocSh ( NULL )
, mpDoc ( NULL )
, mpPrinter( NULL )
{
    registerSlave ( new SwXPrintSettings ( PRINT_SETTINGS_DOCUMENT, mpModel->GetDocShell()->GetDoc() ) );
}

SwXDocumentSettings::~SwXDocumentSettings()
    throw()
{
}

Any SAL_CALL SwXDocumentSettings::queryInterface( const Type& rType )
    throw(RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Sequence< uno::Type > aBaseTypes( 5 );
    uno::Type* pBaseTypes = aBaseTypes.getArray();

    // from MasterPropertySet
    pBaseTypes[0] = ::getCppuType((Reference< XPropertySet >*)0);
    pBaseTypes[1] = ::getCppuType((Reference< XPropertyState >*)0);
    pBaseTypes[2] = ::getCppuType((Reference< XMultiPropertySet >*)0);

    pBaseTypes[3] = ::getCppuType((Reference< XServiceInfo >*)0);
    pBaseTypes[4] = ::getCppuType((Reference< XTypeProvider >*)0);

    return aBaseTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXDocumentSettings::getImplementationId(  )
    throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SwXDocumentSettings::_preSetValues ()
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    mpDoc = mpDocSh->GetDoc();

    if( NULL == mpDoc || NULL == mpDocSh )
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    if (rInfo.mnAttributes & PropertyAttribute::READONLY)
        throw PropertyVetoException ("Property is read-only: " + rInfo.maName, static_cast < cppu::OWeakObject * > ( 0 ) );

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
            mpDoc->setLinkUpdateMode(nMode);
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            sal_Bool bUpdateField = *(sal_Bool*)rValue.getValue();
            SwFldUpdateFlags nFlag = mpDoc->getFieldUpdateFlags(true);
            mpDoc->setFieldUpdateFlags( bUpdateField ?
                                        nFlag == AUTOUPD_FIELD_AND_CHARTS ?
                                        AUTOUPD_FIELD_AND_CHARTS :
                                        AUTOUPD_FIELD_ONLY :
                                        AUTOUPD_OFF );
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            sal_Bool bUpdateChart = *(sal_Bool*)rValue.getValue();
            SwFldUpdateFlags nFlag = mpDoc->getFieldUpdateFlags(true);
            mpDoc->setFieldUpdateFlags( (nFlag == AUTOUPD_FIELD_ONLY || nFlag == AUTOUPD_FIELD_AND_CHARTS ) ?
                                        bUpdateChart ?
                                        AUTOUPD_FIELD_AND_CHARTS :
                                        AUTOUPD_FIELD_ONLY :
                                        AUTOUPD_OFF );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace = sal_False;
            rValue >>= bParaSpace;
            mpDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX, bParaSpace );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpacePage = sal_False;
            rValue >>= bParaSpacePage;
            mpDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, bParaSpacePage );
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TAB_COMPAT, bAlignTab);
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            //the printer must be created
            OUString sPrinterName;
            if( rValue >>= sPrinterName  )
            {
                if( !mpPrinter && !sPrinterName.isEmpty() && mpDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    SfxPrinter* pPrinter = mpDoc->getPrinter( true );
                    if ( OUString ( pPrinter->GetName()) != sPrinterName )
                    {
                        SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), sPrinterName );
                        if( pNewPrinter->IsKnown() )
                        {
                            // set printer only once; in _postSetValues
                            mpPrinter = pNewPrinter;
                        }
                        else
                        {
                            delete pNewPrinter;
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
                                            STREAM_READ );
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
                    SfxPrinter *pPrinter = SfxPrinter::Create ( aStream, pItemSet );

                    // set printer only once; in _postSetValues
                    delete mpPrinter;
                    mpPrinter = pPrinter;
                }
            }
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
        {
            sal_Bool bIsKern = *(sal_Bool*)(rValue).getValue();
            mpDoc->set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION, bIsKern);
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
            mpDoc->setCharacterCompressionType(static_cast < SwCharCompressType > (nMode) );
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            mpDocSh->SetUseUserData( *(sal_Bool*)rValue.getValue() );
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bSaveGlobal );
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
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            mpDocSh->SetSaveVersionOnClose( *(sal_Bool*)rValue.getValue() );
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            mpDocSh->SetQueryLoadTemplate( *(sal_Bool*)rValue.getValue() );
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

            mpDoc->setReferenceDeviceType( bUseVirDev, bHiResVirDev );
        }
        break;
        case HANDLE_IS_LABEL_DOC :
        {
            sal_Bool bSet = sal_False;
            if(!(rValue >>= bSet))
                throw IllegalArgumentException();
            mpDoc->set(IDocumentSettingAccess::LABEL_DOCUMENT, bSet);
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::ADD_FLY_OFFSETS, bTmp);
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::ADD_EXT_LEADING, bTmp);
        }
        break;
        case HANDLE_OLD_NUMBERING: // #111955#
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::OLD_NUMBERING, bTmp);
        }
        break;
        case HANDLE_OUTLINELEVEL_YIELDS_NUMBERING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::OUTLINE_LEVEL_YIELDS_OUTLINE_RULE, bTmp);
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            sal_Bool bState = sal_False;
            if (!(rValue >>= bState))
                throw IllegalArgumentException();
            mpDocSh->Stamp_SetPrintCancelState(bState);
        }
        break;
        case HANDLE_USE_FORMER_LINE_SPACING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::OLD_LINE_SPACING, bTmp);
        }
        break;
        case HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, bTmp);
        }
        break;
        case HANDLE_USE_FORMER_OBJECT_POSITIONING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, bTmp);
        }
        break;
        case HANDLE_USE_FORMER_TEXT_WRAPPING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, bTmp);
        }
        break;
        case HANDLE_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(rValue >>= aNew)
            {
                mpDoc->SetRedlinePassword(aNew);
                if(aNew.getLength())
                {
                    sal_uInt16 eMode = mpDoc->GetRedlineMode();
                    eMode = eMode|nsRedlineMode_t::REDLINE_ON;
                    mpDoc->SetRedlineMode((RedlineMode_t)( eMode ));
                }
            }
        }
        break;
        case HANDLE_CONSIDER_WRAP_ON_OBJPOS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, bTmp );
        }
        break;
        case HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, bTmp);
        }
        break;
        case HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, bTmp);
        }
        break;
        case HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, bTmp);
        }
        break;
        case HANDLE_TABLE_ROW_KEEP:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TABLE_ROW_KEEP, bTmp);
        }
        break;
        case HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, bTmp);
        }
        break;
        case HANDLE_LOAD_READONLY:
        {
            mpDocSh->SetLoadReadonly( *(sal_Bool*)rValue.getValue() );
        }
        break;
        case HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, bTmp);
        }
        break;
        case HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME, bTmp);
        }
        break;
        case HANDLE_UNIX_FORCE_ZERO_EXT_LEADING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING, bTmp);
        }
        break;
        case HANDLE_USE_OLD_PRINTER_METRICS:
            // ignore - this is a dead property
        break;
        case HANDLE_TABS_RELATIVE_TO_INDENT:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT, bTmp);
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
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::PROTECT_FORM, bTmp);
        }
        break;
        // #i89181#
        case HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST, bTmp);
        }
        break;
        case HANDLE_MODIFYPASSWORDINFO:
        {
            uno::Sequence< beans::PropertyValue > aInfo;
            if ( !( rValue >>= aInfo ) )
                throw lang::IllegalArgumentException(
                    OUString( "Value of type Sequence<PropertyValue> expected!" ),
                    uno::Reference< uno::XInterface >(),
                    2 );

            if ( !mpDocSh->SetModifyPasswordInfo( aInfo ) )
                throw beans::PropertyVetoException(
                    OUString( "The hash is not allowed to be changed now!" ),
                    uno::Reference< uno::XInterface >() );
        }
        break;
        case HANDLE_MATH_BASELINE_ALIGNMENT:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT, bTmp );
        }
        break;
        case HANDLE_INVERT_BORDER_SPACING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::INVERT_BORDER_SPACING, bTmp);
        }
        break;
        case HANDLE_COLLAPSE_EMPTY_CELL_PARA:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA, bTmp);
        }
        break;
        case HANDLE_SMALL_CAPS_PERCENTAGE_66:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::SMALL_CAPS_PERCENTAGE_66, bTmp);
        }
        break;
        case HANDLE_TAB_OVERFLOW:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TAB_OVERFLOW, bTmp);
        }
        break;
        case HANDLE_UNBREAKABLE_NUMBERINGS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS, bTmp);
        }
        break;
        case HANDLE_STYLES_NODEFAULT:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::STYLES_NODEFAULT, bTmp);
        }
        break;
        case HANDLE_FLOATTABLE_NOMARGINS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::FLOATTABLE_NOMARGINS, bTmp);
        }
        break;
        case HANDLE_CLIPPED_PICTURES:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::CLIPPED_PICTURES, bTmp);
        }
        break;
        case HANDLE_BACKGROUND_PARA_OVER_DRAWINGS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::BACKGROUND_PARA_OVER_DRAWINGS, bTmp);
        }
        break;
        case HANDLE_EMBED_FONTS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::EMBED_FONTS, bTmp);
        }
        case HANDLE_EMBED_SYSTEM_FONTS:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::EMBED_SYSTEM_FONTS, bTmp);
        }
        break;
        case HANDLE_TAB_OVER_MARGIN:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::TAB_OVER_MARGIN, bTmp);
        }
        break;
        case HANDLE_SURROUND_TEXT_WRAP_SMALL:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->set(IDocumentSettingAccess::SURROUND_TEXT_WRAP_SMALL, bTmp);
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postSetValues ()
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    // set printer only once, namely here!
    if( mpPrinter != NULL )
    {
        // #i86352# the printer is also used as container for options by sfx
        // when setting a printer it should have decent default options
        SfxItemSet aOptions( mpPrinter->GetOptions() );
        SwPrintData aPrtData( mpDoc->getPrintData() );
        SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, aPrtData);
        aOptions.Put(aAddPrinterItem);
        mpPrinter->SetOptions( aOptions );

        mpDoc->setPrinter( mpPrinter, true, true );
    }

    mpPrinter = 0;
    mpDocSh = 0;
    mpDoc = 0;
}

void SwXDocumentSettings::_preGetValues ()
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    if (NULL == mpDocSh)
        throw UnknownPropertyException();
    mpDoc = mpDocSh->GetDoc();
    if (NULL == mpDoc)
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, uno::Any & rValue )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException )
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
            rValue <<= static_cast < sal_Int16 > ( mpDoc->getLinkUpdateMode(true) );
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            SwFldUpdateFlags nFlags = mpDoc->getFieldUpdateFlags(true);
            sal_Bool bFieldUpd = (nFlags == AUTOUPD_FIELD_ONLY || nFlags == AUTOUPD_FIELD_AND_CHARTS );
            rValue.setValue(&bFieldUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            SwFldUpdateFlags nFlags = mpDoc->getFieldUpdateFlags(true);
            sal_Bool bChartUpd = nFlags == AUTOUPD_FIELD_AND_CHARTS;
            rValue.setValue(&bChartUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace = mpDoc->get(IDocumentSettingAccess::PARA_SPACE_MAX);
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpace = mpDoc->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES);
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = mpDoc->get(IDocumentSettingAccess::TAB_COMPAT);
            rValue.setValue(&bAlignTab, ::getBooleanCppuType());
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            SfxPrinter *pPrinter = mpDoc->getPrinter( false );
            rValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            SfxPrinter *pPrinter = mpDoc->getPrinter( false );
            if (pPrinter)
            {
                SvMemoryStream aStream;
                pPrinter->Store( aStream );
                aStream.Seek ( STREAM_SEEK_TO_END );
                sal_uInt32 nSize = aStream.Tell();
                aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                Sequence < sal_Int8 > aSequence( nSize );
                aStream.Read ( aSequence.getArray(), nSize );
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
            sal_Bool bParaSpace = mpDoc->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION);
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            rValue <<= mpDocSh->IsUseUserData();
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            rValue <<= static_cast < sal_Int16 > (mpDoc->getCharacterCompressionType());
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = mpDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
            rValue.setValue(&bSaveGlobal, ::getBooleanCppuType());
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
            sal_Int16 nVirDevType = mpDoc->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ?
                                    ( mpDoc->get(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE) ?
                                      document::PrinterIndependentLayout::HIGH_RESOLUTION :
                                      document::PrinterIndependentLayout::LOW_RESOLUTION ) :
                                    document::PrinterIndependentLayout::DISABLED;
            rValue <<= nVirDevType;
        }
        break;
        case HANDLE_IS_LABEL_DOC:
        {
            sal_Bool bLabel = mpDoc->get(IDocumentSettingAccess::LABEL_DOCUMENT);
            rValue <<= bLabel;
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::ADD_FLY_OFFSETS);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::ADD_EXT_LEADING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_OLD_NUMBERING: // #111955#
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::OLD_NUMBERING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_OUTLINELEVEL_YIELDS_NUMBERING: // #111955#
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::OUTLINE_LEVEL_YIELDS_OUTLINE_RULE);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            sal_Bool bPrintCancelState = mpDocSh->Stamp_GetPrintCancelState();
            rValue.setValue(&bPrintCancelState, ::getBooleanCppuType());
        }
        break;
        case HANDLE_USE_FORMER_LINE_SPACING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::OLD_LINE_SPACING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_ADD_PARA_SPACING_TO_TABLE_CELLS:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_USE_FORMER_OBJECT_POSITIONING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_USE_FORMER_TEXT_WRAPPING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_CHANGES_PASSWORD:
        {
            rValue <<= mpDoc->GetRedlinePassword();
        }
        break;
        case HANDLE_CONSIDER_WRAP_ON_OBJPOS:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_TABLE_ROW_KEEP :
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::TABLE_ROW_KEEP);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_LOAD_READONLY:
        {
            rValue <<= mpDocSh->IsLoadReadonly();
        }
        break;
        case HANDLE_DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAMES:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_UNIX_FORCE_ZERO_EXT_LEADING:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_USE_OLD_PRINTER_METRICS:
        {
            sal_Bool bTmp = sal_False;
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_TABS_RELATIVE_TO_INDENT:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
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
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::PROTECT_FORM);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        // #i89181#
        case HANDLE_TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
        {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_MODIFYPASSWORDINFO:
        {
            rValue <<= mpDocSh->GetModifyPasswordInfo();
        }
        break;
        case HANDLE_MATH_BASELINE_ALIGNMENT:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
    case HANDLE_INVERT_BORDER_SPACING:
    {
            sal_Bool bTmp = mpDoc->get(IDocumentSettingAccess::INVERT_BORDER_SPACING);
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
    }
    break;
        case HANDLE_COLLAPSE_EMPTY_CELL_PARA:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_SMALL_CAPS_PERCENTAGE_66:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::SMALL_CAPS_PERCENTAGE_66 );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_TAB_OVERFLOW:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::TAB_OVERFLOW );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_UNBREAKABLE_NUMBERINGS:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_STYLES_NODEFAULT:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::STYLES_NODEFAULT );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_FLOATTABLE_NOMARGINS:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::FLOATTABLE_NOMARGINS );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_CLIPPED_PICTURES:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::CLIPPED_PICTURES );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_BACKGROUND_PARA_OVER_DRAWINGS:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::BACKGROUND_PARA_OVER_DRAWINGS );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_EMBED_FONTS:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::EMBED_FONTS );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_EMBED_SYSTEM_FONTS:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::EMBED_SYSTEM_FONTS );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_TAB_OVER_MARGIN:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::TAB_OVER_MARGIN );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_SURROUND_TEXT_WRAP_SMALL:
        {
            sal_Bool bTmp = mpDoc->get( IDocumentSettingAccess::SURROUND_TEXT_WRAP_SMALL );
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postGetValues ()
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    mpDocSh = 0;
    mpDoc = 0;
}

// XServiceInfo
OUString SAL_CALL SwXDocumentSettings::getImplementationName(  )
    throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Writer.DocumentSettings");
}

sal_Bool SAL_CALL SwXDocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SwXDocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException, std::exception)
{
    Sequence< OUString > aSeq( 3 );
    aSeq[0] = "com.sun.star.document.Settings";
    aSeq[1] = "com.sun.star.text.DocumentSettings";
    aSeq[2] = "com.sun.star.text.PrintSettings";
    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
