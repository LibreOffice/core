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
