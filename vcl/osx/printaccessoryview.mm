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

#include "sal/config.h"

#include "tools/resary.hxx"

#include "vcl/print.hxx"
#include "vcl/image.hxx"
#include "vcl/virdev.hxx"
#include "vcl/svapp.hxx"
#include "vcl/unohelp.hxx"
#include <vcl/settings.hxx>

#include "osx/printview.h"
#include "osx/salinst.h"
#include "quartz/utils.h"

#include "svdata.hxx"
#include "svids.hrc"

#include "com/sun/star/i18n/XBreakIterator.hpp"
#include "com/sun/star/i18n/WordType.hpp"

#include <map>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

#if MACOSX_SDK_VERSION <= 1040
// as long as you are linking with 10.4 libraries there's no preview
# define VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
#  undef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
#else
// since 10.5 you can use multiple accessory views and have accessory views and a preview
# define MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
#  undef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
#endif

class ControllerProperties;

@interface ControlTarget : NSObject
{
    ControllerProperties* mpController;
}
-(id)initWithControllerMap: (ControllerProperties*)pController;
-(void)triggered:(id)pSender;
-(void)triggeredNumeric:(id)pSender;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
-(void)triggeredPreview:(id)pSender;
#endif
-(void)dealloc;
@end

#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG

@interface AquaPrintPanelAccessoryController : NSViewController< NSPrintPanelAccessorizing >
{
    NSPrintOperation *mpPrintOperation;
    vcl::PrinterController *mpPrinterController;
    PrintAccessoryViewState *mpViewState;
}

-(void)forPrintOperation:(NSPrintOperation*)pPrintOp;
-(void)withPrinterController:(vcl::PrinterController*)pController;
-(void)withViewState:(PrintAccessoryViewState*)pState;

-(NSPrintOperation*)printOperation;
-(vcl::PrinterController*)printerController;
-(PrintAccessoryViewState*)viewState;

-(NSSet*)keyPathsForValuesAffectingPreview;
-(NSArray*)localizedSummaryItems;

-(sal_Int32)updatePrintOperation:(sal_Int32)pLastPageCount;

@end

@implementation AquaPrintPanelAccessoryController

-(void)forPrintOperation:(NSPrintOperation*)pPrintOp
    { mpPrintOperation = pPrintOp; }

-(void)withPrinterController:(vcl::PrinterController*)pController
    { mpPrinterController = pController; }

-(void)withViewState:(PrintAccessoryViewState*)pState
    { mpViewState = pState; }

-(NSPrintOperation*)printOperation
    { return mpPrintOperation; }

-(vcl::PrinterController*)printerController
    { return mpPrinterController; }

-(PrintAccessoryViewState*)viewState
    { return mpViewState; }

-(NSSet*)keyPathsForValuesAffectingPreview
{
    return [ NSSet setWithObject:@"updatePrintOperation" ];
}

-(NSArray*)localizedSummaryItems
{
    return [ NSArray arrayWithObject:
               [ NSDictionary dictionary ] ];
}

-(sal_Int32)updatePrintOperation:(sal_Int32)pLastPageCount
{
    // page range may be changed by option choice
    sal_Int32 nPages = mpPrinterController->getFilteredPageCount();

    mpViewState->bNeedRestart = false;
    if( nPages != pLastPageCount )
    {
        #if OSL_DEBUG_LEVEL > 1
        SAL_INFO( "vcl.osx.print", "number of pages changed" <<
                  " from " << pLastPageCount << " to " << nPages );
        #endif
        mpViewState->bNeedRestart = true;
    }

    NSTabView* pTabView = [[[self view] subviews] objectAtIndex:0];
    NSTabViewItem* pItem = [pTabView selectedTabViewItem];
    if( pItem )
        mpViewState->nLastPage = [pTabView indexOfTabViewItem: pItem];
    else
        mpViewState->nLastPage = 0;

    if( mpViewState->bNeedRestart )
    {
        // AppKit does not give a chance of changing the page count
        // and don't let cancel the dialog either
        // hack: send a cancel message to the modal window displaying views
        NSWindow* pNSWindow = [NSApp modalWindow];
        if( pNSWindow )
            [pNSWindow cancelOperation: nil];
        [[mpPrintOperation printInfo] setJobDisposition: NSPrintCancelJob];
    }

    return nPages;
}

@end

#endif

class ControllerProperties
{
    std::map< int, rtl::OUString >      maTagToPropertyName;
    std::map< int, sal_Int32 >          maTagToValueInt;
    std::map< NSView*, NSView* >        maViewPairMap;
    std::vector< NSObject* >            maViews;
    int                                 mnNextTag;
    sal_Int32                           mnLastPageCount;
    ResStringArray                      maLocalizedStrings;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    vcl::PrinterController*             mpController;
    NSPrintOperation*                   mpOp;
    PrintAccessoryViewState*            mpState;
    NSBox*                              mpPreviewBox;
    NSImageView*                        mpPreview;  // print preview is not provided "by default"
    NSView*                             mpAccessoryView;
    NSTabView*                          mpTabView;
    NSTextField*                        mpPageEdit;
    NSStepper*                          mpStepper;
#else
    AquaPrintPanelAccessoryController*  mpAccessoryController;
#endif

public:
    ControllerProperties(
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
                          vcl::PrinterController* i_pController
                        , NSPrintOperation* i_pOp
                        , PrintAccessoryViewState* i_pState
                        , NSView* i_pAccessoryView
                        , NSTabView* i_pTabView
#else
                          AquaPrintPanelAccessoryController* i_pAccessoryController
#endif
                         )
    : mnNextTag( 0 )
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    , mnLastPageCount( i_pController->getFilteredPageCount() )
    , mpController( i_pController )
    , mpOp( i_pOp )
    , mpState( i_pState )
    , mpPreviewBox( nil )
    , mpPreview( nil )
    , mpAccessoryView( i_pAccessoryView )
    , mpTabView( i_pTabView )
    , mpPageEdit( nil )
    , mpStepper( nil )
#else
    , mnLastPageCount( [i_pAccessoryController printerController]->getFilteredPageCount() )
#endif
    , maLocalizedStrings( VclResId( SV_PRINT_NATIVE_STRINGS ) )
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
    , mpAccessoryController( i_pAccessoryController )
#endif
    {
        assert( maLocalizedStrings.Count() >= 5 && "resources not found" );
    }

    rtl::OUString getMoreString()
    {
        return maLocalizedStrings.Count() >= 4
               ? OUString( maLocalizedStrings.GetString( 3 ) )
               : OUString( "More" );
    }

