/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ValueBinding.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package integration.forms;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XText;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextCursor;
import com.sun.star.form.binding.XValueBinding;
import com.sun.star.form.binding.XBindableValue;

import integration.forms.DocumentHelper;
import integration.forms.TableCellTextBinding;

public class ValueBinding extends integration.forms.TestCase
{
    /** Creates a new instance of ValueBinding */
    public ValueBinding()
    {
        super( DocumentType.WRITER );
    }

    public static boolean isInteractiveTest()
    {
        return true;
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkBindingProperties"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Control Value Binding Test";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.before();
        prepareDocument();
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.waitForUserInput();
        super.closeDocument();
    }

    /* ------------------------------------------------------------------ */
    public void checkBindingProperties() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();

        // insert a table with exactly one cell. The content of this table will be synced with
        // the content of a form control
        XTextDocument textDoc = (XTextDocument)UnoRuntime.queryInterface( XTextDocument.class,  m_document.getDocument() );
        XText documentText = textDoc.getText();
        XTextCursor textCursor = documentText.createTextCursor();

        XTextTable table = (XTextTable)UnoRuntime.queryInterface( XTextTable.class,
            m_document.createInstance( "com.sun.star.text.TextTable" )
        );
        table.initialize( 1, 1 );
        documentText.insertTextContent( textCursor, table, false );

        // insert our sample control
        XPropertySet textControl = m_formLayer.insertControlLine( "DatabaseTextField", "Test", "", 10 );

        // create a value binding for the first cell of the table
        XValueBinding cellBinding = new TableCellTextBinding( table.getCellByName( "A1" ) );
        // and bind it to the control
        XBindableValue bindable = (XBindableValue)UnoRuntime.queryInterface(
            XBindableValue.class, textControl
        );
        bindable.setValueBinding( cellBinding );

        // toggle the view to alive mode
        m_document.getCurrentView( ).toggleFormDesignMode( );
    }
 }
