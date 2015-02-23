package org.libreoffice;

import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.TextSelectionHandle;

public class InvalidationHandler {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();

    public InvalidationHandler() {
    }

    public void processMessage(int messageID, String payload) {
        switch (messageID) {
            case Document.CALLBACK_INVALIDATE_TILES:
                invalidateTiles(payload);
                break;
            case Document.CALLBACK_INVALIDATE_VISIBLE_CURSOR:
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

    private RectF convertCallbackMessageStringToRectF(String text) {
        if (text.equals("EMPTY")) {
            return null;
        }

        String[] coordinates = text.split(",");

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

    private void invalidateCursor(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            TextSelection textSelection = LibreOfficeMainActivity.mAppContext.getTextSelection();
            textSelection.positionHandle(TextSelectionHandle.HandleType.MIDDLE, underSelection);
            textSelection.showHandle(TextSelectionHandle.HandleType.MIDDLE);

            TextCursorLayer textCursorLayer = LibreOfficeMainActivity.mAppContext.getTextCursorLayer();
            textCursorLayer.positionCursor(rect);
            textCursorLayer.showCursor();
        }
    }

    private void invalidateTiles(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            LOKitShell.sendTileInvalidationRequest(rect);
        }
    }

    private void invalidateSelectionStart(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            TextSelection textSelection = LibreOfficeMainActivity.mAppContext.getTextSelection();
            textSelection.positionHandle(TextSelectionHandle.HandleType.START, underSelection);
            textSelection.showHandle(TextSelectionHandle.HandleType.START);

            textSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);

            TextCursorLayer textCursorLayer = LibreOfficeMainActivity.mAppContext.getTextCursorLayer();
            textCursorLayer.hideCursor();
        }
    }

    private void invalidateSelectionEnd(String payload) {
        RectF rect = convertCallbackMessageStringToRectF(payload);
        if (rect != null) {
            RectF underSelection = new RectF(rect.centerX(), rect.bottom, rect.centerX(), rect.bottom);
            TextSelection textSelection = LibreOfficeMainActivity.mAppContext.getTextSelection();
            textSelection.positionHandle(TextSelectionHandle.HandleType.END, underSelection);
            textSelection.showHandle(TextSelectionHandle.HandleType.END);

            textSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);

            TextCursorLayer textCursorLayer = LibreOfficeMainActivity.mAppContext.getTextCursorLayer();
            textCursorLayer.hideCursor();
        }
    }

    private void invalidateSelection(String payload) {
        if (payload.isEmpty()) {
            TextSelection textSelection = LibreOfficeMainActivity.mAppContext.getTextSelection();
            textSelection.hideHandle(TextSelectionHandle.HandleType.START);
            textSelection.hideHandle(TextSelectionHandle.HandleType.END);
        }
    }

}