    rtl::OUString getPrintSelectionString()
    {
        return maLocalizedStrings.Count() >= 5
               ? OUString( maLocalizedStrings.GetString( 4 ) )
               : OUString( "Print selection only" );
    }

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    void updatePrintJob()
    {
        // page range may be changed by option choice
        sal_Int32 nPages = mpController->getFilteredPageCount();

        mpState->bNeedRestart = false;
        if( nPages != mnLastPageCount )
        {
            #if OSL_DEBUG_LEVEL > 1
            SAL_INFO( "vcl.osx.print", "number of pages changed" <<
                      " from " << mnLastPageCount << " to " << nPages );
            #endif
            mpState->bNeedRestart = true;
        }
        mnLastPageCount = nPages;

        NSTabViewItem* pItem = [mpTabView selectedTabViewItem];
        if( pItem )
            mpState->nLastPage = [mpTabView indexOfTabViewItem: pItem];
        else
            mpState->nLastPage = 0;

        if( mpState->bNeedRestart )
        {
            // AppKit does not give a chance of changing the page count
            // and don't let cancel the dialog either
            // hack: send a cancel message to the modal window displaying views
            NSWindow* pNSWindow = [NSApp modalWindow];
            if( pNSWindow )
                [pNSWindow cancelOperation: nil];
            [[mpOp printInfo] setJobDisposition: NSPrintCancelJob];
        }
        else
        {
            sal_Int32 nPage = [mpStepper intValue];
            updatePreviewImage( nPage-1 );
        }
    }
#endif

    int addNameTag( const rtl::OUString& i_rPropertyName )
    {
        int nNewTag = mnNextTag++;
        maTagToPropertyName[ nNewTag ] = i_rPropertyName;
        return nNewTag;
    }

    int addNameAndValueTag( const rtl::OUString& i_rPropertyName, sal_Int32 i_nValue )
    {
        int nNewTag = mnNextTag++;
        maTagToPropertyName[ nNewTag ] = i_rPropertyName;
        maTagToValueInt[ nNewTag ] = i_nValue;
        return nNewTag;
    }

    void addObservedControl( NSObject* i_pView )
    {
        maViews.push_back( i_pView );
    }

    void addViewPair( NSView* i_pLeft, NSView* i_pRight )
    {
        maViewPairMap[ i_pLeft ] = i_pRight;
        maViewPairMap[ i_pRight ] = i_pLeft;
    }

    NSView* getPair( NSView* i_pLeft ) const
    {
        NSView* pRight = nil;
        std::map< NSView*, NSView* >::const_iterator it = maViewPairMap.find( i_pLeft );
        if( it != maViewPairMap.end() )
            pRight = it->second;
        return pRight;
    }

