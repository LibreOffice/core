/*
 * Created on 31.10.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.export;

import java.util.Iterator;

import com.sun.star.wizards.web.data.CGArgument;
import com.sun.star.wizards.web.data.CGExporter;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class ConfiguredExporter extends FilterExporter {

    /* (non-Javadoc)
         * @see com.sun.star.wizards.web.export.Exporter#init(com.sun.star.wizards.web.data.CGExporter)
         */
    public void init(CGExporter exporter) {
        super.init(exporter);
        for (Iterator i = exporter.cp_Arguments.keys().iterator(); i.hasNext();) {
            Object key = i.next();
            if (!key.equals("Filter")) {
                Object value = exporter.cp_Arguments.getElement(key);
                props.put(key,cast( ((CGArgument)value).cp_Value ));
            }
        }
    }

    private Object cast(String s) {
        String s1 = s.substring(1);
        char c = s.charAt(0);
        switch (c) {
            case '$' : return s1;
            case '%' : return Integer.valueOf(s1);
            case '#' : return Short.valueOf(s1);
            case '&' : return Double.valueOf(s1);
            case 'f' : if (s.equals("false")) return Boolean.FALSE;
                    break;
            case 't' : if (s.equals("true")) return Boolean.TRUE;
                    break;
        }
        return null;
    }

}

