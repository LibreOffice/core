import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLDecoder;

import com.sun.star.uno.XComponentContext;
import com.sun.star.script.framework.provider.PathUtils;
import drafts.com.sun.star.script.framework.runtime.XScriptContext;

public class DebugRunner {

    private static final String FILE_URL_PREFIX =
            System.getProperty("os.name").startsWith("Windows") == true ?
            "file:///" : "file://";

    public void go(final XScriptContext xsctxt, String language, String uri,
        String filename) {

        OOScriptDebugger debugger;
        String path = "";

        if (language.equals("JavaScript")) {
            debugger = new OORhinoDebugger();
        }
        else if (language.equals("BeanShell")) {
            debugger = new OOBeanShellDebugger();
        }
        else {
            return;
        }

        if (uri.startsWith(FILE_URL_PREFIX)) {
            uri = URLDecoder.decode(uri);
            String s = uri.substring(FILE_URL_PREFIX.length());
            File f = new File(s);

            if (f.exists()) {
                if (f.isDirectory()) {
                    if (!filename.equals("")) {
                        path = new File(f, filename).getAbsolutePath();
                    }
                }
                else {
                    path = f.getAbsolutePath();
                }
            }
            debugger.go(xsctxt, path);
        }
        else {
            if (!uri.endsWith("/")) {
                uri += "/";
            }

            String script = uri + filename;
            InputStream is;

            try {
                is = PathUtils.getScriptFileStream(
                    script, xsctxt.getComponentContext());

                if (is != null) {
                    debugger.go(xsctxt, is);
                }
            }
            catch (IOException ioe) {
                System.out.println("Error loading script: " + script);
            }
        }
    }
}
