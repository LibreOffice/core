package org.libreoffice;

import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.TextSelectionHandle;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Parses (interprets) and handles invalidation messages from LibreOffice.
 */
public class InvalidationHandler {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();

    private TextCursorLayer mTextCursorLayer;
    private TextSelection mTextSelection;
    private OverlayState mState;

    public InvalidationHandler(LibreOfficeMainActivity mainActivity) {
        mTextCursorLayer = mainActivity.getTextCursorLayer();
        mTextSelection = mainActivity.getTextSelection();
        mState = OverlayState.NONE;
    }

    /**
     * Processes invalidation message
     *
     * @param messageID - ID of the message
     * @param payload   - additional invalidation message payload
     */
    public void processMessage(int messageID, String payload) {
        switch (messageID) {
            case Document.CALLBACK_INVALIDATE_TILES:
                invalidateTiles(payload);
                break;
            case Document.CALLBACK_INVALIDATE_VISIBLE_CURSOR:
                invalidateCursor(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION:
                invalidateSelection(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION_START:
                invalidateSelectionStart(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION_END:
                invalidateSelectionEnd(payload);
                break;
        }
    }

    public OverlayState getOverlayState() {
        return mState;
    }

    public void setOverlayState(OverlayState state) {
        this.mState = state;
    }

    /**
     * Parses the payload text with rectangle coordinates and converts to rectangle in pixel coordinates
     *
     * @param payload - invalidation message payload text
     * @return rectangle in pixel coordinates
     */
    private RectF convertPayloadToRectangle(String payload) {
        String payloadWithoutWhitespace = payload.replaceAll("\\s",""); // remove all whitespace from the string

        if (payloadWithoutWhitespace.isEmpty() || payloadWithoutWhitespace.equals("EMPTY")) {
            return null;
        }

        String[] coordinates = payloadWithoutWhitespace.split(",");

        if (coordinates.length != 4) {
            return null;
        }

        int width = Integer.decode(coordinates[0]);
        int height = Integer.decode(coordinates[1]);
        int x = Integer.decode(coordinates[2]);
        int y = Integer.decode(coordinates[3]);

        float dpi = (float) LOKitShell.getDpi();

        RectF rect = new RectF(
                LOKitTileProvider.twipToPixel(x, dpi),
                LOKitTileProvider.twipToPixel(y, dpi),
                LOKitTileProvider.twipToPixel(x + width, dpi),
                LOKitTileProvider.twipToPixel(y + height, dpi)
        );

        return rect;
    }

    /**
     * Parses the payload text with more rectangles (separated by ';') and converts to a list of rectangles.
     *
     * @param payload - invalidation message payload text
     * @return list of rectangles
     */
    private List<RectF> convertPayloadToRectangles(String payload) {
        List<RectF> rectangles = new ArrayList<RectF>();
        String[] rectangleArray = payload.split(";");

        for (String coordinates : rectangleArray) {
            RectF rectangle = convertPayloadToRectangle(coordinates);
            if (rectangle != null) {
                rectangles.add(rectangle);
            }

        }

        return rectangles;
    }

    /**
     * Handles the tile invalidation message
     *
     * @param payload
     */
    private void invalidateTiles(String payload) {
        RectF rectangle = convertPayloadToRectangle(payload);
        if (rectangle != null) {
            LOKitShell.sendTileInvalidationRequest(rectangle);
        }
    }

    /**
     * Handles the cursor invalidation message
     *
     * @param payload
     */
    private void invalidateCursor(String payload) {
        RectF cursorRectangle = convertPayloadToRectangle(payload);
        if (cursorRectangle != null) {
            if (mState == OverlayState.CURSOR) {
                mTextSelection.positionHandle(TextSelectionHandle.HandleType.MIDDLE, cursorRectangle);
                mTextSelection.showHandle(TextSelectionHandle.HandleType.MIDDLE);
                mTextSelection.hideHandle(TextSelectionHandle.HandleType.START);
                mTextSelection.hideHandle(TextSelectionHandle.HandleType.END);
            }
            mTextCursorLayer.positionCursor(cursorRectangle);
            mTextCursorLayer.showCursor();
        }
    }

    /**
     * Handles the selection start invalidation message
     *
     * @param payload
     */
    private void invalidateSelectionStart(String payload) {
        if (mState == OverlayState.NONE) {
            return;
        }
        RectF selectionRectangle = convertPayloadToRectangle(payload);
        if (selectionRectangle != null) {
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.START, selectionRectangle);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
            mState = OverlayState.SELECTION;
        }
    }

    /**
     * Handles the selection end invalidation message
     *
     * @param payload
     */
    private void invalidateSelectionEnd(String payload) {
        if (mState == OverlayState.NONE) {
            return;
        }
        RectF selectionRect = convertPayloadToRectangle(payload);
        if (selectionRect != null) {
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.END, selectionRect);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.END);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
            mState = OverlayState.SELECTION;
        }
    }

    /**
     * Handles the selection invalidation message
     *
     * @param payload
     */
    private void invalidateSelection(String payload) {
        if (mState == OverlayState.NONE) {
            return;
        }
        if (payload.isEmpty()) {
            mState = OverlayState.CURSOR;
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.END);
            mTextCursorLayer.changeSelections(Collections.EMPTY_LIST);
        } else {
            mState = OverlayState.SELECTION;
            List<RectF> rects = convertPayloadToRectangles(payload);
            mTextCursorLayer.changeSelections(rects);
        }
    }

    public enum OverlayState {
        NONE,
        CURSOR,
        SELECTION
    }
}
