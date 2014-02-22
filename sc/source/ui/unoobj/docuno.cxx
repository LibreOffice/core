/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scitems.hxx"
#include <svx/fmdpage.hxx>
#include <svx/fmview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <svx/unoshape.hxx>

#include <officecfg/Office/Common.hxx>
#include <svl/numuno.hxx>
#include <svl/smplhint.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/charclass.hxx>
#include <tools/multisel.hxx>
#include <tools/resary.hxx>
#include <toolkit/awt/vclxdevice.hxx>

#include <ctype.h>
#include <float.h>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XSelectedSheetsSupplier.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "docuno.hxx"
#include "cellsuno.hxx"
#include "nameuno.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "notesuno.hxx"
#include "styleuno.hxx"
#include "linkuno.hxx"
#include "servuno.hxx"
#include "targuno.hxx"
#include "convuno.hxx"
#include "optuno.hxx"
#include "forbiuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "docfunc.hxx"
#include "postit.hxx"
#include "dociter.hxx"
#include "formulacell.hxx"
#include "drwlayer.hxx"
#include "rangeutl.hxx"
#include "markdata.hxx"
#include "docoptio.hxx"
#include "unonames.hxx"
#include "shapeuno.hxx"
#include "viewuno.hxx"
#include "tabvwsh.hxx"
#include "printfun.hxx"
#include "pfuncache.hxx"
#include "scmod.hxx"
#include "ViewSettingsSequenceDefines.hxx"
#include "sheetevents.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "platforminfo.hxx"
#include "interpre.hxx"
#include "formulagroup.hxx"

using namespace com::sun::star;


#define SC_UNO_VBAGLOBNAME "VBAGlobalConstantName"




static const SfxItemPropertyMapEntry* lcl_GetDocOptPropertyMap()
{
    static const SfxItemPropertyMapEntry aDocOptPropertyMap_Impl[] =
    {
        {OUString(SC_UNO_APPLYFMDES),              0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_AREALINKS),               0, getCppuType((uno::Reference<sheet::XAreaLinks>*)0),               0, 0},
        {OUString(SC_UNO_AUTOCONTFOC),             0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_BASICLIBRARIES),          0, getCppuType((uno::Reference< script::XLibraryContainer >*)0),     beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_DIALOGLIBRARIES),         0, getCppuType((uno::Reference< script::XLibraryContainer >*)0),     beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_VBAGLOBNAME),             0, getCppuType(static_cast< const OUString * >(0)),                  beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_CALCASSHOWN),             PROP_UNO_CALCASSHOWN, getBooleanCppuType(),                          0, 0},
        {OUString(SC_UNONAME_CLOCAL),              0, getCppuType((lang::Locale*)0),                                    0, 0},
        {OUString(SC_UNO_CJK_CLOCAL),              0, getCppuType((lang::Locale*)0),                                    0, 0},
        {OUString(SC_UNO_CTL_CLOCAL),              0, getCppuType((lang::Locale*)0),                                    0, 0},
        {OUString(SC_UNO_COLLABELRNG),             0, getCppuType((uno::Reference<sheet::XLabelRanges>*)0),             0, 0},
        {OUString(SC_UNO_DDELINKS),                0, getCppuType((uno::Reference<container::XNameAccess>*)0),          0, 0},
        {OUString(SC_UNO_DEFTABSTOP),              PROP_UNO_DEFTABSTOP, getCppuType((sal_Int16*)0),                     0, 0},
        {OUString(SC_UNO_EXTERNALDOCLINKS),        0, getCppuType((uno::Reference<sheet::XExternalDocLinks>*)0),        0, 0},
        {OUString(SC_UNO_FORBIDDEN),               0, getCppuType((uno::Reference<i18n::XForbiddenCharacters>*)0),      beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_HASDRAWPAGES),            0, getBooleanCppuType(),                                             beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_IGNORECASE),              PROP_UNO_IGNORECASE, getBooleanCppuType(),                           0, 0},
        {OUString(SC_UNO_ITERENABLED),             PROP_UNO_ITERENABLED, getBooleanCppuType(),                          0, 0},
        {OUString(SC_UNO_ITERCOUNT),               PROP_UNO_ITERCOUNT, getCppuType((sal_Int32*)0),                      0, 0},
        {OUString(SC_UNO_ITEREPSILON),             PROP_UNO_ITEREPSILON, getCppuType((double*)0),                       0, 0},
        {OUString(SC_UNO_LOOKUPLABELS),            PROP_UNO_LOOKUPLABELS, getBooleanCppuType(),                         0, 0},
        {OUString(SC_UNO_MATCHWHOLE),              PROP_UNO_MATCHWHOLE, getBooleanCppuType(),                           0, 0},
        {OUString(SC_UNO_NAMEDRANGES),             0, getCppuType((uno::Reference<sheet::XNamedRanges>*)0),             0, 0},
        {OUString(SC_UNO_DATABASERNG),             0, getCppuType((uno::Reference<sheet::XDatabaseRanges>*)0),          0, 0},
        {OUString(SC_UNO_NULLDATE),                PROP_UNO_NULLDATE, getCppuType((util::Date*)0),                      0, 0},
        {OUString(SC_UNO_ROWLABELRNG),             0, getCppuType((uno::Reference<sheet::XLabelRanges>*)0),             0, 0},
        {OUString(SC_UNO_SHEETLINKS),              0, getCppuType((uno::Reference<container::XNameAccess>*)0),          0, 0},
        {OUString(SC_UNO_SPELLONLINE),             PROP_UNO_SPELLONLINE, getBooleanCppuType(),                          0, 0},
        {OUString(SC_UNO_STANDARDDEC),             PROP_UNO_STANDARDDEC, getCppuType((sal_Int16*)0),                    0, 0},
        {OUString(SC_UNO_REGEXENABLED),            PROP_UNO_REGEXENABLED, getBooleanCppuType(),                         0, 0},
        {OUString(SC_UNO_RUNTIMEUID),              0, getCppuType(static_cast< const OUString * >(0)),                  beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_HASVALIDSIGNATURES),      0, getBooleanCppuType(),                                             beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_ISLOADED),                0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_ISUNDOENABLED),           0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_ISADJUSTHEIGHTENABLED),   0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_ISEXECUTELINKENABLED),    0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_ISCHANGEREADONLYENABLED), 0, getBooleanCppuType(),                                             0, 0},
        {OUString(SC_UNO_REFERENCEDEVICE),         0, getCppuType((uno::Reference<awt::XDevice>*)0),                    beans::PropertyAttribute::READONLY, 0},
        {OUString("BuildId"),                      0, ::getCppuType(static_cast< const OUString * >(0)),                0, 0},
        {OUString(SC_UNO_CODENAME),                0, getCppuType(static_cast< const OUString * >(0)),                  0, 0},
        {OUString(SC_UNO_INTEROPGRABBAG),          0, ::getCppuType((uno::Sequence< beans::PropertyValue >*)0), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDocOptPropertyMap_Impl;
}



static const SfxItemPropertyMapEntry* lcl_GetColumnsPropertyMap()
{
    static const SfxItemPropertyMapEntry aColumnsPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_MANPAGE),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_NEWPAGE),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_CELLVIS),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_OWIDTH),   0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_CELLWID),  0,  getCppuType((sal_Int32*)0),    0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aColumnsPropertyMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetRowsPropertyMap()
{
    static const SfxItemPropertyMapEntry aRowsPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_CELLHGT),  0,  getCppuType((sal_Int32*)0),    0, 0 },
        {OUString(SC_UNONAME_CELLFILT), 0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_OHEIGHT),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_MANPAGE),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_NEWPAGE),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_CELLVIS),  0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_CELLBACK), ATTR_BACKGROUND, ::getCppuType((const sal_Int32*)0), 0, MID_BACK_COLOR },
        {OUString(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND, ::getBooleanCppuType(), 0, MID_GRAPHIC_TRANSPARENT },
        
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aRowsPropertyMap_Impl;
}

using sc::HMMToTwips;
using sc::TwipsToHMM;



#define SCMODELOBJ_SERVICE          "com.sun.star.sheet.SpreadsheetDocument"
#define SCDOCSETTINGS_SERVICE       "com.sun.star.sheet.SpreadsheetDocumentSettings"
#define SCDOC_SERVICE               "com.sun.star.document.OfficeDocument"

SC_SIMPLE_SERVICE_INFO( ScAnnotationsObj, "ScAnnotationsObj", "com.sun.star.sheet.CellAnnotations" )
SC_SIMPLE_SERVICE_INFO( ScDrawPagesObj, "ScDrawPagesObj", "com.sun.star.drawing.DrawPages" )
SC_SIMPLE_SERVICE_INFO( ScScenariosObj, "ScScenariosObj", "com.sun.star.sheet.Scenarios" )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettingsObj, "ScSpreadsheetSettingsObj", "com.sun.star.sheet.SpreadsheetDocumentSettings" )
SC_SIMPLE_SERVICE_INFO( ScTableColumnsObj, "ScTableColumnsObj", "com.sun.star.table.TableColumns" )
SC_SIMPLE_SERVICE_INFO( ScTableRowsObj, "ScTableRowsObj", "com.sun.star.table.TableRows" )
SC_SIMPLE_SERVICE_INFO( ScTableSheetsObj, "ScTableSheetsObj", "com.sun.star.sheet.Spreadsheets" )



class ScPrintUIOptions : public vcl::PrinterOptionsHelper
{
public:
    ScPrintUIOptions();
    void SetDefaults();
};

ScPrintUIOptions::ScPrintUIOptions()
{
    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    sal_Bool bSuppress = rPrintOpt.GetSkipEmpty();

    ResStringArray aStrings( ScResId( SCSTR_PRINT_OPTIONS ) );
    OSL_ENSURE( aStrings.Count() >= 10, "resource incomplete" );
    if( aStrings.Count() < 10 ) 
        return;

    sal_Int32 nNumProps= 9, nIdx = 0;

    m_aUIProperties.realloc(nNumProps);

    
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/scalc/ui/printeroptions.ui");

    
    SvtModuleOptions aOpt;
    OUString aAppGroupname( aStrings.GetString( 9 ) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aOpt.GetModuleName( SvtModuleOptions::E_SCALC ) );
    m_aUIProperties[nIdx++].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, OUString());

    
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pages", OUString(aStrings.GetString(0)), OUString());

    
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("includeemptypages", OUString( aStrings.GetString( 1 ) ),
                                                  ".HelpID:vcl:PrintDialog:IsIncludeEmptyPages:CheckBox",
                                                  "IsIncludeEmptyPages",
                                                  ! bSuppress);
    
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", OUString(aStrings.GetString(2)),
                                                      OUString(),
                                                      aPrintRangeOpt);

    
    uno::Sequence< OUString > aChoices( 3 ), aHelpIds( 3 ), aWidgetIds( 3 );
    aChoices[0] = aStrings.GetString( 3 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0";
    aWidgetIds[0] = "printallsheets";
    aChoices[1] = aStrings.GetString( 4 );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1";
    aWidgetIds[1] = "printselectedsheets";
    aChoices[2] = aStrings.GetString( 5 );
    aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2";
    aWidgetIds[2] = "printselectedcells";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds, "PrintContent",
                                                    aChoices, nContent );

    
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("fromwhich", OUString(aStrings.GetString(6)),
                                                      OUString(),
                                                      aPrintRangeOpt);

    
    OUString aPrintRangeName( "PrintRange" );
    aChoices.realloc( 2 );
    aHelpIds.realloc( 2 );
    aWidgetIds.realloc( 2 );
    aChoices[0] = aStrings.GetString( 7 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:0";
    aWidgetIds[0] = "printallpages";
    aChoices[1] = aStrings.GetString( 8 );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:1";
    aWidgetIds[1] = "printpages";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds,
                                                    aPrintRangeName,
                                                    aChoices,
                                                    0 );

    
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
    m_aUIProperties[nIdx++].Value = setEditControlOpt("pagerange", OUString(),
                                                      ".HelpID:vcl:PrintDialog:PageRange:Edit",
                                                      "PageRange", OUString(), aPageRangeOpt);

    assert(nIdx == nNumProps);
}

