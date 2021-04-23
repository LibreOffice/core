package org.libreoffice;

import android.content.Intent;
import android.graphics.PointF;
import android.graphics.RectF;
import android.net.Uri;
import android.util.Log;
import android.widget.EditText;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.canvas.SelectionHandle;
import org.libreoffice.kit.Document;
import org.libreoffice.kit.Office;
import org.libreoffice.overlay.DocumentOverlay;
import org.mozilla.gecko.gfx.GeckoLayerClient;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Parses (interprets) and handles invalidation messages from LibreOffice.
 */
public class InvalidationHandler implements Document.MessageCallback, Office.MessageCallback {
    private static final String LOGTAG = InvalidationHandler.class.getSimpleName();
    private final DocumentOverlay mDocumentOverlay;
    private final GeckoLayerClient mLayerClient;
    private OverlayState mState;
    private boolean mKeyEvent = false;
    private final LibreOfficeMainActivity mContext;

    private int currentTotalPageNumber = 0; // total page number of the current document

    public InvalidationHandler(LibreOfficeMainActivity context) {
        mContext = context;
        mDocumentOverlay = mContext.getDocumentOverlay();
        mLayerClient = mContext.getLayerClient();
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
            // enable handling of hyperlinks and search result even in the Viewer
            if (messageID != Document.CALLBACK_INVALIDATE_TILES
                    && messageID != Document.CALLBACK_DOCUMENT_PASSWORD
                    && messageID != Document.CALLBACK_HYPERLINK_CLICKED
                    && messageID != Document.CALLBACK_SEARCH_RESULT_SELECTION
                    && messageID != Document.CALLBACK_TEXT_SELECTION
                    && messageID != Document.CALLBACK_TEXT_SELECTION_START
                    && messageID != Document.CALLBACK_TEXT_SELECTION_END)
                return;
        }
        switch (messageID) {
            case Document.CALLBACK_INVALIDATE_TILES:
                invalidateTiles(payload);
                break;
            case Document.CALLBACK_UNO_COMMAND_RESULT:
                unoCommandResult(payload);
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
                mContext.startActivity(urlIntent);
                break;
            case Document.CALLBACK_STATE_CHANGED:
                stateChanged(payload);
                break;
            case Document.CALLBACK_SEARCH_RESULT_SELECTION:
                searchResultSelection(payload);
                // when doing a search, CALLBACK_SEARCH_RESULT_SELECTION is called in addition
                // to the CALLBACK_TEXT_SELECTION{,_START,_END} callbacks and the handling of
                // the previous 3 makes the cursor shown in addition to the selection rectangle,
                // so hide the cursor again to just show the selection rectangle for the search result
                mDocumentOverlay.hideCursor();
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.MIDDLE);
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.START);
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.END);
                break;
            case Document.CALLBACK_SEARCH_NOT_FOUND:
                Log.d(LOGTAG, "LOK_CALLBACK: Search not found.");
                // this callback is never caught. Hope someone fix this.
                break;
            case Document.CALLBACK_CELL_CURSOR:
                invalidateCellCursor(payload);
                break;
            case Document.CALLBACK_INVALIDATE_HEADER:
                invalidateHeader();
                break;
            case Document.CALLBACK_CELL_ADDRESS:
                cellAddress(payload);
                break;
            case Document.CALLBACK_CELL_FORMULA:
                cellFormula(payload);
                break;
            case Document.CALLBACK_DOCUMENT_PASSWORD:
                documentPassword();
                break;
            case Document.CALLBACK_DOCUMENT_SIZE_CHANGED:
                pageSizeChanged(payload);
            default:

                Log.d(LOGTAG, "LOK_CALLBACK uncaught: " + messageID + " : " + payload);
        }
    }

    private void unoCommandResult(String payload) {
        try {
            JSONObject payloadObject = new JSONObject(payload);
            if (payloadObject.getString("commandName").equals(".uno:Save")) {
                if (payloadObject.getString("success").equals("true")) {
                    mContext.saveFileToOriginalSource();
                }
            }else if(payloadObject.getString("commandName").equals(".uno:Name") ||
                    payloadObject.getString("commandName").equals(".uno:RenamePage")){
                //success returns false even though its true for some reason,
                LOKitShell.getMainHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        mContext.getTileProvider().resetParts();
                        mContext.getDocumentPartViewListAdapter().notifyDataSetChanged();
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        Toast.makeText(mContext, mContext.getString(R.string.part_name_changed), Toast.LENGTH_SHORT).show();
                    }
                });
            } else if(payloadObject.getString("commandName").equals(".uno:Remove") ||
                    payloadObject.getString("commandName").equals(".uno:DeletePage") ) {
                LOKitShell.getMainHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        mContext.getTileProvider().resetParts();
                        mContext.getDocumentPartViewListAdapter().notifyDataSetChanged();
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        Toast.makeText(mContext, mContext.getString(R.string.part_deleted), Toast.LENGTH_SHORT).show();
                    }
                });
            }
        }catch(JSONException e){
            e.printStackTrace();
        }
    }

    private void cellFormula(final String payload) {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                ((EditText)mContext.findViewById(R.id.calc_formula)).setText(payload);
            }
        });
    }

    private void cellAddress(final String payload) {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                ((EditText)mContext.findViewById(R.id.calc_address)).setText(payload);
            }
        });
    }

    private void invalidateHeader() {
        LOKitShell.sendEvent(new LOEvent(LOEvent.UPDATE_CALC_HEADERS));
    }

    private void documentPassword() {
        mContext.setPasswordProtected(true);
        mContext.promptForPassword();
        synchronized (this) {
            try {
                this.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        mContext.setPassword();
    }

    private void invalidateCellCursor(String payload) {
        RectF cellCursorRect = convertPayloadToRectangle(payload);

        if (cellCursorRect != null) {
            mDocumentOverlay.showCellSelection(cellCursorRect);
            moveViewportToMakeSelectionVisible(cellCursorRect);
        }
    }

    /**
     * Handles the search result selection message, which is a JSONObject
     *
     * @param payload
     */
    private void searchResultSelection(String payload) {
        RectF selectionRectangle = null;
        try {
            JSONObject collectiveResult = new JSONObject(payload);
            JSONArray searchResult = collectiveResult.getJSONArray("searchResultSelection");
            if (searchResult.length() == 1) {
                String rectangle = searchResult.getJSONObject(0).getString("rectangles");
                selectionRectangle = convertPayloadToRectangle(rectangle);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        if (selectionRectangle != null) {
            moveViewportToMakeSelectionVisible(selectionRectangle);
        }
    }

    /**
     * Move the viewport to show the selection. The selection will appear at the
     * viewport position depending on where the selection is relative to the
     * viewport (either selection is above, below, on left or right). The difference
     * between this method and moveViewportToMakeCursorVisible() is that this method
     * takes into account the width and height of the selection and zooms out
     * accordingly.
     *
     * @param selectionRectangle - selection position on the document
     */
    public void moveViewportToMakeSelectionVisible(RectF selectionRectangle) {
        RectF moveToRect = mLayerClient.getViewportMetrics().getCssViewport();
        if (moveToRect.contains(selectionRectangle)) {
            return;
        }

        float newLeft = moveToRect.left;
        float newTop = moveToRect.top;

        // if selection rectangle is wider or taller than current viewport, we need to zoom out
        float oldZoom = mLayerClient.getViewportMetrics().getZoomFactor();
        float widthRatio = 1f;
        float heightRatio = 1f;
        if (moveToRect.width() < selectionRectangle.width()) {
            widthRatio = selectionRectangle.width() / moveToRect.width() / 0.85f; // 0.85f gives some margin (must < 0.9)
        }
        if (moveToRect.height() < selectionRectangle.height()) {
            heightRatio = selectionRectangle.height() / moveToRect.height() / 0.45f; // 0.45f gives some margin (must < 0.5)
        }
        float newZoom = widthRatio > heightRatio ? oldZoom/widthRatio : oldZoom/heightRatio;

        // if selection is out of viewport we need to adjust accordingly
        if (selectionRectangle.right < moveToRect.left || selectionRectangle.left < moveToRect.left) {
            newLeft = selectionRectangle.left - (moveToRect.width() * 0.1f) * oldZoom / newZoom; // 0.1f gives left margin
        } else if (selectionRectangle.right > moveToRect.right || selectionRectangle.left > moveToRect.right) {
            newLeft = selectionRectangle.right - (moveToRect.width() * 0.9f) * oldZoom / newZoom; // 0.9f gives right margin
        }

        if (selectionRectangle.top < moveToRect.top || selectionRectangle.bottom < moveToRect.top) {
            newTop = selectionRectangle.top - (moveToRect.height() * 0.1f) * oldZoom / newZoom; // 0.1f gives top margin
        } else if (selectionRectangle.bottom > moveToRect.bottom || selectionRectangle.top > moveToRect.bottom){
            newTop = selectionRectangle.bottom - (moveToRect.height() * 0.5f) * oldZoom / newZoom; // 0.5 f gives bottom margin
        }

        LOKitShell.moveViewportTo(mContext, new PointF(newLeft, newTop), newZoom);
    }

    private void pageSizeChanged(String payload){
        if(mContext.getTileProvider().isTextDocument()){
            String[] bounds = payload.split(",");
            int pageWidth = Integer.parseInt(bounds[0]);
            int pageHeight = Integer.parseInt(bounds[1].trim());
            LOKitShell.sendEvent(new LOEvent(LOEvent.PAGE_SIZE_CHANGED, pageWidth, pageHeight));
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
        if (!mContext.getTileProvider().isReady()) {
            Log.w(LOGTAG, "tile provider not ready, ignoring payload "+payload);
            return;
        }
        if (parts[0].equals(".uno:Bold")) {
            mContext.getFormattingController().onToggleStateChanged(Document.BOLD, pressed);
        } else if (parts[0].equals(".uno:Italic")) {
            mContext.getFormattingController().onToggleStateChanged(Document.ITALIC, pressed);
        } else if (parts[0].equals(".uno:Underline")) {
            mContext.getFormattingController().onToggleStateChanged(Document.UNDERLINE, pressed);
        } else if (parts[0].equals(".uno:Strikeout")) {
            mContext.getFormattingController().onToggleStateChanged(Document.STRIKEOUT, pressed);
        } else if (parts[0].equals(".uno:CharFontName")) {
            mContext.getFontController().selectFont(value);
        } else if (parts[0].equals(".uno:FontHeight")) {
            mContext.getFontController().selectFontSize(value);
        } else if (parts[0].equals(".uno:LeftPara")) {
            mContext.getFormattingController().onToggleStateChanged(Document.ALIGN_LEFT, pressed);
        } else if (parts[0].equals(".uno:CenterPara")) {
            mContext.getFormattingController().onToggleStateChanged(Document.ALIGN_CENTER, pressed);
        } else if (parts[0].equals(".uno:RightPara")) {
            mContext.getFormattingController().onToggleStateChanged(Document.ALIGN_RIGHT, pressed);
        } else if (parts[0].equals(".uno:JustifyPara")) {
            mContext.getFormattingController().onToggleStateChanged(Document.ALIGN_JUSTIFY, pressed);
        } else if (parts[0].equals(".uno:DefaultBullet")) {
            mContext.getFormattingController().onToggleStateChanged(Document.BULLET_LIST, pressed);
        } else if (parts[0].equals(".uno:DefaultNumbering")) {
            mContext.getFormattingController().onToggleStateChanged(Document.NUMBERED_LIST, pressed);
        } else if (parts[0].equals(".uno:Color")) {
            mContext.getFontController().colorPaletteListener.updateColorPickerPosition(Integer.parseInt(value));
        } else if (mContext.getTileProvider().isTextDocument() && parts[0].equals(".uno:BackColor")) {
            mContext.getFontController().backColorPaletteListener.updateColorPickerPosition(Integer.parseInt(value));
        } else if (mContext.getTileProvider().isPresentation() && parts[0].equals(".uno:CharBackColor")) {
            mContext.getFontController().backColorPaletteListener.updateColorPickerPosition(Integer.parseInt(value));
        } else if (mContext.getTileProvider().isSpreadsheet() && parts[0].equals(".uno:BackgroundColor")) {
            mContext.getFontController().backColorPaletteListener.updateColorPickerPosition(Integer.parseInt(value));
        } else if (parts[0].equals(".uno:StatePageNumber")) {
            // get the total page number and compare to the current value and update accordingly
            String[] splitStrings = parts[1].split(" ");
            int totalPageNumber = Integer.valueOf(splitStrings[splitStrings.length - 1]);
            if (totalPageNumber != currentTotalPageNumber) {
                currentTotalPageNumber = totalPageNumber;
                // update part page rectangles stored in DocumentOverlayView object
                LOKitShell.sendEvent(new LOEvent(LOEvent.UPDATE_PART_PAGE_RECT));
            }
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
    public RectF convertPayloadToRectangle(String payload) {
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

        float dpi = LOKitShell.getDpi(mContext);

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
    public List<RectF> convertPayloadToRectangles(String payload) {
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

        LOKitShell.moveViewportTo(mContext, new PointF(newLeft, newTop), null);
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
            mDocumentOverlay.changeSelections(Collections.<RectF>emptyList());
            if (mContext.getTileProvider().isSpreadsheet()) {
                mDocumentOverlay.showHeaderSelection(null);
            }
            mContext.getToolbarController().showHideClipboardCutAndCopy(false);
        } else {
            List<RectF> rectangles = convertPayloadToRectangles(payload);
            if (mState != OverlayState.SELECTION) {
                changeStateTo(OverlayState.TRANSITION);
            }
            changeStateTo(OverlayState.SELECTION);
            mDocumentOverlay.changeSelections(rectangles);
            if (mContext.getTileProvider().isSpreadsheet()) {
                mDocumentOverlay.showHeaderSelection(rectangles.get(0));
            }
            String selectedText = mContext.getTileProvider().getTextSelection("");
            mContext.getToolbarController().showClipboardActions(selectedText);
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
        if (previous == OverlayState.NONE &&
                !mContext.getToolbarController().getEditModeStatus()) {
            mContext.getToolbarController().switchToEditMode();
        } else if (next == OverlayState.NONE &&
                mContext.getToolbarController().getEditModeStatus()) {
            mContext.getToolbarController().switchToViewMode();
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
        mContext.hideSoftKeyboard();
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
        mContext.showSoftKeyboardOrFormattingToolbar();
        if (previous == OverlayState.TRANSITION) {
            mDocumentOverlay.showHandle(SelectionHandle.HandleType.MIDDLE);
            mDocumentOverlay.showCursor();
        }
    }

    /**
     * Handle a transition to OverlayState.TRANSITION state.
     */
    private void handleTransitionState(OverlayState previous) {
        switch (previous) {
            case SELECTION:
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.START);
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.END);
                mDocumentOverlay.hideSelections();
                break;
            case CURSOR:
                mDocumentOverlay.hideHandle(SelectionHandle.HandleType.MIDDLE);
                break;
            case GRAPHIC_SELECTION:
                mDocumentOverlay.hideGraphicSelection();
                break;
        }
    }

    /**
     * Handle a transition to OverlayState.GRAPHIC_SELECTION state.
     */
    private void handleGraphicSelectionState(OverlayState previous) {
        mDocumentOverlay.showGraphicSelection();
        mContext.hideSoftKeyboard();
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
