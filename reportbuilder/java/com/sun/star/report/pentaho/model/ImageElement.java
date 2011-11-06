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

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;

import org.jfree.report.expressions.FormulaExpression;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class ImageElement extends ReportElement
{

    private FormulaExpression formula;

    public ImageElement()
    {
    }

    public FormulaExpression getFormula()
    {
        return formula;
    }

    public void setFormula(final FormulaExpression formula)
    {
        this.formula = formula;
    }

    public String getScaleMode()
    {
        String val = (String) getAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.SCALE);
        if (OfficeToken.TRUE.equals(val))
        {
            val = OfficeToken.ANISOTROPIC;
        }
        else if (OfficeToken.FALSE.equals(val) || val == null)
        {
            val = OfficeToken.NONE;
        }
        return val;
    }

    public boolean isPreserveIRI()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.PRESERVE_IRI));
    }

    public void setPreserveIRI(final boolean preserveIRI)
    {
        setAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.PRESERVE_IRI, String.valueOf(preserveIRI));
    }

    public String getImageData()
    {
        return (String) getAttribute(OfficeNamespaces.FORM_NS, OfficeToken.IMAGE_DATA);
    }
}
