/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.Random;

import android.text.TextUtils;

final class Protocol {
    private Protocol() {
    }

    public static final String CHARSET = "UTF-8";

    public static final class Ports {
        private Ports() {
        }

        public static final int SERVER_SEARCH = 1598;
        public static final int CLIENT_CONNECTION = 1599;
    }

    public static final class Addresses {
        private Addresses() {
        }

        public static final String SERVER_SEARCH = "239.0.0.1";
    }

    public static final class Messages {
        private Messages() {
        }

        public static final String PAIRED = "LO_SERVER_SERVER_PAIRED";
        public static final String VALIDATING = "LO_SERVER_VALIDATING_PIN";
        public static final String ADVERTISE = "LOREMOTE_ADVERTISE";

        public static final String SLIDE_SHOW_STARTED = "slideshow_started";
        public static final String SLIDE_SHOW_FINISHED = "slideshow_finished";
        public static final String SLIDE_UPDATED = "slide_updated";
        public static final String SLIDE_PREVIEW = "slide_preview";
        public static final String SLIDE_NOTES = "slide_notes";
    }

    public static final class Commands {
        private Commands() {
        }

        public static final String PAIR_WITH_SERVER = "LO_SERVER_CLIENT_PAIR";
        public static final String SEARCH_SERVERS = "LOREMOTE_SEARCH";

        public static final String TRANSITION_NEXT = "transition_next";
        public static final String TRANSITION_PREVIOUS = "transition_previous";
        public static final String GO_TO_SLIDE = "goto_slide";
        public static final String PRESENTATION_BLANK_SCREEN = "presentation_blank_screen";
        public static final String PRESENTATION_RESUME = "presentation_resume";
        public static final String PRESENTATION_START = "presentation_start";
        public static final String PRESENTATION_STOP = "presentation_stop";

        private static final String DELIMITER_PARAMETER = "\n";
        private static final String DELIMITER_COMMAND = "\n\n";

        public static String prepareCommand(String aCommand) {
            return String.format("%s%s", aCommand, DELIMITER_COMMAND);
        }

        public static String prepareCommand(String... aParameters) {
            String aCommand = TextUtils.join(DELIMITER_PARAMETER, aParameters);

            return prepareCommand(aCommand);
        }
    }

    public static final class Pin {
        private static final int NUMBERS_COUNT = 4;

        private Pin() {
        }

        public static String generate() {
            return new Pin().generatePinText();
        }

        private String generatePinText() {
            int aPin = preventLeadingZeros(generatePinNumber());

            return String.format(buildPinFormat(), aPin);
        }

        private int generatePinNumber() {
            int aMaximumPin = (int) Math.pow(10, NUMBERS_COUNT) - 1;

            return new Random().nextInt(aMaximumPin);
        }

        private int preventLeadingZeros(int aPin) {
            // Pin cannot have leading zeros.
            // LibreOffice Impress doesn’t allow to enter leading zeros.
            // Bug exists at least at LibreOffice 4.1.

            int aMinimumPin = (int) Math.pow(10, NUMBERS_COUNT - 1);

            if (aPin >= aMinimumPin) {
                return aPin;
            }

            return aPin + aMinimumPin;
        }

        private String buildPinFormat() {
            return String.format("%%0%sd", Integer.toString(NUMBERS_COUNT));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
