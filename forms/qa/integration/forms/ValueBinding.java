/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package integration.forms;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XText;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextCursor;
import com.sun.star.form.binding.XValueBinding;
import com.sun.star.form.binding.XBindableValue;

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
