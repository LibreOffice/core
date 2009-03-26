/************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aquaprintview.mm,v $
 * $Revision: 1.5.56.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aquaprintview.h"
#include "salinst.h"
#include "vcl/print.hxx"

#include <map>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

/* Note: the accesory view as implemented here is already deprecated in Leopard. Unfortunately
   as long as our baseline is Tiger we cannot gain the advantages over multiple accessory views
   as well havs haing accessory views AND a preview (as long as you are linked vs. 10.4 libraries
   the preview insists on not being present. This is unfortunate.
*/

class ListenerProperties
{
    vcl::PrinterListener*               mpListener;
    std::map< int, rtl::OUString >      maTagToPropertyName;
    std::map< int, sal_Int32 >          maTagToValueInt;
    std::vector< NSObject* >            maViews;
    int                                 mnNextTag;
    sal_Int32                           mnLastPageCount;
    bool*                               mpNeedRestart;
    NSPrintOperation*                   mpOp;
    
    public:
    ListenerProperties( vcl::PrinterListener* i_pListener, NSPrintOperation* i_pOp, bool* i_pNeedRestart )
    : mpListener( i_pListener ),
      mnNextTag( 0 ),
      mnLastPageCount( i_pListener->getPageCount() ),
      mpNeedRestart( i_pNeedRestart ),
      mpOp( i_pOp )
    {
        *mpNeedRestart = false;
    }
    
    void updatePrintJob()
    {
        // TODO: refresh page count etc from mpListener 

        // page range may have changed depending on options
        sal_Int32 nPages = mpListener->getPageCount();
        #if OSL_DEBUG_LEVEL > 1
        if( nPages != mnLastPageCount )
            fprintf( stderr, "trouble: number of pages changed from %ld to %ld !\n", mnLastPageCount, nPages );
        #endif
        *mpNeedRestart = (nPages != mnLastPageCount);
        mnLastPageCount = nPages;
        if( *mpNeedRestart )
        {
            // Warning: bad hack ahead
            // Apple does not give as a chance of changing the page count,
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
            [[mpOp printInfo] setJobDisposition: NSPrintCancelJob];
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
    
    void changePropertyWithIntValue( int i_nTag )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        std::map< int, sal_Int32 >::const_iterator value_it = maTagToValueInt.find( i_nTag );
        if( name_it != maTagToPropertyName.end() && value_it != maTagToValueInt.end() )
        {
            PropertyValue* pVal = mpListener->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= value_it->second;
                updatePrintJob();
            }
        }
    }
    
