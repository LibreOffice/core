/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

class PropertyUndoAction : public cppu::WeakImplHelper<XUndoAction>
{
public:
    PropertyUndoAction(Reference<XPropertySet>& i_component, const OUString& i_propertyName,
                       const Any& i_newValue)
    {
        m_component = i_component;
        m_propertyName = i_propertyName;
        m_newValue = i_newValue;

        m_oldValue = i_component->getPropertyValue(m_propertyName);
        i_component->setPropertyValue(m_propertyName, m_newValue);
    }

    virtual OUString getTitle() override { return u"some dummy Undo Action"_ustr; }

    virtual void undo() override
    {
        try
        {
            m_component->setPropertyValue(m_propertyName, m_oldValue);
        }
        catch (Exception ex)
        {
            throw UndoFailedException(u""_ustr, static_cast<cppu::OWeakObject*>(this), Any(ex));
        }
    }

    virtual void redo() override
    {
        try
        {
            m_component->setPropertyValue(m_propertyName, m_newValue);
        }
        catch (Exception ex)
        {
            throw UndoFailedException(u""_ustr, static_cast<cppu::OWeakObject*>(this), Any(ex));
        }
    }

private:
    Reference<XPropertySet> m_component;
    OUString m_propertyName;
    Any m_oldValue;
    Any m_newValue;
};

class ChartDocumentTest : public DocumentTest
{
public:
    ChartDocumentTest(const Reference<XComponent>& rxDoc)
    {
        m_textDocument = rxDoc;

        // create an OLE shape in the document
        Reference<XMultiServiceFactory> factory(m_textDocument, UNO_QUERY);
        Reference<XPropertySet> shapeProps(
            factory->createInstance(u"com.sun.star.text.TextEmbeddedObject"_ustr), UNO_QUERY);
        shapeProps->setPropertyValue(u"CLSID"_ustr,
                                     Any(u"12dcae26-281f-416f-a234-c3086127382e"_ustr));

        Reference<XShape> shape(shapeProps, UNO_QUERY);
        shape->setSize(css::awt::Size(16000, 9000));

        Reference<XTextContent> chartTextContent(shapeProps, UNO_QUERY);

        // get the model interface for the document
        Reference<XModel> xDocModel(m_textDocument, UNO_QUERY);
        // get the current controller for the document - as a controller is tied to a view,
        // this gives us the currently active view for the document.
        Reference<XController> xController = xDocModel->getCurrentController();

        Reference<XSelectionSupplier> selSupplier(xController, UNO_QUERY);
        Any selection = selSupplier->getSelection();
        Reference<XTextRange> textRange = getAssociatedTextRange(selection);
        if (textRange == nullptr)
            throw RuntimeException(u"can't locate a text range"_ustr);

        // insert the chart
        textRange->getText()->insertTextContent(textRange, chartTextContent, false);

        // retrieve the chart model
        Reference<css::chart2::XChartDocument> chartDoc(shapeProps->getPropertyValue(u"Model"_ustr),
                                                        UNO_QUERY);
        // insert default chart for the test to use.
        chartDoc->createDefaultChart();
        m_chartDocument = chartDoc;

        // actually activate the object
        Reference<XEmbeddedObject> embeddedChart(
            shapeProps->getPropertyValue(u"EmbeddedObject"_ustr), UNO_QUERY);
        embeddedChart->doVerb(EmbedVerbs::MS_OLEVERB_SHOW);

        int state = embeddedChart->getCurrentState();
        if (state != EmbedStates::UI_ACTIVE)
            CPPUNIT_FAIL("unable to activate the embedded chart");
    }

    virtual OUString getDocumentDescription() override { return u"chart document"_ustr; }

    virtual void initializeDocument() override
    {
        Reference<XPropertySet> wallProperties = impl_getWallProperties();
        wallProperties->setPropertyValue(u"FillStyle"_ustr, Any(css::drawing::FillStyle_SOLID));
        wallProperties->setPropertyValue(u"FillColor"_ustr, Any(sal_Int32(0x00FFFFFF)));
    }

