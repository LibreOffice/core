/*************************************************************************
 *
 *  $RCSfile: SwXDocumentSettings.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:01:13 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfx2/sfxbasecontroller.hxx>
#endif

#ifndef _SW_XDOCUMENT_SETTINGS_HXX
#include <SwXDocumentSettings.hxx>
#endif
#ifndef _SW_XPRINTPREVIEWSETTINGS_HXX_
#include <SwXPrintPreviewSettings.hxx>
#endif
#ifndef _COMPHELPER_MASTERPROPERTSETINFO_HXX_
#include <comphelper/MasterPropertySetInfo.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CHCMPRSE_HXX
#include <chcmprse.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#include <cmdid.h>
#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _COMPHELPER_TYPEGENERATION_HXX_
#include <comphelper/TypeGeneration.hxx>
#endif
#ifndef _UNOMOD_HXX
#include <unomod.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


using namespace rtl;
using namespace comphelper;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;

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
    /* Stampit It disable the print cancel button of the shown progress dialog. */
    HANDLE_ALLOW_PRINTJOB_CANCEL
};

MasterPropertySetInfo * lcl_createSettingsInfo()
{
    static PropertyInfo aWriterSettingsInfoMap[] =
    {
        { RTL_CONSTASCII_STRINGPARAM("ForbiddenCharacters"),        HANDLE_FORBIDDEN_CHARS,                 CPPUTYPE_REFFORBCHARS,      0,   0},
        { RTL_CONSTASCII_STRINGPARAM("LinkUpdateMode"),             HANDLE_LINK_UPDATE_MODE,                CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("FieldAutoUpdate"),            HANDLE_FIELD_AUTO_UPDATE,               CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ChartAutoUpdate"),            HANDLE_CHART_AUTO_UPDATE,               CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacing"),        HANDLE_ADD_PARA_TABLE_SPACING,          CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacingAtStart"), HANDLE_ADD_PARA_TABLE_SPACING_AT_START, CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AlignTabStopPosition"),       HANDLE_ALIGN_TAB_STOP_POSITION,         CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterName"),                HANDLE_PRINTER_NAME,                    CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterSetup"),               HANDLE_PRINTER_SETUP,                   CPPUTYPE_SEQINT8,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsKernAsianPunctuation"),     HANDLE_IS_KERN_ASIAN_PUNCTUATION,       CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CharacterCompressionType"),   HANDLE_CHARACTER_COMPRESSION_TYPE,      CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ApplyUserData"),              HANDLE_APPLY_USER_DATA,                 CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveGlobalDocumentLinks"),    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,      CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseDataSource"),  HANDLE_CURRENT_DATABASE_DATA_SOURCE,    CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommand"),     HANDLE_CURRENT_DATABASE_COMMAND,        CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommandType"), HANDLE_CURRENT_DATABASE_COMMAND_TYPE,   CPPUTYPE_INT32,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveVersionOnClose"),         HANDLE_SAVE_VERSION_ON_CLOSE,           CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("UpdateFromTemplate"),         HANDLE_UPDATE_FROM_TEMPLATE,            CPPUTYPE_BOOLEAN,           0,   0},

        { RTL_CONSTASCII_STRINGPARAM("PrinterIndependentLayout"),   HANDLE_PRINTER_INDEPENDENT_LAYOUT,      CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsLabelDocument"),            HANDLE_IS_LABEL_DOC,                    CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddFrameOffsets"),            HANDLE_IS_ADD_FLY_OFFSET,               CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddExternalLeading"),         HANDLE_IS_ADD_EXTERNAL_LEADING,         CPPUTYPE_BOOLEAN,           0,   0},
        /* Stampit It disable the print cancel button of the shown progress dialog. */
        { RTL_CONSTASCII_STRINGPARAM("AllowPrintJobCancel"),        HANDLE_ALLOW_PRINTJOB_CANCEL,           CPPUTYPE_BOOLEAN,           0,   0},
