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

package complex.sfx2.undo;

import com.sun.star.chart2.XAxis;
import com.sun.star.chart2.XCoordinateSystem;
import com.sun.star.chart2.XCoordinateSystemContainer;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.chart2.XChartDocument;
import com.sun.star.chart2.XDiagram;
import com.sun.star.container.XIndexAccess;
import com.sun.star.document.UndoFailedException;
import com.sun.star.document.XUndoAction;
import com.sun.star.document.XUndoManager;
import com.sun.star.document.XUndoManagerSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.embed.EmbedStates;
import com.sun.star.embed.EmbedVerbs;
import com.sun.star.embed.XEmbeddedObject;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.XSelectionSupplier;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.OfficeDocument;
import static org.junit.Assert.*;

/**
 * @author frank.schoenheit@oracle.com
 */
public class ChartDocumentTest implements DocumentTest
{
    public ChartDocumentTest( final XMultiServiceFactory i_orb ) throws com.sun.star.uno.Exception, InterruptedException
    {
        m_textDocument = OfficeDocument.blankDocument( i_orb, DocumentType.WRITER );

        // create a OLE shape in the document
        final XMultiServiceFactory factory = UnoRuntime.queryInterface( XMultiServiceFactory.class, m_textDocument.getDocument() );
        final String shapeServiceName = "com.sun.star.text.TextEmbeddedObject";
        final XPropertySet shapeProps = UnoRuntime.queryInterface( XPropertySet.class, factory.createInstance( shapeServiceName ) );
        shapeProps.setPropertyValue("CLSID", "12dcae26-281f-416f-a234-c3086127382e");

        final XShape shape = UnoRuntime.queryInterface( XShape.class, shapeProps );
        shape.setSize( new Size( 16000, 9000 ) );

        final XTextContent chartTextContent = UnoRuntime.queryInterface( XTextContent.class, shapeProps );

        final XSelectionSupplier selSupplier = UnoRuntime.queryInterface( XSelectionSupplier.class,
                m_textDocument.getCurrentView().getController() );
        final Object selection = selSupplier.getSelection();
        final XTextRange textRange = getAssociatedTextRange( selection );
        if ( textRange == null )
            throw new RuntimeException( "can't locate a text range" );

        // insert the chart
        textRange.getText().insertTextContent(textRange, chartTextContent, false);

        // retrieve the chart model
        XChartDocument chartDoc = UnoRuntime.queryInterface( XChartDocument.class, shapeProps.getPropertyValue( "Model" ) );
        m_chartDocument = new OfficeDocument( i_orb, chartDoc );

        // actually activate the object
        final XEmbeddedObject embeddedChart = UnoRuntime.queryInterface( XEmbeddedObject.class,
            shapeProps.getPropertyValue( "EmbeddedObject" ) );
        embeddedChart.doVerb( EmbedVerbs.MS_OLEVERB_SHOW );

        final int state = embeddedChart.getCurrentState();
        if ( state != EmbedStates.UI_ACTIVE )
            fail( "unable to activate the embedded chart" );
    }

    public String getDocumentDescription()
    {
        return "chart document";
    }

    public void initializeDocument() throws com.sun.star.uno.Exception
    {
        final XPropertySet wallProperties = impl_getWallProperties();
        wallProperties.setPropertyValue( "FillStyle", com.sun.star.drawing.FillStyle.SOLID );
        wallProperties.setPropertyValue( "FillColor", 0x00FFFFFF );
    }

    public void closeDocument()
    {
        m_textDocument.close();
    }

    private XPropertySet impl_getWallProperties()
    {
        final XChartDocument chartDoc = UnoRuntime.queryInterface( XChartDocument.class, m_chartDocument.getDocument() );
        final XDiagram diagram = chartDoc.getFirstDiagram();
        final XPropertySet wallProperties = diagram.getWall();
        return wallProperties;
    }

    private XPropertySet impl_getYAxisProperties()
    {
        XPropertySet axisProperties = null;
        try
        {
            final XChartDocument chartDoc = UnoRuntime.queryInterface( XChartDocument.class, m_chartDocument.getDocument() );
            final XDiagram diagram = chartDoc.getFirstDiagram();
            final XCoordinateSystemContainer coordContainer = UnoRuntime.queryInterface( XCoordinateSystemContainer.class, diagram );
            final XCoordinateSystem[] coordSystems = coordContainer.getCoordinateSystems();
            final XCoordinateSystem coordSystem = coordSystems[0];
            final XAxis primaryYAxis = coordSystem.getAxisByDimension( 1, 0 );
            axisProperties = UnoRuntime.queryInterface( XPropertySet.class, primaryYAxis );
        }
        catch ( Exception ex )
        {
            fail( "internal error: could not retrieve primary Y axis properties" );
        }
        return axisProperties;
    }

