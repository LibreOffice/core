/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.ArrayList;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Base64;

public class Receiver {

    public Receiver(Context aContext) {
        mContext = aContext;
        mSlideShow = new SlideShow(mContext);
    }

    private Context mContext;

    private SlideShow mSlideShow;

    public SlideShow getSlideShow() {
        return mSlideShow;
    }

    public boolean isSlideShowRunning() {
        return (mSlideShow.getSize() > 0);
    }

    public void parseCommand(ArrayList<String> aCommand) {
        if (aCommand.size() == 0)
            return; // E.g. if empty line received for whatever reason.
        String aInstruction = aCommand.get(0);
        if (aInstruction.equals("slideshow_started")) {
            int aSlideShowlength = Integer.parseInt(aCommand.get(1));
            int aCurrentSlide = Integer.parseInt(aCommand.get(2));
            mSlideShow.setLength(aSlideShowlength);
            mSlideShow.setCurrentSlide(aCurrentSlide);
            Intent aIntent = new Intent(
                            CommunicationService.MSG_SLIDESHOW_STARTED);
            LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
        } else {
            if (mSlideShow == null)
                return;

            if (aInstruction.equals("slide_updated")) {

                int aSlideNumber = Integer.parseInt(aCommand.get(1));

                mSlideShow.setCurrentSlide(aSlideNumber);

                Intent aIntent = new Intent(
                                CommunicationService.MSG_SLIDE_CHANGED);
                aIntent.putExtra("slide_number", aSlideNumber);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aIntent);
            } else if (aInstruction.equals("slide_preview")) {
                int aSlideNumber = Integer.parseInt(aCommand.get(1));
                String aImageString = aCommand.get(2);
                byte[] aImage = Base64.decode(aImageString, Base64.DEFAULT);

                // Store image internally
                mSlideShow.putImage(aSlideNumber, aImage);

                Intent aIntent = new Intent(
                                CommunicationService.MSG_SLIDE_PREVIEW);
                aIntent.putExtra("slide_number", aSlideNumber);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aIntent);
            } else if (aInstruction.equals("slide_notes")) {
                int aSlideNumber = Integer.parseInt(aCommand.get(1));
                String aNotes = new String();
                for (int i = 2; i < aCommand.size(); i++) {
                    aNotes += aCommand.get(i);
                }

                // Store image internally
                mSlideShow.putNotes(aSlideNumber, aNotes);

                Intent aIntent = new Intent(
                                CommunicationService.MSG_SLIDE_NOTES);
                aIntent.putExtra("slide_number", aSlideNumber);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aIntent);
            }

        }

    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */