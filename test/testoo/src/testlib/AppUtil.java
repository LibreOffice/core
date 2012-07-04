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



package testlib;
import static testlib.UIMap.*;

import java.io.File;

import org.openoffice.test.common.Condition;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;
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
//      patch();
    }

    /**
     * This method is used to start OpenOffice and make it ready for testing.
     *
     * @param cleanUserInstallation if use a totally clean user installation data
     * @param userInstallation Specify user installation directory. If it's null, the default will be used.
     */
    public static void initApp(boolean cleanUserInstallation, String userInstallation) {
        File newUserInstallation = userInstallation == null ? app.getDefaultUserInstallation() : new File(fullPath(userInstallation));
        if (!newUserInstallation.equals(app.getUserInstallation())) {
            // user installation changed...
            app.kill();
            app.setUserInstallation(userInstallation == null ? null : newUserInstallation);
        }

        patch(cleanUserInstallation);

        //try to reset application
        for (int i = 0; i < 3; i++) {
            try {
                if (app.exists()) {
                    app.reset();
                    openStartcenter();
                    if (startcenter.exists(2))
                        return;
                }
            } catch (Exception e){

            }

            app.kill();
            if (app.start() != 0)
                throw new Error("OpenOffice can't be started! Testing aborted!");
            sleep(3); // this sleep is important.
            app.waitForExistence(30, 5);
        }
    }

    /**
     * @see initApp(boolean cleanUserInstallation, String userInstallation)
     */
    public static void initApp(boolean cleanUserInstallation) {
        initApp(cleanUserInstallation, System.getProperty("openoffice.userinstallation"));
    }

    /**
     * @see initApp(boolean cleanUserInstallation, String userInstallation)
     */
    public static void initApp() {
        initApp(false);
    }

    public static void openStartcenter() {
        if (startcenter.exists())
            return;

        if (SystemUtil.isMac()) {
            SystemUtil.execScript("osascript -e 'tell app \"OpenOffice.org\" to activate'", false);
            typeKeys("<command n>");
        }

    }


    public static void submitOpenDlg(String path) {
        FilePicker_Path.setText(path);
        FilePicker_Open.click();
        sleep(1);
    }

    /**
     * Fetch a test file to test space
     * @param path
     * @return a absolute path on the local
     */
    public static String testFile(String path) {
        File source = new File("data", path);
        File target = Testspace.getFile("data/" + path);
        FileUtil.copyFile(source, target);
        return target.getAbsolutePath();
    }

    public static String fullPath(String relativePath) {
        File file = new File(relativePath);
        if (file.isAbsolute())
            return relativePath;
        return Testspace.getPath(relativePath);
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

    /**
     * In order to automatically test OO, some settings/files need to be modified
     */
    public static void patch(boolean force) {
        File userInstallationDir = app.getUserInstallation();
        File patchMark = new File(userInstallationDir, "automationenabled");
        if (!force && patchMark.exists())
            return;

        // remove user installation dir
        app.kill();
        sleep(1);
        FileUtil.deleteFile(userInstallationDir);
        app.start();
        sleep(10);
        app.kill();
        sleep(1);

        FileUtil.copyFile(new File("patch/Common.xcu"), new File(userInstallationDir, "user/registry/data/org/openoffice/Office/Common.xcu"));
        FileUtil.copyFile(new File("patch/Setup.xcu"), new File(userInstallationDir, "user/registry/data/org/openoffice/Setup.xcu"));
        FileUtil.copyFile(new File("patch/registrymodifications.xml"), new File(userInstallationDir, "user/registrymodifications.xcu"));
        FileUtil.writeStringToFile(patchMark.getAbsolutePath(), "patched for automation");
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

                for(VclWindow w : windows) {
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

        }.waitForTrue("Time out wait window to be active.",  120, 2);
    }
}