void ScPrintUIOptions::SetDefaults()
{
    

    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    sal_Bool bSuppress = rPrintOpt.GetSkipEmpty();

    for (sal_Int32 nUIPos=0; nUIPos<m_aUIProperties.getLength(); ++nUIPos)
    {
        uno::Sequence<beans::PropertyValue> aUIProp;
        if ( m_aUIProperties[nUIPos].Value >>= aUIProp )
        {
            for (sal_Int32 nPropPos=0; nPropPos<aUIProp.getLength(); ++nPropPos)
            {
                OUString aName = aUIProp[nPropPos].Name;
                if ( aName == "Property" )
                {
                    beans::PropertyValue aPropertyValue;
                    if ( aUIProp[nPropPos].Value >>= aPropertyValue )
                    {
                        if ( aPropertyValue.Name == "PrintContent" )
                        {
                            aPropertyValue.Value <<= nContent;
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                        else if ( aPropertyValue.Name == "IsIncludeEmptyPages" )
                        {
                            ScUnoHelpFunctions::SetBoolInAny( aPropertyValue.Value, ! bSuppress );
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                    }
                }
            }
            m_aUIProperties[nUIPos].Value <<= aUIProp;
        }
    }
}

void ScModelObj::CreateAndSet(ScDocShell* pDocSh)
{
    if (pDocSh)
        pDocSh->SetBaseModel( new ScModelObj(pDocSh) );
}

ScModelObj::ScModelObj( ScDocShell* pDocSh ) :
    SfxBaseModel( pDocSh ),
    aPropSet( lcl_GetDocOptPropertyMap() ),
    pDocShell( pDocSh ),
    pPrintFuncCache( NULL ),
    pPrinterOptions( NULL ),
    maChangesListeners( m_aMutex )
{
    
    if ( pDocShell )
    {
        pDocShell->GetDocument()->AddUnoObject(*this);      
    }
}

ScModelObj::~ScModelObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (xNumberAgg.is())
        xNumberAgg->setDelegator(uno::Reference<uno::XInterface>());

    delete pPrintFuncCache;
    delete pPrinterOptions;
}

uno::Reference< uno::XAggregation> ScModelObj::GetFormatter()
{
    
    if ( !xNumberAgg.is() && pDocShell )
    {
        
        
        comphelper::increment( m_refCount );
        
        
        uno::Reference<util::XNumberFormatsSupplier> xFormatter(new SvNumberFormatsSupplierObj(pDocShell->GetDocument()->GetFormatTable() ));
        {
            xNumberAgg.set(uno::Reference<uno::XAggregation>( xFormatter, uno::UNO_QUERY ));
            
        }

        
        xFormatter = NULL;

        if (xNumberAgg.is())
            xNumberAgg->setDelegator( (cppu::OWeakObject*)this );
        comphelper::decrement( m_refCount );
    } 
    return xNumberAgg;
}

ScDocument* ScModelObj::GetDocument() const
{
    if (pDocShell)
        return pDocShell->GetDocument();
    return NULL;
}

SfxObjectShell* ScModelObj::GetEmbeddedObject() const
{
    return pDocShell;
}

void ScModelObj::UpdateAllRowHeights()
{
    if (pDocShell)
        pDocShell->UpdateAllRowHeights(NULL);
}

void ScModelObj::BeforeXMLLoading()
{
    if (pDocShell)
        pDocShell->BeforeXMLLoading();
}

void ScModelObj::AfterXMLLoading(bool bRet)
{
    if (pDocShell)
        pDocShell->AfterXMLLoading(bRet);
}

ScSheetSaveData* ScModelObj::GetSheetSaveData()
{
    if (pDocShell)
        return pDocShell->GetSheetSaveData();
    return NULL;
}

void ScModelObj::RepaintRange( const ScRange& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PAINT_GRID );
}

void ScModelObj::RepaintRange( const ScRangeList& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PAINT_GRID );
}

uno::Any SAL_CALL ScModelObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XSpreadsheetDocument )
    SC_QUERYINTERFACE( document::XActionLockable )
    SC_QUERYINTERFACE( sheet::XCalculatable )
    SC_QUERYINTERFACE( util::XProtectable )
    SC_QUERYINTERFACE( drawing::XDrawPagesSupplier )
    SC_QUERYINTERFACE( sheet::XGoalSeek )
    SC_QUERYINTERFACE( sheet::XConsolidatable )
    SC_QUERYINTERFACE( sheet::XDocumentAuditing )
    SC_QUERYINTERFACE( style::XStyleFamiliesSupplier )
    SC_QUERYINTERFACE( view::XRenderable )
    SC_QUERYINTERFACE( document::XLinkTargetSupplier )
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( lang::XMultiServiceFactory )
    SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( util::XChangesNotifier )
    SC_QUERYINTERFACE( sheet::opencl::XOpenCLSelection )

    uno::Any aRet(SfxBaseModel::queryInterface( rType ));
    if ( !aRet.hasValue()
        && rType != ::getCppuType((uno::Reference< com::sun::star::document::XDocumentEventBroadcaster>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::frame::XController>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::frame::XFrame>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::script::XInvocation>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::beans::XFastPropertySet>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::awt::XWindow>*)0))
    {
        GetFormatter();
        if ( xNumberAgg.is() )
            aRet = xNumberAgg->queryAggregation( rType );
    }

    return aRet;
}

void SAL_CALL ScModelObj::acquire() throw()
{
    SfxBaseModel::acquire();
}

void SAL_CALL ScModelObj::release() throw()
{
    SfxBaseModel::release();
}

uno::Sequence<uno::Type> SAL_CALL ScModelObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(SfxBaseModel::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        uno::Sequence<uno::Type> aAggTypes;
        if ( GetFormatter().is() )
        {
            const uno::Type& rProvType = ::getCppuType((uno::Reference<lang::XTypeProvider>*) 0);
            uno::Any aNumProv(xNumberAgg->queryAggregation(rProvType));
            if(aNumProv.getValueType() == rProvType)
            {
                uno::Reference<lang::XTypeProvider> xNumProv(
                    *(uno::Reference<lang::XTypeProvider>*)aNumProv.getValue());
                aAggTypes = xNumProv->getTypes();
            }
        }
        long nAggLen = aAggTypes.getLength();
        const uno::Type* pAggPtr = aAggTypes.getConstArray();

        const long nThisLen = 16;
        aTypes.realloc( nParentLen + nAggLen + nThisLen );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSpreadsheetDocument>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<document::XActionLockable>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<sheet::XCalculatable>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<util::XProtectable>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<drawing::XDrawPagesSupplier>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XGoalSeek>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<sheet::XConsolidatable>*)0);
        pPtr[nParentLen + 7] = getCppuType((const uno::Reference<sheet::XDocumentAuditing>*)0);
        pPtr[nParentLen + 8] = getCppuType((const uno::Reference<style::XStyleFamiliesSupplier>*)0);
        pPtr[nParentLen + 9] = getCppuType((const uno::Reference<view::XRenderable>*)0);
        pPtr[nParentLen +10] = getCppuType((const uno::Reference<document::XLinkTargetSupplier>*)0);
        pPtr[nParentLen +11] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[nParentLen +12] = getCppuType((const uno::Reference<lang::XMultiServiceFactory>*)0);
        pPtr[nParentLen +13] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
        pPtr[nParentLen +14] = getCppuType((const uno::Reference<util::XChangesNotifier>*)0);
        pPtr[nParentLen +15] = getCppuType((const uno::Reference<sheet::opencl::XOpenCLSelection>*)0);

        long i;
        for (i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                    

        for (i=0; i<nAggLen; i++)
            pPtr[nParentLen+nThisLen+i] = pAggPtr[i];   
    }
    return aTypes;
}

namespace
{
    class theScModelObjImplementationId : public rtl::Static< UnoTunnelIdInit, theScModelObjImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScModelObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    return theScModelObjImplementationId::get().getSeq();
}

void ScModelObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;       
            if (xNumberAgg.is())
            {
                SvNumberFormatsSupplierObj* pNumFmt =
                    SvNumberFormatsSupplierObj::getImplementation(
                        uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
                if ( pNumFmt )
                    pNumFmt->SetNumberFormatter( NULL );
            }

            DELETEZ( pPrintFuncCache );     
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            
            

            DELETEZ( pPrintFuncCache );

            
            if ( pDocShell )
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                if ( pDoc->GetVbaEventProcessor().is() )
                {
                    
                    if ( pDoc->HasAnyCalcNotification() && pDoc->HasAnySheetEventScript( SC_SHEETEVENT_CALCULATE, true ) )
                        HandleCalculateEvents();
                }
                else
                {
                    if ( pDoc->HasAnySheetEventScript( SC_SHEETEVENT_CALCULATE ) )
                        HandleCalculateEvents();
                }
            }
        }
    }
    else if ( rHint.ISA( ScPointerChangedHint ) )
    {
        sal_uInt16 nFlags = ((const ScPointerChangedHint&)rHint).GetFlags();
        if (nFlags & SC_POINTERCHANGED_NUMFMT)
        {
            

            if (GetFormatter().is())
            {
                SvNumberFormatsSupplierObj* pNumFmt =
                    SvNumberFormatsSupplierObj::getImplementation(
                        uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
                if ( pNumFmt && pDocShell )
                    pNumFmt->SetNumberFormatter( pDocShell->GetDocument()->GetFormatTable() );
            }
        }
    }

    
    SfxBaseModel::Notify( rBC, rHint );     
}



uno::Reference<sheet::XSpreadsheets> SAL_CALL ScModelObj::getSheets() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScTableSheetsObj(pDocShell);
    return NULL;
}



uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getStyleFamilies()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScStyleFamiliesObj(pDocShell);
    return NULL;
}



static OutputDevice* lcl_GetRenderDevice( const uno::Sequence<beans::PropertyValue>& rOptions )
{
    OutputDevice* pRet = NULL;
    const beans::PropertyValue* pPropArray = rOptions.getConstArray();
    long nPropCount = rOptions.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        OUString aPropName(rProp.Name);

        if (aPropName.equalsAscii( SC_UNONAME_RENDERDEV ))
        {
            uno::Reference<awt::XDevice> xRenderDevice(rProp.Value, uno::UNO_QUERY);
            if ( xRenderDevice.is() )
            {
                VCLXDevice* pDevice = VCLXDevice::GetImplementation( xRenderDevice );
                if ( pDevice )
                {
                    pRet = pDevice->GetOutputDevice();
                    pRet->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
                }
            }
        }
    }
    return pRet;
}

static bool lcl_ParseTarget( const OUString& rTarget, ScRange& rTargetRange, Rectangle& rTargetRect,
                        bool& rIsSheet, ScDocument* pDoc, SCTAB nSourceTab )
{
    

    ScAddress aAddress;
    ScRangeUtil aRangeUtil;
    SCTAB nNameTab;
    sal_Int32 nNumeric = 0;

    bool bRangeValid = false;
    bool bRectValid = false;

    if ( rTargetRange.Parse( rTarget, pDoc ) & SCA_VALID )
    {
        bRangeValid = true;             
    }
    else if ( aAddress.Parse( rTarget, pDoc ) & SCA_VALID )
    {
        rTargetRange = aAddress;
        bRangeValid = true;             
    }
    else if ( aRangeUtil.MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange, RUTL_NAMES ) ||
              aRangeUtil.MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange, RUTL_DBASE ) )
    {
        bRangeValid = true;             
    }
    else if ( comphelper::string::isdigitAsciiString(rTarget) &&
              ( nNumeric = rTarget.toInt32() ) > 0 && nNumeric <= MAXROW+1 )
    {
        
        rTargetRange = ScAddress( 0, (SCROW)(nNumeric-1), nSourceTab );     
        bRangeValid = true;             
    }
    else if ( pDoc->GetTable( rTarget, nNameTab ) )
    {
        rTargetRange = ScAddress(0,0,nNameTab);
        bRangeValid = true;             
        rIsSheet = true;                
    }
    else
    {
        

        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if ( pDrawLayer )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB i=0; i<nTabCount && !bRangeValid; i++)
            {
                SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(i));
                OSL_ENSURE(pPage,"Page ?");
                if (pPage)
                {
                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                    SdrObject* pObject = aIter.Next();
                    while (pObject && !bRangeValid)
                    {
                        if ( ScDrawLayer::GetVisibleName( pObject ) == rTarget )
                        {
                            rTargetRect = pObject->GetLogicRect();              
                            rTargetRange = pDoc->GetRange( i, rTargetRect );    
                            bRangeValid = bRectValid = true;                    
                        }
                        pObject = aIter.Next();
                    }
                }
            }
        }
    }
    if ( bRangeValid && !bRectValid )
    {
        
        rTargetRect = pDoc->GetMMRect( rTargetRange.aStart.Col(), rTargetRange.aStart.Row(),
                                       rTargetRange.aEnd.Col(),   rTargetRange.aEnd.Row(),
                                       rTargetRange.aStart.Tab() );
    }

    return bRangeValid;
}