    void changePropertyWithIntValue( int i_nTag )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        std::map< int, sal_Int32 >::const_iterator value_it = maTagToValueInt.find( i_nTag );
        if( name_it != maTagToPropertyName.end() && value_it != maTagToValueInt.end() )
        {
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
            vcl::PrinterController * mpController = [mpAccessoryController printerController];
#endif
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= value_it->second;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
                updatePrintJob();
#else
                mnLastPageCount = [mpAccessoryController updatePrintOperation: mnLastPageCount];
#endif
            }
        }
    }

    void changePropertyWithIntValue( int i_nTag, sal_Int64 i_nValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
            vcl::PrinterController * mpController = [mpAccessoryController printerController];
#endif
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= i_nValue;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
                updatePrintJob();
#else
                mnLastPageCount = [mpAccessoryController updatePrintOperation: mnLastPageCount];
#endif
            }
        }
    }

    void changePropertyWithBoolValue( int i_nTag, bool i_bValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
            vcl::PrinterController * mpController = [mpAccessoryController printerController];
#endif
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                // ugly
                if( name_it->second.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PrintContent")) )
                   pVal->Value <<= i_bValue ? sal_Int32(2) : sal_Int32(0);
               else
                   pVal->Value <<= i_bValue;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
                updatePrintJob();
#else
                mnLastPageCount = [mpAccessoryController updatePrintOperation: mnLastPageCount];
#endif
            }
        }
    }

    void changePropertyWithStringValue( int i_nTag, const rtl::OUString& i_rValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
            vcl::PrinterController * mpController = [mpAccessoryController printerController];
#endif
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= i_rValue;
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
                updatePrintJob();
#else
                mnLastPageCount = [mpAccessoryController updatePrintOperation: mnLastPageCount];
#endif
            }
        }
    }

    void updateEnableState()
    {
        for( std::vector< NSObject* >::iterator it = maViews.begin(); it != maViews.end(); ++it )
        {
            NSObject* pObj = *it;
            NSControl* pCtrl = nil;
            NSCell* pCell = nil;
            if( [pObj isKindOfClass: [NSControl class]] )
                pCtrl = (NSControl*)pObj;
            else if( [pObj isKindOfClass: [NSCell class]] )
                pCell = (NSCell*)pObj;

            int nTag = pCtrl ? [pCtrl tag] :
                       pCell ? [pCell tag] :
                       -1;

            std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( nTag );
            if( name_it != maTagToPropertyName.end() && ! name_it->second.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PrintContent")) )
            {
#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
                vcl::PrinterController * mpController = [mpAccessoryController printerController];
#endif
                BOOL bEnabled = mpController->isUIOptionEnabled( name_it->second ) ? YES : NO;
                if( pCtrl )
                {
                    [pCtrl setEnabled: bEnabled];
                    NSView* pOther = getPair( pCtrl );
                    if( pOther && [pOther isKindOfClass: [NSControl class]] )
                        [(NSControl*)pOther setEnabled: bEnabled];
                }
                else if( pCell )
                    [pCell setEnabled: bEnabled];
            }
        }
    }

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG

    void updatePreviewImage( sal_Int32 i_nPage )
    {
        sal_Int32 nPages = mpController->getFilteredPageCount();
        NSRect aViewFrame = [mpPreview frame];
        Size aPixelSize( static_cast<long>(aViewFrame.size.width),
                         static_cast<long>(aViewFrame.size.height) );
        if( i_nPage >= 0 && nPages > i_nPage )
        {
            GDIMetaFile aMtf;
            PrinterController::PageSize aPageSize( mpController->getFilteredPageFile( i_nPage, aMtf ) );
            auto aDev(VclPtr<VirtualDevice>::Create());
            if( mpController->getPrinter()->GetPrinterOptions().IsConvertToGreyscales() )
                aDev->SetDrawMode( aDev->GetDrawMode() | ( DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText | 
                                                         DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient ) );
            // see salprn.cxx, currently we pretend to be a 720dpi device on printers
            aDev->SetReferenceDevice( 720, 720 );
            aDev->EnableOutput();
            Size aLogicSize( aDev->PixelToLogic( aPixelSize, MapMode( MAP_100TH_MM ) ) );
            double fScaleX = double(aLogicSize.Width())/double(aPageSize.aSize.Width());
            double fScaleY = double(aLogicSize.Height())/double(aPageSize.aSize.Height());
            double fScale = (fScaleX < fScaleY) ? fScaleX : fScaleY;
            // #i104784# if we render the page too small then rounding issues result in
            // layout artifacts looking really bad. So scale the page unto a device that is not
            // full page size but not too small either. This also results in much better visual
            // quality of the preview, e.g. when its height approaches the number of text lines
            if( fScale < 0.1 )
                fScale = 0.1;
            aMtf.WindStart();
            aMtf.Scale( fScale, fScale );
            aMtf.WindStart();
            aLogicSize.Width() = long(double(aPageSize.aSize.Width()) * fScale);
            aLogicSize.Height() = long(double(aPageSize.aSize.Height()) * fScale);
            aPixelSize = aDev->LogicToPixel( aLogicSize, MapMode( MAP_100TH_MM ) );
            aDev->SetOutputSizePixel( aPixelSize );
            aMtf.WindStart();
            aDev->SetMapMode( MapMode( MAP_100TH_MM ) );
            aMtf.Play( aDev.get(), Point( 0, 0 ), aLogicSize );
            aDev->EnableMapMode( FALSE );
            Image aImage( aDev->GetBitmap( Point( 0, 0 ), aPixelSize ) );
            NSImage* pImage = CreateNSImage( aImage );
            [mpPreview setImage: [pImage autorelease]];
        }
        else
            [mpPreview setImage: nil];
    }

    void setupPreview( ControlTarget* i_pCtrlTarget )
    {
        if( maLocalizedStrings.Count() < 3 )
            return;

        // get the preview control
        NSRect aPreviewFrame = [mpAccessoryView frame];
        aPreviewFrame.origin.x = 0;
        aPreviewFrame.origin.y = 5;
        aPreviewFrame.size.width = 190;
        aPreviewFrame.size.height -= 7;

        // create a box to put the preview controls in
        mpPreviewBox = [[NSBox alloc] initWithFrame: aPreviewFrame];
        [mpPreviewBox setTitle: [CreateNSString( maLocalizedStrings.GetString( 0 ) ) autorelease]];
        [mpAccessoryView addSubview: [mpPreviewBox autorelease]];

        // now create the image view of the preview
        NSSize aMargins = [mpPreviewBox contentViewMargins];
        aPreviewFrame.origin.x = 0;
        aPreviewFrame.origin.y = 34;
        aPreviewFrame.size.width -= 2*(aMargins.width+1);
        aPreviewFrame.size.height -= 61;
        mpPreview = [[NSImageView alloc] initWithFrame: aPreviewFrame];
        [mpPreview setImageScaling: NSImageScaleProportionallyDown];
        [mpPreview setImageAlignment: NSImageAlignCenter];
        [mpPreview setImageFrameStyle: NSImageFrameNone];
        [mpPreviewBox addSubview: [mpPreview autorelease]];

        // add a label
        sal_Int32 nPages = mpController->getFilteredPageCount();
        rtl::OUStringBuffer aBuf( 16 );
        aBuf.append( "/ " );
        aBuf.append( rtl::OUString::number( nPages ) );

        NSString* pText = CreateNSString( aBuf.makeStringAndClear() );
        NSRect aTextRect = { { 100, 5 }, { 100, 22 } };
        NSTextView* aPagesLabel = [[NSTextView alloc] initWithFrame: aTextRect];
        [aPagesLabel setFont: [NSFont controlContentFontOfSize: 0]];
        [aPagesLabel setEditable: NO];
        [aPagesLabel setSelectable: NO];
        [aPagesLabel setDrawsBackground: NO];
        [aPagesLabel setString: [pText autorelease]];
        [aPagesLabel setToolTip: [CreateNSString( maLocalizedStrings.GetString( 2 ) ) autorelease]];
        [mpPreviewBox addSubview: [aPagesLabel autorelease]];

        NSRect aFieldRect = { { 45, 5 }, { 35, 25 } };
        mpPageEdit = [[NSTextField alloc] initWithFrame: aFieldRect];
        [mpPageEdit setEditable: YES];
        [mpPageEdit setSelectable: YES];
        [mpPageEdit setDrawsBackground: YES];
        [mpPageEdit setToolTip: [CreateNSString( maLocalizedStrings.GetString( 1 ) ) autorelease]];
        [mpPreviewBox addSubview: [mpPageEdit autorelease]];

        // add a stepper control
        NSRect aStepFrame = { { 85, 5 }, { 15, 25 } };
        mpStepper = [[NSStepper alloc] initWithFrame: aStepFrame];
        [mpStepper setIncrement: 1];
        [mpStepper setValueWraps: NO];
        [mpPreviewBox addSubview: [mpStepper autorelease]];

        // constrain the text field to decimal numbers
        NSNumberFormatter* pFormatter = [[NSNumberFormatter alloc] init];
        [pFormatter setFormatterBehavior: NSNumberFormatterBehavior10_4];
        [pFormatter setMinimum: [[NSNumber numberWithInt: 1] autorelease]];
        [pFormatter setMaximum: [[NSNumber numberWithInt: nPages] autorelease]];
        [pFormatter setNumberStyle: NSNumberFormatterDecimalStyle];
        [pFormatter setAllowsFloats: NO];
        [pFormatter setMaximumFractionDigits: 0];
        [mpPageEdit setFormatter: pFormatter];
        [mpStepper setMinValue: 1];
        [mpStepper setMaxValue: nPages];

        [mpPageEdit setIntValue: 1];
        [mpStepper setIntValue: 1];

        // connect target and action
        [mpStepper setTarget: i_pCtrlTarget];
        [mpStepper setAction: @selector(triggeredPreview:)];
        [mpPageEdit setTarget: i_pCtrlTarget];
        [mpPageEdit setAction: @selector(triggeredPreview:)];

        // set first preview image
        updatePreviewImage( 0 );
    }

    void changePreview( NSObject* i_pSender )
    {
        if( [i_pSender isMemberOfClass: [NSTextField class]] )
        {
            NSTextField* pField = (NSTextField*)i_pSender;
            if( pField == mpPageEdit ) // sanity check
            {
                sal_Int32 nPage = [pField intValue];
                [mpStepper setIntValue: nPage];
                updatePreviewImage( nPage-1 );
            }
        }
        else if( [i_pSender isMemberOfClass: [NSStepper class]] )
        {
            NSStepper* pStepper = (NSStepper*)i_pSender;
            if( pStepper == mpStepper ) // sanity check
            {
                sal_Int32 nPage = [pStepper intValue];
                [mpPageEdit setIntValue: nPage];
                updatePreviewImage( nPage-1 );
            }
        }
    }

#endif

};

static OUString filterAccelerator( rtl::OUString const & rText )
{
    rtl::OUStringBuffer aBuf( rText.getLength() );
    for( sal_Int32 nIndex = 0; nIndex != -1; )
        aBuf.append( rText.getToken( 0, '~', nIndex ) );
    return aBuf.makeStringAndClear();
}

@implementation ControlTarget

