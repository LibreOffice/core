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


package com.sun.star.report.pentaho.model;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.structure.Section;

/**
 * A paragraph that contains variables-declarations. This paragraph will be
 * printed in the first cell of the first table (if there's one).
 *
 * The VariablesDeclarationSection is an auto-generated structure element that
 * has no model-representation. The section itself is empty and simply acts
 * as flag for the output-processor. The output processor itself is responsible
 * to maintain the variables.
 *
 * @author Thomas Morgner
 * @since 19.03.2007
 */
public class VariablesDeclarationSection extends Section
{

    public VariablesDeclarationSection()
    {
        setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        setType("variables-section");
    }
}
