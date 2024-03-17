# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Helper for creating an OLE chart

from com.sun.star.text import VertOrientation
from com.sun.star.text import HoriOrientation


class ChartHelper(object):

    def __init__(self, container_doc):
        self._CHART_CLASS_ID = "12dcae26-281f-416f-a234-c3086127382e"
        self._container_document = container_doc

    def insert_ole_chart_in_writer(self, upper_left, extent, chart_service_name: str):
        """Inserts an OLE chart into a Writer document.

        Args:
            upper_left (Point): The upper-left corner of the chart.
            extent (Size): The size of the chart.
            chart_service_name (str): The name of the chart service to use.

        Returns:
            The ChartDocument inserted
        """
        result = None

        try:
            text_content = self._container_document.createInstance(
                "com.sun.star.text.TextEmbeddedObject"
            )

            if text_content is not None:
                text_content.setPropertyValue("CLSID", self._CHART_CLASS_ID)

                text = self._container_document.getText()
                cursor = text.createTextCursor()

                # insert embedded object in text -> object will be created
                text.insertTextContent(cursor, text_content, True)

                # set size and position
                text_content.setSize(extent)
                text_content.setPropertyValue("VertOrient", int(VertOrientation.NONE))
                text_content.setPropertyValue("HoriOrient", int(HoriOrientation.NONE))
                text_content.setPropertyValue("VertOrientPosition", upper_left.Y)
                text_content.setPropertyValue("HoriOrientPosition", upper_left.X)

                # Get chart document and set diagram
                result = text_content.getPropertyValue("Model")
                result.setDiagram(result.createInstance(chart_service_name))

        except Exception as err:
            print(f"caught exception: {err}")

        return result

    def insert_ole_chart_in_draw(self, upper_left, extent, chart_service_name):
        """Inserts an OLE chart into a Draw document.

        Args:
            upper_left (Point): The upper-left corner of the chart.
            extent (Size): The size of the chart.
            chart_service_name (str): The name of the chart service to use.

        Returns:
            The ChartDocument inserted
        """
        result = None

        try:
            # Get First page
            page = self._container_document.getDrawPages().getByIndex(0)

        except AttributeError:
            # try interface for single draw page (e.g. spreadsheet)
            page = self._container_document.getDrawPage()

        except Exception as err:
            print(f"First page not found in shape collection: {err}")

        try:
            # Create an OLE shape
            shape = self._container_document.createInstance(
                "com.sun.star.drawing.OLE2Shape"
            )

            # Insert the shape into the page
            page.add(shape)
            shape.setPosition(upper_left)
            shape.setSize(extent)

            # set the class id for charts
            shape.setPropertyValue("CLSID", self._CHART_CLASS_ID)

            # retrieve the chart document as model of the OLE shape
            result = shape.getPropertyValue("Model")

            # create a diagram via the factory and set this as new diagram
            result.setDiagram(result.createInstance(chart_service_name))

        except Exception as err:
            print(f"Couldn't change the OLE shape into a chart: {err}")

        return result

# vim: set shiftwidth=4 softtabstop=4 expandtab:
