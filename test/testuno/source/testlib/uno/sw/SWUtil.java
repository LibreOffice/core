package testlib.uno.sw;

import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

public class SWUtil {

    private static final UnoApp app = new UnoApp();
    public static void saveAsDoc(XTextDocument document, String url) {
        saveAs(document, "MS Word 97", url);

    }

    public static void saveAsODT(XTextDocument document, String url) {
        saveAs(document, "writer8", url);
    }

    public static void saveAs(XTextDocument document, String filterValue, String url) {
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);

        PropertyValue[] propsValue = new PropertyValue[1];
        propsValue[0] = new PropertyValue();
        propsValue[0].Name = "FilterName";
        propsValue[0].Value = filterValue;
        try {
            store.storeAsURL(url, propsValue);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static XTextDocument newDocument() {
        try {
            return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
