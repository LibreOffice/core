/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

interface MessagesListener {
    public void onPinValidation();

    public void onSuccessfulPairing();

    public void onSlideShowStart(int aSlidesCount, int aCurrentSlideIndex);

    public void onSlideShowFinish();

    public void onSlideChanged(int aCurrentSlideIndex);

    public void onSlidePreview(int aSlideIndex, byte[] aPreview);

    public void onSlideNotes(int aSlideIndex, String aNotes);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