-(id)initWithControllerMap: (ControllerProperties*)pController
{
    if( (self = [super init]) )
    {
        mpController = pController;
    }
    return self;
}

-(void)triggered:(id)pSender
{
    if( [pSender isMemberOfClass: [NSPopUpButton class]] )
    {
        NSPopUpButton* pBtn = (NSPopUpButton*)pSender;
        NSMenuItem* pSelected = [pBtn selectedItem];
        if( pSelected )
        {
            int nTag = [pSelected tag];
            mpController->changePropertyWithIntValue( nTag );
        }
    }
    else if( [pSender isMemberOfClass: [NSButton class]] )
    {
        NSButton* pBtn = (NSButton*)pSender;
        int nTag = [pBtn tag];
        mpController->changePropertyWithBoolValue( nTag, [pBtn state] == NSOnState );
    }
    else if( [pSender isMemberOfClass: [NSMatrix class]] )
    {
        NSObject* pObj = [(NSMatrix*)pSender selectedCell];
        if( [pObj isMemberOfClass: [NSButtonCell class]] )
        {
            NSButtonCell* pCell = (NSButtonCell*)pObj;
            int nTag = [pCell tag];
            mpController->changePropertyWithIntValue( nTag );
        }
    }
    else if( [pSender isMemberOfClass: [NSTextField class]] )
    {
        NSTextField* pField = (NSTextField*)pSender;
        int nTag = [pField tag];
        rtl::OUString aValue = GetOUString( [pSender stringValue] );
        mpController->changePropertyWithStringValue( nTag, aValue );
    }
    else
    {
        SAL_INFO( "vcl.osx.print", "Unsupported class" <<
                  ( [pSender class] ? [NSStringFromClass([pSender class]) UTF8String] : "nil" ) );
    }
    mpController->updateEnableState();
}

-(void)triggeredNumeric:(id)pSender
{
    if( [pSender isMemberOfClass: [NSTextField class]] )
    {
        NSTextField* pField = (NSTextField*)pSender;
        int nTag = [pField tag];
        sal_Int64 nValue = [pField intValue];
        
        NSView* pOther = mpController->getPair( pField );
        if( pOther )
            [(NSControl*)pOther setIntValue: nValue];

        mpController->changePropertyWithIntValue( nTag, nValue );
    }
    else if( [pSender isMemberOfClass: [NSStepper class]] )
    {
        NSStepper* pStep = (NSStepper*)pSender;
        int nTag = [pStep tag];
        sal_Int64 nValue = [pStep intValue];

        NSView* pOther = mpController->getPair( pStep );
        if( pOther )
            [(NSControl*)pOther setIntValue: nValue];

        mpController->changePropertyWithIntValue( nTag, nValue );
    }
    else
    {
        SAL_INFO( "vcl.osx.print", "Unsupported class" <<
                  ([pSender class] ? [NSStringFromClass([pSender class]) UTF8String] : "nil") );
    }
    mpController->updateEnableState();
}

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
-(void)triggeredPreview:(id)pSender
{
    mpController->changePreview( pSender );
}
#endif

-(void)dealloc
{
    delete mpController;
    [super dealloc];
}

@end

struct ColumnItem
{
    NSControl*      pControl;
    long            nOffset;
    NSControl*      pSubControl;
    
    ColumnItem( NSControl* i_pControl = nil, long i_nOffset = 0, NSControl* i_pSub = nil )
    : pControl( i_pControl )
    , nOffset( i_nOffset )
    , pSubControl( i_pSub )
    {}
    
    long getWidth() const
    {
        long nWidth = 0;
        if( pControl )
        {
            NSRect aCtrlRect = [pControl frame];
            nWidth = aCtrlRect.size.width;
            nWidth += nOffset;
            if( pSubControl )
            {
                NSRect aSubRect = [pSubControl frame];
                nWidth += aSubRect.size.width;
                nWidth += aSubRect.origin.x - (aCtrlRect.origin.x + aCtrlRect.size.width);
            }
        }
        return nWidth;
    }
};

static void adjustViewAndChildren( NSView* pNSView, NSSize& rMaxSize,
                                   std::vector< ColumnItem >& rLeftColumn,
                                   std::vector< ColumnItem >& rRightColumn
                                  )
{
    // balance columns

    // first get overall column widths
    long nLeftWidth = 0;
    long nRightWidth = 0;
    for( size_t i = 0; i < rLeftColumn.size(); i++ )
    {
        long nW = rLeftColumn[i].getWidth();
        if( nW > nLeftWidth )
            nLeftWidth = nW;
    }
    for( size_t i = 0; i < rRightColumn.size(); i++ )
    {
        long nW = rRightColumn[i].getWidth();
        if( nW > nRightWidth )
            nRightWidth = nW;
    }

    // right align left column
    for( size_t i = 0; i < rLeftColumn.size(); i++ )
    {
        if( rLeftColumn[i].pControl )
        {
            NSRect aCtrlRect = [rLeftColumn[i].pControl frame];
            long nX = nLeftWidth - aCtrlRect.size.width;
            if( rLeftColumn[i].pSubControl )
            {
                NSRect aSubRect = [rLeftColumn[i].pSubControl frame];
                nX -= aSubRect.size.width + (aSubRect.origin.x - (aCtrlRect.origin.x + aCtrlRect.size.width));
                aSubRect.origin.x = nLeftWidth - aSubRect.size.width;
                [rLeftColumn[i].pSubControl setFrame: aSubRect];
            }
            aCtrlRect.origin.x = nX;
            [rLeftColumn[i].pControl setFrame: aCtrlRect];
        }
    }

    // left align right column
    for( size_t i = 0; i < rRightColumn.size(); i++ )
    {
        if( rRightColumn[i].pControl )
        {
            NSRect aCtrlRect = [rRightColumn[i].pControl frame];
            long nX = nLeftWidth + 3;
            if( rRightColumn[i].pSubControl )
            {
                NSRect aSubRect = [rRightColumn[i].pSubControl frame];
                aSubRect.origin.x = nX + aSubRect.origin.x - aCtrlRect.origin.x; 
                [rRightColumn[i].pSubControl setFrame: aSubRect];
            }
            aCtrlRect.origin.x = nX;
            [rRightColumn[i].pControl setFrame: aCtrlRect];
        }
    }

    NSArray* pSubViews = [pNSView subviews];
    unsigned int nViews = [pSubViews count];
    NSRect aUnion = NSZeroRect;

    // get the combined frame of all subviews
    for( unsigned int n = 0; n < nViews; n++ )
    {
        aUnion = NSUnionRect( aUnion, [[pSubViews objectAtIndex: n] frame] );
    }

    // move everything so it will fit
    for( unsigned int n = 0; n < nViews; n++ )
    {
        NSView* pCurSubView = [pSubViews objectAtIndex: n];
        NSRect aFrame = [pCurSubView frame];
        aFrame.origin.x -= aUnion.origin.x - 5;
        aFrame.origin.y -= aUnion.origin.y - 5;
        [pCurSubView setFrame: aFrame];
    }

    // resize the view itself
    aUnion.size.height += 10;
    aUnion.size.width += 20;
    [pNSView setFrameSize: aUnion.size];

    if( aUnion.size.width > rMaxSize.width )
        rMaxSize.width = aUnion.size.width;
    if( aUnion.size.height > rMaxSize.height )
        rMaxSize.height = aUnion.size.height;
}

