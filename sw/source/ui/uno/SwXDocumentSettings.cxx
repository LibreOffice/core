/*************************************************************************
 *
 *  $RCSfile: SwXDocumentSettings.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mtg $ $Date: 2001-07-20 12:54:09 $
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
#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif
#pragma hdrstop

#ifndef _SW_XDOCUMENT_SETTINGS_HXX
#include <SwXDocumentSettings.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
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
    HANDLE_PRINTER_NAME,
    HANDLE_PRINTER_SETUP,
    HANDLE_IS_KERN_ASIAN_PUNCTUATION,
    HANDLE_CHARACTER_COMPRESSION_TYPE,
    HANDLE_APPLY_USER_DATA,
    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,
    HANDLE_CURRENT_DATABASE_DATA_SOURCE,
    HANDLE_CURRENT_DATABASE_COMMAND,
    HANDLE_CURRENT_DATABASE_COMMAND_TYPE,
    HANDLE_SAVE_VERSION_ON_CLOSE
};
PropertySetInfo * lcl_createSettingsInfo()
{
    static PropertyMapEntry aWriterSettingsInfoMap[] =
    {
        { RTL_CONSTASCII_STRINGPARAM("ForbiddenCharacters"),        HANDLE_FORBIDDEN_CHARS,                 &::getCppuType((Reference<XForbiddenCharacters>*)0),    0,   0},
        { RTL_CONSTASCII_STRINGPARAM("LinkUpdateMode"),             HANDLE_LINK_UPDATE_MODE,                &::getCppuType((sal_Int16*)0),          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("FieldAutoUpdate"),            HANDLE_FIELD_AUTO_UPDATE,               &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ChartAutoUpdate"),            HANDLE_CHART_AUTO_UPDATE,               &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacing"),        HANDLE_ADD_PARA_TABLE_SPACING,          &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacingAtStart"), HANDLE_ADD_PARA_TABLE_SPACING_AT_START, &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterName"),                HANDLE_PRINTER_NAME,                    &::getCppuType((const OUString*)0),     0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterSetup"),               HANDLE_PRINTER_SETUP,                   &::getCppuType((const uno::Sequence < sal_Int8 > *)0),  0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsKernAsianPunctuation"),     HANDLE_IS_KERN_ASIAN_PUNCTUATION,       &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CharacterCompressionType"),   HANDLE_CHARACTER_COMPRESSION_TYPE,      &::getCppuType((sal_Int16*)0),          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ApplyUserData"),              HANDLE_APPLY_USER_DATA,                 &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveGlobalDocumentLinks"),    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,      &::getBooleanCppuType(),                0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseDataSource"),  HANDLE_CURRENT_DATABASE_DATA_SOURCE,    &::getCppuType((const OUString*)0),     0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommand"),     HANDLE_CURRENT_DATABASE_COMMAND,        &::getCppuType((const OUString*)0),     0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommandType"), HANDLE_CURRENT_DATABASE_COMMAND_TYPE,   &::getCppuType((const sal_Int16*)0),    0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveVersionOnClose"),         HANDLE_SAVE_VERSION_ON_CLOSE,           &::getBooleanCppuType(),                0,   0},
        { NULL, 0, 0, NULL, 0, 0 }
    };

    PropertySetInfo *pInfo = new PropertySetInfo ( aWriterSettingsInfoMap );
    return pInfo;
}

SwXDocumentSettings::SwXDocumentSettings ( SwXTextDocument * pModel )
: PropertySetHelper ( lcl_createSettingsInfo () )
, mxModel ( pModel )
, mpModel ( pModel )
{
}

SwXDocumentSettings::~SwXDocumentSettings()
{
}
Any SAL_CALL SwXDocumentSettings::queryInterface( const Type& rType )
    throw(RuntimeException)
{
        return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface*       > ( this )  ,
                                        static_cast< XWeak*         > ( this )  ,
                                        // my own interfaces
                                        static_cast< XPropertySet*      > ( this )  ,
                                        static_cast< XPropertyState*        > ( this )  ,
                                        static_cast< XMultiPropertySet*     > ( this ) );
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

void SwXDocumentSettings::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    SwDocShell* pDocSh = mpModel->GetDocShell();
    SwDoc* pDoc = pDocSh->GetDoc();

    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    for( ; *ppEntries; ppEntries++, pValues++ )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_FORBIDDEN_CHARS:
                break;
            case HANDLE_LINK_UPDATE_MODE:
            {
                sal_Int16 nMode;
                *pValues >>= nMode;
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
                pDoc->SetLinkUpdMode(nMode);
            }
            break;
            case HANDLE_FIELD_AUTO_UPDATE:
            {
                sal_Bool bUpdateField = *(sal_Bool*)(*pValues).getValue();
                sal_Int16 nFlag = pDoc->GetFldUpdateFlags();
                pDoc->SetFldUpdateFlags( bUpdateField ?
                        nFlag == AUTOUPD_FIELD_AND_CHARTS ? AUTOUPD_FIELD_AND_CHARTS
                        : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
            }
            break;
            case HANDLE_CHART_AUTO_UPDATE:
            {
                sal_Bool bUpdateChart = *(sal_Bool*)(*pValues).getValue();
                sal_Int16 nFlag = pDoc->GetFldUpdateFlags();
                pDoc->SetFldUpdateFlags ( (nFlag == AUTOUPD_FIELD_ONLY || nFlag == AUTOUPD_FIELD_AND_CHARTS )
                        ? bUpdateChart ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
            }
            break;
            case HANDLE_ADD_PARA_TABLE_SPACING:
            {
                sal_Bool bParaSpace;
                *pValues >>= bParaSpace;
                pDoc->SetParaSpaceMax( bParaSpace, pDoc->IsParaSpaceMaxAtPages());
            }
            break;
            case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
            {
                sal_Bool bParaSpacePage;
                *pValues >>= bParaSpacePage;
                pDoc->SetParaSpaceMax( pDoc->IsParaSpaceMax(), bParaSpacePage);
            }
            break;
            case HANDLE_PRINTER_NAME:
            {
                SfxPrinter *pPrinter = pDoc->GetPrt ( sal_False );
                if (pPrinter)
                {
                    OUString sPrinterName;
                    if (*pValues >>= sPrinterName )
                    {
                        SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), sPrinterName );
                        if (pNewPrinter->IsKnown())
                            pDoc->SetPrt ( pNewPrinter );
                        else
                            delete pNewPrinter;
                    }
                    else
                        throw IllegalArgumentException();
                }
            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                Sequence < sal_Int8 > aSequence;
                if ( *pValues >>= aSequence )
                {
                    sal_uInt32 nSize = aSequence.getLength();
                    SvMemoryStream aStream (aSequence.getArray(), nSize, STREAM_READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    static sal_uInt16 __READONLY_DATA nRange[] =
                    {
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_HTML_MODE,  SID_HTML_MODE,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0
                    };
                    SfxItemSet *pItemSet = new SfxItemSet( pDoc->GetAttrPool(), nRange );
                    SfxPrinter *pPrinter = SfxPrinter::Create ( aStream, pItemSet );

                    pDoc->SetPrt( pPrinter );

                    if ( !pPrinter->IsOriginal() )
                    {
                        pDocSh->UpdateFontList();
                        SdrModel * pDrawModel = pDoc->GetDrawModel();
                        if ( pDrawModel )
                            pDrawModel->SetRefDevice( pPrinter );
                        pDoc->SetOLEPrtNotifyPending();
                    }
                }
                else
                    throw IllegalArgumentException();
            }
            break;
            case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
            {
                sal_Bool bIsKern = *(sal_Bool*)(*pValues).getValue();
                pDoc->SetKernAsianPunctuation( bIsKern );
                SwEditShell* pEditSh = pDoc->GetEditShell();
                if(pEditSh)
                    pEditSh->ChgHyphenation();
            }
            break;
            case HANDLE_CHARACTER_COMPRESSION_TYPE:
            {
                sal_Int16 nMode;
                *pValues >>= nMode;
                switch (nMode)
                {
                    case CHARCOMPRESS_NONE:
                    case CHARCOMPRESS_PUNCTUATION:
                    case CHARCOMPRESS_PUNCTUATION_KANA:
                        break;
                    default:
                        throw IllegalArgumentException();
                }
                pDoc->SetCharCompressType(static_cast < SwCharCompressType > (nMode) );
            }
            break;
            case HANDLE_APPLY_USER_DATA:
            {
                SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                sal_Bool bUseUserData = *(sal_Bool*)(*pValues).getValue();
                rInfo.SetUseUserData(bUseUserData);
            }
            break;
            case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
            {
                sal_Bool bSaveGlobal = *(sal_Bool*)(*pValues).getValue();
                pDoc->SetGlblDocSaveLinks( bSaveGlobal );
            }
            break;
            case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
            {
                SwDBData& rData = pDoc->GetDBData();
                if ( *pValues >>= rData.sDataSource )
                    pDoc->ChgDBData( rData );
            }
            break;
            case HANDLE_CURRENT_DATABASE_COMMAND:
            {
                SwDBData& rData = pDoc->GetDBData();
                if ( *pValues >>= rData.sCommand )
                    pDoc->ChgDBData( rData );
            }
            break;
            case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
            {
                SwDBData& rData = pDoc->GetDBData();
                if ( *pValues >>= rData.nCommandType )
                    pDoc->ChgDBData( rData );
            }
            break;
            case HANDLE_SAVE_VERSION_ON_CLOSE:
            {
                SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                sal_Bool bSaveVersion = *(sal_Bool*)(*pValues).getValue();
                rInfo.SetSaveVersionOnClose ( bSaveVersion );
            }
            break;
            default:
                throw UnknownPropertyException();
        }
    }
}
void SwXDocumentSettings::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    SwDocShell* pDocSh = mpModel->GetDocShell();
    SwDoc* pDoc = pDocSh->GetDoc();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    for( ; *ppEntries; ppEntries++, pValue++ )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_FORBIDDEN_CHARS:
            {
                Reference<XForbiddenCharacters> xRet(*mpModel->GetPropertyHelper(), UNO_QUERY);
                *pValue <<= xRet;
            }
            break;
            case HANDLE_LINK_UPDATE_MODE:
            {
                *pValue <<= static_cast < sal_Int16 > ( pDoc->GetLinkUpdMode() );
            }
            break;
            case HANDLE_FIELD_AUTO_UPDATE:
            {
                sal_uInt16 nFlags = pDoc->GetFldUpdateFlags();
                BOOL bFieldUpd = (nFlags == AUTOUPD_FIELD_ONLY || nFlags == AUTOUPD_FIELD_AND_CHARTS );
                (*pValue).setValue(&bFieldUpd, ::getBooleanCppuType());
            }
            break;
            case HANDLE_CHART_AUTO_UPDATE:
            {
                sal_uInt16 nFlags = pDoc->GetFldUpdateFlags();
                BOOL bChartUpd = nFlags == AUTOUPD_FIELD_AND_CHARTS;
                (*pValue).setValue(&bChartUpd, ::getBooleanCppuType());
            }
            break;
            case HANDLE_ADD_PARA_TABLE_SPACING:
            {
                sal_Bool bParaSpace = pDoc->IsParaSpaceMax();
                (*pValue).setValue(&bParaSpace, ::getBooleanCppuType());
            }
            break;
            case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
            {
                sal_Bool bParaSpace = pDoc->IsParaSpaceMaxAtPages();
                (*pValue).setValue(&bParaSpace, ::getBooleanCppuType());
            }
            break;
            case HANDLE_PRINTER_NAME:
            {
                SfxPrinter *pPrinter = pDoc->GetPrt ( sal_False );
                *pValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();

            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                SfxPrinter *pPrinter = pDocSh->GetDoc()->GetPrt ( sal_False );
                if (pPrinter)
                {
                    SvMemoryStream aStream;
                    pPrinter->Store( aStream );
                    aStream.Seek ( STREAM_SEEK_TO_END );
                    sal_uInt32 nSize = aStream.Tell();
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    Sequence < sal_Int8 > aSequence( nSize );
                    aStream.Read ( aSequence.getArray(), nSize );
                    *pValue <<= aSequence;
                }
            }
            break;
            case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
            {
                sal_Bool bParaSpace = pDoc->IsKernAsianPunctuation();
                (*pValue).setValue(&bParaSpace, ::getBooleanCppuType());
            }
            break;
            case HANDLE_APPLY_USER_DATA:
            {
                SfxDocumentInfo &rInfo = pDocSh->GetDocInfo();
                sal_Bool bUseUserInfo = rInfo.IsUseUserData();
                (*pValue).setValue(&bUseUserInfo, ::getBooleanCppuType());
            }
            break;
            case HANDLE_CHARACTER_COMPRESSION_TYPE:
            {
                *pValue <<= static_cast < sal_Int16 > (pDoc->GetCharCompressType());
            }
            break;
            case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
            {
                sal_Bool bSaveGlobal = pDoc->IsGlblDocSaveLinks();
                (*pValue).setValue(&bSaveGlobal, ::getBooleanCppuType());
            }
            break;
            case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
            {
                const SwDBData& rData = pDoc->GetDBDesc();
                *pValue <<= rData.sDataSource;
            }
            break;
            case HANDLE_CURRENT_DATABASE_COMMAND:
            {
                const SwDBData& rData = pDoc->GetDBDesc();
                *pValue <<= rData.sCommand;
            }
            break;
            case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
            {
                const SwDBData& rData = pDoc->GetDBDesc();
                *pValue <<= rData.nCommandType;
            }
            break;
            case HANDLE_SAVE_VERSION_ON_CLOSE:
            {
                SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                sal_Bool bSaveVersion = rInfo.IsSaveVersionOnClose();
                (*pValue).setValue(&bSaveVersion, ::getBooleanCppuType());
            }
            break;
            default:
                throw UnknownPropertyException();
        }
    }
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
    Sequence< OUString > aSeq( 2 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings") );
    aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.Settings") );
    return aSeq;
}
