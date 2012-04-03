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
-(IosSalFrame*)getSalFrame;
@end

@interface SalFrameView : UIView
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
@end

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
