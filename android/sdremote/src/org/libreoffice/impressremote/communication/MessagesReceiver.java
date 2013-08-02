/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import android.text.TextUtils;
import android.util.Base64;

class MessagesReceiver implements Runnable {
    private final BufferedReader mMessagesReader;

    private final MessagesListener mMessagesListener;

    public MessagesReceiver(ServerConnection aServerConnection, MessagesListener aMessagesListener) {
        mMessagesReader = buildMessagesReader(aServerConnection);

        mMessagesListener = aMessagesListener;

        Thread mMessagesListenerThread = new Thread(this);
        mMessagesListenerThread.start();
    }

    private BufferedReader buildMessagesReader(ServerConnection aServerConnection) {
        try {
            InputStream aMessagesStream = aServerConnection.buildMessagesStream();

            return new BufferedReader(
                new InputStreamReader(aMessagesStream, Protocol.CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("Unable to create messages reader.");
        }
    }

    @Override
    public void run() {
        while (true) {
            List<String> aMessage = readMessage();

            if (aMessage == null) {
                return;
            }

            parseMessage(aMessage);
        }
    }

    private List<String> readMessage() {
        List<String> aMessage = new ArrayList<String>();

        String aMessageParameter = readMessageParameter();

        while (true) {
            if (aMessageParameter == null) {
                break;
            }

            if (TextUtils.isEmpty(aMessageParameter)) {
                break;
            }

            aMessage.add(aMessageParameter);

            aMessageParameter = readMessageParameter();
        }

        if (aMessageParameter == null) {
            return null;
        }

        return aMessage;
    }

    private String readMessageParameter() {
        try {
            return mMessagesReader.readLine();
        } catch (IOException e) {
            return null;
        }
    }

    private void parseMessage(List<String> aMessage) {
        if (aMessage.isEmpty()) {
            return;
        }

        String aMessageType = aMessage.get(0);

        if (Protocol.Messages.VALIDATING.equals(aMessageType)) {
            mMessagesListener.onPinValidation();
            return;
        }

        if (Protocol.Messages.PAIRED.equals(aMessageType)) {
            mMessagesListener.onSuccessfulPairing();
            return;
        }

        if (Protocol.Messages.SLIDE_SHOW_STARTED.equals(aMessageType)) {
            int aSlidesCount = parseSlidesCount(aMessage, 1);
            int aCurrentSlideIndex = parseSlideIndex(aMessage, 2);

            mMessagesListener.onSlideShowStart(aSlidesCount, aCurrentSlideIndex);
            return;
        }

        if (Protocol.Messages.SLIDE_SHOW_FINISHED.equals(aMessageType)) {
            mMessagesListener.onSlideShowFinish();
            return;
        }

        if (Protocol.Messages.SLIDE_UPDATED.equals(aMessageType)) {
            int aCurrentSlideIndex = parseSlideIndex(aMessage, 1);

            mMessagesListener.onSlideChanged(aCurrentSlideIndex);
            return;
        }

        if (Protocol.Messages.SLIDE_PREVIEW.equals(aMessageType)) {
            int aSlideIndex = parseSlideIndex(aMessage, 1);
            byte[] aSlidePreview = parseSlidePreview(aMessage, 2);

            mMessagesListener.onSlidePreview(aSlideIndex, aSlidePreview);
            return;
        }

        if (Protocol.Messages.SLIDE_NOTES.equals(aMessageType)) {
            int aSlideIndex = parseSlideIndex(aMessage, 1);
            String aSlideNotes = parseSlideNotes(aMessage, 2);

            mMessagesListener.onSlideNotes(aSlideIndex, aSlideNotes);
        }
    }

    private int parseSlidesCount(List<String> aMessage, int aMessageParameterIndex) {
        return Integer.parseInt(aMessage.get(aMessageParameterIndex));
    }

    private int parseSlideIndex(List<String> aMessage, int aMessageParameterIndex) {
        return Integer.parseInt(aMessage.get(aMessageParameterIndex));
    }

    private byte[] parseSlidePreview(List<String> aMessage, int aMessageParameterIndex) {
        String aPreviewAsString = aMessage.get(aMessageParameterIndex);

        return Base64.decode(aPreviewAsString, Base64.DEFAULT);
    }

    private String parseSlideNotes(List<String> aMessage, int aMessageParameterIndex) {
        StringBuilder aNotesBuilder = new StringBuilder();

        for (int aNoteIndex = aMessageParameterIndex; aNoteIndex < aMessage.size(); aNoteIndex++) {
            aNotesBuilder.append(aMessage.get(aNoteIndex));
        }

        return aNotesBuilder.toString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
