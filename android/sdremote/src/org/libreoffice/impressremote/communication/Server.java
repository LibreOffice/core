package org.libreoffice.impressremote.communication;

public class Server {

    public enum Protocol {
        NETWORK, BLUETOOTH
    };

    private Protocol mProtocol;
    private String mAddress;
    private String mName;
    private long mTimeDiscovered;
    /**
     * Signifies a Server that shouldn't be automatically removed from the list.
     * Used e.g. for the emulator.
     */
    protected boolean mNoTimeout = false;

    protected Server(Protocol aProtocol, String aAddress, String aName,
                    long aTimeDiscovered) {
        mProtocol = aProtocol;
        mAddress = aAddress;
        mName = aName;
        mTimeDiscovered = aTimeDiscovered;
    }

    public Protocol getProtocol() {
        return mProtocol;
    }

    public String getAddress() {
        return mAddress;
    }

    /**
     * Get a human friendly name for the server.
     *
     * @return The name.
     */
    public String getName() {
        return mName;
    }

    public long getTimeDiscovered() {
        return mTimeDiscovered;
    }

}
