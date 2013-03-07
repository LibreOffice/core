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
package org.libreoffice.report.pentaho.styles;

/**
 * A hash key for the stylemapper.
 *
 * @since 12.03.2007
 */
public final class StyleMapperKey
{

    private final String elementNamespace;
    private final String elementName;
    private final String attributeNamespace;
    private final String attributeName;
    private final int hashCode;

    public StyleMapperKey(final String elementNamespace,
            final String elementName,
            final String attributeNamespace,
            final String attributeName)
    {
        if (elementNamespace == null)
        {
            throw new NullPointerException();
        }
        if (elementName == null)
        {
            throw new NullPointerException();
        }

        this.elementNamespace = elementNamespace;
        this.elementName = elementName;
        this.attributeNamespace = attributeNamespace;
        this.attributeName = attributeName;
        this.hashCode = computeHashCode();
    }

    public String getElementNamespace()
    {
        return elementNamespace;
    }

    public String getElementName()
    {
        return elementName;
    }

    public String getAttributeNamespace()
    {
        return attributeNamespace;
    }

    public String getAttributeName()
    {
        return attributeName;
    }

    public boolean equals(final Object o)
    {
        if (this != o)
        {
            if (o == null || getClass() != o.getClass())
            {
                return false;
            }

            final StyleMapperKey that = (StyleMapperKey) o;

            if ((attributeName != null ? !attributeName.equals(that.attributeName) : that.attributeName != null) || (attributeNamespace != null ? !attributeNamespace.equals(that.attributeNamespace) : that.attributeNamespace != null) || !elementName.equals(that.elementName) || !elementNamespace.equals(that.elementNamespace))
            {
                return false;
            }
        }

        return true;
    }

    private int computeHashCode()
    {
        int result = elementNamespace.hashCode();
        result = 31 * result + elementName.hashCode();
        result = 31 * result + (attributeNamespace != null ? attributeNamespace.hashCode() : 0);
        result = 31 * result + (attributeName != null ? attributeName.hashCode() : 0);
        return result;
    }

    public int hashCode()
    {
        return hashCode;
    }
}