static void adjustTabViews( NSTabView* pTabView, NSSize aTabSize )
{
    // loop over all contained tab pages
    NSArray* pTabbedViews = [pTabView tabViewItems];
    int nViews = [pTabbedViews count];
    for( int i = 0; i < nViews; i++ )
    {
        NSTabViewItem* pItem = (NSTabViewItem*)[pTabbedViews objectAtIndex: i];
        NSView* pNSView = [pItem view];
        if( pNSView )
        {
            NSRect aRect = [pNSView frame];
            double nDiff = aTabSize.height - aRect.size.height;
            aRect.size = aTabSize;
            [pNSView setFrame: aRect];
            
            NSArray* pSubViews = [pNSView subviews];
            unsigned int nSubViews = [pSubViews count];

            // move everything up
            for( unsigned int n = 0; n < nSubViews; n++ )
            {
                NSView* pCurSubView = [pSubViews objectAtIndex: n];
                NSRect aFrame = [pCurSubView frame];
                aFrame.origin.y += nDiff;
                // give separators the correct width
                // separators are currently the only NSBoxes we use
                if( [pCurSubView isMemberOfClass: [NSBox class]] )
                {
                    aFrame.size.width = aTabSize.width - aFrame.origin.x - 10;
                }
                [pCurSubView setFrame: aFrame];
            }
        }
    }
}

static NSControl* createLabel( const rtl::OUString& i_rText )
{
    NSString* pText = CreateNSString( i_rText );
    NSRect aTextRect = { NSZeroPoint, {20, 15} };
    NSTextField* pTextView = [[NSTextField alloc] initWithFrame: aTextRect];
    [pTextView setFont: [NSFont controlContentFontOfSize: 0]];
    [pTextView setEditable: NO];
    [pTextView setSelectable: NO];
    [pTextView setDrawsBackground: NO];
    [pTextView setBordered: NO];
    [pTextView setStringValue: pText];
    [pTextView sizeToFit];
    [pText release];
    return pTextView;
}

static sal_Int32 findBreak( const rtl::OUString& i_rText, sal_Int32 i_nPos )
{
    sal_Int32 nRet = i_rText.getLength();
    Reference< i18n::XBreakIterator > xBI( vcl::unohelper::CreateBreakIterator() );
    if( xBI.is() )
    {
        i18n::Boundary aBoundary =
                xBI->getWordBoundary( i_rText, i_nPos,
                                      Application::GetSettings().GetLanguageTag().getLocale(),
                                      i18n::WordType::ANYWORD_IGNOREWHITESPACES,
                                      true );
        nRet = aBoundary.endPos;
    }
    return nRet;
}

static void linebreakCell( NSCell* pBtn, const rtl::OUString& i_rText )
{
    NSString* pText = CreateNSString( i_rText );
    [pBtn setTitle: pText];
    [pText release];
    NSSize aSize = [pBtn cellSize];
    if( aSize.width > 280 )
    {
        // need two lines
        sal_Int32 nLen = i_rText.getLength();
        sal_Int32 nIndex = nLen / 2;
        nIndex = findBreak( i_rText, nIndex );
        if( nIndex < nLen )
        {
            rtl::OUStringBuffer aBuf( i_rText );
            aBuf[nIndex] = '\n';
            pText = CreateNSString( aBuf.makeStringAndClear() );
            [pBtn setTitle: pText];
            [pText release];
        }
    }
}

static void addSubgroup( NSView* pCurParent, long& rCurY, const rtl::OUString& rText )
{
    NSControl* pTextView = createLabel( rText );
    [pCurParent addSubview: [pTextView autorelease]];                
    NSRect aTextRect = [pTextView frame];
    // move to nCurY
    aTextRect.origin.y = rCurY - aTextRect.size.height;
    [pTextView setFrame: aTextRect];
    
    NSRect aSepRect = { { aTextRect.size.width + 1, aTextRect.origin.y }, { 100, 6 } };
    NSBox* pBox = [[NSBox alloc] initWithFrame: aSepRect];
    [pBox setBoxType: NSBoxSeparator];
    [pCurParent addSubview: [pBox autorelease]];
    
    // update nCurY
    rCurY = aTextRect.origin.y - 5;
}

static void addBool( NSView* pCurParent, long& rCurX, long& rCurY, long nAttachOffset,
                    const rtl::OUString& rText, bool bEnabled,
                    const rtl::OUString& rProperty, bool bValue,
                    std::vector<ColumnItem >& rRightColumn,
                    ControllerProperties* pControllerProperties,
                    ControlTarget* pCtrlTarget
                    )
{
    NSRect aCheckRect = { { static_cast<CGFloat>(rCurX + nAttachOffset), 0 }, { 0, 15 } };
    NSButton* pBtn = [[NSButton alloc] initWithFrame: aCheckRect];
    [pBtn setButtonType: NSSwitchButton];                
    [pBtn setState: bValue ? NSOnState : NSOffState];
    if( ! bEnabled )
        [pBtn setEnabled: NO];
    linebreakCell( [pBtn cell], rText );
    [pBtn sizeToFit];
    
    rRightColumn.push_back( ColumnItem( pBtn ) );
    
    // connect target
    [pBtn setTarget: pCtrlTarget];
    [pBtn setAction: @selector(triggered:)];
    int nTag = pControllerProperties->addNameTag( rProperty );
    pControllerProperties->addObservedControl( pBtn );
    [pBtn setTag: nTag];
    
    aCheckRect = [pBtn frame];
    // #i115837# add a murphy factor; it can apparently occasionally happen
    // that sizeToFit does not a perfect job and that the button linebreaks again
    // if - and only if - there is already a '\n' contained in the text and the width
    // is minimally of
    aCheckRect.size.width += 1;
    
    // move to rCurY
    aCheckRect.origin.y = rCurY - aCheckRect.size.height;
    [pBtn setFrame: aCheckRect];

    [pCurParent addSubview: [pBtn autorelease]];
    
    // update rCurY
    rCurY = aCheckRect.origin.y - 5;
}