bool ScModelObj::FillRenderMarkData( const uno::Any& aSelection,
                                     const uno::Sequence< beans::PropertyValue >& rOptions,
                                     ScMarkData& rMark,
                                     ScPrintSelectionStatus& rStatus, OUString& rPagesStr ) const
{
    OSL_ENSURE( !rMark.IsMarked() && !rMark.IsMultiMarked(), "FillRenderMarkData: MarkData must be empty" );
    OSL_ENSURE( pDocShell, "FillRenderMarkData: DocShell must be set" );

    bool bDone = false;

    uno::Reference<frame::XController> xView;

    
    sal_Bool bSelectedSheetsOnly = false;
    sal_Bool bIncludeEmptyPages = true;

    bool bHasPrintContent = false;
    sal_Int32 nPrintContent = 0;        
    sal_Int32 nPrintRange = 0;          
    OUString aPageRange;           

    for( sal_Int32 i = 0, nLen = rOptions.getLength(); i < nLen; i++ )
    {
        if ( rOptions[i].Name == "IsOnlySelectedSheets" )
        {
            rOptions[i].Value >>= bSelectedSheetsOnly;
        }
        else if ( rOptions[i].Name == "IsIncludeEmptyPages" )
        {
            rOptions[i].Value >>= bIncludeEmptyPages;
        }
        else if ( rOptions[i].Name == "PageRange" )
        {
            rOptions[i].Value >>= aPageRange;
        }
        else if ( rOptions[i].Name == "PrintRange" )
        {
            rOptions[i].Value >>= nPrintRange;
        }
        else if ( rOptions[i].Name == "PrintContent" )
        {
            bHasPrintContent = true;
            rOptions[i].Value >>= nPrintContent;
        }
        else if ( rOptions[i].Name == "View" )
        {
            rOptions[i].Value >>= xView;
        }
    }

    
    if ( bHasPrintContent )
        bSelectedSheetsOnly = ( nPrintContent != 0 );

    uno::Reference<uno::XInterface> xInterface(aSelection, uno::UNO_QUERY);
    if ( xInterface.is() )
    {
        ScCellRangesBase* pSelObj = ScCellRangesBase::getImplementation( xInterface );
        uno::Reference< drawing::XShapes > xShapes( xInterface, uno::UNO_QUERY );
        if ( pSelObj && pSelObj->GetDocShell() == pDocShell )
        {
            bool bSheet = ( ScTableSheetObj::getImplementation( xInterface ) != NULL );
            bool bCursor = pSelObj->IsCursorOnly();
            const ScRangeList& rRanges = pSelObj->GetRangeList();

            rMark.MarkFromRangeList( rRanges, false );
            rMark.MarkToSimple();

            if ( rMark.IsMultiMarked() )
            {
                
                
                ScRange aMultiMarkArea;
                rMark.GetMultiMarkArea( aMultiMarkArea );
                rMark.ResetMark();
                rMark.SetMarkArea( aMultiMarkArea );
            }

            if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
            {
                

                if ( bCursor || bSheet )                
                {
                    rMark.ResetMark();      
                    rStatus.SetMode( SC_PRINTSEL_CURSOR );
                }
                else
                    rStatus.SetMode( SC_PRINTSEL_RANGE );

                rStatus.SetRanges( rRanges );
                bDone = true;
            }
            
        }
        else if( xShapes.is() )
        {
            
            uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY );
            if( xIndexAccess.is() )
            {
                
                uno::Reference< drawing::XShape > xShape( xIndexAccess->getByIndex(0), uno::UNO_QUERY );
                SvxShape* pShape = SvxShape::getImplementation( xShape );
                if( pShape )
                {
                    SdrObject *pSdrObj = pShape->GetSdrObject();
                    if( pDocShell )
                    {
                        ScDocument* pDoc = pDocShell->GetDocument();
                        if( pDoc && pSdrObj )
                        {
                            Rectangle aObjRect = pSdrObj->GetCurrentBoundRect();
                            SCTAB nCurrentTab = ScDocShell::GetCurTab();
                            ScRange aRange = pDoc->GetRange( nCurrentTab, aObjRect );
                            rMark.SetMarkArea( aRange );

                            if( rMark.IsMarked() && !rMark.IsMultiMarked() )
                            {
                                rStatus.SetMode( SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS );
                                bDone = true;
                            }
                        }
                    }
                }
            }
        }
        else if ( ScModelObj::getImplementation( xInterface ) == this )
        {
            
            

            SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
                rMark.SelectTable( nTab, true );
            rStatus.SetMode( SC_PRINTSEL_DOCUMENT );
            bDone = true;
        }
        
    }

    
    uno::Reference<sheet::XSelectedSheetsSupplier> xSelectedSheets(xView, uno::UNO_QUERY);
    if (bSelectedSheetsOnly && xSelectedSheets.is())
    {
        uno::Sequence<sal_Int32> aSelected = xSelectedSheets->getSelectedSheets();
        ScMarkData::MarkedTabsType aSelectedTabs;
        SCTAB nMaxTab = pDocShell->GetDocument()->GetTableCount() -1;
        for (sal_Int32 i = 0, n = aSelected.getLength(); i < n; ++i)
        {
            SCTAB nSelected = static_cast<SCTAB>(aSelected[i]);
            if (ValidTab(nSelected, nMaxTab))
                aSelectedTabs.insert(static_cast<SCTAB>(aSelected[i]));
        }
        rMark.SetSelectedTabs(aSelectedTabs);
    }

    ScPrintOptions aNewOptions;
    aNewOptions.SetSkipEmpty( !bIncludeEmptyPages );
    aNewOptions.SetAllSheets( !bSelectedSheetsOnly );
    rStatus.SetOptions( aNewOptions );

    
    if ( nPrintRange == 1 )
        rPagesStr = aPageRange;
    else
        rPagesStr = "";

    return bDone;
}


sal_Int32 SAL_CALL ScModelObj::getRendererCount(const uno::Any& aSelection,
    const uno::Sequence<beans::PropertyValue>& rOptions)
        throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
        return 0;

    
    
    

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        delete pPrintFuncCache;
        pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
    }
    sal_Int32 nPages = pPrintFuncCache->GetPageCount();

    sal_Int32 nSelectCount = nPages;
    if ( !aPagesStr.isEmpty() )
    {
        StringRangeEnumerator aRangeEnum( aPagesStr, 0, nPages-1 );
        nSelectCount = aRangeEnum.size();
    }
    return nSelectCount;
}

static sal_Int32 lcl_GetRendererNum( sal_Int32 nSelRenderer, const OUString& rPagesStr, sal_Int32 nTotalPages )
{
    if ( rPagesStr.isEmpty() )
        return nSelRenderer;

    StringRangeEnumerator aRangeEnum( rPagesStr, 0, nTotalPages-1 );
    StringRangeEnumerator::Iterator aIter = aRangeEnum.begin();
    StringRangeEnumerator::Iterator aEnd  = aRangeEnum.end();
    for ( ; nSelRenderer > 0 && aIter != aEnd; --nSelRenderer )
        ++aIter;

    return *aIter; 
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScModelObj::getRenderer( sal_Int32 nSelRenderer,
                                    const uno::Any& aSelection, const uno::Sequence<beans::PropertyValue>& rOptions  )
                                throw (lang::IllegalArgumentException,
                                       uno::RuntimeException,
                                       std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    
    long nTotalPages = 0;
    if ( FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
    {
        if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
        {
            delete pPrintFuncCache;
            pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
        }
        nTotalPages = pPrintFuncCache->GetPageCount();
    }
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
    {
        if ( nSelRenderer == 0 )
        {
            

            SCTAB nCurTab = 0;      
            ScPrintFunc aDefaultFunc( pDocShell, pDocShell->GetPrinter(), nCurTab );
            Size aTwips = aDefaultFunc.GetPageSize();
            awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

            uno::Sequence<beans::PropertyValue> aSequence(1);
            beans::PropertyValue* pArray = aSequence.getArray();
            pArray[0].Name = SC_UNONAME_PAGESIZE;
            pArray[0].Value <<= aPageSize;

            if( ! pPrinterOptions )
                pPrinterOptions = new ScPrintUIOptions;
            else
                pPrinterOptions->SetDefaults();
            pPrinterOptions->appendPrintUIOptions( aSequence );
            return aSequence;
        }
        else
            throw lang::IllegalArgumentException();
    }

    

    SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );

    ScRange aRange;
    const ScRange* pSelRange = NULL;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }
    ScPrintFunc aFunc( pDocShell, pDocShell->GetPrinter(), nTab,
                        pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
    aFunc.SetRenderFlag( true );

    Range aPageRange( nRenderer+1, nRenderer+1 );
    MultiSelection aPage( aPageRange );
    aPage.SetTotalRange( Range(0,RANGE_MAX) );
    aPage.Select( aPageRange );

    long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
    long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, false, NULL );

    ScRange aCellRange;
    sal_Bool bWasCellRange = aFunc.GetLastSourceRange( aCellRange );
    Size aTwips = aFunc.GetPageSize();
    awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

    long nPropCount = bWasCellRange ? 3 : 2;
    uno::Sequence<beans::PropertyValue> aSequence(nPropCount);
    beans::PropertyValue* pArray = aSequence.getArray();
    pArray[0].Name = SC_UNONAME_PAGESIZE;
    pArray[0].Value <<= aPageSize;
    
    pArray[1].Name = SC_UNONAME_INC_NP_AREA;
    pArray[1].Value = uno::makeAny( sal_True );
    if ( bWasCellRange )
    {
        table::CellRangeAddress aRangeAddress( nTab,
                        aCellRange.aStart.Col(), aCellRange.aStart.Row(),
                        aCellRange.aEnd.Col(), aCellRange.aEnd.Row() );
        pArray[2].Name = SC_UNONAME_SOURCERANGE;
        pArray[2].Value <<= aRangeAddress;
    }

    if( ! pPrinterOptions )
        pPrinterOptions = new ScPrintUIOptions;
    else
        pPrinterOptions->SetDefaults();
    pPrinterOptions->appendPrintUIOptions( aSequence );
    return aSequence;
}

