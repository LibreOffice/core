package org.libreoffice;

import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.TextSelectionHandle;

/**
 * Parses (interprets) and handles invalidation messages from LibreOffice.
 */
public class InvalidationHandler {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();

    private TextCursorLayer mTextCursorLayer;
    private TextSelection mTextSelection;

    public InvalidationHandler(LibreOfficeMainActivity mainActivity) {
        mTextCursorLayer = mainActivity.getTextCursorLayer();
        mTextSelection = mainActivity.getTextSelection();
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

    /**
     * Parses the invalidation message text and converts to rectangle in pixel coordinates
     *
     * @param messageText - invalidation message text
     * @return rectangle in pixel coordinates
     */
    private RectF convertCallbackMessageStringToRectF(String messageText) {
        if (messageText.equals("EMPTY")) {
            return null;
        }

        String[] coordinates = messageText.split(",");

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
     * Handles the cursor invalidation message
     *
     * @param payload
     */
    private void invalidateCursor(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.MIDDLE, underSelection);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.MIDDLE);
            mTextCursorLayer.positionCursor(rect);
            mTextCursorLayer.showCursor();
        }
    }

    /**
     * Handles the tile invalidation message
     *
     * @param payload
     */
    private void invalidateTiles(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            LOKitShell.sendTileInvalidationRequest(rect);
        }
    }

    /**
     * Handles the selection start invalidation message
     *
     * @param payload
     */
    private void invalidateSelectionStart(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.START, underSelection);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
            mTextCursorLayer.hideCursor();
        }
    }

    /**
     * Handles the selection end invalidation message
     *
     * @param payload
     */
    private void invalidateSelectionEnd(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.END, underSelection);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.END);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
            mTextCursorLayer.hideCursor();
        }
    }

    /**
     * Handles the selection invalidation message
     *
     * @param payload
     */
    private void invalidateSelection(String payload) {
        if (payload.isEmpty()) {
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.END);
        }
    }

}