/*
 * As OS said, we don't have a view when we need to set this, so I have to
 * find another solution before adding them to this property set - MTG
        { RTL_CONSTASCII_STRINGPARAM("IsGridVisible"),              HANDLE_IS_GRID_VISIBLE,                 CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsSnapToGrid"),               HANDLE_IS_SNAP_TO_GRID,                 CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsSynchroniseAxes"),          HANDLE_IS_SYNCHRONISE_AXES,             CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("HorizontalGridResolution"),   HANDLE_HORIZONTAL_GRID_RESOLUTION,      CPPUTYPE_INT32,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("HorizontalGridSubdivision"),  HANDLE_HORIZONTAL_GRID_SUBDIVISION,     CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("VerticalGridResolution"),     HANDLE_VERTICAL_GRID_RESOLUTION,        CPPUTYPE_INT32,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("VerticalGridSubdivision"),    HANDLE_VERTICAL_GRID_SUBDIVISION,       CPPUTYPE_INT16,             0,   0},

        { RTL_CONSTASCII_STRINGPARAM("ShowParagraphEnd"),           HANDLE_SHOW_PARAGRAPH_END,              CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowOptionalHyphens"),        HANDLE_SHOW_OPTIONAL_HYPHENS,           CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowSpaces"),                 HANDLE_SHOW_SPACES,                     CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowTabs"),                   HANDLE_SHOW_TABS,                       CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowBreaks"),                 HANDLE_SHOW_BREAKS,                     CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowHiddenText"),             HANDLE_SHOW_HIDDEN_TEXT,                CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowHiddenParagraphs"),       HANDLE_SHOW_HIDDEN_PARAGRAPHS,          CPPUTYPE_BOOLEAN,           0,   0},

        { RTL_CONSTASCII_STRINGPARAM("ShowTextLimitGuide"),         HANDLE_SHOW_TEXT_LIMIT_GUIDE,           CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowTableLimitGuide"),        HANDLE_SHOW_TABLE_LIMIT_GUIDE,          CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowSectionLimitGuide"),      HANDLE_SHOW_SECTION_LIMIT_GUIDE,        CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowGuidesWhileMoving"),      HANDLE_SHOW_GUIDES_WHILE_MOVING,        CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowSimpleHandles"),          HANDLE_SHOW_SIMPLE_HANDLES,             CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ShowLargeHandles"),           HANDLE_SHOW_LARGE_HANDLES,              CPPUTYPE_BOOLEAN,           0,   0},
*/
        { NULL, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
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
    registerSlave ( new SwXPrintPreviewSettings ( mpModel->GetDocShell()->GetDoc() ) );
}

SwXDocumentSettings::~SwXDocumentSettings()
    throw()
{
}

