/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowHeightMapper.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.report.pentaho.parser.stylemapper.table;

import com.sun.star.report.pentaho.parser.StyleMapper;
import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.keys.box.BoxStyleKeys;

/**
 * Creation-Date: 03.07.2006, 13:08:27
 *
 * @author Thomas Morgner
 */
public class RowHeightMapper implements StyleMapper
{
  public RowHeightMapper()
  {
  }

  public void updateStyle(final String uri,
                          final String attrName,
                          final String attrValue,
                          final CSSDeclarationRule targetRule)
  {
    targetRule.setPropertyValueAsString(BoxStyleKeys.HEIGHT, attrValue);
  }
}