    virtual void closeDocument() override
    {
        try
        {
            Reference<XCloseable> closeDoc(m_textDocument, UNO_QUERY);
            closeDoc->close(true);
        }
        catch (CloseVetoException)
        {
        }
    }

private:
    Reference<XPropertySet> impl_getWallProperties()
    {
        Reference<XChartDocument> chartDoc(m_chartDocument);
        Reference<css::chart2::XDiagram> diagram = chartDoc->getFirstDiagram();
        Reference<XPropertySet> wallProperties = diagram->getWall();
        return wallProperties;
    }

    Reference<XPropertySet> impl_getYAxisProperties()
    {
        Reference<XPropertySet> axisProperties;
        Reference<XChartDocument> chartDoc(m_chartDocument);
        Reference<css::chart2::XDiagram> diagram = chartDoc->getFirstDiagram();
        Reference<XCoordinateSystemContainer> coordContainer(diagram, UNO_QUERY);
        Sequence<Reference<XCoordinateSystem>> coordSystems
            = coordContainer->getCoordinateSystems();
        Reference<XCoordinateSystem> coordSystem = coordSystems[0];
        Reference<XAxis> primaryYAxis = coordSystem->getAxisByDimension(1, 0);
        axisProperties = Reference<XPropertySet>(primaryYAxis, UNO_QUERY);
        return axisProperties;
    }

    Reference<XUndoManager> impl_getUndoManager()
    {
        Reference<XUndoManagerSupplier> undoManagerSupp(m_chartDocument, UNO_QUERY);
        Reference<XUndoManager> undoManager = undoManagerSupp->getUndoManager();
        return undoManager;
    }

public:
    void doSingleModification() override
    {
        Reference<XPropertySet> wallProperties = impl_getWallProperties();

        // simulate an Undo action, as long as the chart implementation doesn't add Undo actions itself
        Reference<XUndoManager> undoManager = impl_getUndoManager();
        undoManager->addUndoAction(
            new PropertyUndoAction(wallProperties, u"FillColor"_ustr, Any(sal_Int32(0xCCFF44))));
        // (the UndoAction will actually set the property value)
    }

    virtual void verifyInitialDocumentState() override
    {
        Reference<XPropertySet> wallProperties = impl_getWallProperties();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FFFFFF),
                             wallProperties->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>());
    }

    virtual void verifySingleModificationDocumentState() override
    {
        Reference<XPropertySet> wallProperties = impl_getWallProperties();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xCCFF44),
                             wallProperties->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>());
    }

    virtual int doMultipleModifications() override
    {
        Reference<XPropertySet> axisProperties = impl_getYAxisProperties();

        Reference<XUndoManager> undoManager = impl_getUndoManager();
        undoManager->addUndoAction(
            new PropertyUndoAction(axisProperties, u"LineWidth"_ustr, Any(sal_Int32(300))));
        undoManager->addUndoAction(
            new PropertyUndoAction(axisProperties, u"LineColor"_ustr, Any(sal_Int32(0x000000))));

        return 2;
    }

    Reference<XComponent> getDocument() { return m_chartDocument; }

private:
    Reference<XTextRange> getAssociatedTextRange(const Any& i_object)
    {
        // possible cases:
        // 1. a container of other objects - e.g. selection of 0 to n text portions, or 1 to n drawing objects
        Reference<XIndexAccess> indexer(i_object, UNO_QUERY);
        if ((indexer != nullptr) && indexer->getCount() > 0)
        {
            int count = indexer->getCount();
            for (int i = 0; i < count; ++i)
            {
                Reference<XTextRange> range = getAssociatedTextRange(indexer->getByIndex(i));
                if (range != nullptr)
                {
                    return range;
                }
            }
        }
        // 2. another TextContent, having an anchor we can use
        Reference<XTextContent> textContent(i_object, UNO_QUERY);
        if (textContent != nullptr)
        {
            Reference<XTextRange> range = textContent->getAnchor();
            if (range != nullptr)
            {
                return range;
            }
        }

        // an object which supports XTextRange directly
        Reference<XTextRange> range(i_object, UNO_QUERY);
        if (range != nullptr)
        {
            return range;
        }

        return nullptr;
    }

    Reference<XComponent> m_textDocument;
    Reference<css::chart2::XChartDocument> m_chartDocument;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