static void addRadio( NSView* pCurParent, long& rCurX, long& rCurY, long nAttachOffset,
                     const rtl::OUString& rText,
                     const rtl::OUString& rProperty, Sequence<rtl::OUString> const & rChoices, sal_Int32 nSelectValue,
                     std::vector<ColumnItem >& rLeftColumn,
                     std::vector<ColumnItem >& rRightColumn,
                     ControllerProperties* pControllerProperties,
                     ControlTarget* pCtrlTarget
                     )
{
    sal_Int32 nOff = 0;
    if( rText.getLength() )
    {
        // add a label
        NSControl* pTextView = createLabel( rText );
        NSRect aTextRect = [pTextView frame];
        aTextRect.origin.x = rCurX + nAttachOffset;
        [pCurParent addSubview: [pTextView autorelease]];
        
        rLeftColumn.push_back( ColumnItem( pTextView ) );
        
        // move to nCurY
        aTextRect.origin.y = rCurY - aTextRect.size.height;
        [pTextView setFrame: aTextRect];
        
        // update nCurY
        rCurY = aTextRect.origin.y - 5;
        
        // indent the radio group relative to the text
        // nOff = 20;
    }
    
    // setup radio matrix
    NSButtonCell* pProto = [[NSButtonCell alloc] init];
    
    NSRect aRadioRect = { { static_cast<CGFloat>(rCurX + nOff), 0 },
                          { static_cast<CGFloat>(280 - rCurX),
                            static_cast<CGFloat>(5*rChoices.getLength()) } };
    [pProto setTitle: @"RadioButtonGroup"];
    [pProto setButtonType: NSRadioButton];
    NSMatrix* pMatrix = [[NSMatrix alloc] initWithFrame: aRadioRect
                                          mode: NSRadioModeMatrix
                                          prototype: (NSCell*)pProto
                                          numberOfRows: rChoices.getLength()
                                          numberOfColumns: 1];
    // set individual titles
    NSArray* pCells = [pMatrix cells];
    for( sal_Int32 m = 0; m < rChoices.getLength(); m++ )
    {
        NSCell* pCell = [pCells objectAtIndex: m];
        linebreakCell( pCell, filterAccelerator( rChoices[m] ) );
        // connect target and action
        [pCell setTarget: pCtrlTarget];
        [pCell setAction: @selector(triggered:)];
        int nTag = pControllerProperties->addNameAndValueTag( rProperty, m );
        pControllerProperties->addObservedControl( pCell );
        [pCell setTag: nTag];
        // set current selection
        if( nSelectValue == m )
            [pMatrix selectCellAtRow: m column: 0];
    }
    [pMatrix sizeToFit];
    aRadioRect = [pMatrix frame];
    
    // move it down, so it comes to the correct position
    aRadioRect.origin.y = rCurY - aRadioRect.size.height;
    [pMatrix setFrame: aRadioRect];
    [pCurParent addSubview: [pMatrix autorelease]];
    
    rRightColumn.push_back( ColumnItem( pMatrix ) );
    
    // update nCurY
    rCurY = aRadioRect.origin.y - 5;
    
    [pProto release];
}

static void addList( NSView* pCurParent, long& rCurX, long& rCurY, long /*nAttachOffset*/,
                    const rtl::OUString& rText,
                    const rtl::OUString& rProperty, Sequence<rtl::OUString> const & rChoices, sal_Int32 nSelectValue,
                    std::vector<ColumnItem >& rLeftColumn,
                    std::vector<ColumnItem >& rRightColumn,
                    ControllerProperties* pControllerProperties,
                    ControlTarget* pCtrlTarget
                    )
{
    // don't indent attached lists, looks bad in the existing cases
    NSControl* pTextView = createLabel( rText );
    [pCurParent addSubview: [pTextView autorelease]];
    rLeftColumn.push_back( ColumnItem( pTextView ) );
    NSRect aTextRect = [pTextView frame];
    aTextRect.origin.x = rCurX /* + nAttachOffset*/;

    // don't indent attached lists, looks bad in the existing cases
    NSRect aBtnRect = { { rCurX /*+ nAttachOffset*/ + aTextRect.size.width, 0 }, { 0, 15 } };
    NSPopUpButton* pBtn = [[NSPopUpButton alloc] initWithFrame: aBtnRect pullsDown: NO];

    // iterate options
    for( sal_Int32 m = 0; m < rChoices.getLength(); m++ )
    {
        NSString* pItemText = CreateNSString( rChoices[m] );
        [pBtn addItemWithTitle: pItemText];
        NSMenuItem* pItem = [pBtn itemWithTitle: pItemText];
        int nTag = pControllerProperties->addNameAndValueTag( rProperty, m );
        [pItem setTag: nTag];
        [pItemText release];
    }

    [pBtn selectItemAtIndex: nSelectValue];
    
    // add the button to observed controls for enabled state changes
    // also add a tag just for this purpose
    pControllerProperties->addObservedControl( pBtn );
    [pBtn setTag: pControllerProperties->addNameTag( rProperty )];

    [pBtn sizeToFit];
    [pCurParent addSubview: [pBtn autorelease]];
    
    rRightColumn.push_back( ColumnItem( pBtn ) );

    // connect target and action
    [pBtn setTarget: pCtrlTarget];
    [pBtn setAction: @selector(triggered:)];
    
    // move to nCurY
    aBtnRect = [pBtn frame];
    aBtnRect.origin.y = rCurY - aBtnRect.size.height;
    [pBtn setFrame: aBtnRect];
    
    // align label
    aTextRect.origin.y = aBtnRect.origin.y + (aBtnRect.size.height - aTextRect.size.height)/2;
    [pTextView setFrame: aTextRect];

    // update rCurY
    rCurY = aBtnRect.origin.y - 5;
}

