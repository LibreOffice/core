package org.libreoffice.impressremote.communication;

public interface MessagesListener {
    public void onPinValidation();

    public void onSuccessfulPairing();

    public void onSlideShowStart(int aSlidesCount, int aCurrentSlideIndex);

    public void onSlideShowFinish();

    public void onSlideChanged(int aCurrentSlideIndex);

    public void onSlidePreview(int aSlideIndex, byte[] aPreview);

    public void onSlideNotes(int aSlideIndex, String aNotes);
}
