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


#include "optimizerdialog.hxx"


#include "pppoptimizer.hxx"
#include "graphiccollector.hxx"
#include "pagecollector.hxx"
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>
#include <o3tl/string_view.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

void OptimizerDialog::InitNavigationBar()
{
    m_xHelp->hide();
}

void OptimizerDialog::UpdateControlStatesPage0()
{
    int nSelectedItem = -1;
    std::vector<OUString> aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        for ( std::vector<OptimizerSettings>::size_type i = 1; i < rList.size(); i++ )
        {
            aItemList.push_back(rList[i].maName);
            if ( nSelectedItem < 0 )
            {
                if ( rList[ i ] == rList[ 0 ] )
                    nSelectedItem = static_cast< short >( i - 1 );
            }
        }
    }
    bool bRemoveButtonEnabled = false;
    if ( nSelectedItem >= 0 )
    {
        if ( nSelectedItem > 2 )    // only allowing to delete custom themes, the first can|t be deleted
            bRemoveButtonEnabled = true;
    }
    mpPage0->UpdateControlStates(aItemList, nSelectedItem, bRemoveButtonEnabled);
}

void OptimizerDialog::InitPage0()
{
    UpdateControlStatesPage0();
}

void OptimizerDialog::UpdateControlStatesPage1()
{
    bool bDeleteUnusedMasterPages( GetConfigProperty( TK_DeleteUnusedMasterPages, false ) );
    bool bDeleteHiddenSlides( GetConfigProperty( TK_DeleteHiddenSlides, false ) );
    bool bDeleteNotesPages( GetConfigProperty( TK_DeleteNotesPages, false ) );
    mpPage1->UpdateControlStates(bDeleteUnusedMasterPages, bDeleteHiddenSlides, bDeleteNotesPages);
}

void OptimizerDialog::InitPage1()
{
    Sequence< OUString > aCustomShowList;
    Reference< XModel > xModel( mxController->getModel() );
    if ( xModel.is() )
    {
        Reference< XCustomPresentationSupplier > aXCPSup( xModel, UNO_QUERY_THROW );
        Reference< XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
        if ( aXCont.is() )
            aCustomShowList = aXCont->getElementNames();
    }
    mpPage1->Init(aCustomShowList);

    UpdateControlStatesPage1();
}

void OptimizerDialog::UpdateControlStatesPage2()
{
    bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, false ) );
    bool bRemoveCropArea( GetConfigProperty( TK_RemoveCropArea, false ) );
    bool bEmbedLinkedGraphics( GetConfigProperty( TK_EmbedLinkedGraphics, true ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, sal_Int32(90) ) );
    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, sal_Int32(0) ) );

    mpPage2->UpdateControlStates(bJPEGCompression, nJPEGQuality, bRemoveCropArea, nImageResolution, bEmbedLinkedGraphics);
}

void OptimizerDialog::InitPage2()
{
    UpdateControlStatesPage2();
}

void OptimizerDialog::UpdateControlStatesPage3()
{
    bool bConvertOLEObjects( GetConfigProperty( TK_OLEOptimization, false ) );
    sal_Int16 nOLEOptimizationType( GetConfigProperty( TK_OLEOptimizationType, sal_Int16(0) ) );

    mpPage3->UpdateControlStates(bConvertOLEObjects, nOLEOptimizationType);
}

void OptimizerDialog::InitPage3()
{
    int nOLECount = 0;
    Reference< XModel > xModel( mxController->getModel() );
    Reference< XDrawPagesSupplier > xDrawPagesSupplier( xModel, UNO_QUERY_THROW );
    Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_SET_THROW );
    for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
    {
        Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
        for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
        {
            Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
            if ( xShape->getShapeType() == "com.sun.star.drawing.OLE2Shape" )
                nOLECount++;
        }
    }

    mpPage3->Init(nOLECount ? getString(STR_OLE_OBJECTS_DESC) : getString(STR_NO_OLE_OBJECTS_DESC));

    UpdateControlStatesPage3();
}

static OUString ImpValueOfInMB( sal_Int64 rVal, sal_Unicode nSeparator )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( aVal.indexOf( '.' ) );
    if ( nX >= 0 )
    {
        aVal.setLength( nX + 2 );
        aVal[nX] = nSeparator;
    }
    aVal.append( " MB" );
    return aVal.makeStringAndClear();
}

void OptimizerDialog::UpdateControlStatesPage4()
{
    bool bSaveAs( GetConfigProperty( TK_SaveAs, true ) );
    if ( mbIsReadonly )
        bSaveAs = true;

    std::vector<OUString> aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        for ( std::vector<OptimizerSettings>::size_type w = 1; w < rList.size(); w++ )
            aItemList.push_back(rList[ w ].maName);
    }

    // now check if it is sensible to enable the combo box
    bool bSaveSettingsEnabled = true;
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        for ( std::vector<OptimizerSettings>::size_type w = 1; w < rList.size(); w++ )
        {
            if ( rList[ w ] == rList[ 0 ] )
            {
                bSaveSettingsEnabled = false;
                break;
            }
        }
    }

    std::vector< OUString > aSummaryStrings;

    // taking care of deleted slides
    sal_Int32 nDeletedSlides = 0;
    OUString sTKCustomShowName(mpPage1->Get_TK_CustomShowName());
    if (!sTKCustomShowName.isEmpty())
        SetConfigProperty(TK_CustomShowName, Any(sTKCustomShowName));
    if ( GetConfigProperty( TK_DeleteHiddenSlides, false ) )
    {
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_SET_THROW );
        for( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY_THROW );

            bool bVisible = true;
            if ( xPropSet->getPropertyValue( u"Visible"_ustr ) >>= bVisible )
            {
                if (!bVisible )
                    nDeletedSlides++;
            }
        }
    }
    if ( GetConfigProperty( TK_DeleteUnusedMasterPages, false ) )
    {
        std::vector< PageCollector::MasterPageEntity > aMasterPageList;
        PageCollector::CollectMasterPages( mxController->getModel(), aMasterPageList );
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_SET_THROW );
        nDeletedSlides += std::count_if(aMasterPageList.begin(), aMasterPageList.end(),
            [](const PageCollector::MasterPageEntity& rEntity) { return !rEntity.bUsed; });
    }
    if ( nDeletedSlides > 1 )
    {
        OUString aStr( getString( STR_DELETE_SLIDES ) );
        OUString aPlaceholder( u"%SLIDES"_ustr  );
        sal_Int32 i = aStr.indexOf( aPlaceholder );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nDeletedSlides ) );
        aSummaryStrings.push_back( aStr );
    }

