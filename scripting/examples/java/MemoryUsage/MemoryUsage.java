import java.util.Random;
import java.util.Date;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.lang.XComponent;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.awt.ActionEvent;

import com.sun.star.script.provider.XScriptContext;

public class MemoryUsage {

    public void updateMemoryUsage(XScriptContext ctxt, ActionEvent evt)
        throws Exception {

        Runtime runtime = Runtime.getRuntime();
        Random generator = new Random();
        Date date = new Date();

        int len = (int)(generator.nextFloat() * runtime.freeMemory() / 5);
        byte[] bytes = new byte[len];
        addEntry(ctxt, date.toString(), runtime.totalMemory(), runtime.freeMemory());
    }

    private void addEntry(XScriptContext ctxt, String date, long total, long free)
        throws Exception {

        XComponent comp = ctxt.getDocument();

        XSpreadsheetDocument doc = (XSpreadsheetDocument)
            UnoRuntime.queryInterface(XSpreadsheetDocument.class, comp);

        XIndexAccess index = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, doc.getSheets());

        XSpreadsheet sheet = (XSpreadsheet) AnyConverter.toObject(
            new Type(com.sun.star.sheet.XSpreadsheet.class), index.getByIndex(0));

        sheet.getCellByPosition(0, 1).setValue(total - free);
        sheet.getCellByPosition(1, 1).setValue(free);
        sheet.getCellByPosition(2, 1).setValue(total);
        sheet.getCellByPosition(0, 2).setFormula(date);
    }
}
