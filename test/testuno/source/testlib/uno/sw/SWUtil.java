package testlib.uno.sw;

import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
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

    /**
     * Insert a bookmark into text document
     * @param document text document
     * @param textCursor which part will be bookmarked
     * @param bookmarkName bookmark name
     * @throws Exception
     */
    public static void insertBookmark(XTextDocument document, XTextCursor textCursor, String bookmarkName) throws Exception {
        XMultiServiceFactory xDocFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        Object xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark");
        XTextContent xBookmarkAsTextContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xBookmark);
        XNamed xBookmarkAsNamed = (XNamed) UnoRuntime.queryInterface(XNamed.class, xBookmark);
        xBookmarkAsNamed.setName(bookmarkName);
        document.getText().insertTextContent(textCursor, xBookmarkAsTextContent, true);
    }

}
