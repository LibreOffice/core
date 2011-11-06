/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho.parser.table;

import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 14.03.2007
 */
public class CoveredCellReadHandler extends ElementReadHandler
{

    private final Element coveredCell;

    public CoveredCellReadHandler()
    {
        coveredCell = new Section();
    }

    public Element getElement()
    {
        return coveredCell;
    }
}
