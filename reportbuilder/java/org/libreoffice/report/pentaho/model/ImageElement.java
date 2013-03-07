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
package org.libreoffice.report.pentaho.model;

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.report.expressions.FormulaExpression;

/**
 * Todo: Document me!
 *
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