void SAL_CALL ScModelObj::render( sal_Int32 nSelRenderer, const uno::Any& aSelection,
                                    const uno::Sequence<beans::PropertyValue>& rOptions )
                                throw(lang::IllegalArgumentException,
                                      uno::RuntimeException,
                                      std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
        throw lang::IllegalArgumentException();

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        delete pPrintFuncCache;
        pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
    }
    long nTotalPages = pPrintFuncCache->GetPageCount();
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
        throw lang::IllegalArgumentException();

    OutputDevice* pDev = lcl_GetRenderDevice( rOptions );
    if ( !pDev )
        throw lang::IllegalArgumentException();

    SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );
    ScDocument* pDoc = pDocShell->GetDocument();

    FmFormView* pDrawView = NULL;

    
    ScDrawLayer* pModel = pDoc->GetDrawLayer();

    if( pModel )
    {
        pDrawView = new FmFormView( pModel, pDev );
        pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));
        pDrawView->SetPrintPreview( true );
    }

    ScRange aRange;
    const ScRange* pSelRange = NULL;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    
    

    ScPrintFunc aFunc( pDev, pDocShell, nTab, pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
    aFunc.SetDrawView( pDrawView );
    aFunc.SetRenderFlag( true );
    if( aStatus.GetMode() == SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS )
        aFunc.SetExclusivelyDrawOleAndDrawObjects();

    Range aPageRange( nRenderer+1, nRenderer+1 );
    MultiSelection aPage( aPageRange );
    aPage.SetTotalRange( Range(0,RANGE_MAX) );
    aPage.Select( aPageRange );

    long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
    long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );
    if ( nRenderer == nTabStart )
    {
        

        if ( pPDFData && pPDFData->GetIsExportBookmarks() )
        {
            
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            sal_Int32 nDestID = pPDFData->CreateDest( aArea );
            OUString aTabName;
            pDoc->GetName( nTab, aTabName );
            sal_Int32 nParent = -1;     
            pPDFData->CreateOutlineItem( nParent, aTabName, nDestID );
        }
        
        if( pPDFData && pPDFData->GetIsExportNamedDestinations() )
        {
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            OUString aTabName;
            pDoc->GetName( nTab, aTabName );
            
            pPDFData->CreateNamedDest( aTabName, aArea );
        }
    }

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, true, NULL );

    

    if ( pPDFData )
    {
        

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIter = rBookmarks.begin();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIEnd = rBookmarks.end();
        while ( aIter != aIEnd )
        {
            OUString aBookmark = aIter->aBookmark;
            if ( aBookmark.toChar() == (sal_Unicode) '#' )
            {
                

                OUString aTarget( aBookmark.copy( 1 ) );

                ScRange aTargetRange;
                Rectangle aTargetRect;      
                bool bIsSheet = false;
                bool bValid = lcl_ParseTarget( aTarget, aTargetRange, aTargetRect, bIsSheet, pDoc, nTab );

                if ( bValid )
                {
                    sal_Int32 nPage = -1;
                    Rectangle aArea;
                    if ( bIsSheet )
                    {
                        
                        
                        nPage = pPrintFuncCache->GetTabStart( aTargetRange.aStart.Tab() );
                        aArea = pDev->PixelToLogic( Rectangle( 0,0,0,0 ) );
                    }
                    else
                    {
                        pPrintFuncCache->InitLocations( aMark, pDev );      

                        ScPrintPageLocation aLocation;
                        if ( pPrintFuncCache->FindLocation( aTargetRange.aStart, aLocation ) )
                        {
                            nPage = aLocation.nPage;

                            
                            ScRange aLocRange = aLocation.aCellRange;
                            Rectangle aLocationMM = pDoc->GetMMRect(
                                       aLocRange.aStart.Col(), aLocRange.aStart.Row(),
                                       aLocRange.aEnd.Col(),   aLocRange.aEnd.Row(),
                                       aLocRange.aStart.Tab() );
                            Rectangle aLocationPixel = aLocation.aRectangle;

                            
                            

                            Fraction aScaleX( aLocationPixel.GetWidth(), aLocationMM.GetWidth() );
                            Fraction aScaleY( aLocationPixel.GetHeight(), aLocationMM.GetHeight() );

                            long nX1 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Left() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nX2 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Right() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nY1 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Top() - aLocationMM.Top(), 1 ) * aScaleY );
                            long nY2 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Bottom() - aLocationMM.Top(), 1 ) * aScaleY );

                            if ( nX1 > aLocationPixel.Right() ) nX1 = aLocationPixel.Right();
                            if ( nX2 > aLocationPixel.Right() ) nX2 = aLocationPixel.Right();
                            if ( nY1 > aLocationPixel.Bottom() ) nY1 = aLocationPixel.Bottom();
                            if ( nY2 > aLocationPixel.Bottom() ) nY2 = aLocationPixel.Bottom();

                            
                            
                            

                            aArea = pDev->PixelToLogic( Rectangle( nX1, nY1, nX2, nY2 ) );
                        }
                    }

                    if ( nPage >= 0 )
                        pPDFData->SetLinkDest( aIter->nLinkId, pPDFData->CreateDest( aArea, nPage ) );
                }
            }
            else
            {
                
                pPDFData->SetLinkURL( aIter->nLinkId, aBookmark );
            }
            ++aIter;
        }
        rBookmarks.clear();
    }

    if ( pDrawView )
        pDrawView->HideSdrPage();
    delete pDrawView;
}



uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getLinks() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScLinkTargetTypesObj(pDocShell);
    return NULL;
}



sal_Bool SAL_CALL ScModelObj::isActionLocked() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bLocked = false;
    if (pDocShell)
        bLocked = ( pDocShell->GetLockCount() != 0 );
    return bLocked;
}

void SAL_CALL ScModelObj::addActionLock() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->LockDocument();
}

void SAL_CALL ScModelObj::removeActionLock() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->UnlockDocument();
}

void SAL_CALL ScModelObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->SetLockCount(nLock);
}

sal_Int16 SAL_CALL ScModelObj::resetActionLocks() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_uInt16 nRet = 0;
    if (pDocShell)
    {
        nRet = pDocShell->GetLockCount();
        pDocShell->SetLockCount(0);
    }
    return nRet;
}

void SAL_CALL ScModelObj::lockControllers() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxBaseModel::lockControllers();
    if (pDocShell)
        pDocShell->LockPaint();
}

void SAL_CALL ScModelObj::unlockControllers() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (hasControllersLocked())
    {
        SfxBaseModel::unlockControllers();
        if (pDocShell)
            pDocShell->UnlockPaint();
    }
}



void SAL_CALL ScModelObj::calculate() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->DoRecalc(true);
    else
    {
        OSL_FAIL("keine DocShell");     
    }
}

void SAL_CALL ScModelObj::calculateAll() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->DoHardRecalc(true);
    else
    {
        OSL_FAIL("keine DocShell");     
    }
}

sal_Bool SAL_CALL ScModelObj::isAutomaticCalculationEnabled() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument()->GetAutoCalc();

    OSL_FAIL("keine DocShell");     
    return false;
}

void SAL_CALL ScModelObj::enableAutomaticCalculation( sal_Bool bEnabledIn )
                                                throw(uno::RuntimeException)
{
    bool bEnabled(bEnabledIn);
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if ( pDoc->GetAutoCalc() != bEnabled )
        {
            pDoc->SetAutoCalc( bEnabled );
            pDocShell->SetDocumentModified();
        }
    }
    else
    {
        OSL_FAIL("keine DocShell");     
    }
}



void SAL_CALL ScModelObj::protect( const OUString& aPassword ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    
    if ( pDocShell && !pDocShell->GetDocument()->IsDocProtected() )
    {
        OUString aString(aPassword);
        pDocShell->GetDocFunc().Protect( TABLEID_DOC, aString, true );
    }
}

void SAL_CALL ScModelObj::unprotect( const OUString& aPassword )
                        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OUString aString(aPassword);
        sal_Bool bDone = pDocShell->GetDocFunc().Unprotect( TABLEID_DOC, aString, true );
        if (!bDone)
            throw lang::IllegalArgumentException();
    }
}

sal_Bool SAL_CALL ScModelObj::isProtected() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument()->IsDocProtected();

    OSL_FAIL("keine DocShell");     
    return false;
}



uno::Reference<drawing::XDrawPages> SAL_CALL ScModelObj::getDrawPages() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScDrawPagesObj(pDocShell);

    OSL_FAIL("keine DocShell");     
    return NULL;
}



sheet::GoalResult SAL_CALL ScModelObj::seekGoal(
                                const table::CellAddress& aFormulaPosition,
                                const table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sheet::GoalResult aResult;
    aResult.Divergence = DBL_MAX;       
    if (pDocShell)
    {
        WaitObject aWait( pDocShell->GetActiveDialogParent() );
        OUString aGoalString(aGoalValue);
        ScDocument* pDoc = pDocShell->GetDocument();
        double fValue = 0.0;
        sal_Bool bFound = pDoc->Solver(
                    (SCCOL)aFormulaPosition.Column, (SCROW)aFormulaPosition.Row, aFormulaPosition.Sheet,
                    (SCCOL)aVariablePosition.Column, (SCROW)aVariablePosition.Row, aVariablePosition.Sheet,
                    aGoalString, fValue );
        aResult.Result = fValue;
        if (bFound)
            aResult.Divergence = 0.0;   
    }
    return aResult;
}



uno::Reference<sheet::XConsolidationDescriptor> SAL_CALL ScModelObj::createConsolidationDescriptor(
                                sal_Bool bEmpty ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScConsolidationDescriptor* pNew = new ScConsolidationDescriptor;
    if ( pDocShell && !bEmpty )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        const ScConsolidateParam* pParam = pDoc->GetConsolidateDlgData();
        if (pParam)
            pNew->SetParam( *pParam );
    }
    return pNew;
}

void SAL_CALL ScModelObj::consolidate(
        const uno::Reference<sheet::XConsolidationDescriptor>& xDescriptor )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    
    
    
    

    ScConsolidationDescriptor aImpl;
    aImpl.setFunction( xDescriptor->getFunction() );
    aImpl.setSources( xDescriptor->getSources() );
    aImpl.setStartOutputPosition( xDescriptor->getStartOutputPosition() );
    aImpl.setUseColumnHeaders( xDescriptor->getUseColumnHeaders() );
    aImpl.setUseRowHeaders( xDescriptor->getUseRowHeaders() );
    aImpl.setInsertLinks( xDescriptor->getInsertLinks() );

    if (pDocShell)
    {
        const ScConsolidateParam& rParam = aImpl.GetParam();
        pDocShell->DoConsolidate( rParam, true );
        pDocShell->GetDocument()->SetConsolidateDlgData( &rParam );
    }
}



void SAL_CALL ScModelObj::refreshArrows() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->GetDocFunc().DetectiveRefresh();
}


uno::Reference< container::XIndexAccess > SAL_CALL ScModelObj::getViewData(  )
    throw (uno::RuntimeException)
{
    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        SolarMutexGuard aGuard;
        if (pDocShell && pDocShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
        {
            uno::Reference < container::XIndexContainer > xCont = document::IndexedPropertyValues::create( ::comphelper::getProcessComponentContext() );
            xRet.set( xCont, uno::UNO_QUERY_THROW );

            uno::Sequence< beans::PropertyValue > aSeq;
            aSeq.realloc(1);
            OUString sName;
            pDocShell->GetDocument()->GetName( pDocShell->GetDocument()->GetVisibleTab(), sName );
            OUString sOUName(sName);
            aSeq[0].Name = SC_ACTIVETABLE;
            aSeq[0].Value <<= sOUName;
            xCont->insertByIndex( 0, uno::makeAny( aSeq ) );
        }
    }

    return xRet;
}




