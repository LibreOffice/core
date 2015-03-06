package org.libreoffice;

import android.content.Intent;
import android.graphics.RectF;
import android.net.Uri;
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
public class InvalidationHandler implements Document.MessageCallback {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();

    private TileProvider mTileProvider;
    private final TextCursorLayer mTextCursorLayer;
    private final TextSelection mTextSelection;

    private OverlayState mState;

    public InvalidationHandler(LibreOfficeMainActivity mainActivity) {
        mTextCursorLayer = mainActivity.getTextCursorLayer();
        mTextSelection = mainActivity.getTextSelection();
        mTileProvider = null;
        mState = OverlayState.NONE;
    }

    public void setTileProvider(TileProvider tileProvider) {
        mTileProvider = tileProvider;
    }

    /**
     * Processes callback message
     *
     * @param messageID - ID of the message
     * @param payload   - additional invalidation message payload
     */
    @Override
    public void messageRetrieved(int messageID, String payload) {
        if (!LOKitShell.isEditingEnabled()) {
            return;
        }
        switch (messageID) {
            case Document.CALLBACK_INVALIDATE_TILES:
                invalidateTiles(payload);
                break;
            case Document.CALLBACK_INVALIDATE_VISIBLE_CURSOR:
                invalidateCursor(payload);
                break;
            case Document.CALLBACK_TEXT_SELECTION:
                textSelection(payload);
                break;
            case Document.CALLBACK_TEXT_SELECTION_START:
                textSelectionStart(payload);
                break;
            case Document.CALLBACK_TEXT_SELECTION_END:
                textSelectionEnd(payload);
                break;
            case Document.CALLBACK_CURSOR_VISIBLE:
                cursorVisibility(payload);
                break;
            case Document.CALLBACK_HYPERLINK_CLICKED:
                if (!payload.startsWith("http://") && !payload.startsWith("https://")) {
                    payload = "http://" + payload;
                }

                Intent urlIntent = new Intent(Intent.ACTION_VIEW);
                urlIntent.setData(Uri.parse(payload));
                LibreOfficeMainActivity.mAppContext.startActivity(urlIntent);
                break;
        }
    }

    /**
     * Parses the payload text with rectangle coordinates and converts to rectangle in pixel coordinates
     *
     * @param payload - invalidation message payload text
     * @return rectangle in pixel coordinates
     */
    private RectF convertPayloadToRectangle(String payload) {
        String payloadWithoutWhitespace = payload.replaceAll("\\s", ""); // remove all whitespace from the string

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
    private synchronized void invalidateCursor(String payload) {
        RectF cursorRectangle = convertPayloadToRectangle(payload);
        if (cursorRectangle != null) {
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.MIDDLE, cursorRectangle);
            mTextCursorLayer.positionCursor(cursorRectangle);

            if (mState == OverlayState.TRANSITION_TO_CURSOR) {
                changeStateTo(OverlayState.CURSOR);
            };
        }
    }

    /**
     * Handles the text selection start message
     *
     * @param payload
     */
    private synchronized void textSelectionStart(String payload) {
        RectF selectionRect = convertPayloadToRectangle(payload);
        if (selectionRect != null) {
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.START, selectionRect);
        }
    }

    /**
     * Handles the text selection end message
     *
     * @param payload
     */
    private synchronized void textSelectionEnd(String payload) {
        RectF selectionRect = convertPayloadToRectangle(payload);
        if (selectionRect != null) {
            mTextSelection.positionHandle(TextSelectionHandle.HandleType.END, selectionRect);
        }
    }

    /**
     * Handles the text selection message
     *
     * @param payload
     */
    private synchronized void textSelection(String payload) {
        if (payload.isEmpty() || payload.equals("EMPTY")) {
            if (mState == OverlayState.SELECTION) {
                changeStateTo(OverlayState.TRANSITION_TO_CURSOR);
            }
            mTextCursorLayer.changeSelections(Collections.EMPTY_LIST);
        } else {
            if (mState == OverlayState.TRANSITION_TO_SELECTION) {
                changeStateTo(OverlayState.SELECTION);
            }
            List<RectF> rects = convertPayloadToRectangles(payload);
            mTextCursorLayer.changeSelections(rects);
        }
    }

    /**
     * Handles the cursor visibility message
     *
     * @param payload
     */
    private synchronized void cursorVisibility(String payload) {
        if (payload.equals("true")) {
            mTextCursorLayer.showCursor();
        } else if (payload.equals("false")) {
            mTextCursorLayer.hideCursor();
        }
    }

    public synchronized void changeStateTo(OverlayState next) {
        changeState(mState, next);
    }

    private synchronized void changeState(OverlayState previous, OverlayState next) {
        if (isInvalidTransition(previous, next)) {
            return;
        }

        Log.i(LOGTAG, "State change: " + previous.name() + " -> " + next.name());

        mState = next;

        switch (next) {
            case CURSOR:
                handleCursorState(previous);
                break;
            case TRANSITION_TO_CURSOR:
                handleTransitionToCursorState(previous);
                break;
            case SELECTION:
                handleSelectionState(previous);
                break;
            case TRANSITION_TO_SELECTION:
                handleTransitionToSelectionState(previous);
                break;
            case NONE:
                handleNoneState(previous);
                break;
        }
    }

    private void handleNoneState(OverlayState previous) {
        if (previous == OverlayState.NONE) {
            return;
        }
        // Just hide everything
        if (mTileProvider != null) {
            mTileProvider.setTextSelectionReset();
        }
        mTextSelection.hideHandle(TextSelectionHandle.HandleType.START);
        mTextSelection.hideHandle(TextSelectionHandle.HandleType.END);
        mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
        mTextCursorLayer.hideSelections();
        mTextCursorLayer.hideCursor();
        LibreOfficeMainActivity.mAppContext.hideSoftKeyboard();
    }

    private void handleTransitionToSelectionState(OverlayState previous) {
        if (previous == OverlayState.CURSOR) {
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.MIDDLE);
        }
    }

    private void handleSelectionState(OverlayState previous) {
        if (previous == OverlayState.TRANSITION_TO_SELECTION) {
            mTextSelection.showHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.showHandle(TextSelectionHandle.HandleType.END);
            mTextCursorLayer.showSelections();
        }
    }

    private void handleCursorState(OverlayState previous) {
        if (previous == OverlayState.CURSOR) {
            LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
        } else if (previous == OverlayState.TRANSITION_TO_CURSOR) {
            LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
            mTextSelection.showHandle(TextSelectionHandle.HandleType.MIDDLE);
            mTextCursorLayer.showCursor();
        }
    }

    private void handleTransitionToCursorState(OverlayState previous) {
        if (previous == OverlayState.SELECTION) {
            if (mTileProvider != null) {
                mTileProvider.setTextSelectionReset();
            }
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.START);
            mTextSelection.hideHandle(TextSelectionHandle.HandleType.END);
            mTextCursorLayer.hideSelections();
        }
    }

    private boolean isInvalidTransition(OverlayState previous, OverlayState next) {
        return (previous == OverlayState.CURSOR && next == OverlayState.TRANSITION_TO_CURSOR)
            || (previous == OverlayState.SELECTION && next == OverlayState.TRANSITION_TO_SELECTION);
    }

    public enum OverlayState {
        NONE,
        CURSOR,
        TRANSITION_TO_CURSOR,
        SELECTION,
        TRANSITION_TO_SELECTION
    }
}