    void changePropertyWithBoolValue( int i_nTag, sal_Bool i_bValue )
    {
        std::map< int, rtl::OUString >::const_iterator name_it = maTagToPropertyName.find( i_nTag );
        if( name_it != maTagToPropertyName.end() )
        {
            PropertyValue* pVal = mpListener->getValue( name_it->second );
            if( pVal )
            {
                pVal->Value <<= i_bValue;
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
            if( name_it != maTagToPropertyName.end() )
            {
                MacOSBOOL bEnabled = mpListener->isUIOptionEnabled( name_it->second ) ? YES : NO;
                if( pCtrl )
                    [pCtrl setEnabled: bEnabled];
                else if( pCell )
                    [pCell setEnabled: bEnabled];
            }
        }
    }
};

@interface ControlTarget : NSObject
{
    ListenerProperties* mpListener;
}
-(id)initWithListenerMap: (ListenerProperties*)pListener;
-(void)triggered:(id)pSender;
-(void)dealloc;
@end

@implementation ControlTarget
-(id)initWithListenerMap: (ListenerProperties*)pListener
{
    if( (self = [super init]) )
    {
        mpListener = pListener; 
    }
    return self;
}
-(void)triggered:(id)pSender;
{
    if( [pSender isMemberOfClass: [NSPopUpButton class]] )
    {
        NSPopUpButton* pBtn = (NSPopUpButton*)pSender;
        NSMenuItem* pSelected = [pBtn selectedItem];
        if( pSelected )
        {
            int nTag = [pSelected tag];
            mpListener->changePropertyWithIntValue( nTag );
        }
    }
    else if( [pSender isMemberOfClass: [NSButton class]] )
    {
        NSButton* pBtn = (NSButton*)pSender;
        int nTag = [pBtn tag];
        mpListener->changePropertyWithBoolValue( nTag, [pBtn state] == NSOnState );
    }
    else if( [pSender isMemberOfClass: [NSMatrix class]] )
    {
        NSObject* pObj = [(NSMatrix*)pSender selectedCell];
        if( [pObj isMemberOfClass: [NSButtonCell class]] )
        {
            NSButtonCell* pCell = (NSButtonCell*)pObj;
            int nTag = [pCell tag];
            mpListener->changePropertyWithIntValue( nTag );
        }
    }
    else
    {
        DBG_ERROR( "unsupported class" );
    }
    mpListener->updateEnableState();
}
-(void)dealloc
{
    delete mpListener;
    [super dealloc];
}
@end

static void adjustViewAndChildren( NSView* pView, NSSize& rMaxSize )
{
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
    aUnion.size.width += 10;
    [pView setFrameSize: aUnion.size];
    
    if( aUnion.size.width > rMaxSize.width )
        rMaxSize.width = aUnion.size.width;
    if( aUnion.size.height > rMaxSize.height )
        rMaxSize.height = aUnion.size.height;
}

@implementation AquaPrintAccessoryView
+(NSObject*)setupPrinterPanel: (NSPrintOperation*)pOp withListener: (vcl::PrinterListener*)pListener  withRestartCondition: (bool*)pbRestart;
{
    const Sequence< PropertyValue >& rOptions( pListener->getUIOptions() );
    if( rOptions.getLength() == 0 )
        return nil;

    ListenerProperties* pListenerProperties = new ListenerProperties( pListener, pOp, pbRestart );
    ControlTarget* pCtrlTarget = [[ControlTarget alloc] initWithListenerMap: pListenerProperties];

    NSView* pCurParent = 0;
    long nCurY = 0;
    long nCurX = 0;
    NSRect aViewFrame = { { 0, 0 }, {400, 400 } };
    NSSize aMaxTabSize = { 0, 0 };
    NSTabView* pTabView = [[NSTabView alloc] initWithFrame: aViewFrame];
    
    for( int i = 0; i < rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

        // extract ui element
        bool bEnabled = true;
        rtl::OUString aCtrlType;
        rtl::OUString aText;
        rtl::OUString aPropertyName;
        Sequence< rtl::OUString > aChoices;

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAscii( "Text" ) )
            {
                rEntry.Value >>= aText;
            }
            else if( rEntry.Name.equalsAscii( "ControlType" ) )
            {
                rEntry.Value >>= aCtrlType;
            }
            else if( rEntry.Name.equalsAscii( "Choices" ) )
            {
                rEntry.Value >>= aChoices;
            }
            else if( rEntry.Name.equalsAscii( "Property" ) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
            }
            else if( rEntry.Name.equalsAscii( "Enabled" ) )
            {
                sal_Bool bValue = sal_True;
                rEntry.Value >>= bValue;
                bEnabled = bValue;
            }
        }