uno::Reference<beans::XPropertySetInfo> SAL_CALL ScModelObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScModelObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        const ScDocOptions& rOldOpt = pDoc->GetDocOptions();
        ScDocOptions aNewOpt = rOldOpt;
        
        
        bool bHardRecalc = !pDoc->IsImportingXML();

        sal_Bool bOpt = ScDocOptionsHelper::setPropertyValue( aNewOpt, aPropSet.getPropertyMap(), aPropertyName, aValue );
        if (bOpt)
        {
            
            if ( aString.equalsAscii( SC_UNO_IGNORECASE ) ||
                 aString.equalsAscii( SC_UNONAME_REGEXP ) ||
                 aString.equalsAscii( SC_UNO_LOOKUPLABELS ) )
                bHardRecalc = false;
        }
        else if ( aString.equalsAscii( SC_UNONAME_CLOCAL ) )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                pDoc->GetLanguage( eLatin, eCjk, eCtl );
                eLatin = ScUnoConversion::GetLanguage(aLocale);
                pDoc->SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aString.equalsAscii( SC_UNO_CODENAME ) )
        {
            OUString sCodeName;
            if ( aValue >>= sCodeName )
                pDoc->SetCodeName( sCodeName );
        }
        else if ( aString.equalsAscii( SC_UNO_CJK_CLOCAL ) )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                pDoc->GetLanguage( eLatin, eCjk, eCtl );
                eCjk = ScUnoConversion::GetLanguage(aLocale);
                pDoc->SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aString.equalsAscii( SC_UNO_CTL_CLOCAL ) )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                pDoc->GetLanguage( eLatin, eCjk, eCtl );
                eCtl = ScUnoConversion::GetLanguage(aLocale);
                pDoc->SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aString.equalsAscii( SC_UNO_APPLYFMDES ) )
        {
            
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetOpenInDesignMode( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_OPEN_READONLY );
        }
        else if ( aString.equalsAscii( SC_UNO_AUTOCONTFOC ) )
        {
            
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetAutoControlFocus( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_AUTOCONTROLFOCUS );
        }
        else if ( aString.equalsAscii( SC_UNO_ISLOADED ) )
        {
            pDocShell->SetEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString.equalsAscii( SC_UNO_ISUNDOENABLED ) )
        {
            sal_Bool bUndoEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            pDoc->EnableUndo( bUndoEnabled );
            pDocShell->GetUndoManager()->SetMaxUndoActionCount(
                bUndoEnabled
                ? officecfg::Office::Common::Undo::Steps::get() : 0);
        }
        else if ( aString.equalsAscii( SC_UNO_ISADJUSTHEIGHTENABLED ) )
        {
            bool bOldAdjustHeightEnabled = pDoc->IsAdjustHeightEnabled();
            bool bAdjustHeightEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if( bOldAdjustHeightEnabled != bAdjustHeightEnabled )
                pDoc->EnableAdjustHeight( bAdjustHeightEnabled );
        }
        else if ( aString.equalsAscii( SC_UNO_ISEXECUTELINKENABLED ) )
        {
            pDoc->EnableExecuteLink( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString.equalsAscii( SC_UNO_ISCHANGEREADONLYENABLED ) )
        {
            pDoc->EnableChangeReadOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString.equalsAscii( "BuildId" ) )
        {
            aValue >>= maBuildId;
        }
        else if ( aString.equalsAscii( "SavedObject" ) )    
        {
            OUString aObjName;
            aValue >>= aObjName;
            if ( !aObjName.isEmpty() )
                pDoc->RestoreChartListener( aObjName );
        }
        else if ( aString.equalsAscii( SC_UNO_INTEROPGRABBAG ) )
        {
            setGrabBagItem(aValue);
        }

        if ( aNewOpt != rOldOpt )
        {
            pDoc->SetDocOptions( aNewOpt );
            
            if ( bHardRecalc )
                pDocShell->DoHardRecalc( true );
            pDocShell->SetDocumentModified();
        }
    }
}

uno::Any SAL_CALL ScModelObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);
    uno::Any aRet;

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        const ScDocOptions& rOpt = pDoc->GetDocOptions();
        aRet = ScDocOptionsHelper::getPropertyValue( rOpt, aPropSet.getPropertyMap(), aPropertyName );
        if ( aRet.hasValue() )
        {
            
        }
        else if ( aString.equalsAscii( SC_UNONAME_CLOCAL ) )
        {
            LanguageType eLatin, eCjk, eCtl;
            pDoc->GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eLatin );
            aRet <<= aLocale;
        }
        else if ( aString.equalsAscii( SC_UNO_CODENAME ) )
        {
            OUString sCodeName = pDoc->GetCodeName();
            aRet <<= sCodeName;
        }

        else if ( aString.equalsAscii( SC_UNO_CJK_CLOCAL ) )
        {
            LanguageType eLatin, eCjk, eCtl;
            pDoc->GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCjk );
            aRet <<= aLocale;
        }
        else if ( aString.equalsAscii( SC_UNO_CTL_CLOCAL ) )
        {
            LanguageType eLatin, eCjk, eCtl;
            pDoc->GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCtl );
            aRet <<= aLocale;
        }
        else if ( aString.equalsAscii( SC_UNO_NAMEDRANGES ) )
        {
            aRet <<= uno::Reference<sheet::XNamedRanges>(new ScGlobalNamedRangesObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_DATABASERNG ) )
        {
            aRet <<= uno::Reference<sheet::XDatabaseRanges>(new ScDatabaseRangesObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_UNNAMEDDBRNG ) )
        {
            aRet <<= uno::Reference<sheet::XUnnamedDatabaseRanges>(new ScUnnamedDatabaseRangesObj(pDocShell));
        }
        else if ( aString.equalsAscii( SC_UNO_COLLABELRNG ) )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, true ));
        }
        else if ( aString.equalsAscii( SC_UNO_ROWLABELRNG ) )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, false ));
        }
        else if ( aString.equalsAscii( SC_UNO_AREALINKS ) )
        {
            aRet <<= uno::Reference<sheet::XAreaLinks>(new ScAreaLinksObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_DDELINKS ) )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScDDELinksObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_EXTERNALDOCLINKS ) )
        {
            aRet <<= uno::Reference<sheet::XExternalDocLinks>(new ScExternalDocLinksObj(pDocShell));
        }
        else if ( aString.equalsAscii( SC_UNO_SHEETLINKS ) )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScSheetLinksObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_APPLYFMDES ) )
        {
            
            ScDrawLayer* pModel = pDoc->GetDrawLayer();
            sal_Bool bOpenInDesign = pModel ? pModel->GetOpenInDesignMode() : sal_True;
            ScUnoHelpFunctions::SetBoolInAny( aRet, bOpenInDesign );
        }
        else if ( aString.equalsAscii( SC_UNO_AUTOCONTFOC ) )
        {
            
            ScDrawLayer* pModel = pDoc->GetDrawLayer();
            sal_Bool bAutoControlFocus = pModel ? pModel->GetAutoControlFocus() : false;
            ScUnoHelpFunctions::SetBoolInAny( aRet, bAutoControlFocus );
        }
        else if ( aString.equalsAscii( SC_UNO_FORBIDDEN ) )
        {
            aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
        }
        else if ( aString.equalsAscii( SC_UNO_HASDRAWPAGES ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetDocument()->GetDrawLayer() != 0) );
        }
        else if ( aString.equalsAscii( SC_UNO_BASICLIBRARIES ) )
        {
            aRet <<= pDocShell->GetBasicContainer();
        }
        else if ( aString.equalsAscii( SC_UNO_DIALOGLIBRARIES ) )
        {
            aRet <<= pDocShell->GetDialogContainer();
        }
        else if ( aString.equalsAscii( SC_UNO_VBAGLOBNAME ) )
        {
            /*  #i111553# This property provides the name of the constant that
                will be used to store this model in the global Basic manager.
                That constant will be equivelant to 'ThisComponent' but for
                each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
                constant can co-exist, as required by VBA. */
            aRet <<= OUString( "ThisExcelDoc" );
        }
        else if ( aString.equalsAscii( SC_UNO_RUNTIMEUID ) )
        {
            aRet <<= getRuntimeUID();
        }
        else if ( aString.equalsAscii( SC_UNO_HASVALIDSIGNATURES ) )
        {
            aRet <<= hasValidSignatures();
        }
        else if ( aString.equalsAscii( SC_UNO_ISLOADED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, !pDocShell->IsEmpty() );
        }
        else if ( aString.equalsAscii( SC_UNO_ISUNDOENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsUndoEnabled() );
        }
        else if ( aString.equalsAscii( SC_UNO_ISADJUSTHEIGHTENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsAdjustHeightEnabled() );
        }
        else if ( aString.equalsAscii( SC_UNO_ISEXECUTELINKENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsExecuteLinkEnabled() );
        }
        else if ( aString.equalsAscii( SC_UNO_ISCHANGEREADONLYENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsChangeReadOnlyEnabled() );
        }
        else if ( aString.equalsAscii( SC_UNO_REFERENCEDEVICE ) )
        {
            VCLXDevice* pXDev = new VCLXDevice();
            pXDev->SetOutputDevice( pDoc->GetRefDevice() );
            aRet <<= uno::Reference< awt::XDevice >( pXDev );
        }
        else if ( aString.equalsAscii( "BuildId" ) )
        {
            aRet <<= maBuildId;
        }
        else if ( aString.equalsAscii( "InternalDocument" ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetCreateMode() == SFX_CREATE_MODE_INTERNAL) );
        }
        else if ( aString.equalsAscii( SC_UNO_INTEROPGRABBAG ) )
        {
            getGrabBagItem(aRet);
        }
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScModelObj )



css::uno::Reference<css::uno::XInterface> ScModelObj::create(
    OUString const & aServiceSpecifier,
    css::uno::Sequence<css::uno::Any> const * arguments)
{
    uno::Reference<uno::XInterface> xRet;
    OUString aNameStr(aServiceSpecifier);
    sal_uInt16 nType = ScServiceProvider::GetProviderType(aNameStr);
    if ( nType != SC_SERVICE_INVALID )
    {
        
        
        switch ( nType )
        {
            case SC_SERVICE_GRADTAB:    xRet.set(xDrawGradTab);     break;
            case SC_SERVICE_HATCHTAB:   xRet.set(xDrawHatchTab);    break;
            case SC_SERVICE_BITMAPTAB:  xRet.set(xDrawBitmapTab);   break;
            case SC_SERVICE_TRGRADTAB:  xRet.set(xDrawTrGradTab);   break;
            case SC_SERVICE_MARKERTAB:  xRet.set(xDrawMarkerTab);   break;
            case SC_SERVICE_DASHTAB:    xRet.set(xDrawDashTab);     break;
            case SC_SERVICE_CHDATAPROV: xRet.set(xChartDataProv);   break;
            case SC_SERVICE_VBAOBJECTPROVIDER: xRet.set(xObjProvider); break;
        }

        
        
        bool bCreate =
            ! ( nType == SC_SERVICE_CHDATAPROV &&
                ( pDocShell->GetCreateMode() == SFX_CREATE_MODE_INTERNAL ));
        
        
        OSL_ASSERT( bCreate );

        if ( !xRet.is() && bCreate )
        {
            xRet.set(ScServiceProvider::MakeInstance( nType, pDocShell ));

            
            switch ( nType )
            {
                case SC_SERVICE_GRADTAB:    xDrawGradTab.set(xRet);     break;
                case SC_SERVICE_HATCHTAB:   xDrawHatchTab.set(xRet);    break;
                case SC_SERVICE_BITMAPTAB:  xDrawBitmapTab.set(xRet);   break;
                case SC_SERVICE_TRGRADTAB:  xDrawTrGradTab.set(xRet);   break;
                case SC_SERVICE_MARKERTAB:  xDrawMarkerTab.set(xRet);   break;
                case SC_SERVICE_DASHTAB:    xDrawDashTab.set(xRet);     break;
                case SC_SERVICE_CHDATAPROV: xChartDataProv.set(xRet);   break;
                case SC_SERVICE_VBAOBJECTPROVIDER: xObjProvider.set(xRet); break;
            }
        }
    }
    else
    {
        
        

        try
        {
            xRet = arguments == 0
                ? SvxFmMSFactory::createInstance(aServiceSpecifier)
                : SvxFmMSFactory::createInstanceWithArguments(
                    aServiceSpecifier, *arguments);
            
        }
        catch ( lang::ServiceNotRegisteredException & )
        {
        }

        
        

        uno::Reference<drawing::XShape> xShape( xRet, uno::UNO_QUERY );
        if ( xShape.is() )
        {
            xRet.clear();               
            new ScShapeObj( xShape );   
            xRet.set(xShape);
        }
    }
    return xRet;
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstance(
                                const OUString& aServiceSpecifier )
                                throw(uno::Exception, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return create(aServiceSpecifier, 0);
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstanceWithArguments(
                                const OUString& ServiceSpecifier,
                                const uno::Sequence<uno::Any>& aArgs )
                                throw(uno::Exception, uno::RuntimeException)
{
    

    SolarMutexGuard aGuard;
    uno::Reference<uno::XInterface> xInt(create(ServiceSpecifier, &aArgs));

    if ( aArgs.getLength() )
    {
        

        uno::Reference<lang::XInitialization> xInit( xInt, uno::UNO_QUERY );
        if ( xInit.is() )
            xInit->initialize( aArgs );
    }

    return xInt;
}

uno::Sequence<OUString> SAL_CALL ScModelObj::getAvailableServiceNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    
    
    

    uno::Sequence<OUString> aMyServices(ScServiceProvider::GetAllServiceNames());
    uno::Sequence<OUString> aDrawServices(SvxFmMSFactory::getAvailableServiceNames());

    return concatServiceNames( aMyServices, aDrawServices );
}


OUString SAL_CALL ScModelObj::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "ScModelObj" );
}

