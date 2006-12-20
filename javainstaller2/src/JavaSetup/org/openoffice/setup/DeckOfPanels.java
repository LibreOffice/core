package org.openoffice.setup;

import java.util.HashMap;

public class DeckOfPanels {

    private PanelController mCurrent;
    private HashMap mDeck;

    public DeckOfPanels() {
        mDeck = new HashMap();
        mCurrent = null;
    }

    void addPanel(PanelController panel, String name) {
            mDeck.put(name, panel);
    }

    public PanelController getCurrentPanel () {
        return mCurrent;
    }

    public PanelController setCurrentPanel(String name) {
        mCurrent = (PanelController)mDeck.get(name);
        return mCurrent;
    }
}
