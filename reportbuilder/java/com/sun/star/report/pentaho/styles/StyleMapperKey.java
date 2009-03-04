/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleMapperKey.java,v $
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
package com.sun.star.report.pentaho.styles;

/**
 * A hash key for the stylemapper.
 *
 * @author Thomas Morgner
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

            if ((attributeName != null ? !attributeName.equals(that.attributeName) : that.attributeName != null) ||
                    (attributeNamespace != null ? !attributeNamespace.equals(that.attributeNamespace) : that.attributeNamespace != null) ||
                    !elementName.equals(that.elementName) || !elementNamespace.equals(that.elementNamespace))
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