sal_Bool SAL_CALL ScModelObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScModelObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aRet(3);
    aRet[0] = SCMODELOBJ_SERVICE;
    aRet[1] = SCDOCSETTINGS_SERVICE;
    aRet[2] = SCDOC_SERVICE;
    return aRet;
}



sal_Int64 SAL_CALL ScModelObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }

    if ( rId.getLength() == 16 &&
        0 == memcmp( SfxObjectShell::getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pDocShell ));
    }

    
    

    sal_Int64 nRet = SfxBaseModel::getSomething( rId );
    if ( nRet )
        return nRet;

    if ( GetFormatter().is() )
    {
        const uno::Type& rTunnelType = ::getCppuType((uno::Reference<lang::XUnoTunnel>*) 0);
        uno::Any aNumTunnel(xNumberAgg->queryAggregation(rTunnelType));
        if(aNumTunnel.getValueType() == rTunnelType)
        {
            uno::Reference<lang::XUnoTunnel> xTunnelAgg(
                *(uno::Reference<lang::XUnoTunnel>*)aNumTunnel.getValue());
            return xTunnelAgg->getSomething( rId );
        }
    }

    return 0;
}

namespace
{
    class theScModelObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScModelObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScModelObj::getUnoTunnelId()
{
    return theScModelObjUnoTunnelId::get().getSeq();
}

ScModelObj* ScModelObj::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScModelObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScModelObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}



void ScModelObj::addChangesListener( const uno::Reference< util::XChangesListener >& aListener )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maChangesListeners.addInterface( aListener );
}

void ScModelObj::removeChangesListener( const uno::Reference< util::XChangesListener >& aListener )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maChangesListeners.removeInterface( aListener );
}

bool ScModelObj::HasChangesListeners() const
{
    if ( maChangesListeners.getLength() > 0 )
        return true;

    
    return pDocShell && pDocShell->GetDocument()->HasAnySheetEventScript(SC_SHEETEVENT_CHANGE);
}

void ScModelObj::NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
    const uno::Sequence< beans::PropertyValue >& rProperties )
{
    if ( pDocShell && HasChangesListeners() )
    {
        util::ChangesEvent aEvent;
        aEvent.Source.set( static_cast< cppu::OWeakObject* >( this ) );
        aEvent.Base <<= aEvent.Source;

        size_t nRangeCount = rRanges.size();
        aEvent.Changes.realloc( static_cast< sal_Int32 >( nRangeCount ) );
        for ( size_t nIndex = 0; nIndex < nRangeCount; ++nIndex )
        {
            uno::Reference< table::XCellRange > xRangeObj;

            ScRange aRange( *rRanges[ nIndex ] );
            if ( aRange.aStart == aRange.aEnd )
            {
                xRangeObj.set( new ScCellObj( pDocShell, aRange.aStart ) );
            }
            else
            {
                xRangeObj.set( new ScCellRangeObj( pDocShell, aRange ) );
            }

            util::ElementChange& rChange = aEvent.Changes[ static_cast< sal_Int32 >( nIndex ) ];
            rChange.Accessor <<= rOperation;
            rChange.Element <<= rProperties;
            rChange.ReplacedElement <<= xRangeObj;
        }

        ::cppu::OInterfaceIteratorHelper aIter( maChangesListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
                static_cast< util::XChangesListener* >( aIter.next() )->changesOccurred( aEvent );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    
    
    if ( rOperation.equalsAscii("cell-change") && pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( rRanges, false );
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTabCount = pDoc->GetTableCount();
        ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            SCTAB nTab = *itr;
            const ScSheetEvents* pEvents = pDoc->GetSheetEvents(nTab);
            if (pEvents)
            {
                const OUString* pScript = pEvents->GetScript(SC_SHEETEVENT_CHANGE);
                if (pScript)
                {
                    ScRangeList aTabRanges;     
                    size_t nRangeCount = rRanges.size();
                    for ( size_t nIndex = 0; nIndex < nRangeCount; ++nIndex )
                    {
                        ScRange aRange( *rRanges[ nIndex ] );
                        if ( aRange.aStart.Tab() == nTab )
                            aTabRanges.Append( aRange );
                    }
                    size_t nTabRangeCount = aTabRanges.size();
                    if ( nTabRangeCount > 0 )
                    {
                        uno::Reference<uno::XInterface> xTarget;
                        if ( nTabRangeCount == 1 )
                        {
                            ScRange aRange( *aTabRanges[ 0 ] );
                            if ( aRange.aStart == aRange.aEnd )
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellObj( pDocShell, aRange.aStart ) ) );
                            else
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangeObj( pDocShell, aRange ) ) );
                        }
                        else
                            xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangesObj( pDocShell, aTabRanges ) ) );

                        uno::Sequence<uno::Any> aParams(1);
                        aParams[0] <<= xTarget;

                        uno::Any aRet;
                        uno::Sequence<sal_Int16> aOutArgsIndex;
                        uno::Sequence<uno::Any> aOutArgs;

                        /*ErrCode eRet =*/ pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                    }
                }
            }
        }
    }
}

void ScModelObj::HandleCalculateEvents()
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        
        
        if ( pDoc->IsDocVisible() )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
            {
                if (pDoc->HasCalcNotification(nTab))
                {
                    if (const ScSheetEvents* pEvents = pDoc->GetSheetEvents( nTab ))
                    {
                        if (const OUString* pScript = pEvents->GetScript(SC_SHEETEVENT_CALCULATE))
                        {
                            uno::Any aRet;
                            uno::Sequence<uno::Any> aParams;
                            uno::Sequence<sal_Int16> aOutArgsIndex;
                            uno::Sequence<uno::Any> aOutArgs;
                            pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                        }
                    }

                    try
                    {
                        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( pDoc->GetVbaEventProcessor(), uno::UNO_SET_THROW );
                        uno::Sequence< uno::Any > aArgs( 1 );
                        aArgs[ 0 ] <<= nTab;
                        xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( SC_SHEETEVENT_CALCULATE ), aArgs );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }
        }
        pDoc->ResetCalcNotifications();
    }
}



sal_Bool ScModelObj::isOpenCLEnabled()
    throw (uno::RuntimeException)
{
    return ScInterpreter::GetGlobalConfig().mbOpenCLEnabled;
}

void ScModelObj::enableOpenCL(sal_Bool bEnable)
    throw (uno::RuntimeException)
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLEnabled = bEnable;
    ScInterpreter::SetGlobalConfig(aConfig);
}

void ScModelObj::enableAutomaticDeviceSelection(sal_Bool bForce)
    throw (uno::RuntimeException)
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLAutoSelect = true;
    ScInterpreter::SetGlobalConfig(aConfig);
    ScFormulaOptions aOptions = SC_MOD()->GetFormulaOptions();
    aOptions.SetCalcConfig(aConfig);
    SC_MOD()->SetFormulaOptions(aOptions);
    sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(), true, bForce);
}

void ScModelObj::disableAutomaticDeviceSelection()
    throw (uno::RuntimeException)
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLAutoSelect = false;
    ScInterpreter::SetGlobalConfig(aConfig);
    ScFormulaOptions aOptions = SC_MOD()->GetFormulaOptions();
    aOptions.SetCalcConfig(aConfig);
    SC_MOD()->SetFormulaOptions(aOptions);
}

void ScModelObj::selectOpenCLDevice( sal_Int32 nPlatform, sal_Int32 nDevice )
    throw (uno::RuntimeException)
{
    if(nPlatform < 0 || nDevice < 0)
        throw uno::RuntimeException();

    std::vector<sc::OpenclPlatformInfo> aPlatformInfo;
    sc::FormulaGroupInterpreter::fillOpenCLInfo(aPlatformInfo);
    if(size_t(nPlatform) >= aPlatformInfo.size())
        throw uno::RuntimeException();

    if(size_t(nDevice) >= aPlatformInfo[nPlatform].maDevices.size())
        throw uno::RuntimeException();

    OUString aDeviceString = aPlatformInfo[nPlatform].maVendor + " " + aPlatformInfo[nPlatform].maDevices[nDevice].maName;
    sc::FormulaGroupInterpreter::switchOpenCLDevice(aDeviceString, false);
}

sal_Int32 ScModelObj::getPlatformID()
    throw (uno::RuntimeException)
{
    sal_Int32 nPlatformId;
    sal_Int32 nDeviceId;
    sc::FormulaGroupInterpreter::getOpenCLDeviceInfo(nDeviceId, nPlatformId);
    return nPlatformId;
}

sal_Int32 ScModelObj::getDeviceID()
    throw (uno::RuntimeException)
{
    sal_Int32 nPlatformId;
    sal_Int32 nDeviceId;
    sc::FormulaGroupInterpreter::getOpenCLDeviceInfo(nDeviceId, nPlatformId);
    return nDeviceId;
}

uno::Sequence< sheet::opencl::OpenCLPlatform > ScModelObj::getOpenCLPlatforms()
    throw (uno::RuntimeException)
{
    std::vector<sc::OpenclPlatformInfo> aPlatformInfo;
    sc::FormulaGroupInterpreter::fillOpenCLInfo(aPlatformInfo);

    uno::Sequence<sheet::opencl::OpenCLPlatform> aRet(aPlatformInfo.size());
    for(size_t i = 0; i < aPlatformInfo.size(); ++i)
    {
        aRet[i].Name = aPlatformInfo[i].maName;
        aRet[i].Vendor = aPlatformInfo[i].maVendor;

        aRet[i].Devices.realloc(aPlatformInfo[i].maDevices.size());
        for(size_t j = 0; j < aPlatformInfo[i].maDevices.size(); ++j)
        {
            const sc::OpenclDeviceInfo& rDevice = aPlatformInfo[i].maDevices[j];
            aRet[i].Devices[j].Name = rDevice.maName;
            aRet[i].Devices[j].Vendor = rDevice.maVendor;
            aRet[i].Devices[j].Driver = rDevice.maDriver;
        }
    }

    return aRet;
}




ScDrawPagesObj::ScDrawPagesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDrawPagesObj::~ScDrawPagesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDrawPagesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}

uno::Reference<drawing::XDrawPage> ScDrawPagesObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    if (pDocShell)
    {
        ScDrawLayer* pDrawLayer = pDocShell->MakeDrawLayer();
        OSL_ENSURE(pDrawLayer,"kann Draw-Layer nicht anlegen");
        if ( pDrawLayer && nIndex >= 0 && nIndex < pDocShell->GetDocument()->GetTableCount() )
        {
            SdrPage* pPage = pDrawLayer->GetPage((sal_uInt16)nIndex);
            OSL_ENSURE(pPage,"Draw-Page nicht gefunden");
            if (pPage)
            {
                return uno::Reference<drawing::XDrawPage> (pPage->getUnoPage(), uno::UNO_QUERY);
            }
        }
    }
    return NULL;
}



uno::Reference<drawing::XDrawPage> SAL_CALL ScDrawPagesObj::insertNewByIndex( sal_Int32 nPos )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<drawing::XDrawPage> xRet;
    if (pDocShell)
    {
        OUString aNewName;
        pDocShell->GetDocument()->CreateValidTabName(aNewName);
        if ( pDocShell->GetDocFunc().InsertTable( static_cast<SCTAB>(nPos),
                                                  aNewName, true, true ) )
            xRet.set(GetObjectByIndex_Impl( nPos ));
    }
    return xRet;
}

void SAL_CALL ScDrawPagesObj::remove( const uno::Reference<drawing::XDrawPage>& xPage )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SvxDrawPage* pImp = SvxDrawPage::getImplementation( xPage );
    if ( pDocShell && pImp )
    {
        SdrPage* pPage = pImp->GetSdrPage();
        if (pPage)
        {
            SCTAB nPageNum = static_cast<SCTAB>(pPage->GetPageNum());
            pDocShell->GetDocFunc().DeleteTable( nPageNum, true, true );
        }
    }
}



sal_Int32 SAL_CALL ScDrawPagesObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument()->GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScDrawPagesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<drawing::XDrawPage> xPage(GetObjectByIndex_Impl(nIndex));
    if (xPage.is())
        return uno::makeAny(xPage);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDrawPagesObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<drawing::XDrawPage>*)0);
}

