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

#include "aqua/aquaprintview.h"
#include "aqua/salinst.h"
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

/* Note: the accesory view as implemented here is already deprecated in Leopard. Unfortunately
   as long as our baseline is Tiger we cannot gain the advantages over multiple accessory views
   as well havs having accessory views AND a preview (as long as you are linked vs. 10.4 libraries
   the preview insists on not being present. This is unfortunate.
*/

class ControllerProperties;

@interface ControlTarget : NSObject
{
    ControllerProperties* mpController;
}
-(id)initWithControllerMap: (ControllerProperties*)pController;
-(void)triggered:(id)pSender;
-(void)triggeredNumeric:(id)pSender;
-(void)triggeredPreview:(id)pSender;
-(void)dealloc;
@end


class ControllerProperties
{
    vcl::PrinterController*             mpController;
    std::map< int, rtl::OUString >      maTagToPropertyName;
    std::map< int, sal_Int32 >          maTagToValueInt;
    std::map< NSView*, NSView* >        maViewPairMap;
    std::vector< NSObject* >            maViews;
    int                                 mnNextTag;
    sal_Int32                           mnLastPageCount;
    PrintAccessoryViewState*            mpState;
    NSPrintOperation*                   mpOp;
    NSView*                             mpAccessoryView;
    NSTabView*                          mpTabView;
    NSBox*                              mpPreviewBox;
    NSImageView*                        mpPreview;
    NSTextField*                        mpPageEdit;
    NSStepper*                          mpStepper;
    NSTextView*                         mpPagesLabel;
    ResStringArray                      maLocalizedStrings;        

    public:
    ControllerProperties( vcl::PrinterController* i_pController,
                          NSPrintOperation* i_pOp,
                          NSView* i_pAccessoryView,
                          NSTabView* i_pTabView,
                          PrintAccessoryViewState* i_pState )
    : mpController( i_pController ),
      mnNextTag( 0 ),
      mnLastPageCount( i_pController->getFilteredPageCount() ),
      mpState( i_pState ),
      mpOp( i_pOp ),
      mpAccessoryView( i_pAccessoryView ),
      mpTabView( i_pTabView ),
      mpPreviewBox( nil ),
      mpPreview( nil ),
      mpPageEdit( nil ),
      mpStepper( nil ),
      mpPagesLabel( nil ),
      maLocalizedStrings( VclResId( SV_PRINT_NATIVE_STRINGS ) )
    {
        mpState->bNeedRestart = false;
        DBG_ASSERT( maLocalizedStrings.Count() >= 5, "resources not found !" );
    }
    
