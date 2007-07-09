/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AbstractReportElementLayoutController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.pentaho.model.ReportElement;
import com.sun.star.report.pentaho.model.OfficeGroupSection;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.DataRow;
import org.jfree.report.DataFlags;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Group;
import org.jfree.report.expressions.Expression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.AbstractLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.formula.lvalues.LValue;
import org.jfree.formula.lvalues.ContextLookup;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 05.03.2007
 * @noinspection CloneableClassWithoutClone
 */
public abstract class AbstractReportElementLayoutController
    extends AbstractLayoutController
{
  public static final int NOT_STARTED = 0;
  public static final int FINISHED = 2;

  private int state;

  protected AbstractReportElementLayoutController()
  {
  }


  /**
   * Advances the processing position.
   *
   * @param target the report target that receives generated events.
   * @return the new layout controller instance representing the new state.
   *
   * @throws org.jfree.report.DataSourceException        if there was a problem reading data from
   *                                    the datasource.
   * @throws org.jfree.report.ReportProcessingException  if there was a general problem during
   *                                    the report processing.
   * @throws org.jfree.report.ReportDataFactoryException if a query failed.
   */
  public LayoutController advance(final ReportTarget target)
      throws DataSourceException, ReportDataFactoryException,
      ReportProcessingException
  {
    if (state != AbstractReportElementLayoutController.NOT_STARTED)
    {
      throw new IllegalStateException();
    }

    boolean isPrintableContent = true;
    final ReportElement text = (ReportElement) getNode();
    // Tests we have to perform:
    // 1. Print when group changes. We can know whether a group changed by
    //    looking at the newly introduced iteration counter.
    //
    //    Whether we use the next one or the one after that depends on whether
    //    this element is a child of a group-header or group-footer.
    if (text.isPrintWhenGroupChanges())
    {
      // if this is set to true, then we print the element only if this is the
      // first occurrence in this group.
      if (isGroupChanged() == false)
      {
//        Log.debug ("Group Change Condition");
        isPrintableContent = false;
      }
    }

    // 2. Print repeated values. This never applies to static text or static
    //    elements.
    if (text.isPrintRepeatedValues() == false)
    {
      // If this is set to true, we evaluate the formula of the element and
      // try to derive whether there was a change.
      if (isValueChanged() == false)
      {
//        Log.debug ("Value Repeat Condition");
        isPrintableContent = false;
      }
    }

    // 3. Evaluate the Display Condition
    final Expression dc = text.getDisplayCondition();
    if (dc != null)
    {
      final Object o = LayoutControllerUtil.evaluateExpression
          (getFlowController(), text, dc);
      if (Boolean.FALSE.equals(o))
      {
//        Log.debug ("DISPLAY Condition forbids printing");
        isPrintableContent = false;
      }
    }

    if (isPrintableContent == false)
    {
      // There is no printable content at all. Set the state to FINISHED
      return join (getFlowController());
    }
    else
    {
      // delegate to the handler ..
      return delegateContentGeneration(target);
    }

  }

  protected abstract boolean isValueChanged();

  protected boolean isGroupChanged()
  {
    // search the group.
    final SectionLayoutController slc = findGroup();
    if (slc == null)
    {
      // Always print the content of the report header and footer and
      // the page header and footer.
      return true;
    }

    // we are in the first iteration, so yes, the group has changed recently.
    return slc.getIterationCount() == 0;
  }

  private SectionLayoutController findGroup()
  {
    LayoutController parent = getParent();
    boolean skipNext = false;
    while (parent != null)
    {
      if (parent instanceof SectionLayoutController == false)
      {
        parent = parent.getParent();
        continue;
      }

      final SectionLayoutController slc = (SectionLayoutController) parent;
      final Element element = slc.getElement();
      if (element instanceof OfficeGroupSection)
      {
        // This is a header or footer. So we take the next group instead.
        skipNext = true;
        parent = parent.getParent();
        continue;
      }

      if (element instanceof Group == false)
      {
        parent = parent.getParent();
        continue;
      }

      if (skipNext)
      {
        skipNext = false;
        parent = parent.getParent();
        continue;
      }

      return (SectionLayoutController) parent;
    }
    return null;
  }

  /**
   * Joins with a delegated process flow. This is generally called from a child
   * flow and should *not* (I mean it!) be called from outside. If you do,
   * you'll suffer.
   *
   * @param flowController the flow controller of the parent.
   * @return the joined layout controller that incorperates all changes from the
   *         delegate.
   */
  public LayoutController join(final FlowController flowController)
      throws DataSourceException, ReportDataFactoryException,
      ReportProcessingException
  {
    final AbstractReportElementLayoutController alc =
        (AbstractReportElementLayoutController) clone();
    alc.state = AbstractReportElementLayoutController.FINISHED;
    return alc;
  }

  protected abstract LayoutController delegateContentGeneration (final ReportTarget target)
      throws ReportProcessingException, ReportDataFactoryException,
      DataSourceException;

  /**
   * Checks, whether the layout controller would be advanceable. If this method
   * returns true, it is generally safe to call the 'advance()' method.
   *
   * @return true, if the layout controller is advanceable, false otherwise.
   */
  public boolean isAdvanceable()
  {
    return state != AbstractReportElementLayoutController.FINISHED;
  }

  protected boolean isReferenceChanged (final LValue lValue)
  {
    if (lValue instanceof ContextLookup)
    {
      final ContextLookup rval = (ContextLookup) lValue;
      final String s = rval.getName();
      final DataRow view = getFlowController().getMasterRow().getGlobalView();
      try
      {
        final DataFlags flags = view.getFlags(s);
        if (flags != null)
        {
          if (flags.isChanged())
          {
//            Log.debug ("Reference " + s + " is changed");
            return true;
          }
        }
//        Log.debug ("Reference " + s + " is unchanged");
      }
      catch (DataSourceException e)
      {
        // ignore .. assume that the reference has not changed.
      }
    }
    final LValue[] childValues = lValue.getChildValues();
    for (int i = 0; i < childValues.length; i++)
    {
      final LValue value = childValues[i];
      if (isReferenceChanged(value))
      {
        return true;
      }
    }
//    Log.debug ("Unchanged.");
    return false;
  }

  public int getState()
  {
    return state;
  }

  protected void setState(final int state)
  {
    this.state = state;
  }
}
