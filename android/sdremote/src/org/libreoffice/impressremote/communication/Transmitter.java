/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.graphics.Color;

/**
 * Interface to send commands to the server.
 */
public class Transmitter {
    private final Client mClient;

    public Transmitter(Client aClient) {
        this.mClient = aClient;
    }

    public void performNextTransition() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.TRANSITION_NEXT));
    }

    public void performPreviousTransition() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.TRANSITION_PREVIOUS));
    }

    public void setCurrentSlide(int slideIndex) {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.GOTO_SLIDE,
                Integer.toString(slideIndex)));
    }

    /**
     * Blank the screen to the default colour (set server-side), which is
     * generally black. This is slightly faster than using
     * <code> setUpBlankScreen( colour ) </code>.
     */
    public void setUpBlankScreen() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_BLANK_SCREEN));
    }

    /**
     * Set the screen to a specific colour. Only use if a non default colour is
     * needed.
     *
     * @param aColor blank screen color
     */
    public void setUpBlankScreen(Color aColor) {
        // FIXME: check how to get colour in integer form.

        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_BLANK_SCREEN,
                aColor.toString()));
    }

    public void resumePresentation() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_RESUME));
    }

    public void startPresentation() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_START));
    }

    public void stopPresentation() {
        mClient.sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_STOP));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