static void addEdit( NSView* pCurParent, long& rCurX, long& rCurY, long nAttachOffset,
                    const rtl::OUString& rCtrlType,
                    const rtl::OUString& rText,
                    const rtl::OUString& rProperty, const PropertyValue* pValue,
                    sal_Int64 nMinValue, sal_Int64 nMaxValue,
                    std::vector<ColumnItem >& rLeftColumn,
                    std::vector<ColumnItem >& rRightColumn,
                    ControllerProperties* pControllerProperties,
                    ControlTarget* pCtrlTarget
                    )
{
    sal_Int32 nOff = 0;
    if( rText.getLength() )
    {
        // add a label
        NSControl* pTextView = createLabel( rText );
        [pCurParent addSubview: [pTextView autorelease]];
        
        rLeftColumn.push_back( ColumnItem( pTextView ) );
        
        // move to nCurY
        NSRect aTextRect = [pTextView frame];
        aTextRect.origin.x = rCurX + nAttachOffset;
        aTextRect.origin.y = rCurY - aTextRect.size.height;
        [pTextView setFrame: aTextRect];
        
        // update nCurY
        rCurY = aTextRect.origin.y - 5;
        
        // and set the offset for the real edit field
        nOff = aTextRect.size.width + 5;
    }
    
    NSRect aFieldRect = { { static_cast<CGFloat>(rCurX + nOff + nAttachOffset), 0 }, { 100, 25 } };
    NSTextField* pFieldView = [[NSTextField alloc] initWithFrame: aFieldRect];
    [pFieldView setEditable: YES];
    [pFieldView setSelectable: YES];
    [pFieldView setDrawsBackground: YES];
    [pFieldView sizeToFit]; // FIXME: this does nothing
    [pCurParent addSubview: [pFieldView autorelease]];
    
    rRightColumn.push_back( ColumnItem( pFieldView ) );
    
    // add the field to observed controls for enabled state changes
    // also add a tag just for this purpose
    pControllerProperties->addObservedControl( pFieldView );
    int nTag = pControllerProperties->addNameTag( rProperty );
    [pFieldView setTag: nTag];
    // pControllerProperties->addNamedView( pFieldView, aPropertyName );

    // move to nCurY
    aFieldRect.origin.y = rCurY - aFieldRect.size.height;
    [pFieldView setFrame: aFieldRect];

    if( rCtrlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Range" ) ) )
    {
        // add a stepper control
        NSRect aStepFrame = { { aFieldRect.origin.x + aFieldRect.size.width + 5,
                                aFieldRect.origin.y },
                            { 15, aFieldRect.size.height } };
        NSStepper* pStep = [[NSStepper alloc] initWithFrame: aStepFrame];
        [pStep setIncrement: 1];
        [pStep setValueWraps: NO];
        [pStep setTag: nTag];
        [pCurParent addSubview: [pStep autorelease]];
        
        rRightColumn.back().pSubControl = pStep;
        
        pControllerProperties->addObservedControl( pStep );
        [pStep setTarget: pCtrlTarget];
        [pStep setAction: @selector(triggered:)];
        
        // constrain the text field to decimal numbers
        NSNumberFormatter* pFormatter = [[NSNumberFormatter alloc] init];
        [pFormatter setFormatterBehavior: NSNumberFormatterBehavior10_4];
        [pFormatter setNumberStyle: NSNumberFormatterDecimalStyle];
        [pFormatter setAllowsFloats: NO];
        [pFormatter setMaximumFractionDigits: 0];
        if( nMinValue != nMaxValue )
        {
            [pFormatter setMinimum: [[NSNumber numberWithInt: nMinValue] autorelease]];
            [pStep setMinValue: nMinValue];
            [pFormatter setMaximum: [[NSNumber numberWithInt: nMaxValue] autorelease]];
            [pStep setMaxValue: nMaxValue];
        }
        [pFieldView setFormatter: pFormatter];

        sal_Int64 nSelectVal = 0;
        if( pValue && pValue->Value.hasValue() )
            pValue->Value >>= nSelectVal;
        
        [pFieldView setIntValue: nSelectVal];
        [pStep setIntValue: nSelectVal];

        pControllerProperties->addViewPair( pFieldView, pStep );
        // connect target and action
        [pFieldView setTarget: pCtrlTarget];
        [pFieldView setAction: @selector(triggeredNumeric:)];
        [pStep setTarget: pCtrlTarget];
        [pStep setAction: @selector(triggeredNumeric:)];
    }
    else
    {
        // connect target and action
        [pFieldView setTarget: pCtrlTarget];
        [pFieldView setAction: @selector(triggered:)];

        if( pValue && pValue->Value.hasValue() )
        {
            rtl::OUString aValue;
            pValue->Value >>= aValue;
            if( aValue.getLength() )
            {
                NSString* pText = CreateNSString( aValue );
                [pFieldView setStringValue: pText];
                [pText release];
            }
        }
    }

    // update nCurY
    rCurY = aFieldRect.origin.y - 5;
}

@implementation AquaPrintAccessoryView

+(NSObject*)setupPrinterPanel: (NSPrintOperation*)pOp
               withController: (vcl::PrinterController*)pController
                    withState: (PrintAccessoryViewState*)pState
{
    const Sequence< PropertyValue >& rOptions( pController->getUIOptions() );
    if( rOptions.getLength() == 0 )
        return nil;

    NSRect aViewFrame = { NSZeroPoint, { 600, 400 } };
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    NSRect aTabViewFrame = { { 190, 0 }, { 410, 400 } };
#else
    NSRect aTabViewFrame = aViewFrame;
#endif

    NSView* pAccessoryView = [[NSView alloc] initWithFrame: aViewFrame];
    NSTabView* pTabView = [[NSTabView alloc] initWithFrame: aTabViewFrame];
    [pAccessoryView addSubview: [pTabView autorelease]];

#ifdef MODERN_IMPLEMENTATION_OF_PRINT_DIALOG
    // create the accessory controller
    AquaPrintPanelAccessoryController* pAccessoryController =
            [[AquaPrintPanelAccessoryController alloc] initWithNibName: nil bundle: nil];
    [pAccessoryController setView: [pAccessoryView autorelease]];
    [pAccessoryController forPrintOperation: pOp];
    [pAccessoryController withPrinterController: pController];
    [pAccessoryController withViewState: pState];
#endif

    NSView* pCurParent = 0;
    long nCurY = 0;
    long nCurX = 0;
    NSSize aMaxTabSize = NSZeroSize;

    ControllerProperties* pControllerProperties =
#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
            new ControllerProperties( pController, pOp, pState, pAccessoryView, pTabView );
#else
            new ControllerProperties( pAccessoryController );
#endif
    ControlTarget* pCtrlTarget = [[ControlTarget alloc] initWithControllerMap: pControllerProperties];

    std::vector< ColumnItem > aLeftColumn, aRightColumn;

    // ugly:
    // prepend a "selection" checkbox if the properties have such a selection in PrintContent
    bool bAddSelectionCheckBox = false, bSelectionBoxEnabled = false, bSelectionBoxChecked = false;

    for( int i = 0; i < rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

        rtl::OUString aCtrlType;
        rtl::OUString aPropertyName;
        Sequence< rtl::OUString > aChoices;
        Sequence< sal_Bool > aChoicesDisabled;
        sal_Int32 aSelectionChecked = 0;
        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ControlType")) )
            {
                rEntry.Value >>= aCtrlType;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Choices")) )
            {
                rEntry.Value >>= aChoices;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ChoicesDisabled")) )
            {
                rEntry.Value >>= aChoicesDisabled;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Property")) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
                if( aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PrintContent")) )
                    aVal.Value >>= aSelectionChecked;
            }
        }
        if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Radio")) &&
            aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PrintContent")) &&
            aChoices.getLength() > 2 )
        {
            bAddSelectionCheckBox = true;
            bSelectionBoxEnabled = aChoicesDisabled.getLength() < 2 || ! aChoicesDisabled[2];
            bSelectionBoxChecked = (aSelectionChecked==2);
            break;
        }
    }

    for( int i = 0; i < rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

        // extract ui element
        rtl::OUString aCtrlType;
        rtl::OUString aText;
        rtl::OUString aPropertyName;
        rtl::OUString aGroupHint;
        Sequence< rtl::OUString > aChoices;
        bool bEnabled = true;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        long nAttachOffset = 0;
        bool bIgnore = false;

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Text")) )
            {
                rEntry.Value >>= aText;
                aText = filterAccelerator( aText );
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ControlType")) )
            {
                rEntry.Value >>= aCtrlType;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Choices")) )
            {
                rEntry.Value >>= aChoices;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Property")) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Enabled")) )
            {
                sal_Bool bValue = sal_True;
                rEntry.Value >>= bValue;
                bEnabled = bValue ? true : false;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MinValue")) )
            {
                rEntry.Value >>= nMinValue;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MaxValue")) )
            {
                rEntry.Value >>= nMaxValue;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("AttachToDependency")) )
            {
                nAttachOffset = 20;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("InternalUIOnly")) )
            {
                sal_Bool bValue = sal_False;
                rEntry.Value >>= bValue;
                bIgnore = bValue ? true : false;
            }
            else if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("GroupingHint")) )
            {
                rEntry.Value >>= aGroupHint;
            }
        }

        if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Group")) ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Subgroup")) ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Radio")) ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("List"))  ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Edit"))  ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Range"))  ||
            aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Bool")) )
        {
            bool bIgnoreSubgroup = false;

            // with `setAccessoryView' method only one accessory view can be set
            // so create this single accessory view as tabbed for grouping
            if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Group"))
                || ! pCurParent
                || ( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Subgroup")) && nCurY < -250 && ! bIgnore ) 
               )
            {
                rtl::OUString aGroupTitle( aText );
                if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Subgroup")) )
                    aGroupTitle = pControllerProperties->getMoreString();

                // set size of current parent
                if( pCurParent )
                    adjustViewAndChildren( pCurParent, aMaxTabSize, aLeftColumn, aRightColumn );

                // new tab item
                if( ! aText.getLength() )
                    aText = "OOo";
                NSString* pLabel = CreateNSString( aGroupTitle );
                NSTabViewItem* pItem = [[NSTabViewItem alloc] initWithIdentifier: pLabel ];
                [pItem setLabel: pLabel];
                [pTabView addTabViewItem: pItem];
                pCurParent = [[NSView alloc] initWithFrame: aTabViewFrame];
                [pItem setView: pCurParent];
                [pLabel release];

                nCurX = 20; // reset indent
                nCurY = 0;  // reset Y
                // clear columns
                aLeftColumn.clear();
                aRightColumn.clear();

                if( bAddSelectionCheckBox )
                {
                    addBool( pCurParent, nCurX, nCurY, 0,
                             pControllerProperties->getPrintSelectionString(), bSelectionBoxEnabled,
                             OUString( "PrintContent" ), bSelectionBoxChecked,
                             aRightColumn, pControllerProperties, pCtrlTarget );
                    bAddSelectionCheckBox = false;
                }
            }

            if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Subgroup")) && pCurParent )
            {
                bIgnoreSubgroup = bIgnore;
                if( bIgnore )
                    continue;
                
                addSubgroup( pCurParent, nCurY, aText );
            }
            else if( bIgnoreSubgroup || bIgnore )
            {
                continue;
            }
            else if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Bool")) && pCurParent )
            {
                sal_Bool bVal = sal_False;
                PropertyValue* pVal = pController->getValue( aPropertyName );
                if( pVal )
                    pVal->Value >>= bVal;
                addBool( pCurParent, nCurX, nCurY, nAttachOffset,
                         aText, true, aPropertyName, bVal?true:false,
                         aRightColumn, pControllerProperties, pCtrlTarget );
            }
            else if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Radio")) && pCurParent )
            {
                // get currently selected value
                sal_Int32 nSelectVal = 0;
                PropertyValue* pVal = pController->getValue( aPropertyName );
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= nSelectVal;

                addRadio( pCurParent, nCurX, nCurY, nAttachOffset,
                          aText, aPropertyName, aChoices, nSelectVal,
                          aLeftColumn, aRightColumn,
                          pControllerProperties, pCtrlTarget );
            }
            else if( aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("List")) && pCurParent )
            {
                PropertyValue* pVal = pController->getValue( aPropertyName );
                sal_Int32 aSelectVal = 0;
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= aSelectVal;

                addList( pCurParent, nCurX, nCurY, nAttachOffset,
                         aText, aPropertyName, aChoices, aSelectVal,
                         aLeftColumn, aRightColumn,
                         pControllerProperties, pCtrlTarget );
            }
            else if( (aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Edit"))
                || aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Range"))) && pCurParent )
            {
                // current value
                PropertyValue* pVal = pController->getValue( aPropertyName );
                addEdit( pCurParent, nCurX, nCurY, nAttachOffset,
                         aCtrlType, aText, aPropertyName, pVal,
                         nMinValue, nMaxValue,
                         aLeftColumn, aRightColumn,
                         pControllerProperties, pCtrlTarget );
            }
        }
        else
        {
            SAL_INFO( "vcl.osx.print", "Unsupported UI option \"" << aCtrlType << "\"");
        }
    }

    pControllerProperties->updateEnableState();
    adjustViewAndChildren( pCurParent, aMaxTabSize, aLeftColumn, aRightColumn );

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    // leave some space for preview
    if( aMaxTabSize.height < 200 )
        aMaxTabSize.height = 200;
