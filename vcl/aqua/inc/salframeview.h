/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframeview.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:13:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_SALFRAMEVIEW_H
#define _VCL_SALFRAMEVIEW_H

@interface SalFrameWindow : NSWindow
{
    AquaSalFrame*       mpFrame;
}
-(id)initWithSalFrame: (AquaSalFrame*)pFrame;
-(MacOSBOOL)canBecomeKeyWindow;
-(void)windowDidBecomeKey: (NSNotification*)pNotification;
-(void)windowDidResignKey: (NSNotification*)pNotification;
-(void)windowDidChangeScreen: (NSNotification*)pNotification;
-(void)windowDidMove: (NSNotification*)pNotification;
-(void)windowDidResize: (NSNotification*)pNotification;
-(void)windowDidMiniaturize: (NSNotification*)pNotification;
-(void)windowDidDeminiaturize: (NSNotification*)pNotification;
-(MacOSBOOL)windowShouldClose: (NSNotification*)pNotification;
-(void)dockMenuItemTriggered: (id)sender;
-(AquaSalFrame*)getSalFrame;
-(MacOSBOOL)containsMouse;
@end

@interface SalFrameView : NSView <NSTextInput>
{
    AquaSalFrame*       mpFrame;

    // for NSTextInput
    id mpLastEvent;
    BOOL mbNeedSpecialKeyHandle;
    BOOL mbInKeyInput;
    BOOL mbKeyHandled;
    NSRange mMarkedRange;
    NSRange mSelectedRange;
}
+(void)unsetMouseFrame: (AquaSalFrame*)pFrame;
-(id)initWithSalFrame: (AquaSalFrame*)pFrame;
-(MacOSBOOL)acceptsFirstResponder;
-(MacOSBOOL)acceptsFirstMouse: (NSEvent *)pEvent;
-(MacOSBOOL)isOpaque;
-(void)drawRect: (NSRect)aRect;
-(void)mouseDown: (NSEvent*)pEvent;
-(void)mouseDragged: (NSEvent*)pEvent;
-(void)mouseUp: (NSEvent*)pEvent;
-(void)mouseMoved: (NSEvent*)pEvent;
-(void)mouseEntered: (NSEvent*)pEvent;
-(void)mouseExited: (NSEvent*)pEvent;
-(void)rightMouseDown: (NSEvent*)pEvent;
-(void)rightMouseDragged: (NSEvent*)pEvent;
-(void)rightMouseUp: (NSEvent*)pEvent;
-(void)otherMouseDown: (NSEvent*)pEvent;
-(void)otherMouseDragged: (NSEvent*)pEvent;
-(void)otherMouseUp: (NSEvent*)pEvent;
-(void)scrollWheel: (NSEvent*)pEvent;
-(void)keyDown: (NSEvent*)pEvent;
-(void)flagsChanged: (NSEvent*)pEvent;
-(void)sendMouseEventToFrame:(NSEvent*)pEvent button:(USHORT)nButton eventtype:(USHORT)nEvent;
-(void)sendKeyInputAndReleaseToFrame: (USHORT)nKeyCode character: (sal_Unicode)aChar;
-(MacOSBOOL)sendSingleCharacter:(NSEvent*)pEvent;
-(MacOSBOOL)handleKeyDownException:(NSEvent*)pEvent;
/*
    text action methods
*/
-(void)insertText:(id)aString;
-(void)insertTab: (id)aSender;
-(void)insertBacktab: (id)aSender;
-(void)moveLeft: (id)aSender;
-(void)moveRight: (id)aSender;
-(void)moveUp: (id)aSender;
-(void)moveDown: (id)aSender;
-(void)insertNewline: (id)aSender;
-(void)deleteBackward: (id)aSender;
-(void)deleteForward: (id)aSender;
-(void)cancelOperation: (id)aSender;
/* set the correct pointer for our view */
-(void)resetCursorRects;
@end

#endif
