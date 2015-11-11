package org.libreoffice;

import android.content.Intent;
import android.graphics.PointF;
import android.graphics.RectF;
import android.net.Uri;
import android.util.Log;

import org.libreoffice.canvas.SelectionHandle;
import org.libreoffice.kit.Document;
import org.libreoffice.overlay.DocumentOverlay;
import org.mozilla.gecko.gfx.GeckoLayerClient;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Parses (interprets) and handles invalidation messages from LibreOffice.
 */
public class InvalidationHandler implements Document.MessageCallback {
    private static String LOGTAG = InvalidationHandler.class.getSimpleName();
    private final DocumentOverlay mDocumentOverlay;
    private final GeckoLayerClient mLayerClient;
    private OverlayState mState;
    private boolean mKeyEvent = false;

    public InvalidationHandler(LibreOfficeMainActivity mainActivity) {
        mDocumentOverlay = mainActivity.getDocumentOverlay();
        mLayerClient = LibreOfficeMainActivity.getLayerClient();
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
            // enable handling of hyperlinks even in the Viewer
            if (messageID != Document.CALLBACK_INVALIDATE_TILES && messageID != Document.CALLBACK_HYPERLINK_CLICKED)
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
            case Document.CALLBACK_STATE_CHANGED:
                stateChanged(payload);
                break;
            default:
                Log.d(LOGTAG, "LOK_CALLBACK uncatched: " + messageID + " : " + payload);
        }
    }

    private void stateChanged(String payload) {
        String[] parts = payload.split("=");
        if (parts.length < 2) {
            Log.e(LOGTAG, "LOK_CALLBACK_STATE_CHANGED unexpected payload: " + payload);
            return;
        }
        final String value = parts[1];
        boolean pressed = Boolean.parseBoolean(value);

        if (parts[0].equals(".uno:Bold")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.BOLD, pressed);
        } else if (parts[0].equals(".uno:Italic")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.ITALIC, pressed);
        } else if (parts[0].equals(".uno:Underline")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.UNDERLINE, pressed);
        } else if (parts[0].equals(".uno:Strikeout")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.STRIKEOUT, pressed);
        } else if (parts[0].equals(".uno:CharFontName")) {
            LOKitShell.getFontController().selectFont(value);
        } else if (parts[0].equals(".uno:FontHeight")) {
            LOKitShell.getFontController().selectFontSize(value);
        } else if (parts[0].equals(".uno:LeftPara")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.ALIGN_LEFT, pressed);
        } else if (parts[0].equals(".uno:CenterPara")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.ALIGN_CENTER, pressed);
        } else if (parts[0].equals(".uno:RightPara")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.ALIGN_RIGHT, pressed);
        } else if (parts[0].equals(".uno:JustifyPara")) {
            LOKitShell.getFormattingController().onToggleStateChanged(Document.ALIGN_JUSTIFY, pressed);
        } else {
            Log.d(LOGTAG, "LOK_CALLBACK_STATE_CHANGED type uncatched: " + payload);
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

        float dpi = LOKitShell.getDpi();

        return new RectF(
                LOKitTileProvider.twipToPixel(x, dpi),
                LOKitTileProvider.twipToPixel(y, dpi),
                LOKitTileProvider.twipToPixel(x + width, dpi),
                LOKitTileProvider.twipToPixel(y + height, dpi)
        );
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
            mDocumentOverlay.positionCursor(cursorRectangle);
            mDocumentOverlay.positionHandle(SelectionHandle.HandleType.MIDDLE, cursorRectangle);

            if (mState == OverlayState.TRANSITION || mState == OverlayState.CURSOR) {
                changeStateTo(OverlayState.CURSOR);
            }

            if (mKeyEvent) {
                moveViewportToMakeCursorVisible(cursorRectangle);
                mKeyEvent = false;
            }
        }
    }

    /**
     * Move the viewport to show the cursor. The cursor will appear at the
     * viewport position depending on where the cursor is relative to the
     * viewport (either cursor is above, below, on left or right).
     *
     * @param cursorRectangle - cursor position on the document
     */
    public void moveViewportToMakeCursorVisible(RectF cursorRectangle) {
        RectF moveToRect = mLayerClient.getViewportMetrics().getCssViewport();
        if (moveToRect.contains(cursorRectangle)) {
            return;
        }

        float newLeft = moveToRect.left;
        float newTop = moveToRect.top;

        if (cursorRectangle.right < moveToRect.left || cursorRectangle.left < moveToRect.left) {
            newLeft = cursorRectangle.left - (moveToRect.width() * 0.1f);
        } else if (cursorRectangle.right > moveToRect.right || cursorRectangle.left > moveToRect.right) {
            newLeft = cursorRectangle.right - (moveToRect.width() * 0.9f);
        }

        if (cursorRectangle.top < moveToRect.top || cursorRectangle.bottom < moveToRect.top) {
            newTop = cursorRectangle.top - (moveToRect.height() * 0.1f);
        } else if (cursorRectangle.bottom > moveToRect.bottom || cursorRectangle.top > moveToRect.bottom) {
            newTop = cursorRectangle.bottom - (moveToRect.height() / 2.0f);
        }

        LOKitShell.moveViewportTo(new PointF(newLeft, newTop), null);
    }

    /**
     * Handles the text selection start message
     *
     * @param payload
     */
    private synchronized void textSelectionStart(String payload) {
        RectF selectionRect = convertPayloadToRectangle(payload);
        if (selectionRect != null) {
            mDocumentOverlay.positionHandle(SelectionHandle.HandleType.START, selectionRect);
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
            mDocumentOverlay.positionHandle(SelectionHandle.HandleType.END, selectionRect);
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
            mDocumentOverlay.changeSelections(Collections.EMPTY_LIST);
        } else {
            List<RectF> rectangles = convertPayloadToRectangles(payload);
            if (mState != OverlayState.SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
            changeStateTo(OverlayState.SELECTION);
            mDocumentOverlay.changeSelections(rectangles);
        }
    }

    /**
     * Handles the cursor visibility message
     *
     * @param payload
     */
    private synchronized void cursorVisibility(String payload) {
        if (payload.equals("true")) {
            mDocumentOverlay.showCursor();
            if (mState != OverlayState.SELECTION) {
                mDocumentOverlay.showHandle(SelectionHandle.HandleType.MIDDLE);
            }
        } else if (payload.equals("false")) {
            mDocumentOverlay.hideCursor();
            mDocumentOverlay.hideHandle(SelectionHandle.HandleType.MIDDLE);
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
            mDocumentOverlay.changeGraphicSelection(rectangle);
            if (mState != OverlayState.GRAPHIC_SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
            changeStateTo(OverlayState.GRAPHIC_SELECTION);
        }
    }

    /**
     * Trigger a transition to a new overlay state.
     *
     * @param next - new state to transition to
     */
    public synchronized void changeStateTo(OverlayState next) {
        changeState(mState, next);
    }

    /**
     * Executes a transition from old overlay state to a new overlay state.
     *
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
        mDocumentOverlay.hideHandle(SelectionHandle.HandleType.START);
        mDocumentOverlay.hideHandle(SelectionHandle.HandleType.END);
        mDocumentOverlay.hideHandle(SelectionHandle.HandleType.MIDDLE);
        mDocumentOverlay.hideSelections();
        mDocumentOverlay.hideCursor();
        mDocumentOverlay.hideGraphicSelection();
        LibreOfficeMainActivity.mAppContext.hideSoftKeyboard();
    }

    /**
     * Handle a transition to OverlayState.SELECTION state.
     */
    private void handleSelectionState(OverlayState previous) {
        mDocumentOverlay.showHandle(SelectionHandle.HandleType.START);
        mDocumentOverlay.showHandle(SelectionHandle.HandleType.END);
        mDocumentOverlay.showSelections();
    }

    /**
     * Handle a transition to OverlayState.CURSOR state.
     */
    private void handleCursorState(OverlayState previous) {
        LibreOfficeMainActivity.mAppContext.showSoftKeyboardOrFormattingToolbar();
        if (previous == OverlayState.TRANSITION) {
            mDocumentOverlay.showHandle(SelectionHandle.HandleType.MIDDLE);
            mDocumentOverlay.showCursor();
        }
    }

    /**
     * Handle a transition to OverlayState.TRANSITION state.
     */
    private void handleTransitionState(OverlayState previous) {
        if (previous == OverlayState.SELECTION) {
            mDocumentOverlay.hideHandle(SelectionHandle.HandleType.START);
            mDocumentOverlay.hideHandle(SelectionHandle.HandleType.END);
            mDocumentOverlay.hideSelections();
        } else if (previous == OverlayState.CURSOR) {
            mDocumentOverlay.hideHandle(SelectionHandle.HandleType.MIDDLE);
        } else if (previous == OverlayState.GRAPHIC_SELECTION) {
            mDocumentOverlay.hideGraphicSelection();
        }
    }

    /**
     * Handle a transition to OverlayState.GRAPHIC_SELECTION state.
     */
    private void handleGraphicSelectionState(OverlayState previous) {
        mDocumentOverlay.showGraphicSelection();
        LibreOfficeMainActivity.mAppContext.hideSoftKeyboard();
    }

    /**
     * The current state the overlay is in.
     */
    public OverlayState getCurrentState() {
        return mState;
    }

    /**
     * A key event happened (i.e. user started typing).
     */
    public void keyEvent() {
        mKeyEvent = true;
    }

    /**
     * The states the overlay.
     */
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
