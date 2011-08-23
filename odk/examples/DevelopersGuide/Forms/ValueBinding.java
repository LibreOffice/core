/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XText;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextCursor;
import com.sun.star.form.binding.XValueBinding;
import com.sun.star.form.binding.XBindableValue;

public class ValueBinding extends DocumentBasedExample
{
    /** Creates a new instance of ValueBinding */
    public ValueBinding()
    {
        super( DocumentType.WRITER );
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
        documentText.insertString( textCursor, "Below, there's a table cell, and a text field. ", false );
        documentText.insertString( textCursor, "Both are linked via an external value binding.\n", false );
        documentText.insertString( textCursor, "That means that anything you insert into the table cell is reflected in the ", false );
        documentText.insertString( textCursor, "text field, and vice versa.\n", false );

        XTextTable table = (XTextTable)UnoRuntime.queryInterface( XTextTable.class,
            m_document.createInstance( "com.sun.star.text.TextTable" )
        );
        table.initialize( 1, 1 );
        documentText.insertTextContent( textCursor, table, false );

        // insert our sample control
        XPropertySet textControl = m_formLayer.insertControlLine( "DatabaseTextField", "enter some text", "", 30 );

        // create a value binding for the first cell of the table
        XValueBinding cellBinding = new TableCellTextBinding( table.getCellByName( "A1" ) );
        // and bind it to the control
        XBindableValue bindable = (XBindableValue)UnoRuntime.queryInterface(
            XBindableValue.class, textControl
        );
        bindable.setValueBinding( cellBinding );
    }

    /* ------------------------------------------------------------------ */
    /** class entry point
    */
    public static void main(String argv[]) throws java.lang.Exception
    {
        ValueBinding aSample = new ValueBinding();
        aSample.run( argv );
    }
 }