    private XUndoManager impl_getUndoManager()
    {
        final XUndoManagerSupplier undoManagerSupp = UnoRuntime.queryInterface( XUndoManagerSupplier.class, m_chartDocument.getDocument() );
        final XUndoManager undoManager = undoManagerSupp.getUndoManager();
        return undoManager;
    }

    public void doSingleModification() throws com.sun.star.uno.Exception
    {
        final XPropertySet wallProperties = impl_getWallProperties();

        // simulate an Undo action, as long as the chart implementation doesn't add Undo actions itself
        final XUndoManager undoManager = impl_getUndoManager();
        undoManager.addUndoAction( new PropertyUndoAction( wallProperties, "FillColor", 0xCCFF44 ) );
            // (the UndoAction will actually set the property value)
    }

    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception
    {
        final XPropertySet wallProperties = impl_getWallProperties();
        assertEquals( 0x00FFFFFF, ((Integer)wallProperties.getPropertyValue( "FillColor" )).intValue() );
    }

    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception
    {
        final XPropertySet wallProperties = impl_getWallProperties();
        assertEquals( 0xCCFF44, ((Integer)wallProperties.getPropertyValue( "FillColor" )).intValue() );
    }

    public int doMultipleModifications() throws com.sun.star.uno.Exception
    {
        final XPropertySet axisProperties = impl_getYAxisProperties();

        final XUndoManager undoManager = impl_getUndoManager();
        undoManager.addUndoAction( new PropertyUndoAction( axisProperties, "LineWidth", 300 ) );
        undoManager.addUndoAction( new PropertyUndoAction( axisProperties, "LineColor", 0x000000 ) );

        return 2;
    }

    public OfficeDocument getDocument()
    {
        return m_chartDocument;
    }

    private XTextRange getAssociatedTextRange( final Object i_object ) throws WrappedTargetException, IndexOutOfBoundsException
    {
        // possible cases:
        // 1. a container of other objects - e.g. selection of 0 to n text portions, or 1 to n drawing objects
        final XIndexAccess indexer = UnoRuntime.queryInterface( XIndexAccess.class, i_object );
        if ((indexer != null) && indexer.getCount() > 0) {
            final int count = indexer.getCount();
            for (int i = 0; i < count; ++i) {
                final XTextRange range = getAssociatedTextRange( indexer.getByIndex(i) );
                if (range != null) {
                    return range;
                }
            }
        }
        // 2. another TextContent, having an anchor we can use
        final XTextContent textContent = UnoRuntime.queryInterface(XTextContent.class, i_object);
        if (textContent != null) {
            final XTextRange range = textContent.getAnchor();
            if (range != null) {
                return range;
            }
        }

        // an object which supports XTextRange directly
        final XTextRange range = UnoRuntime.queryInterface(XTextRange.class, i_object);
        if (range != null) {
            return range;
        }

        return null;
    }

    private static class PropertyUndoAction implements XUndoAction
    {
        PropertyUndoAction( final XPropertySet i_component, final String i_propertyName, final Object i_newValue ) throws com.sun.star.uno.Exception
        {
            m_component = i_component;
            m_propertyName = i_propertyName;
            m_newValue = i_newValue;

            m_oldValue = i_component.getPropertyValue( m_propertyName );
            i_component.setPropertyValue( m_propertyName, m_newValue );
        }

        public String getTitle()
        {
            return "some dummy Undo Action";
        }

        public void undo() throws UndoFailedException
        {
            try
            {
                m_component.setPropertyValue( m_propertyName, m_oldValue );
            }
            catch ( com.sun.star.uno.Exception ex )
            {
                throw new UndoFailedException( "", this, ex );
            }
        }

        public void redo() throws UndoFailedException
        {
            try
            {
                m_component.setPropertyValue( m_propertyName, m_newValue );
            }
            catch ( com.sun.star.uno.Exception ex )
            {
                throw new UndoFailedException( "", this, ex );
            }
        }

        private final XPropertySet  m_component;
        private final String        m_propertyName;
        private final Object        m_oldValue;
        private final Object        m_newValue;
    }

    private final OfficeDocument    m_textDocument;
    private final OfficeDocument    m_chartDocument;
}
