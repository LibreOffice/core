package org.libreoffice;

import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.TextSelectionHandle;

import java.util.ArrayList;
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
                Log.i(LOGTAG, "Cursor: " + payload);
                invalidateCursor(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION:
                Log.i(LOGTAG, "Selection: " + payload);
                invalidateSelection(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION_START:
                Log.i(LOGTAG, "Selection start: " + payload);
                invalidateSelectionStart(payload);
                break;
            case Document.CALLBACK_INVALIDATE_TEXT_SELECTION_END:
                Log.i(LOGTAG, "Selection end: " + payload);
                invalidateSelectionEnd(payload);
                break;
        }
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
        if (payload.equals("EMPTY")) {
            return null;
        }

        String[] coordinates = payload.split(",");

        if (coordinates.length != 4) {
            return null;
        }
        int width = Integer.decode(coordinates[0].trim());
        int height = Integer.decode(coordinates[1].trim());
        int x = Integer.decode(coordinates[2].trim());
        int y = Integer.decode(coordinates[3].trim());

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
            RectF rectangle = convertPayloadToRectangle(payload);
            rectangles.add(rectangle);
        }

        return rectangles;
    }

    /**
     * From input rectangle, create a new rectangle which positions under the input rectangle.
     *
     * @param rectangle - input rectangle
     * @return new rectangle positioned under the input rectangle
     */
    private RectF createRectangleUnderSelection(RectF rectangle) {
        return new RectF(rectangle.centerX(), rectangle.bottom, rectangle.centerX(), rectangle.bottom);
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
        if (mState == OverlayState.CURSOR) {
            RectF cursorRectangle = convertPayloadToRectangle(payload);
            if (cursorRectangle != null) {
                TextSelection textSelection = LibreOfficeMainActivity.mAppContext.getTextSelection();
                textSelection.positionHandle(TextSelectionHandle.HandleType.MIDDLE, createRectangleUnderSelection(cursorRectangle));
                textSelection.showHandle(TextSelectionHandle.HandleType.MIDDLE);
                textSelection.hideHandle(TextSelectionHandle.HandleType.START);
                textSelection.hideHandle(TextSelectionHandle.HandleType.END);

                TextCursorLayer textCursorLayer = LibreOfficeMainActivity.mAppContext.getTextCursorLayer();
                textCursorLayer.positionCursor(cursorRectangle);
                textCursorLayer.showCursor();
            }
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
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.START, createRectangleUnderSelection(selectionRectangle));
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
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.END, createRectangleUnderSelection(selectionRect));
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
