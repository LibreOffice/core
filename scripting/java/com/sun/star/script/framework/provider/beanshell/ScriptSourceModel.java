/*************************************************************************
*
*  $RCSfile: ScriptSourceModel.java,v $
*
*  $Revision: 1.4 $
*
*  last change: $Author: hr $ $Date: 2004-07-23 14:03:38 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*
************************************************************************/
package com.sun.star.script.framework.provider.beanshell;

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import java.net.URL;

import java.lang.reflect.InvocationTargetException;
import drafts.com.sun.star.script.provider.XScriptContext;

public class ScriptSourceModel {

    private int currentPosition = -1;
    private URL file = null;
    private ScriptSourceView view = null;

    public ScriptSourceModel(URL file ) {
        this.file = file;
    }

    private String load() throws IOException {
        StringBuffer buf = new StringBuffer();
        InputStream in = file.openStream();

        byte[] contents = new byte[1024];
        int len = 0;

        while ((len = in.read(contents, 0, 1024)) != -1) {
            buf.append(new String(contents, 0, len));
        }

        try {
            in.close();
        }
        catch (IOException ignore) {
        }

        return buf.toString();
    }

    public String getText() {
        String result = "";

        try {
            result = load();
        }
        catch (IOException ioe) {
            // do nothing, empty string will be returned
        }

        return result;
    }

    public int getCurrentPosition() {
        return this.currentPosition;
    }

    public void setView(ScriptSourceView view) {
        this.view = view;
    }

    public boolean isModified() {
        return false;
    }

    public Object execute(final XScriptContext context, ClassLoader cl )
        throws Exception
    {
        Object result = null;
        // Thread execThread = new Thread() {
            // public void run() {
                if ( cl != null )
                {
                    // sets this threads class loader
                    // hopefully any threads spawned by this
                    // will inherit this cl
                    // this enables any class files imported
                    // from the interpreter to be loaded
                    // note: setting the classloader on the
                    // interpreter has a slightly different
                    // meaning in that the classloader for
                    // the interpreter seems only to look for
                    // source files ( bla.java ) in the classpath
                    Thread.currentThread().setContextClassLoader(cl);
                }
                bsh.Interpreter interpreter = new bsh.Interpreter();
                if ( cl != null )
                {
                    // additionally set class loader on the interpreter
                    // to allow it to load java classes defined in source
                    // files e.g. bla.java
                    interpreter.getNameSpace().clear();
                }


                // reset position
                currentPosition = -1;
                view.update();

                interpreter.set("context", context);
                interpreter.set("ARGUMENTS", new Object[0]);

                if (view.isModified()) {
                    result = interpreter.eval(view.getText());
                }
                else {
                    result = interpreter.eval(getText());
                }
            // }
        // };
        // execThread.start();
        return result;
    }
    public void indicateErrorLine( int lineNum )
    {
        System.out.println("Beanshell indicateErrorLine " + lineNum );
        currentPosition = lineNum - 1;
        view.update();
    }
}