// generating graphic compression info
    sal_Int32 nGraphics = 0;
    bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, false ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, sal_Int32(90) ) );
    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, sal_Int32(0) ) );
    GraphicSettings aGraphicSettings( bJPEGCompression, nJPEGQuality, GetConfigProperty( TK_RemoveCropArea, false ),
                                        nImageResolution, GetConfigProperty( TK_EmbedLinkedGraphics, true ) );
    GraphicCollector::CountGraphics( mxContext, mxController->getModel(), aGraphicSettings, nGraphics );
    if ( nGraphics > 1 )
    {
        OUString aStr( getString( STR_OPTIMIZE_IMAGES ) );
        OUString aImagePlaceholder( u"%IMAGES"_ustr  );
        OUString aQualityPlaceholder( u"%QUALITY"_ustr  );
        OUString aResolutionPlaceholder( u"%RESOLUTION"_ustr  );
        sal_Int32 i = aStr.indexOf( aImagePlaceholder );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aImagePlaceholder.getLength(), OUString::number( nGraphics ) );

        sal_Int32 j = aStr.indexOf( aQualityPlaceholder );
        if ( j >= 0 )
            aStr = aStr.replaceAt( j, aQualityPlaceholder.getLength(), OUString::number( nJPEGQuality ) );

        sal_Int32 k = aStr.indexOf( aResolutionPlaceholder );
        if ( k >= 0 )
            aStr = aStr.replaceAt( k, aResolutionPlaceholder.getLength(), OUString::number( nImageResolution ) );

        aSummaryStrings.push_back( aStr );
    }

    if ( GetConfigProperty( TK_OLEOptimization, false ) )
    {
        sal_Int32 nOLEReplacements = 0;
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_SET_THROW );
        for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
            {
                Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
                if ( xShape->getShapeType() == "com.sun.star.drawing.OLE2Shape" )
                    nOLEReplacements++;
            }
        }
        if ( nOLEReplacements > 1 )
        {
            OUString aStr( getString( STR_CREATE_REPLACEMENT ) );
            OUString aPlaceholder( u"%OLE"_ustr  );
            sal_Int32 i = aStr.indexOf( aPlaceholder );
            if ( i >= 0 )
                aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nOLEReplacements ) );
            aSummaryStrings.push_back( aStr );
        }
    }
    while( aSummaryStrings.size() < 3 )
        aSummaryStrings.emplace_back( );

    sal_Int64 nCurrentFileSize = 0;
    sal_Int64 nEstimatedFileSize = 0;
    Reference< XStorable > xStorable( mxController->getModel(), UNO_QUERY );
    if ( xStorable.is() && xStorable->hasLocation() )
        nCurrentFileSize = PPPOptimizer::GetFileSize( xStorable->getLocation() );

    if ( nCurrentFileSize )
    {
        double fE = static_cast< double >( nCurrentFileSize );
        if ( nImageResolution )
        {
            double v = ( static_cast< double >( nImageResolution ) + 75.0 ) / 300.0;
            if ( v < 1.0 )
                fE *= v;
        }
        if ( bJPEGCompression )
        {
            double v = 0.75 - ( ( 100.0 - static_cast< double >( nJPEGQuality ) ) / 400.0 ) ;
            fE *= v;
        }
        nEstimatedFileSize = static_cast< sal_Int64 >( fE );
    }
    sal_Unicode nSeparator = '.';
    OUString aStr( getString( STR_FILESIZESEPARATOR ) );
    if ( !aStr.isEmpty() )
        nSeparator = aStr[ 0 ];
    mpPage4->UpdateControlStates(bSaveAs, bSaveSettingsEnabled, aItemList,
                                 aSummaryStrings,
                                 ImpValueOfInMB(nCurrentFileSize, nSeparator),
                                 ImpValueOfInMB(nEstimatedFileSize, nSeparator));
    SetConfigProperty( TK_EstimatedFileSize, Any( nEstimatedFileSize ) );
}

void OptimizerDialog::InitPage4()
{
    // creating a default session name that hasn't been used yet
    OUString aSettingsName;
    OUString aDefault( getString( STR_MY_SETTINGS ) );
    sal_Int32 nSession = 1;
    std::vector<OptimizerSettings>::size_type i;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    do
    {
        OUString aTemp = aDefault + OUString::number( nSession++ );
        for ( i = 1; i < rList.size(); i++ )
        {
            if ( rList[ i ].maName == aTemp )
                break;
        }
        if ( i == rList.size() )
            aSettingsName = aTemp;
    }
    while( aSettingsName.isEmpty() );

    mpPage4->Init(aSettingsName, mbIsReadonly);

    UpdateControlStatesPage4();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