    rtl::OUString getMoreString()
    {
        return maLocalizedStrings.Count() >= 4
               ? rtl::OUString( maLocalizedStrings.GetString( 3 ) )
               : rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "More" ) );
    }
    
    rtl::OUString getPrintSelectionString()
    {
        return maLocalizedStrings.Count() >= 5
               ? rtl::OUString( maLocalizedStrings.GetString( 4 ) )
               : rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Print selection only" ) );
    }
    
    void updatePrintJob()
    {
        // TODO: refresh page count etc from mpController 

        // page range may have changed depending on options
        sal_Int32 nPages = mpController->getFilteredPageCount();
        #if OSL_DEBUG_LEVEL > 1
        if( nPages != mnLastPageCount )
            fprintf( stderr, "trouble: number of pages changed from %ld to %ld !\n", mnLastPageCount, nPages );
        #endif
        mpState->bNeedRestart = (nPages != mnLastPageCount);
        NSTabViewItem* pItem = [mpTabView selectedTabViewItem];
        if( pItem )
            mpState->nLastPage = [mpTabView indexOfTabViewItem: pItem];
        else
            mpState->nLastPage = 0;
        mnLastPageCount = nPages;
        if( mpState->bNeedRestart )
        {
            #if 0
            // Warning: bad hack ahead
            // Apple does not give us a chance of changing the page count,
            // and they don't let us cancel the dialog either
            // hack: send a cancel message to the window displaying our views.
            // this is ugly.
            for( std::vector< NSObject* >::iterator it = maViews.begin(); it != maViews.end(); ++it )
            {
                if( [*it isKindOfClass: [NSView class]] )
                {
                    NSView* pView = (NSView*)*it;
                    NSWindow* pWindow = [pView window];
                    if( pWindow )
                    {
                        [pWindow cancelOperation: nil];
                        break;
                    }
                }
            }
            #else
            NSWindow* pWindow = [NSApp modalWindow];
            if( pWindow )
                [pWindow cancelOperation: nil];
            #endif
            [[mpOp printInfo] setJobDisposition: NSPrintCancelJob];
        }
        else
        {
            sal_Int32 nPage = [mpStepper intValue];
            updatePreviewImage( nPage-1 );
        }
    }
    
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
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= value_it->second;
                updatePrintJob();
            }
        }
    }

    void changePropertyWithIntValue( int i_nTag, sal_Int64 i_nValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= i_nValue;
                updatePrintJob();
            }
        }
    }
    
    void changePropertyWithBoolValue( int i_nTag, sal_Bool i_bValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                // ugly
                if( name_it->second.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PrintContent")) )
                   pVal->Value <<= i_bValue ? sal_Int32(2) : sal_Int32(0);
               else
                   pVal->Value <<= i_bValue;
                updatePrintJob(); 
            }
        }
    }
    
    void changePropertyWithStringValue( int i_nTag, const rtl::OUString& i_rValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
            PropertyValue* pVal = mpController->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= i_rValue;
                updatePrintJob(); 
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
    
    void updatePreviewImage( sal_Int32 i_nPage )
    {
        sal_Int32 nPages = mpController->getFilteredPageCount();
        NSRect aViewFrame = [mpPreview frame];
        Size aPixelSize( static_cast<long>(aViewFrame.size.width),
                         static_cast<long>(aViewFrame.size.height) );
        if( i_nPage >= 0 && nPages > i_nPage )
        {
            GDIMetaFile aMtf;
            PrinterController::PageSize aPageSize( mpController->getFilteredPageFile( i_nPage, aMtf, false ) );
            VirtualDevice aDev;
            if( mpController->getPrinter()->GetPrinterOptions().IsConvertToGreyscales() )
                aDev.SetDrawMode( aDev.GetDrawMode() | ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT | 
                                                         DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
            // see salprn.cxx, currently we pretend to be a 720dpi device on printers
            aDev.SetReferenceDevice( 720, 720 );
            aDev.EnableOutput( TRUE );
            Size aLogicSize( aDev.PixelToLogic( aPixelSize, MapMode( MAP_100TH_MM ) ) );
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
            aPixelSize = aDev.LogicToPixel( aLogicSize, MapMode( MAP_100TH_MM ) );
            aDev.SetOutputSizePixel( aPixelSize );
            aMtf.WindStart();
            aDev.SetMapMode( MapMode( MAP_100TH_MM ) );
            aMtf.Play( &aDev, Point( 0, 0 ), aLogicSize );
            aDev.EnableMapMode( FALSE );
            Image aImage( aDev.GetBitmap( Point( 0, 0 ), aPixelSize ) );
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
        [mpPreview setImageScaling: NSScaleProportionally];
        [mpPreview setImageAlignment: NSImageAlignCenter];
        [mpPreview setImageFrameStyle: NSImageFrameNone];
        [mpPreviewBox addSubview: [mpPreview autorelease]];
    
        // add a label
        sal_Int32 nPages = mpController->getFilteredPageCount();
        rtl::OUStringBuffer aBuf( 16 );
        aBuf.appendAscii( "/ " );
        aBuf.append( rtl::OUString::number( nPages ) );
    
        NSString* pText = CreateNSString( aBuf.makeStringAndClear() );
        NSRect aTextRect = { { 100, 5 }, { 100, 22 } };
        mpPagesLabel = [[NSTextView alloc] initWithFrame: aTextRect];
        [mpPagesLabel setFont: [NSFont controlContentFontOfSize: 0]];
        [mpPagesLabel setEditable: NO];
        [mpPagesLabel setSelectable: NO];
        [mpPagesLabel setDrawsBackground: NO];
        [mpPagesLabel setString: [pText autorelease]];
        [mpPagesLabel setToolTip: [CreateNSString( maLocalizedStrings.GetString( 2 ) ) autorelease]];
        [mpPreviewBox addSubview: [mpPagesLabel autorelease]];
    
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
};

static void filterAccelerator( rtl::OUString& io_rText )
{
    rtl::OUStringBuffer aBuf( io_rText.getLength() );
    for( sal_Int32 nIndex = 0; nIndex != -1; )
        aBuf.append( io_rText.getToken( 0, '~', nIndex ) );
    io_rText = aBuf.makeStringAndClear();
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
        SAL_INFO( "vcl.aqua.print", "Unsupported class" << ([pSender class] ? [NSStringFromClass([pSender class]) UTF8String] : "nil"));
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
        SAL_INFO( "vcl.aqua.print", "Unsupported class" << ([pSender class] ? [NSStringFromClass([pSender class]) UTF8String] : "nil"));
    }
    mpController->updateEnableState();
}
-(void)triggeredPreview:(id)pSender
{
    mpController->changePreview( pSender );
}
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

static void adjustViewAndChildren( NSView* pView, NSSize& rMaxSize,
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
    
    NSArray* pSubViews = [pView subviews];
    unsigned int nViews = [pSubViews count];
    NSRect aUnion = { { 0, 0 }, { 0, 0 } };

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
    [pView setFrameSize: aUnion.size];
    
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
        NSView* pView = [pItem view];
        if( pView )
        {
            NSRect aRect = [pView frame];
            double nDiff = aTabSize.height - aRect.size.height;
            aRect.size = aTabSize;
            [pView setFrame: aRect];
            
            NSArray* pSubViews = [pView subviews];
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
    NSRect aTextRect = { { 0, 0 }, {20, 15} };
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
        i18n::Boundary aBoundary = xBI->getWordBoundary( i_rText, i_nPos,
                                                         Application::GetSettings().GetLanguageTag().getLocale(),
                                                         i18n::WordType::ANYWORD_IGNOREWHITESPACES,
                                                         sal_True );
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
                    const rtl::OUString& rText, sal_Bool bEnabled,
                    const rtl::OUString& rProperty, sal_Bool bValue,
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
                     const rtl::OUString& rProperty, Sequence< rtl::OUString > rChoices, sal_Int32 nSelectValue,
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
    
    NSRect aRadioRect = { { static_cast<CGFloat>(rCurX + nOff), 0 }, { static_cast<CGFloat>(280 - rCurX), static_cast<CGFloat>(5*rChoices.getLength()) } };
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
        filterAccelerator( rChoices[m] );
        linebreakCell( pCell, rChoices[m] );
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
                    const rtl::OUString& rProperty, const Sequence< rtl::OUString > rChoices, sal_Int32 nSelectValue,
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
                    const rtl::OUString rCtrlType,
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

// In 10.5 and later:
// 'setAccessoryView:' is deprecated

// Make deprecation warnings just warnings in a -Werror compilation.

#if HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY
// #pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif

@implementation AquaPrintAccessoryView
+(NSObject*)setupPrinterPanel: (NSPrintOperation*)pOp withController: (vcl::PrinterController*)pController  withState: (PrintAccessoryViewState*)pState
{
    const Sequence< PropertyValue >& rOptions( pController->getUIOptions() );
    if( rOptions.getLength() == 0 )
        return nil;

    NSView* pCurParent = 0;
    long nCurY = 0;
    long nCurX = 0;
    NSRect aViewFrame = { { 0, 0 }, {600, 400 } };
    NSRect aTabViewFrame = { { 190, 0 }, {410, 400 } };
    NSSize aMaxTabSize = { 0, 0 };
    NSView* pAccessoryView = [[NSView alloc] initWithFrame: aViewFrame];
    NSTabView* pTabView = [[NSTabView alloc] initWithFrame: aTabViewFrame];
    [pAccessoryView addSubview: [pTabView autorelease]];
    
    sal_Bool bIgnoreSubgroup = sal_False;
    
    ControllerProperties* pControllerProperties = new ControllerProperties( pController, pOp, pAccessoryView, pTabView, pState );
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
        bool bEnabled = true;
        rtl::OUString aCtrlType;
        rtl::OUString aText;
        rtl::OUString aPropertyName;
        rtl::OUString aGroupHint;
        Sequence< rtl::OUString > aChoices;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        long nAttachOffset = 0;
        sal_Bool bIgnore = sal_False;

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Text")) )
            {
                rEntry.Value >>= aText;
                filterAccelerator( aText );
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
                bEnabled = bValue;
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
                rEntry.Value >>= bIgnore;
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
            // since our build target is MacOSX 10.4 we can have only one accessory view
            // so we have a single accessory view that is tabbed for grouping
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
                    aText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OOo" ) );
                NSString* pLabel = CreateNSString( aGroupTitle );
                NSTabViewItem* pItem = [[NSTabViewItem alloc] initWithIdentifier: pLabel ];
                [pItem setLabel: pLabel];
                [pTabView addTabViewItem: pItem];
                pCurParent = [[NSView alloc] initWithFrame: aTabViewFrame];
                [pItem setView: pCurParent];
                [pLabel release];
                
                // reset indent
                nCurX = 20;
                // reset Y
                nCurY = 0;
                // clear columns
                aLeftColumn.clear();
                aRightColumn.clear();

                if( bAddSelectionCheckBox )
                {
                    addBool( pCurParent, nCurX, nCurY, 0,
                             pControllerProperties->getPrintSelectionString(), bSelectionBoxEnabled,
                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) ), bSelectionBoxChecked,
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
                         aText, true, aPropertyName, bVal,
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
            else if( (aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Edit")) || aCtrlType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Range"))) && pCurParent )
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
            SAL_INFO( "vcl.aqua.print", "Unsupported UI option \"" << aCtrlType << "\"");
        }
    }
        
    pControllerProperties->updateEnableState();
    adjustViewAndChildren( pCurParent, aMaxTabSize, aLeftColumn, aRightColumn );
    
    // leave some space for the preview
    if( aMaxTabSize.height < 200 )
        aMaxTabSize.height = 200;
    
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
    
    pControllerProperties->setupPreview( pCtrlTarget );

    // set the accessory view
    [pOp setAccessoryView: [pAccessoryView autorelease]];

    // set the current selecte tab item
    if( pState->nLastPage >= 0 && pState->nLastPage < [pTabView numberOfTabViewItems] )
        [pTabView selectTabViewItemAtIndex: pState->nLastPage];
        
    return pCtrlTarget;
}

// #pragma GCC diagnostic pop

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
