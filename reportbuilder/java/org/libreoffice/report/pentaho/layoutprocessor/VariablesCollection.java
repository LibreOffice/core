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
package org.libreoffice.report.pentaho.layoutprocessor;

import org.libreoffice.report.pentaho.model.FormattedTextElement;

import java.util.ArrayList;
import java.util.List;


/**
 * A variables collection is used to collect all FormattedTextElement objects
 * of a repeated header or footer. Later, for each of these elements a variable
 * setter is inserted into a hidden (in fact just very small) paragraph. These
 * variables can later be read using the 'variable-get' construct.
 *
 * From the idea, this is equal to the 'strings' declaration of CSS3, although
 * this code is explicit instead of declarative.
 *
 * @since 22.03.2007
 */
public class VariablesCollection
{

    private VariablesCollection parent;
    private String namePrefix;
    private List<FormattedTextElement> variables;

    public VariablesCollection(final String namePrefix)
    {
        this(namePrefix, null);
    }

    public VariablesCollection(final String namePrefix, final VariablesCollection parent)
    {
        if (namePrefix == null)
        {
            throw new NullPointerException("NamePrefix cannot be null");
        }

        this.namePrefix = namePrefix;
        this.parent = parent;
        this.variables = new ArrayList<FormattedTextElement>();
    }

    public VariablesCollection getParent()
    {
        return parent;
    }

    public String getNamePrefix()
    {
        return namePrefix;
    }

    public String addVariable(final FormattedTextElement element)
    {
        variables.add(element);
        final int size = variables.size();
        return namePrefix + size;
    }

    public FormattedTextElement[] getVariables()
    {
        return variables.toArray(new FormattedTextElement[variables.size()]);
    }

    public int getVariablesCount()
    {
        return variables.size();
    }
}
