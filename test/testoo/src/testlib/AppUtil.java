/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testlib;
import static testlib.UIMap.*;


import java.io.File;
import java.net.MalformedURLException;

import org.openoffice.test.common.Condition;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.widgets.VclWindow;

public class AppUtil extends Tester {
    static String user_installation = Testspace.getPath("oouser"); // Use automation special user data
    static String user_installation_url = null;
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
        patch();
    }

    private static boolean isFirstInitApp = true;

    public static void initApp() {

        // kill all soffice processes to avoid testing the wrong instance
        if (isFirstInitApp) {
            isFirstInitApp = false;
            app.kill();
            sleep(2);
        }

        // Start soffice if necessary
        if (!app.exists()) {
            int code = app.start("\"-env:UserInstallation=" + user_installation_url + "\"");
            if (code != 0) {
                throw new Error("OpenOffice can't be started! Testing aborted!");
            }
            sleep(3); // this sleep is important.
            app.waitForExistence(30, 5);
        }

        boolean tryAgain = false;
        try {
            app.reset();
            if (!startcenter.exists()) {
                if (SystemUtil.isMac()) {
                    SystemUtil.execScript("osascript -e 'tell app \"OpenOffice.org\" to activate'", false);
                    typeKeys("<command n>");
                    if (!startcenter.exists())
                        tryAgain = true;

                } else {
                    tryAgain = true;
                }
            }
        } catch (Exception e) {
            tryAgain = true;
        }

        // Give the second chance
        if (tryAgain) {
            app.kill();
            sleep(2);
            int code = app.start("\"-env:UserInstallation=" + user_installation_url + "\"");
            if (code != 0) {
                throw new Error("OpenOffice can't be started! Testing aborted!");
            }
            sleep(3); // this sleep is important.
            app.waitForExistence(30, 5);
            app.reset();
        }

        openStartcenter();
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

    public static File getUserInstallationDir() {
        return new File(user_installation);
    }
    /**
     * In order to automatically test OO, some settings/files need to be modified
     */
    public static void patch() {
        File profileDir = new File(user_installation);
        try {
            user_installation_url = profileDir.toURL().toString();
        } catch (MalformedURLException e) {
            //ignore never to occur
        }
        user_installation_url = user_installation_url.replace("file:/", "file:///");
        if (profileDir.exists())
            return;

        app.kill(); // make sure no any soffice process exists
        sleep(2);
        app.start("\"-env:UserInstallation=" + user_installation_url + "\"");
        sleep(10);
        app.kill();
        sleep(2);
        FileUtil.copyFile(new File("patch/Common.xcu"), new File(user_installation, "user/registry/data/org/openoffice/Office/Common.xcu"));
        FileUtil.copyFile(new File("patch/Setup.xcu"), new File(user_installation, "user/registry/data/org/openoffice/Setup.xcu"));
        FileUtil.copyFile(new File("patch/registrymodifications.xml"), new File(user_installation, "user/registrymodifications.xcu"));

    }

//  private static final Pattern VARIABLE_PATTERN = Pattern.compile("\\$\\{([^\\}]+)\\} | \\$([^/]+)");
//
//  public static String resolveEnv(String text) {
//      Matcher matcher = VARIABLE_PATTERN.matcher(text);
//      StringBuffer result = new StringBuffer();
//      while (matcher.find()) {
//          String str = System.getenv(matcher.group(1));
//          if (str == null)
//              str = matcher.group();
//          matcher.appendReplacement(result, str.replace("\\", "\\\\")
//                  .replace("$", "\\$"));
//      }
//      matcher.appendTail(result);
//      return result.toString();
//  }

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

    public static void main(String[] args) {
//      initApp();
    }
}
