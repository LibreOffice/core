/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;

public class CapsTextView extends TextView {
    public CapsTextView(Context aContext) {
        super(aContext);
    }

    public CapsTextView(Context aContext, AttributeSet aAttributeSet) {
        super(aContext, aAttributeSet);
    }

    public CapsTextView(Context aContext, AttributeSet aAttributeSet, int aDefinedStyle) {
        super(aContext, aAttributeSet, aDefinedStyle);
    }

    @Override
    public void setText(CharSequence aText, BufferType aBufferType) {
        super.setText(aText.toString().toUpperCase(), aBufferType);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
