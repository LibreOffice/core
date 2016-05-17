/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.uno.UnoRuntime;

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
    @Override
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();

        // insert a table with exactly one cell. The content of this table will be synced with
        // the content of a form control
        XTextDocument textDoc = UnoRuntime.queryInterface( XTextDocument.class,  m_document.getDocument() );
        XText documentText = textDoc.getText();
        XTextCursor textCursor = documentText.createTextCursor();
        documentText.insertString( textCursor, "Below, there's a table cell, and a text field. ", false );
        documentText.insertString( textCursor, "Both are linked via an external value binding.\n", false );
        documentText.insertString( textCursor, "That means that anything you insert into the table cell is reflected in the ", false );
        documentText.insertString( textCursor, "text field, and vice versa.\n", false );

        XTextTable table = UnoRuntime.queryInterface( XTextTable.class,
            m_document.createInstance( "com.sun.star.text.TextTable" )
        );
        table.initialize( 1, 1 );
        documentText.insertTextContent( textCursor, table, false );

        // insert our sample control
        XPropertySet textControl = m_formLayer.insertControlLine( "DatabaseTextField", "enter some text", "", 30 );

        // create a value binding for the first cell of the table
        XValueBinding cellBinding = new TableCellTextBinding( table.getCellByName( "A1" ) );
        // and bind it to the control
        XBindableValue bindable = UnoRuntime.queryInterface(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