        if( aCtrlType.equalsAscii( "Group" ) ||
            aCtrlType.equalsAscii( "Subgroup" ) ||
            aCtrlType.equalsAscii( "Radio" ) ||
            aCtrlType.equalsAscii( "List" )  ||
            aCtrlType.equalsAscii( "Bool" ) )
        {
            // since our build target is MacOSX 10.4 we can have only one accessory view
            // so we have a single accessory view that is tabbed for grouping
            if( aCtrlType.equalsAscii( "Group" ) || ! pCurParent )
            {
                // set size of current parent
                if( pCurParent )
                    adjustViewAndChildren( pCurParent, aMaxTabSize );
                
                // new tab item
                if( ! aText.getLength() )
                    aText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OOo" ) );
                NSString* pLabel = CreateNSString( aText );
                NSTabViewItem* pItem = [[NSTabViewItem alloc] initWithIdentifier: pLabel ];
                [pItem setLabel: pLabel];
                [pTabView addTabViewItem: pItem];
                pCurParent = [[NSView alloc] initWithFrame: aViewFrame];
                [pItem setView: pCurParent];
                [pLabel release];
                
                // reset indent
                nCurX = 0;
            }
            
            if( aCtrlType.equalsAscii( "Subgroup" ) && pCurParent )
            {
                NSString* pText = CreateNSString( aText );
                NSRect aTextRect = { { 0, 0 }, { 300, 15 } };
                NSTextView* pTextView = [[NSTextView alloc] initWithFrame: aTextRect];
                [pTextView setEditable: NO];
                [pTextView setSelectable: NO];
                [pTextView setDrawsBackground: NO];
                [pTextView setString: pText];
                [pTextView sizeToFit]; // FIXME: this does nothing
                [pCurParent addSubview: pTextView];

                aTextRect = [pTextView frame];
                // move to nCurY
                aTextRect.origin.y = nCurY - aTextRect.size.height;
                [pTextView setFrame: aTextRect];
                
                // update nCurY
                nCurY = aTextRect.origin.y - 5;
                
                // set indent
                nCurX = 20;

                // cleanup
                [pText release];
            }
            else if( aCtrlType.equalsAscii( "Bool" ) && pCurParent )
            {
                NSString* pText = CreateNSString( aText );
                NSRect aCheckRect = { { nCurX, 0 }, { 0, 15 } };
                NSButton* pBtn = [[NSButton alloc] initWithFrame: aCheckRect];
                [pBtn setButtonType: NSSwitchButton];
                [pBtn setTitle: pText];
                sal_Bool bVal = sal_False;                
                PropertyValue* pVal = pListener->getValue( aPropertyName );
                if( pVal )
                    pVal->Value >>= bVal;
                [pBtn setState: bVal ? NSOnState : NSOffState];
                [pBtn setEnabled: (pListener->isUIOptionEnabled( aPropertyName ) && pVal != NULL) ? YES : NO];
                [pBtn sizeToFit];
                [pCurParent addSubview: pBtn];
                
                // connect target
                [pBtn setTarget: pCtrlTarget];
                [pBtn setAction: @selector(triggered:)];
                int nTag = pListenerProperties->addNameTag( aPropertyName );
                pListenerProperties->addObservedControl( pBtn );
                [pBtn setTag: nTag];

                aCheckRect = [pBtn frame];

                // move to nCurY
                aCheckRect.origin.y = nCurY - aCheckRect.size.height;
                [pBtn setFrame: aCheckRect];

                // update nCurY
                nCurY = aCheckRect.origin.y - 5;
                
                // cleanup
                [pText release];
            }
            else if( aCtrlType.equalsAscii( "Radio" ) && pCurParent )
            {
                if( aText.getLength() )
                {
                    // add a label
                    NSString* pText = CreateNSString( aText );
                    NSRect aTextRect = { { nCurX, 0 }, { 300, 15 } };
                    NSTextView* pTextView = [[NSTextView alloc] initWithFrame: aTextRect];
                    [pTextView setEditable: NO];
                    [pTextView setSelectable: NO];
                    [pTextView setDrawsBackground: NO];
                    [pTextView setString: pText];
                    [pTextView sizeToFit]; // FIXME: this does nothing
                    [pCurParent addSubview: pTextView];
    
                    // move to nCurY
                    aTextRect.origin.y = nCurY - aTextRect.size.height;
                    [pTextView setFrame: aTextRect];
                    
                    // update nCurY
                    nCurY = aTextRect.origin.y - 5;
                    
                    // cleanup
                    [pText release];
                }
                
                // setup radio matrix
                NSButtonCell* pProto = [[NSButtonCell alloc] init];
                
                NSRect aRadioRect = { { nCurX + 20, 0 }, { 280 - nCurX, 5*aChoices.getLength() } };
                [pProto setTitle: @"RadioButtonGroup"];
                [pProto setButtonType: NSRadioButton];
                NSMatrix* pMatrix = [[NSMatrix alloc] initWithFrame: aRadioRect
                                                      mode: NSRadioModeMatrix
                                                      prototype: (NSCell*)pProto
                                                      numberOfRows: aChoices.getLength()
                                                      numberOfColumns: 1];
                // get currently selected value
                rtl::OUString aSelectVal;
                PropertyValue* pVal = pListener->getValue( aPropertyName );
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= aSelectVal;
                // set individual titles
                NSArray* pCells = [pMatrix cells];
                for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                {
                    NSCell* pCell = [pCells objectAtIndex: m];
                    NSString* pTitle = CreateNSString( aChoices[m] );
                    [pCell setTitle: pTitle];
                    // connect target and action
                    [pCell setTarget: pCtrlTarget];
                    [pCell setAction: @selector(triggered:)];
                    int nTag = pListenerProperties->addNameAndValueTag( aPropertyName, m );
                    pListenerProperties->addObservedControl( pCell );
                    [pCell setTag: nTag];
                    [pTitle release];
                    // set current selection
                    if( aSelectVal == aChoices[m] )
                        [pMatrix selectCellAtRow: m column: 0];
                }
                [pMatrix sizeToFit];
                aRadioRect = [pMatrix frame];

                // move it down, so it comes to the correct position
                aRadioRect.origin.y = nCurY - aRadioRect.size.height;
                [pMatrix setFrame: aRadioRect];
                [pCurParent addSubview: pMatrix];

                // update nCurY
                nCurY = aRadioRect.origin.y - 5;
                
                
                [pProto release];
            }
            else if( aCtrlType.equalsAscii( "List" ) && pCurParent )
            {
                NSString* pText = CreateNSString( aText );
                
                // measure the text
                NSFont* pFont = [NSFont labelFontOfSize: 0];
                NSDictionary* pDict = [NSDictionary dictionaryWithObject: pFont
                                                    forKey: NSFontAttributeName];
                             
                NSSize aTextSize = [pText sizeWithAttributes: pDict];
                // FIXME: the only thing reliable about sizeWithAttributes is
                // that the size it outputs is way too small for our NSTextView
                // that would not matter so much if NSTextView's fitToSize actually
                // did something out of the box, alas it doesn't. This probably needs more
                // fiddling with NSTextView's and NSTextContainer's parameters, however
                // since this already almost cost me my sanity a Murphy factor of 1.5
                // will have to suffice for the time being.
                aTextSize.width *= 1.5;
                aTextSize.height += 3;
                NSRect aTextRect = { { nCurX, 0 }, aTextSize };
                NSTextView* pTextView = [[NSTextView alloc] initWithFrame: aTextRect];
                [pTextView setEditable: NO];
                [pTextView setSelectable: NO];
                [pTextView setDrawsBackground: NO];
                [pTextView setString: pText];
                [pTextView setVerticallyResizable: NO];
                [pTextView setHorizontallyResizable: YES];
                [pTextView sizeToFit]; // FIXME: this actually does nothing
                [pCurParent addSubview: pTextView];
                aTextRect = [pTextView frame];


                NSRect aBtnRect = { { nCurX + aTextRect.size.width, 0 }, { 0, 15 } };
                NSPopUpButton* pBtn = [[NSPopUpButton alloc] initWithFrame: aBtnRect pullsDown: NO];

                // iterate options
                for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                {
                    NSString* pItemText = CreateNSString( aChoices[m] );
                    [pBtn addItemWithTitle: pItemText];
                    NSMenuItem* pItem = [pBtn itemWithTitle: pItemText];
                    int nTag = pListenerProperties->addNameAndValueTag( aPropertyName, m );
                    [pItem setTag: nTag];
                    [pItemText release];
                }

                PropertyValue* pVal = pListener->getValue( aPropertyName );
                sal_Int32 aSelectVal = 0;
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= aSelectVal;
                [pBtn selectItemAtIndex: aSelectVal];
                [pBtn setEnabled: (pListener->isUIOptionEnabled( aPropertyName ) && pVal != NULL) ? YES : NO];
                
                // add the button to observed controls for enabled state changes
                // also add a tag just for this purpose
                pListenerProperties->addObservedControl( pBtn );
                [pBtn setTag: pListenerProperties->addNameTag( aPropertyName )];

                [pBtn sizeToFit];
                [pCurParent addSubview: pBtn];

                // connect target and action
                [pBtn setTarget: pCtrlTarget];
                [pBtn setAction: @selector(triggered:)];
                
                // move to nCurY
                aBtnRect = [pBtn frame];
                aBtnRect.origin.y = nCurY - aBtnRect.size.height;
                [pBtn setFrame: aBtnRect];
                
                // align label
                aTextRect.origin.y = aBtnRect.origin.y + (aBtnRect.size.height - aTextRect.size.height)/2;
                [pTextView setFrame: aTextRect];

                // update nCurY
                nCurY = aBtnRect.origin.y - 5;
                
                // cleanup
                [pText release];
            }
        }
        else
        {
            DBG_ERROR( "Unsupported UI option" );
        }
    }
    adjustViewAndChildren( pCurParent, aMaxTabSize );
        
    // find the minimum needed tab size
    NSSize aTabCtrlSize = [pTabView minimumSize];
    aTabCtrlSize.height += aMaxTabSize.height + 10;
    if( aTabCtrlSize.width < aMaxTabSize.width + 10 )
        aTabCtrlSize.width = aMaxTabSize.width + 10;
    [pTabView setFrameSize: aTabCtrlSize];

    // set the accessory view
    [pOp setAccessoryView: pTabView];
        
    return pCtrlTarget;
}

@end
