package testlib.uno.sw;

import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

public class SWUtil {

    private static final UnoApp app = new UnoApp();
    public static void saveAsDoc(XTextDocument document, String url) throws Exception {
        saveAs(document, "MS Word 97", url);

    }

    public static void saveAsODT(XTextDocument document, String url) throws Exception {
        saveAs(document, "writer8", url);
    }

    public static void saveAs(XTextDocument document, String filterValue, String url) throws Exception {
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);

        PropertyValue[] propsValue = new PropertyValue[1];
        propsValue[0] = new PropertyValue();
        propsValue[0].Name = "FilterName";
        propsValue[0].Value = filterValue;
        store.storeAsURL(url, propsValue);

    }

    public static XTextDocument newDocument() throws Exception {
        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));

    }
}
