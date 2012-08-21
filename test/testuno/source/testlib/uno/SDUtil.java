/**
 *
 */
package testlib.uno;

import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
 *
 *
 */
public class SDUtil {

    private SDUtil() {

    }

    public static Object getPageByIndex(XComponent doc, int index) throws Exception {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(XDrawPagesSupplier.class, doc);
        Object drawPages = xDrawPagesSupplier.getDrawPages();
        XIndexAccess xIndexedDrawPages = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, drawPages);
        return xIndexedDrawPages.getByIndex(index);
    }

    public static Object getShapeOfPageByIndex(Object page, int index) throws Exception {
        XDrawPage xDrawPage = (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class, page);
        XShapes m_xdrawShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xDrawPage);
        return m_xdrawShapes.getByIndex(index);
    }

}
