/*
 * Created on 04.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.status;

import java.util.List;
import java.util.Vector;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class ErrorLog implements ErrorReporter {

    private List errors = new Vector();

    public void error(Exception ex, Object obj, String s) {
      errors.add(new Err(ex,obj,s));
    }

    public class Err {
        public Exception exception;
        public Object argument;
        public String string;

        public Err(Exception ex, Object arg, String s) {
            exception = ex;
            argument = arg;
            string = s;
        }

    }



}
