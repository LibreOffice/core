/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportExpression.java,v $
 * $Revision: 1.3 $
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


package com.sun.star.report;

/**
 * Expressions are simple computation components.
 *
 * Expressions are always assumed to be immutable. They are not allowed to
 * change their state and it is not guaranteed, in which order they get called.
 * If the expression has been called before, the last computed value will be
 * available from the datarow.
 *
 * This construct allows us to write expressions in the form
 * "Sum := Sum + Column".
 *
 * Multiple calls to getValue on the same expression instance must return the
 * same value (assuming that the datarow passed in is the same).
 */
public interface ReportExpression
{
  void setParameters (Object[] parameters);
  Object getParameters ();

  Object getValue(DataRow row);
}
