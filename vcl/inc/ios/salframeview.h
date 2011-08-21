/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

@interface SalFrameWindow : UIWindow
{
    IosSalFrame*       mpFrame;
    id mDraggingDestinationHandler;
}
-(id)initWithSalFrame: (IosSalFrame*)pFrame;
-(BOOL)canBecomeKeyWindow;
-(void)displayIfNeeded;
-(void)becomeKeyWindow;
-(void)resignKeyWindow;
-(IosSalFrame*)getSalFrame;
@end

@interface SalFrameView : UIView <UITextInput>
{
    IosSalFrame*       mpFrame;

    // for UITextInput
    UIEvent*        mpLastEvent;
    BOOL            mbNeedSpecialKeyHandle;
    BOOL            mbInKeyInput;
    BOOL            mbKeyHandled;
    NSRange         mMarkedRange;
    NSRange         mSelectedRange;
    id              mDraggingDestinationHandler;
    UIEvent*        mpLastSuperEvent;

    NSTimeInterval  mfLastMagnifyTime;
    float       mfMagnifyDeltaSum;
}
-(id)initWithSalFrame: (IosSalFrame*)pFrame;
-(IosSalFrame*)getSalFrame;
-(BOOL)acceptsFirstResponder;
-(BOOL)isOpaque;
-(void)drawRect: (CGRect)aRect;
-(void)magnifyWithEvent: (UIEvent*)pEvent;
-(void)rotateWithEvent: (UIEvent*)pEvent;
-(void)swipeWithEvent: (UIEvent*)pEvent;
-(void)keyDown: (UIEvent*)pEvent;
-(void)flagsChanged: (UIEvent*)pEvent;
-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar;
-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar modifiers: (unsigned int)nMod;
-(BOOL)sendKeyToFrameDirect: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar modifiers: (unsigned int)nMod;
-(BOOL)sendSingleCharacter:(UIEvent*)pEvent;
-(BOOL)handleKeyDownException:(UIEvent*)pEvent;
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
-(id)parentAttribute;
-(UIView *)viewElementForParent;
@end

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
