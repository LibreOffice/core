/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VCL_SALFRAMEVIEW_H
#define _VCL_SALFRAMEVIEW_H

#include "aqua/aqua11ywrapper.h"

@interface SalFrameWindow : NSWindow
{
    AquaSalFrame*       mpFrame;
    id mDraggingDestinationHandler;
}
-(id)initWithSalFrame: (AquaSalFrame*)pFrame;
-(BOOL)canBecomeKeyWindow;
-(void)displayIfNeeded;
-(void)windowDidBecomeKey: (NSNotification*)pNotification;
-(void)windowDidResignKey: (NSNotification*)pNotification;
-(void)windowDidChangeScreen: (NSNotification*)pNotification;
-(void)windowDidMove: (NSNotification*)pNotification;
-(void)windowDidResize: (NSNotification*)pNotification;
-(void)windowDidMiniaturize: (NSNotification*)pNotification;
-(void)windowDidDeminiaturize: (NSNotification*)pNotification;
-(BOOL)windowShouldClose: (NSNotification*)pNotification;
-(void)dockMenuItemTriggered: (id)sender;
-(AquaSalFrame*)getSalFrame;
-(BOOL)containsMouse;
-(::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;

/* NSDraggingDestination protocol methods
 */
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
-(void)draggingExited:(id <NSDraggingInfo>)sender;
-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
-(void)concludeDragOperation:(id <NSDraggingInfo>)sender;

-(void)registerDraggingDestinationHandler:(id)theHandler;
-(void)unregisterDraggingDestinationHandler:(id)theHandler;
@end

@interface SalFrameView : AquaA11yWrapper <NSTextInput>
{
    AquaSalFrame*       mpFrame;

    // for NSTextInput
    NSEvent*        mpLastEvent;
    BOOL            mbNeedSpecialKeyHandle;
    BOOL            mbInKeyInput;
    BOOL            mbKeyHandled;
    NSRange         mMarkedRange;
    NSRange         mSelectedRange;
    id              mpMouseEventListener;
    id              mDraggingDestinationHandler;
    NSEvent*        mpLastSuperEvent;

    // #i102807# used by magnify event handler
    NSTimeInterval  mfLastMagnifyTime;
    float           mfMagnifyDeltaSum;
}
+(void)unsetMouseFrame: (AquaSalFrame*)pFrame;
-(id)initWithSalFrame: (AquaSalFrame*)pFrame;
-(BOOL)acceptsFirstResponder;
-(BOOL)acceptsFirstMouse: (NSEvent *)pEvent;
-(BOOL)isOpaque;
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
-(void)magnifyWithEvent: (NSEvent*)pEvent;
-(void)rotateWithEvent: (NSEvent*)pEvent;
-(void)swipeWithEvent: (NSEvent*)pEvent;
-(void)keyDown: (NSEvent*)pEvent;
-(void)flagsChanged: (NSEvent*)pEvent;
-(void)sendMouseEventToFrame:(NSEvent*)pEvent button:(sal_uInt16)nButton eventtype:(sal_uInt16)nEvent;
-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar;
-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar modifiers: (unsigned int)nMod;
-(BOOL)sendKeyToFrameDirect: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar modifiers: (unsigned int)nMod;
-(BOOL)sendSingleCharacter:(NSEvent*)pEvent;
-(BOOL)handleKeyDownException:(NSEvent*)pEvent;
-(void)clearLastEvent;
/*
    text action methods
*/
-(void)insertText:(id)aString;
-(void)insertTab: (id)aSender;
-(void)insertBacktab: (id)aSender;
-(void)moveLeft: (id)aSender;
-(void)moveLeftAndModifySelection: (id)aSender;
-(void)moveBackwardAndModifySelection: (id)aSender;
-(void)moveRight: (id)aSender;
-(void)moveRightAndModifySelection: (id)aSender;
-(void)moveForwardAndModifySelection: (id)aSender;
-(void)moveUp: (id)aSender;
-(void)moveDown: (id)aSender;
-(void)moveWordBackward: (id)aSender;
-(void)moveWordBackwardAndModifySelection: (id)aSender;
-(void)moveWordLeftAndModifySelection: (id)aSender;
-(void)moveWordForward: (id)aSender;
-(void)moveWordForwardAndModifySelection: (id)aSender;
-(void)moveWordRightAndModifySelection: (id)aSender;
-(void)moveToEndOfLine: (id)aSender;
-(void)moveToRightEndOfLine: (id)aSender;
-(void)moveToLeftEndOfLine: (id)aSender;
-(void)moveToEndOfLineAndModifySelection: (id)aSender;
-(void)moveToRightEndOfLineAndModifySelection: (id)aSender;
-(void)moveToLeftEndOfLineAndModifySelection: (id)aSender;
-(void)moveToBeginningOfLine: (id)aSender;
-(void)moveToBeginningOfLineAndModifySelection: (id)aSender;
-(void)moveToEndOfParagraph: (id)aSender;
-(void)moveToEndOfParagraphAndModifySelection: (id)aSender;
-(void)moveToBeginningOfParagraph: (id)aSender;
-(void)moveToBeginningOfParagraphAndModifySelection: (id)aSender;
-(void)moveParagraphForward: (id)aSender;
-(void)moveParagraphForwardAndModifySelection: (id)aSender;
-(void)moveParagraphBackward: (id)aSender;
-(void)moveParagraphBackwardAndModifySelection: (id)aSender;
-(void)moveToEndOfDocument: (id)aSender;
-(void)scrollToEndOfDocument: (id)aSender;
-(void)moveToEndOfDocumentAndModifySelection: (id)aSender;
-(void)moveToBeginningOfDocument: (id)aSender;
-(void)scrollToBeginningOfDocument: (id)aSender;
-(void)moveToBeginningOfDocumentAndModifySelection: (id)aSender;
-(void)insertNewline: (id)aSender;
-(void)deleteBackward: (id)aSender;
-(void)deleteForward: (id)aSender;
-(void)cancelOperation: (id)aSender;
-(void)deleteBackwardByDecomposingPreviousCharacter: (id)aSender;
-(void)deleteWordBackward: (id)aSender;
-(void)deleteWordForward: (id)aSender;
-(void)deleteToBeginningOfLine: (id)aSender;
-(void)deleteToEndOfLine: (id)aSender;
-(void)deleteToBeginningOfParagraph: (id)aSender;
-(void)deleteToEndOfParagraph: (id)aSender;
-(void)insertLineBreak: (id)aSender;
-(void)insertParagraphSeparator: (id)aSender;
-(void)selectWord: (id)aSender;
-(void)selectLine: (id)aSender;
-(void)selectParagraph: (id)aSender;
-(void)selectAll: (id)aSender;
-(void)noop: (id)aSender;
/* set the correct pointer for our view */
-(void)resetCursorRects;
-(::com::sun::star::accessibility::XAccessibleContext *)accessibleContext;
-(id)parentAttribute;
-(NSView *)viewElementForParent;
/*
  Event hook for D&D service.

  A drag operation will be invoked on a NSView using
  the method 'dragImage'. This method requires the
  actual mouse event initiating this drag operation.
  Mouse events can only be received by subclassing
  NSView and overriding methods like 'mouseDown' etc.
  hence we implement a event hook here so that the
  D&D service can register as listener for mouse
  messages and use the last 'mouseDown' or
  'mouseDragged' message to initiate the drag
  operation.
*/
-(void)registerMouseEventListener: (id)theListener;
-(void)unregisterMouseEventListener: (id)theListener;

/* NSDraggingDestination protocol methods
 */
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
-(void)draggingExited:(id <NSDraggingInfo>)sender;
-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
-(void)concludeDragOperation:(id <NSDraggingInfo>)sender;

-(void)registerDraggingDestinationHandler:(id)theHandler;
-(void)unregisterDraggingDestinationHandler:(id)theHandler;

@end

#endif
