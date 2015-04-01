package org.libreoffice;

import android.content.Intent;
import android.graphics.RectF;
import android.net.Uri;

import org.libreoffice.canvas.SelectionHandle;
import org.libreoffice.kit.Document;
import org.libreoffice.overlay.TextCursorLayer;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Parses (interprets) and handles invalidation messages from LibreOffice.
 */
public class InvalidationHandler implements Document.MessageCallback {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();
    private final TextCursorLayer mTextCursorLayer;
    private OverlayState mState;

    public InvalidationHandler(LibreOfficeMainActivity mainActivity) {
        mTextCursorLayer = mainActivity.getTextCursorLayer();
        mState = OverlayState.NONE;
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
            case Document.CALLBACK_GRAPHIC_SELECTION:
                graphicSelection(payload);
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

        int x = Integer.decode(coordinates[0]);
        int y = Integer.decode(coordinates[1]);
        int width = Integer.decode(coordinates[2]);
        int height = Integer.decode(coordinates[3]);

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
            mTextCursorLayer.positionCursor(cursorRectangle);
            mTextCursorLayer.positionHandle(SelectionHandle.HandleType.MIDDLE, cursorRectangle);

            if (mState == OverlayState.TRANSITION || mState == OverlayState.CURSOR) {
                changeStateTo(OverlayState.CURSOR);
            }
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
            mTextCursorLayer.positionHandle(SelectionHandle.HandleType.START, selectionRect);
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
            mTextCursorLayer.positionHandle(SelectionHandle.HandleType.END, selectionRect);
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
                changeStateTo(OverlayState.TRANSITION);
            }
            mTextCursorLayer.changeSelections(Collections.EMPTY_LIST);
        } else {
            List<RectF> rectangles = convertPayloadToRectangles(payload);
            if (mState != OverlayState.SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
            changeStateTo(OverlayState.SELECTION);
            mTextCursorLayer.changeSelections(rectangles);
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
            if (mState != OverlayState.SELECTION) {
                mTextCursorLayer.showHandle(SelectionHandle.HandleType.MIDDLE);
            }
        } else if (payload.equals("false")) {
            mTextCursorLayer.hideCursor();
            mTextCursorLayer.hideHandle(SelectionHandle.HandleType.MIDDLE);
        }
    }

    /**
     * Handles the graphic selection change message
     *
     * @param payload
     */
    private void graphicSelection(String payload) {
        if (payload.isEmpty() || payload.equals("EMPTY")) {
            if (mState == OverlayState.GRAPHIC_SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
        } else {
            RectF rectangle = convertPayloadToRectangle(payload);
            mTextCursorLayer.changeGraphicSelection(rectangle);
            if (mState != OverlayState.GRAPHIC_SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
            changeStateTo(OverlayState.GRAPHIC_SELECTION);
        }
    }

    /**
     * Trigger a transition to a new overlay state.
     * @param next - new state to transition to
     */
    public synchronized void changeStateTo(OverlayState next) {
        changeState(mState, next);
    }

    /**
     * Executes a transition from old overlay state to a new overlay state.
     * @param previous - old state
     * @param next - new state
     */
    private synchronized void changeState(OverlayState previous, OverlayState next) {
        mState = next;
        handleGeneralChangeState(previous, next);
        switch (next) {
            case CURSOR:
                handleCursorState(previous);
                break;
            case SELECTION:
                handleSelectionState(previous);
                break;
            case GRAPHIC_SELECTION:
                handleGraphicSelectionState(previous);
                break;
            case TRANSITION:
                handleTransitionState(previous);
                break;
            case NONE:
                handleNoneState(previous);
                break;
        }
    }

    /**
     * Handle a general transition - executed for all transitions.
     */
    private void handleGeneralChangeState(OverlayState previous, OverlayState next) {
        if (previous == OverlayState.NONE) {
            LOKitShell.getToolbarController().switchToEditMode();
        } else if (next == OverlayState.NONE) {
            LOKitShell.getToolbarController().switchToViewMode();
        }
    }

    /**
     * Handle a transition to OverlayState.NONE state.
     */
    private void handleNoneState(OverlayState previous) {
        if (previous == OverlayState.NONE) {
            return;
        }

        // Just hide everything
        mTextCursorLayer.hideHandle(SelectionHandle.HandleType.START);
        mTextCursorLayer.hideHandle(SelectionHandle.HandleType.END);
        mTextCursorLayer.hideHandle(SelectionHandle.HandleType.MIDDLE);
        mTextCursorLayer.hideSelections();
        mTextCursorLayer.hideCursor();
        mTextCursorLayer.hideGraphicSelection();
        LibreOfficeMainActivity.mAppContext.hideSoftKeyboard();
    }

    /**
     * Handle a transition to OverlayState.SELECTION state.
     */
    private void handleSelectionState(OverlayState previous) {
        mTextCursorLayer.showHandle(SelectionHandle.HandleType.START);
        mTextCursorLayer.showHandle(SelectionHandle.HandleType.END);
        mTextCursorLayer.showSelections();
    }

    /**
     * Handle a transition to OverlayState.CURSOR state.
     */
    private void handleCursorState(OverlayState previous) {
        LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
        if (previous == OverlayState.TRANSITION) {
            mTextCursorLayer.showHandle(SelectionHandle.HandleType.MIDDLE);
            mTextCursorLayer.showCursor();
        }
    }

    /**
     * Handle a transition to OverlayState.TRANSITION state.
     */
    private void handleTransitionState(OverlayState previous) {
        if (previous == OverlayState.SELECTION) {
            mTextCursorLayer.hideHandle(SelectionHandle.HandleType.START);
            mTextCursorLayer.hideHandle(SelectionHandle.HandleType.END);
            mTextCursorLayer.hideSelections();
        } else if (previous == OverlayState.CURSOR) {
            mTextCursorLayer.hideHandle(SelectionHandle.HandleType.MIDDLE);
        } else if (previous == OverlayState.GRAPHIC_SELECTION) {
            mTextCursorLayer.hideGraphicSelection();
        }
    }

    /**
     * Handle a transition to OverlayState.GRAPHIC_SELECTION state.
     */
    private void handleGraphicSelectionState(OverlayState previous) {
        mTextCursorLayer.showGraphicSelection();
        LibreOfficeMainActivity.mAppContext.hideSoftKeyboard();
    }

    /**
     * The current state the overlay is in.
     */
    public OverlayState getCurrentState() {
        return mState;
    }

    public enum OverlayState {
        /**
         * State where the overlay is empty
         */
        NONE,
        /**
         * In-between state where we need to transition to a new overlay state.
         * In this state we properly disable the older state and wait to transition
         * to a new state triggered by an invalidation.
         */
        TRANSITION,
        /**
         * State where we operate with the cursor.
         */
        CURSOR,
        /**
         * State where we operate the graphic selection.
         */
        GRAPHIC_SELECTION,
        /**
         * State where we operate the text selection.
         */
        SELECTION
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
