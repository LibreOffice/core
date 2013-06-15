/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Base64;

public class Receiver {
    private final Context mContext;

    private SlideShow mSlideShow;

    public Receiver(Context aContext) {
        this.mContext = aContext;
        this.mSlideShow = new SlideShow(mContext);
    }

    public SlideShow getSlideShow() {
        return mSlideShow;
    }

    public boolean isSlideShowRunning() {
        return mSlideShow.getSlidesCount() > 0;
    }

    public void parseCommand(List<String> aInstruction) {
        if (aInstruction.isEmpty()) {
            return;
        }

        String aCommand = aInstruction.get(0);

        if (aCommand.equals(Protocol.Messages.SLIDESHOW_STARTED)) {
            startSlideShow(aInstruction);
            return;
        }

        if (aCommand.equals(Protocol.Messages.SLIDESHOW_FINISHED)) {
            finishSlideShow();
            return;
        }

        if (mSlideShow == null) {
            return;
        }

        if (aCommand.equals(Protocol.Messages.SLIDE_UPDATED)) {
            setUpCurrentSlide(aInstruction);
            return;
        }

        if (aCommand.equals(Protocol.Messages.SLIDE_PREVIEW)) {
            setUpSlidePreview(aInstruction);
            return;
        }

        if (aCommand.equals(Protocol.Messages.SLIDE_NOTES)) {
            setUpSlideNotes(aInstruction);
        }
    }

    private void startSlideShow(List<String> aInstruction) {
        int aSlideShowSlidesCount = Integer.parseInt(aInstruction.get(1));
        int aCurrentSlideIndex = Integer.parseInt(aInstruction.get(2));

        mSlideShow.setSlidesCount(aSlideShowSlidesCount);
        mSlideShow.setCurrentSlideIndex(aCurrentSlideIndex);

        Intent aStatusConnectedSlideShowRunningIntent = new Intent(
            CommunicationService.STATUS_CONNECTED_SLIDESHOW_RUNNING);
        Intent aSlideChangedIntent = new Intent(
            CommunicationService.MSG_SLIDE_CHANGED);
        aSlideChangedIntent.putExtra("slide_number", aCurrentSlideIndex);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aStatusConnectedSlideShowRunningIntent);
        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aSlideChangedIntent);
    }

    private void finishSlideShow() {
        this.mSlideShow = new SlideShow(mContext);

        Intent aStatusConnectedNoSlideShowIntent = new Intent(
            CommunicationService.STATUS_CONNECTED_NOSLIDESHOW);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aStatusConnectedNoSlideShowIntent);
    }

    private void setUpCurrentSlide(List<String> aInstruction) {
        int aCurrentSlideIndex = Integer.parseInt(aInstruction.get(1));

        mSlideShow.setCurrentSlideIndex(aCurrentSlideIndex);

        Intent aSlideChangedIntent = new Intent(
            CommunicationService.MSG_SLIDE_CHANGED);
        aSlideChangedIntent.putExtra("slide_number", aCurrentSlideIndex);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aSlideChangedIntent);
    }

    private void setUpSlidePreview(List<String> aInstruction) {
        int aSlideIndex = Integer.parseInt(aInstruction.get(1));
        String aImageAsString = aInstruction.get(2);

        byte[] aImage = Base64.decode(aImageAsString, Base64.DEFAULT);
        mSlideShow.setSlidePreview(aSlideIndex, aImage);

        Intent aSlidePreviewChangedIntent = new Intent(
            CommunicationService.MSG_SLIDE_PREVIEW);
        aSlidePreviewChangedIntent.putExtra("slide_number", aSlideIndex);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aSlidePreviewChangedIntent);
    }

    private void setUpSlideNotes(List<String> aInstruction) {
        int aSlideIndex = Integer.parseInt(aInstruction.get(1));
        StringBuilder aNotesBuilder = new StringBuilder();
        for (int aNoteIndex = 2; aNoteIndex < aInstruction
            .size(); aNoteIndex++) {
            aNotesBuilder.append(aInstruction.get(aNoteIndex));
        }
        String aNotes = aNotesBuilder.toString();

        mSlideShow.setSlideNotes(aSlideIndex, aNotes);

        Intent aSlideNotesChangedIntent = new Intent(
            CommunicationService.MSG_SLIDE_NOTES);
        aSlideNotesChangedIntent.putExtra("slide_number", aSlideIndex);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aSlideNotesChangedIntent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
