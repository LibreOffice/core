/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

public class CommandsTransmitter {
    private final BufferedWriter mCommandsWriter;

    public CommandsTransmitter(ServerConnection aServerConnection) {
        mCommandsWriter = buildCommandsWriter(aServerConnection);
    }

    private BufferedWriter buildCommandsWriter(ServerConnection aServerConnection) {
        try {
            OutputStream aCommandsStream = aServerConnection.buildCommandsStream();

            return new BufferedWriter(
                new OutputStreamWriter(aCommandsStream, Protocol.CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("Unable to create commands writer.");
        }
    }

    public void pair(String aDeviceName, String aPin) {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PAIR_WITH_SERVER, aDeviceName,
                aPin));
    }

    private void writeCommand(String aCommand) {
        try {
            mCommandsWriter.write(aCommand);
            mCommandsWriter.flush();
        } catch (IOException e) {
            throw new RuntimeException("Unable to write command.");
        }
    }

    public void performNextTransition() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.TRANSITION_NEXT));
    }

    public void performPreviousTransition() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.TRANSITION_PREVIOUS));
    }

    public void setCurrentSlide(int slideIndex) {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.GOTO_SLIDE,
                Integer.toString(slideIndex)));
    }

    public void setUpBlankScreen() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_BLANK_SCREEN));
    }

    public void resumePresentation() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_RESUME));
    }

    public void startPresentation() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_START));
    }

    public void stopPresentation() {
        writeCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PRESENTATION_STOP));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