#endif

    // now reposition everything again so it is upper bound
    adjustTabViews( pTabView, aMaxTabSize );

    // find the minimum needed tab size
    NSSize aTabCtrlSize = [pTabView minimumSize];
    aTabCtrlSize.height += aMaxTabSize.height + 10;
    if( aTabCtrlSize.width < aMaxTabSize.width + 10 )
        aTabCtrlSize.width = aMaxTabSize.width + 10;
    [pTabView setFrameSize: aTabCtrlSize];
    aViewFrame.size.width = aTabCtrlSize.width + aTabViewFrame.origin.x;
    aViewFrame.size.height = aTabCtrlSize.height + aTabViewFrame.origin.y;
    [pAccessoryView setFrameSize: aViewFrame.size];

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    pControllerProperties->setupPreview( pCtrlTarget );
#endif

#ifdef VINTAGE_IMPLEMENTATION_OF_PRINT_DIALOG
    // set the accessory view
    [pOp setAccessoryView: [pAccessoryView autorelease]];
#else // -(void)setAccessoryView:(NSView *)aView of NSPrintOperation is deprecated since 10.5
    // get the print panel
    NSPrintPanel* pPrintPanel = [pOp printPanel];
    [pPrintPanel setOptions: [pPrintPanel options] | NSPrintPanelShowsPreview];
    // add the accessory controller to the panel
    [pPrintPanel addAccessoryController: [pAccessoryController autorelease]];
#endif

    // set the current selecte tab item
    if( pState->nLastPage >= 0 && pState->nLastPage < [pTabView numberOfTabViewItems] )
        [pTabView selectTabViewItemAtIndex: pState->nLastPage];

    return pCtrlTarget;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
