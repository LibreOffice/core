import java.io.*;
import java.net.URL;
import java.net.URLDecoder;
import drafts.com.sun.star.script.framework.runtime.XScriptContext;

public class DebugRunner {

    private static final String FILE_URL_PREFIX =
            System.getProperty("os.name").startsWith("Windows") == true ?
            "file:///" : "file://";

    public void go(final XScriptContext xsctxt, String language, String uri,
        String filename) {

        OOScriptDebugger debugger;
        InputStream is = null;
        String path = "";

        System.out.println("uri: " + uri + ", language: " + language);

        if (language.equals("Rhino"))
            debugger = new OORhinoDebugger();
        else if (language.equals("BeanShell"))
            debugger = new OOBeanShellDebugger();
        else
            return;

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
        }
        else if (uri.startsWith("http://")) {
            try {
                if (!filename.equals(""))
                    uri = uri + "/" + filename;

                URL url = new URL(uri);
                is = url.openStream();
            }
            catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
        System.out.println("path: " + path);
        debugger.go(xsctxt, path);
    }
}