Any SAL_CALL SwXDocumentSettings::queryInterface( const Type& rType )
    throw(RuntimeException)
{
        return ::cppu::queryInterface ( rType,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface* > ( this ),
                                        static_cast< XWeak* > ( this ),
                                        // my own interfaces
                                        static_cast< XPropertySet*  > ( this ),
                                        static_cast< XPropertyState* > ( this ),
                                        static_cast< XMultiPropertySet* > ( this ),
                                        static_cast< XServiceInfo* > ( this ),
                                        static_cast< XTypeProvider* > ( this ) );
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
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Sequence< uno::Type > aBaseTypes( 5 );
    uno::Type* pBaseTypes = aBaseTypes.getArray();

    // from MasterPropertySet
    pBaseTypes[0] = ::getCppuType((Reference< XPropertySet >*)0);
    pBaseTypes[1] = ::getCppuType((Reference< XPropertyState >*)0);
    pBaseTypes[2] = ::getCppuType((Reference< XMultiPropertySet >*)0);
    //
    pBaseTypes[3] = ::getCppuType((Reference< XServiceInfo >*)0);
    pBaseTypes[4] = ::getCppuType((Reference< XTypeProvider >*)0);

    return aBaseTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXDocumentSettings::getImplementationId(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

void SwXDocumentSettings::_preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    mpDoc = mpDocSh->GetDoc();

    if( NULL == mpDoc || NULL == mpDocSh )
        throw UnknownPropertyException();

}
void SwXDocumentSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    switch( rInfo.mnHandle )
    {
        case HANDLE_FORBIDDEN_CHARS:
            break;
        case HANDLE_LINK_UPDATE_MODE:
        {
            sal_Int16 nMode;
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
            mpDoc->SetLinkUpdMode(nMode);
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            sal_Bool bUpdateField = *(sal_Bool*)rValue.getValue();
            sal_Int16 nFlag = mpDoc->GetFldUpdateFlags();
            mpDoc->SetFldUpdateFlags( bUpdateField ?
                    nFlag == AUTOUPD_FIELD_AND_CHARTS ? AUTOUPD_FIELD_AND_CHARTS
                    : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            sal_Bool bUpdateChart = *(sal_Bool*)rValue.getValue();
            sal_Int16 nFlag = mpDoc->GetFldUpdateFlags();
            mpDoc->SetFldUpdateFlags ( (nFlag == AUTOUPD_FIELD_ONLY || nFlag == AUTOUPD_FIELD_AND_CHARTS )
                    ? bUpdateChart ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace;
            rValue >>= bParaSpace;
            mpDoc->SetParaSpaceMax( bParaSpace, mpDoc->IsParaSpaceMaxAtPages());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpacePage;
            rValue >>= bParaSpacePage;
            mpDoc->SetParaSpaceMax( mpDoc->IsParaSpaceMax(), bParaSpacePage);
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = *(sal_Bool*)rValue.getValue();
            mpDoc->SetTabCompat( bAlignTab );
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            //the printer must be created
            OUString sPrinterName;
            if( rValue >>= sPrinterName  )
            {
                if( !mpPrinter && sPrinterName.getLength() > 0 )
                {
                    SfxPrinter* pPrinter = mpDoc->GetPrt( sal_True );
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
                    static sal_uInt16 __READONLY_DATA nRange[] =
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
            mpDoc->SetKernAsianPunctuation( bIsKern );
            SwEditShell* pEditSh = mpDoc->GetEditShell();
            if(pEditSh)
                pEditSh->ChgHyphenation();
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            sal_Int16 nMode;
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
            mpDoc->SetCharCompressType(static_cast < SwCharCompressType > (nMode) );
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            SfxDocumentInfo& rInfo = mpDocSh->GetDocInfo();
            sal_Bool bUseUserData = *(sal_Bool*)rValue.getValue();
            rInfo.SetUseUserData(bUseUserData);
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = *(sal_Bool*)rValue.getValue();
            mpDoc->SetGlblDocSaveLinks( bSaveGlobal );
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
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.nCommandType )
                mpDoc->ChgDBData( aData );
        }
        break;
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            SfxDocumentInfo& rInfo = mpDocSh->GetDocInfo();
            sal_Bool bSaveVersion = *(sal_Bool*)rValue.getValue();
            rInfo.SetSaveVersionOnClose ( bSaveVersion );
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDocSh->GetDocInfo().SetQueryLoadTemplate( bTmp );
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            sal_Int16 nTmp;
            rValue >>= nTmp;
            if( (nTmp == document::PrinterIndependentLayout::DISABLED ) ||
                (nTmp == document::PrinterIndependentLayout::LOW_RESOLUTION ) ||
                (nTmp == document::PrinterIndependentLayout::HIGH_RESOLUTION ) )
                mpDoc->SetUseVirtualDevice( nTmp );
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_IS_LABEL_DOC :
        {
            sal_Bool bSet;
            if(!(rValue >>= bSet))
                throw IllegalArgumentException();
            mpDoc->SetLabelDoc(bSet);
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->SetAddFlyOffsets( bTmp );
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->SetAddExtLeading( bTmp );
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            sal_Bool bState;
            if (!(rValue >>= bState))
                throw IllegalArgumentException();
            mpDocSh->Stamp_SetPrintCancelState(bState);
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    // set printer only once, namely here!
    if( mpPrinter != NULL )
        mpDoc->SetPrt( mpPrinter, sal_True );

    mpPrinter = 0;
    mpDocSh = 0;
    mpDoc = 0;
}

void SwXDocumentSettings::_preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    mpDoc = mpDocSh->GetDoc();
    if( NULL == mpDoc || NULL == mpDocSh )
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException )
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
            rValue <<= static_cast < sal_Int16 > ( mpDoc->GetLinkUpdMode() );
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            sal_uInt16 nFlags = mpDoc->GetFldUpdateFlags();
            BOOL bFieldUpd = (nFlags == AUTOUPD_FIELD_ONLY || nFlags == AUTOUPD_FIELD_AND_CHARTS );
            rValue.setValue(&bFieldUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            sal_uInt16 nFlags = mpDoc->GetFldUpdateFlags();
            BOOL bChartUpd = nFlags == AUTOUPD_FIELD_AND_CHARTS;
            rValue.setValue(&bChartUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace = mpDoc->IsParaSpaceMax();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpace = mpDoc->IsParaSpaceMaxAtPages();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = mpDoc->IsTabCompat();
            rValue.setValue(&bAlignTab, ::getBooleanCppuType());
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            SfxPrinter *pPrinter = mpDoc->GetPrt ( sal_False );
            rValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            SfxPrinter *pPrinter = mpDocSh->GetDoc()->GetPrt ( sal_False );
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
            sal_Bool bParaSpace = mpDoc->IsKernAsianPunctuation();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            SfxDocumentInfo &rInfo = mpDocSh->GetDocInfo();
            sal_Bool bUseUserInfo = rInfo.IsUseUserData();
            rValue.setValue(&bUseUserInfo, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            rValue <<= static_cast < sal_Int16 > (mpDoc->GetCharCompressType());
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = mpDoc->IsGlblDocSaveLinks();
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
            SfxDocumentInfo& rInfo = mpDocSh->GetDocInfo();
            sal_Bool bSaveVersion = rInfo.IsSaveVersionOnClose();
            rValue.setValue(&bSaveVersion, ::getBooleanCppuType());
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            sal_Bool bTmp = mpDocSh->GetDocInfo().IsQueryLoadTemplate();
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            rValue <<= mpDoc->IsUseVirtualDevice();
        }
        break;
        case HANDLE_IS_LABEL_DOC:
        {
            sal_Bool bLabel = mpDoc->IsLabelDoc();
            rValue <<= bLabel;
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = mpDoc->IsAddFlyOffsets();
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_IS_ADD_EXTERNAL_LEADING:
        {
            sal_Bool bTmp = mpDoc->IsAddExtLeading();
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_ALLOW_PRINTJOB_CANCEL:
        {
            sal_Bool bPrintCancelState = mpDocSh->Stamp_GetPrintCancelState();
            rValue.setValue(&bPrintCancelState, ::getBooleanCppuType());
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = 0;
    mpDoc = 0;
}

// XServiceInfo
OUString SAL_CALL SwXDocumentSettings::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.DocumentSettings"));
}

sal_Bool SAL_CALL SwXDocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    const Sequence< OUString > aSeq( getSupportedServiceNames() );
    sal_Int32 nCount = aSeq.getLength();
    const OUString* pServices = aSeq.getConstArray();
    while( nCount-- )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }
    return sal_True;
}

Sequence< OUString > SAL_CALL SwXDocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq( 3 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings") );
    aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.Settings") );
    aSeq[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.DocumentSettings") );
    return aSeq;
}


