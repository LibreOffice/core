/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.actionbarsherlock.app.SherlockFragment;
import org.libreoffice.impressremote.R;

public class EmptySlideFragment extends SherlockFragment {
    public static EmptySlideFragment newInstance() {
        return new EmptySlideFragment();
    }

    @Override
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstance) {
        return aInflater.inflate(R.layout.fragment_empty_slide, aContainer, false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
