package org.libreoffice.impressremote;

import android.content.Context;
import android.content.Intent;

import org.libreoffice.impressremote.communication.Server;

public final class Intents {
    private Intents() {
    }

    public static final class Actions {
        private Actions() {
        }

        public static final String SERVERS_LIST_CHANGED = "SERVERS_LIST_CHANGED";

        public static final String PAIRING_STARTED = "PAIRING_STARTED";
        public static final String PAIRING_SUCCESSFUL = "PAIRING_SUCCESSFUL";
        public static final String PAIRING_VALIDATION = "PAIRING_VALIDATION";

        public static final String CONNECTION_FAILED = "CONNECTION_FAILED";

        public static final String SLIDE_SHOW_STARTED = "SLIDE_SHOW_STARTED";
        public static final String SLIDE_SHOW_RUNNING = "SLIDE_SHOW_RUNNING";
        public static final String SLIDE_SHOW_STOPPED = "SLIDE_SHOW_STOPPED";

        public static final String SLIDE_CHANGED = "SLIDE_CHANGED";
        public static final String SLIDE_PREVIEW = "SLIDE_PREVIEW";
        public static final String SLIDE_NOTES = "SLIDE_NOTES";
    }

    public static final class Extras {
        private Extras() {
        }

        public static final String PIN = "PIN";

        public static final String SERVER = "SERVER";
        public static final String SERVER_NAME = "SERVER_NAME";

        public static final String SLIDE_INDEX = "SLIDE_INDEX";
    }

    public static Intent buildServersListChangedIntent() {
        return new Intent(Actions.SERVERS_LIST_CHANGED);
    }

    public static Intent buildPairingStartedIntent(String aPin) {
        Intent aIntent = new Intent(Actions.PAIRING_STARTED);
        aIntent.putExtra(Extras.PIN, aPin);

        return aIntent;
    }

    public static Intent buildPairingSuccessfulIntent() {
        return new Intent(Actions.PAIRING_SUCCESSFUL);
    }

    public static Intent buildPairingValidationIntent(String aPin, String aServerName) {
        Intent aIntent = new Intent(Actions.PAIRING_VALIDATION);
        aIntent.putExtra(Extras.PIN, aPin);
        aIntent.putExtra(Extras.SERVER_NAME, aServerName);

        return aIntent;
    }

    public static Intent buildConnectionFailedIntent() {
        return new Intent(Actions.CONNECTION_FAILED);
    }

    public static Intent buildSlideShowRunningIntent() {
        return new Intent(Actions.SLIDE_SHOW_RUNNING);
    }

    public static Intent buildSlideShowStoppedIntent() {
        return new Intent(Actions.SLIDE_SHOW_STOPPED);
    }

    public static Intent buildSlideChangedIntent(int aSlideIndex) {
        Intent aIntent = new Intent(Actions.SLIDE_CHANGED);
        aIntent.putExtra(Extras.SLIDE_INDEX, aSlideIndex);

        return aIntent;
    }

    public static Intent buildSlidePreviewIntent(int aSlideIndex) {
        Intent aIntent = new Intent(Actions.SLIDE_PREVIEW);
        aIntent.putExtra(Extras.SLIDE_INDEX, aSlideIndex);

        return aIntent;
    }

    public static Intent buildSlideNotesIntent(int aSlideIndex) {
        Intent aIntent = new Intent(Actions.SLIDE_NOTES);
        aIntent.putExtra(Extras.SLIDE_INDEX, aSlideIndex);

        return aIntent;
    }

    public static Intent buildComputerConnectionIntent(Context aContext, Server aServer) {
        Intent aIntent = new Intent(aContext, ComputerConnectionActivity.class);
        aIntent.putExtra(Extras.SERVER, aServer);

        return aIntent;
    }
}
