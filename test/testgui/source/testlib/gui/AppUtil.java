/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package testlib.gui;

import static testlib.gui.UIMap.*;

import org.openoffice.test.common.Condition;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.widgets.VclWindow;

public class AppUtil extends Tester {
    static {
        Testspace.getFile("temp").mkdirs();
        // TODO move these shortcut into a file
        setCustomizedShortcut("copy", "ctrl", "c");
        setCustomizedShortcut("select_all", "ctrl", "a");
        setCustomizedShortcut("paste", "ctrl", "v");
        setCustomizedShortcut("cut", "ctrl", "x");
        setCustomizedShortcut("startcenter", "ctrl", "n");
        setCustomizedShortcut("find", "ctrl", "f");
        setCustomizedShortcut("undo", "ctrl", "z");
        setCustomizedShortcut("redo", "ctrl", "y");
        if (SystemUtil.isMac()) {
            setCustomizedShortcut("copy", "command", "c");
            setCustomizedShortcut("select_all", "command", "a");
            setCustomizedShortcut("paste", "command", "v");
            setCustomizedShortcut("cut", "command", "x");
            setCustomizedShortcut("startcenter", "command", "n");
            setCustomizedShortcut("find", "command", "f");
            setCustomizedShortcut("undo", "command", "z");
            setCustomizedShortcut("redo", "command", "shift", "z");
        } else if (SystemUtil.isLinux()) {

        }
    }

    public static void typeKeys(String keys) {
        Tester.typeKeys(keys);
    }

    public static void openStartcenter() {
        if (startcenter.exists())
            return;

        if (SystemUtil.isMac()) {
            SystemUtil.execScript("osascript -e 'tell app \"OpenOffice.org\" to activate'");
            typeKeys("<command n>");
        }

    }

    public static void submitOpenDlg(String path) {
        FilePicker_Path.setText(path);
        FilePicker_Open.click();
        sleep(1);
    }

    public static void submitSaveDlg(String path) {
        FileSave_Path.setText(path);

        String extName = FileUtil.getFileExtName(path).toLowerCase();

        String[] filters = FileSave_FileType.getItemsText();
        int i = 0;
        for (; i < filters.length; i++) {
            String f = filters[i];
            int dotIndex = f.lastIndexOf(".");
            if (dotIndex == -1)
                continue;
            if (extName.equals(f.substring(dotIndex + 1, f.length() - 1)))
                break;
        }
        if (i == filters.length)
            throw new RuntimeException("Can't find the supported doc format!");

        FileSave_FileType.select(i);
        FileSave_Save.click();
        sleep(1);
    }

    public static void submitSaveDlg(String path, String ext) {
        FileSave_Path.setText(path);
        if (ext != null) {
            // change filter
            String[] filters = FileSave_FileType.getItemsText();
            int i = 0;
            for (; i < filters.length; i++) {
                String f = filters[i];
                int dotIndex = f.lastIndexOf(".");
                if (dotIndex == -1)
                    continue;
                if (ext.equals(f.substring(dotIndex + 1, f.length() - 1)))
                    break;
            }
            if (i == filters.length)
                throw new RuntimeException("Can't find the supported doc format!");
        }
        FileSave_FileType.click();
        sleep(1);
    }

    public static void handleBlocker(final VclWindow... windows) {
        new Condition() {
            @Override
            public boolean value() {
                if (ActiveMsgBox.exists()) {
                    try {
                        ActiveMsgBox.ok();
                    } catch (Exception e) {
                        try {
                            ActiveMsgBox.yes();
                        } catch (Exception e1) {
                        }
                    }
                }

                boolean shown = false;

                for (VclWindow w : windows) {
                    if (w.exists()) {
                        shown = true;
                        break;
                    }
                }

                if (!shown)
                    return false;

                if (ActiveMsgBox.exists(2)) {
                    try {
                        ActiveMsgBox.ok();
                    } catch (Exception e) {
                        try {
                            ActiveMsgBox.yes();
                        } catch (Exception e1) {
                        }
                    }
                }

                return true;
            }

        }.waitForTrue("Time out wait window to be active.", 120, 2);
    }
}
