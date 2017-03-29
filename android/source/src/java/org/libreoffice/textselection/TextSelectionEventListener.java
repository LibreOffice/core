/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.textselection;


public interface TextSelectionEventListener {

    /** Notify the controller to pass the selected text to Clipboard Service
     * @param text - String to be copied to clipboard
     */
    void copySelectedText(String text);

    /** Returns if text was pasted successfully
     * @param result - String to be pasted
     */
    void onTextPaste(boolean result);

}
