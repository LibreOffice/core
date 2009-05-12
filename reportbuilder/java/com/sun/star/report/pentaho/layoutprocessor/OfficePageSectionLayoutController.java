package com.sun.star.report.pentaho.layoutprocessor;

import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.structure.Element;
import org.jfree.report.DataSourceException;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.JFreeReportInfo;

/**
 * Todo: Document Me
 *
 * @author Thomas Morgner
 */
public class OfficePageSectionLayoutController extends SectionLayoutController
{
  public OfficePageSectionLayoutController()
  {
  }

  protected AttributeMap computeAttributes(final FlowController flowController, final Element element, final ReportTarget reportTarget) throws DataSourceException
  {
    final AttributeMap map = new AttributeMap( super.computeAttributes(flowController, element, reportTarget));
    map.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "role", "spreadsheet-section");
    map.makeReadOnly();
    return map;
  }
}
