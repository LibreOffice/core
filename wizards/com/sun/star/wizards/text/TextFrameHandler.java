package com.sun.star.wizards.text;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.text.XTextFramesSupplier;
import com.sun.star.uno.UnoRuntime;

public class TextFrameHandler {

    public static XTextFrame getFrameByName(String sFrameName, XTextDocument xTD) throws NoSuchElementException, WrappedTargetException {
        XTextFramesSupplier xFrameSupplier = (XTextFramesSupplier) UnoRuntime.queryInterface(XTextFramesSupplier.class, xTD);
        if (xFrameSupplier.getTextFrames().hasByName(sFrameName)) {
            Object oTextFrame = xFrameSupplier.getTextFrames().getByName(sFrameName);
            XTextFrame xTextFrame = (XTextFrame) UnoRuntime.queryInterface(XTextFrame.class, oTextFrame);
            return xTextFrame;
        }
        return null;
    }

}