sal_Bool SAL_CALL ScDrawPagesObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



ScTableSheetsObj::ScTableSheetsObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableSheetsObj::~ScTableSheetsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableSheetsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



ScTableSheetObj* ScTableSheetsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    if ( pDocShell && nIndex >= 0 && nIndex < pDocShell->GetDocument()->GetTableCount() )
        return new ScTableSheetObj( pDocShell, static_cast<SCTAB>(nIndex) );

    return NULL;
}

ScTableSheetObj* ScTableSheetsObj::GetObjectByName_Impl(const OUString& aName) const
{
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument()->GetTable( aName, nIndex ) )
            return new ScTableSheetObj( pDocShell, nIndex );
    }
    return NULL;
}

void SAL_CALL ScTableSheetsObj::insertNewByName( const OUString& aName, sal_Int16 nPosition )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if (pDocShell)
    {
        OUString aNamStr(aName);
        bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}

void SAL_CALL ScTableSheetsObj::moveByName( const OUString& aName, sal_Int16 nDestination )
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if (pDocShell)
    {
        SCTAB nSource;
        if ( pDocShell->GetDocument()->GetTable( aName, nSource ) )
            bDone = pDocShell->MoveTable( nSource, nDestination, false, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}

void SAL_CALL ScTableSheetsObj::copyByName( const OUString& aName,
    const OUString& aCopy, sal_Int16 nDestination )
        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if (pDocShell)
    {
        OUString aNewStr(aCopy);
        SCTAB nSource;
        if ( pDocShell->GetDocument()->GetTable( aName, nSource ) )
        {
            bDone = pDocShell->MoveTable( nSource, nDestination, true, true );
            if (bDone)
            {
                
                SCTAB nResultTab = static_cast<SCTAB>(nDestination);
                SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();    
                if (nResultTab >= nTabCount)
                    nResultTab = nTabCount - 1;

                bDone = pDocShell->GetDocFunc().RenameTable( nResultTab, aNewStr,
                                                             true, true );
            }
        }
    }
    if (!bDone)
        throw uno::RuntimeException();      
}

void SAL_CALL ScTableSheetsObj::insertByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    sal_Bool bIllArg = false;

    

    if ( pDocShell )
    {
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
        if ( xInterface.is() )
        {
            ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
            if ( pSheetObj && !pSheetObj->GetDocShell() )   
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                OUString aNamStr(aName);
                SCTAB nDummy;
                if ( pDoc->GetTable( aNamStr, nDummy ) )
                {
                    
                    throw container::ElementExistException();
                }
                else
                {
                    SCTAB nPosition = pDoc->GetTableCount();
                    bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr,
                                                                 true, true );
                    if (bDone)
                        pSheetObj->InitInsertSheet( pDocShell, nPosition );
                    
                }
            }
            else
                bIllArg = sal_True;
        }
        else
            bIllArg = sal_True;
    }

    if (!bDone)
    {
        if (bIllArg)
            throw lang::IllegalArgumentException();
        else
            throw uno::RuntimeException();      
    }
}

void SAL_CALL ScTableSheetsObj::replaceByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    sal_Bool bIllArg = false;

    

    if ( pDocShell )
    {
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
        if ( xInterface.is() )
        {
            ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
            if ( pSheetObj && !pSheetObj->GetDocShell() )   
            {
                SCTAB nPosition;
                if ( pDocShell->GetDocument()->GetTable( aName, nPosition ) )
                {
                    if ( pDocShell->GetDocFunc().DeleteTable( nPosition, true, true ) )
                    {
                        
                        OUString aNamStr(aName);
                        bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr, true, true );
                        if (bDone)
                            pSheetObj->InitInsertSheet( pDocShell, nPosition );
                    }
                }
                else
                {
                    
                    throw container::NoSuchElementException();
                }
            }
            else
                bIllArg = sal_True;
        }
        else
            bIllArg = sal_True;
    }

    if (!bDone)
    {
        if (bIllArg)
            throw lang::IllegalArgumentException();
        else
            throw uno::RuntimeException();      
    }
}

void SAL_CALL ScTableSheetsObj::removeByName( const OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument()->GetTable( aName, nIndex ) )
            bDone = pDocShell->GetDocFunc().DeleteTable( nIndex, true, true );
        else 
            throw container::NoSuchElementException();
    }

    if (!bDone)
        throw uno::RuntimeException();      
}

sal_Int32 ScTableSheetsObj::importSheet(
    const uno::Reference < sheet::XSpreadsheetDocument > & xDocSrc,
    const OUString& srcName, const sal_Int32 nDestPosition )
        throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    
    ScDocument* pDocDest = pDocShell->GetDocument();

    
    if ( !xDocSrc.is() )
        throw uno::RuntimeException();
    ScModelObj* pObj = ScModelObj::getImplementation(xDocSrc);
    ScDocShell* pDocShellSrc = static_cast<ScDocShell*>(pObj->GetEmbeddedObject());

    
    SCTAB nIndexSrc;
    if ( !pDocShellSrc->GetDocument()->GetTable( srcName, nIndexSrc ) )
        throw lang::IllegalArgumentException();

    
    SCTAB nCount = pDocDest->GetTableCount();
    SCTAB nIndexDest = static_cast<SCTAB>(nDestPosition);
    if (nIndexDest > nCount || nIndexDest < 0)
        throw lang::IndexOutOfBoundsException();

    
    bool bInsertNew = true;
    bool bNotifyAndPaint = true;
    pDocShell->TransferTab(
        *pDocShellSrc, nIndexSrc, nIndexDest, bInsertNew, bNotifyAndPaint );

    return nIndexDest;
}



uno::Reference< table::XCell > SAL_CALL ScTableSheetsObj::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellByPosition(nColumn, nRow);
}

uno::Reference< table::XCellRange > SAL_CALL ScTableSheetsObj::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellRangeByPosition(nLeft, nTop, nRight, nBottom);
}

uno::Sequence < uno::Reference< table::XCellRange > > SAL_CALL ScTableSheetsObj::getCellRangesByName( const OUString& aRange )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence < uno::Reference < table::XCellRange > > xRet;

    ScRangeList aRangeList;
    ScDocument* pDoc = pDocShell->GetDocument();
    if (ScRangeStringConverter::GetRangeListFromString( aRangeList, aRange, pDoc, ::formula::FormulaGrammar::CONV_OOO, ';' ))
    {
        size_t nCount = aRangeList.size();
        if (nCount)
        {
            xRet.realloc(nCount);
            for( size_t nIndex = 0; nIndex < nCount; nIndex++ )
            {
                const ScRange* pRange = aRangeList[ nIndex ];
                if( pRange )
                    xRet[nIndex] = new ScCellRangeObj(pDocShell, *pRange);
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw lang::IllegalArgumentException();
    return xRet;
}



uno::Reference<container::XEnumeration> SAL_CALL ScTableSheetsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.SpreadsheetsEnumeration"));
}



sal_Int32 SAL_CALL ScTableSheetsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument()->GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScTableSheetsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByIndex_Impl(nIndex));
    if (xSheet.is())
        return uno::makeAny(xSheet);
    else
        throw lang::IndexOutOfBoundsException();

}

uno::Type SAL_CALL ScTableSheetsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSpreadsheet>*)0);
}

sal_Bool SAL_CALL ScTableSheetsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



uno::Any SAL_CALL ScTableSheetsObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByName_Impl(aName));
    if (xSheet.is())
        return uno::makeAny(xSheet);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScTableSheetsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nCount = pDoc->GetTableCount();
        OUString aName;
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        for (SCTAB i=0; i<nCount; i++)
        {
            pDoc->GetName( i, aName );
            pAry[i] = aName;
        }
        return aSeq;
    }
    return uno::Sequence<OUString>();
}

sal_Bool SAL_CALL ScTableSheetsObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument()->GetTable( aName, nIndex ) )
            return sal_True;
    }
    return false;
}



ScTableColumnsObj::ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT, SCCOL nSC, SCCOL nEC) :
    pDocShell( pDocSh ),
    nTab     ( nT ),
    nStartCol( nSC ),
    nEndCol  ( nEC )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableColumnsObj::~ScTableColumnsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableColumnsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



ScTableColumnObj* ScTableColumnsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCCOL nCol = static_cast<SCCOL>(nIndex) + nStartCol;
    if ( pDocShell && nCol <= nEndCol )
        return new ScTableColumnObj( pDocShell, nCol, nTab );

    return NULL;    
}

ScTableColumnObj* ScTableColumnsObj::GetObjectByName_Impl(const OUString& aName) const
{
    SCCOL nCol = 0;
    OUString aString(aName);
    if ( ::AlphaToCol( nCol, aString) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return new ScTableColumnObj( pDocShell, nCol, nTab );

    return NULL;
}

void SAL_CALL ScTableColumnsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartCol+nPosition <= nEndCol &&
            nStartCol+nPosition+nCount-1 <= MAXCOL )
    {
        ScRange aRange( (SCCOL)(nStartCol+nPosition), 0, nTab,
                        (SCCOL)(nStartCol+nPosition+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, NULL, INS_INSCOLS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}

void SAL_CALL ScTableColumnsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartCol+nIndex+nCount-1 <= nEndCol )
    {
        ScRange aRange( (SCCOL)(nStartCol+nIndex), 0, nTab,
                        (SCCOL)(nStartCol+nIndex+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, NULL, DEL_DELCOLS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}



uno::Reference<container::XEnumeration> SAL_CALL ScTableColumnsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.table.TableColumnsEnumeration"));
}



sal_Int32 SAL_CALL ScTableColumnsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return nEndCol - nStartCol + 1;
}

uno::Any SAL_CALL ScTableColumnsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByIndex_Impl(nIndex));
    if (xColumn.is())
        return uno::makeAny(xColumn);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScTableColumnsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScTableColumnsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScTableColumnsObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByName_Impl(aName));
    if (xColumn.is())
        return uno::makeAny(xColumn);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScTableColumnsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCCOL nCount = nEndCol - nStartCol + 1;
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pAry = aSeq.getArray();
    for (SCCOL i=0; i<nCount; i++)
        pAry[i] = ::ScColToAlpha( nStartCol + i );

    return aSeq;
}

sal_Bool SAL_CALL ScTableColumnsObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCCOL nCol = 0;
    OUString aString(aName);
    if ( ::AlphaToCol( nCol, aString) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return sal_True;

    return false;       
}



uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetColumnsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableColumnsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    SCCOLROW nColArr[2];
    nColArr[0] = nStartCol;
    nColArr[1] = nEndCol;
    OUString aNameString(aPropertyName);
    ScDocFunc& rFunc = pDocShell->GetDocFunc();

    if ( aNameString.equalsAscii( SC_UNONAME_CELLWID ) )
    {
        sal_Int32 nNewWidth = 0;
        if ( aValue >>= nNewWidth )
            rFunc.SetWidthOrHeight( true, 1, nColArr, nTab, SC_SIZE_ORIGINAL,
                                    (sal_uInt16)HMMToTwips(nNewWidth), true, true );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLVIS ) )
    {
        sal_Bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight( true, 1, nColArr, nTab, eMode, 0, true, true );
        
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_OWIDTH ) )
    {
        sal_Bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            rFunc.SetWidthOrHeight( true, 1, nColArr, nTab,
                                    SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, true, true );
        
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_NEWPAGE ) || aNameString.equalsAscii( SC_UNONAME_MANPAGE ) )
    {
        
        sal_Bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
            if (bSet)
                rFunc.InsertPageBreak( true, ScAddress(nCol,0,nTab), true, true, true );
            else
                rFunc.RemovePageBreak( true, ScAddress(nCol,0,nTab), true, true, true );
    }
}

uno::Any SAL_CALL ScTableColumnsObj::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument* pDoc = pDocShell->GetDocument();
    OUString aNameString(aPropertyName);
    uno::Any aAny;

    

    if ( aNameString.equalsAscii( SC_UNONAME_CELLWID ) )
    {
        
        sal_uInt16 nWidth = pDoc->GetOriginalWidth( nStartCol, nTab );
        aAny <<= (sal_Int32)TwipsToHMM(nWidth);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLVIS ) )
    {
        bool bVis = !pDoc->ColHidden(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_OWIDTH ) )
    {
        sal_Bool bOpt = !(pDoc->GetColFlags( nStartCol, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_NEWPAGE ) )
    {
        ScBreakType nBreak = pDoc->HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_MANPAGE ) )
    {
        ScBreakType nBreak = pDoc->HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) );
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableColumnsObj )



ScTableRowsObj::ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT, SCROW nSR, SCROW nER) :
    pDocShell( pDocSh ),
    nTab     ( nT ),
    nStartRow( nSR ),
    nEndRow  ( nER )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableRowsObj::~ScTableRowsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableRowsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



ScTableRowObj* ScTableRowsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCROW nRow = static_cast<SCROW>(nIndex) + nStartRow;
    if ( pDocShell && nRow <= nEndRow )
        return new ScTableRowObj( pDocShell, nRow, nTab );

    return NULL;    
}

void SAL_CALL ScTableRowsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartRow+nPosition <= nEndRow &&
            nStartRow+nPosition+nCount-1 <= MAXROW )
    {
        ScRange aRange( 0, (SCROW)(nStartRow+nPosition), nTab,
                        MAXCOL, (SCROW)(nStartRow+nPosition+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, NULL, INS_INSROWS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}

void SAL_CALL ScTableRowsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartRow+nIndex+nCount-1 <= nEndRow )
    {
        ScRange aRange( 0, (SCROW)(nStartRow+nIndex), nTab,
                        MAXCOL, (SCROW)(nStartRow+nIndex+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, NULL, DEL_DELROWS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      
}



uno::Reference<container::XEnumeration> SAL_CALL ScTableRowsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.table.TableRowsEnumeration"));
}



sal_Int32 SAL_CALL ScTableRowsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return nEndRow - nStartRow + 1;
}

uno::Any SAL_CALL ScTableRowsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xRow(GetObjectByIndex_Impl(nIndex));
    if (xRow.is())
        return uno::makeAny(xRow);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScTableRowsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScTableRowsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetRowsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableRowsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocFunc& rFunc = pDocShell->GetDocFunc();
    ScDocument* pDoc = pDocShell->GetDocument();
    SCCOLROW nRowArr[2];
    nRowArr[0] = nStartRow;
    nRowArr[1] = nEndRow;
    OUString aNameString(aPropertyName);

    if ( aNameString.equalsAscii( SC_UNONAME_OHEIGHT ) )
    {
        sal_Int32 nNewHeight = 0;
        if ( pDoc->IsImportingXML() && ( aValue >>= nNewHeight ) )
        {
            

            
            
            pDoc->SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)HMMToTwips(nNewHeight) );
        }
        else
        {
            sal_Bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bOpt)
                rFunc.SetWidthOrHeight( false, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true );
            else
            {
                
            }
        }
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLHGT ) )
    {
        sal_Int32 nNewHeight = 0;
        if ( aValue >>= nNewHeight )
        {
            if (pDoc->IsImportingXML())
            {
                
                
                
                pDoc->SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)HMMToTwips(nNewHeight) );
                pDoc->SetManualHeight( nStartRow, nEndRow, nTab, true );
            }
            else
                rFunc.SetWidthOrHeight( false, 1, nRowArr, nTab, SC_SIZE_ORIGINAL,
                                        (sal_uInt16)HMMToTwips(nNewHeight), true, true );
        }
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLVIS ) )
    {
        sal_Bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight( false, 1, nRowArr, nTab, eMode, 0, true, true );
        
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_VISFLAG ) )
    {
        
        
        pDoc->SetRowHidden(nStartRow, nEndRow, nTab, !ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLFILT ) )
    {
        
        if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            pDoc->SetRowFiltered(nStartRow, nEndRow, nTab, true);
        else
            pDoc->SetRowFiltered(nStartRow, nEndRow, nTab, false);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_NEWPAGE) || aNameString.equalsAscii( SC_UNONAME_MANPAGE) )
    {
        
        sal_Bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
            if (bSet)
                rFunc.InsertPageBreak( false, ScAddress(0,nRow,nTab), true, true, true );
            else
                rFunc.RemovePageBreak( false, ScAddress(0,nRow,nTab), true, true, true );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLBACK ) || aNameString.equalsAscii( SC_UNONAME_CELLTRAN ) )
    {
        
        

        
        

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        xRangeObj->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScTableRowsObj::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument* pDoc = pDocShell->GetDocument();
    OUString aNameString(aPropertyName);
    uno::Any aAny;

    

    if ( aNameString.equalsAscii( SC_UNONAME_CELLHGT ) )
    {
        
        sal_uInt16 nHeight = pDoc->GetOriginalHeight( nStartRow, nTab );
        aAny <<= (sal_Int32)TwipsToHMM(nHeight);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLVIS ) )
    {
        SCROW nLastRow;
        bool bVis = !pDoc->RowHidden(nStartRow, nTab, NULL, &nLastRow);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLFILT ) )
    {
        bool bVis = pDoc->RowFiltered(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_OHEIGHT ) )
    {
        sal_Bool bOpt = !(pDoc->GetRowFlags( nStartRow, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_NEWPAGE ) )
    {
        ScBreakType nBreak = pDoc->HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_MANPAGE ) )
    {
        ScBreakType nBreak = pDoc->HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_CELLBACK ) || aNameString.equalsAscii( SC_UNONAME_CELLTRAN ) )
    {
        
        

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        aAny = xRangeObj->getPropertyValue( aPropertyName );
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableRowsObj )



ScSpreadsheetSettingsObj::~ScSpreadsheetSettingsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScSpreadsheetSettingsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettingsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    
    return NULL;
}

void SAL_CALL ScSpreadsheetSettingsObj::setPropertyValue(
                        const OUString& /* aPropertyName */, const uno::Any& /* aValue */ )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    
}

uno::Any SAL_CALL ScSpreadsheetSettingsObj::getPropertyValue( const OUString& /* aPropertyName */ )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    
    return uno::Any();
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettingsObj )



ScAnnotationsObj::ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAnnotationsObj::~ScAnnotationsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScAnnotationsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}

bool ScAnnotationsObj::GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const
{
    if (!pDocShell)
        return false;

    ScDocument* pDoc = pDocShell->GetDocument();
    rPos = pDoc->GetNotePosition(nIndex);
    return rPos.IsValid();
}

ScAnnotationObj* ScAnnotationsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
    if (pDocShell)
    {
        ScAddress aPos;
        if ( GetAddressByIndex_Impl( nIndex, aPos ) )
            return new ScAnnotationObj( pDocShell, aPos );
    }
    return NULL;
}



void SAL_CALL ScAnnotationsObj::insertNew(
        const table::CellAddress& aPosition, const OUString& rText )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OSL_ENSURE( aPosition.Sheet == nTab, "addAnnotation mit falschem Sheet" );
        ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, nTab );
        pDocShell->GetDocFunc().ReplaceNote( aPos, rText, 0, 0, true );
    }
}

void SAL_CALL ScAnnotationsObj::removeByIndex( sal_Int32 nIndex ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScAddress aPos;
        if ( GetAddressByIndex_Impl( nIndex, aPos ) )
        {
            ScMarkData aMarkData;
            aMarkData.SelectTable( aPos.Tab(), true );
            aMarkData.SetMultiMarkArea( ScRange(aPos) );

            pDocShell->GetDocFunc().DeleteContents( aMarkData, IDF_NOTE, true, true );
        }
    }
}



uno::Reference<container::XEnumeration> SAL_CALL ScAnnotationsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    

    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.CellAnnotationsEnumeration"));
}



sal_Int32 SAL_CALL ScAnnotationsObj::getCount()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        const ScRangeList aRangeList( ScRange( 0, 0, nTab, MAXCOL, MAXROW, nTab) );
        std::vector<sc::NoteEntry> rNotes;
        pDoc->GetNotesInRange(aRangeList, rNotes);
        nCount = rNotes.size();
    }
    return nCount;
}

uno::Any SAL_CALL ScAnnotationsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(GetObjectByIndex_Impl(nIndex));
    if (xAnnotation.is())
        return uno::makeAny(xAnnotation);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScAnnotationsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSheetAnnotation>*)0);
}

sal_Bool SAL_CALL ScAnnotationsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



ScScenariosObj::ScScenariosObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab     ( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScScenariosObj::~ScScenariosObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScScenariosObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



bool ScScenariosObj::GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex )
{
    

    if ( pDocShell )
    {
        OUString aTabName;
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nCount = (SCTAB)getCount();
        for (SCTAB i=0; i<nCount; i++)
            if (pDoc->GetName( nTab+i+1, aTabName ))
                if (aTabName.equals(rName))
                {
                    rIndex = i;
                    return true;
                }
    }

    return false;
}

ScTableSheetObj* ScScenariosObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    sal_uInt16 nCount = (sal_uInt16)getCount();
    if ( pDocShell && nIndex >= 0 && nIndex < nCount )
        return new ScTableSheetObj( pDocShell, nTab+static_cast<SCTAB>(nIndex)+1 );

    return NULL;    
}

ScTableSheetObj* ScScenariosObj::GetObjectByName_Impl(const OUString& aName)
{
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        return new ScTableSheetObj( pDocShell, nTab+nIndex+1 );

    return NULL;    
}

void SAL_CALL ScScenariosObj::addNewByName( const OUString& aName,
                                const uno::Sequence<table::CellRangeAddress>& aRanges,
                                const OUString& aComment )
                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.SelectTable( nTab, true );

        sal_uInt16 nRangeCount = (sal_uInt16)aRanges.getLength();
        if (nRangeCount)
        {
            const table::CellRangeAddress* pAry = aRanges.getConstArray();
            for (sal_uInt16 i=0; i<nRangeCount; i++)
            {
                OSL_ENSURE( pAry[i].Sheet == nTab, "addScenario mit falscher Tab" );
                ScRange aRange( (SCCOL)pAry[i].StartColumn, (SCROW)pAry[i].StartRow, nTab,
                                (SCCOL)pAry[i].EndColumn,   (SCROW)pAry[i].EndRow,   nTab );

                aMarkData.SetMultiMarkArea( aRange );
            }
        }

        OUString aNameStr(aName);
        OUString aCommStr(aComment);

        Color aColor( COL_LIGHTGRAY );  
        sal_uInt16 nFlags = SC_SCENARIO_SHOWFRAME | SC_SCENARIO_PRINTFRAME | SC_SCENARIO_TWOWAY | SC_SCENARIO_PROTECT;

        pDocShell->MakeScenario( nTab, aNameStr, aCommStr, aColor, nFlags, aMarkData );
    }
}

void SAL_CALL ScScenariosObj::removeByName( const OUString& aName )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        pDocShell->GetDocFunc().DeleteTable( nTab+nIndex+1, true, true );
}



uno::Reference<container::XEnumeration> SAL_CALL ScScenariosObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.ScenariosEnumeration"));
}



sal_Int32 SAL_CALL ScScenariosObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCTAB nCount = 0;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if (!pDoc->IsScenario(nTab))
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            SCTAB nNext = nTab + 1;
            while (nNext < nTabCount && pDoc->IsScenario(nNext))
            {
                ++nCount;
                ++nNext;
            }
        }
    }
    return nCount;
}

uno::Any SAL_CALL ScScenariosObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByIndex_Impl(nIndex));
    if (xScen.is())
        return uno::makeAny(xScen);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScScenariosObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XScenario>*)0);
}

sal_Bool SAL_CALL ScScenariosObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScScenariosObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByName_Impl(aName));
    if (xScen.is())
        return uno::makeAny(xScen);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScScenariosObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCTAB nCount = (SCTAB)getCount();
    uno::Sequence<OUString> aSeq(nCount);

    if ( pDocShell )    
    {
        OUString aTabName;
        ScDocument* pDoc = pDocShell->GetDocument();
        OUString* pAry = aSeq.getArray();
        for (SCTAB i=0; i<nCount; i++)
            if (pDoc->GetName( nTab+i+1, aTabName ))
                pAry[i] = aTabName;
    }

    return aSeq;
}

sal_Bool SAL_CALL ScScenariosObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    return GetScenarioIndex_Impl( aName, nIndex );
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
