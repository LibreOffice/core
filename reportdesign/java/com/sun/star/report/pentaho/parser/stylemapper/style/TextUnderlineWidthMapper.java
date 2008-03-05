/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextUnderlineWidthMapper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:46:01 $
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
package com.sun.star.report.pentaho.parser.stylemapper.style;

import com.sun.star.report.pentaho.parser.stylemapper.OneOfConstantsMapper;
import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.keys.border.BorderWidth;
import org.jfree.layouting.input.style.keys.text.TextDecorationWidth;
import org.jfree.layouting.input.style.keys.text.TextStyleKeys;
import org.jfree.layouting.input.style.values.CSSAutoValue;
import org.jfree.layouting.input.style.values.CSSValue;

public class TextUnderlineWidthMapper extends OneOfConstantsMapper
{

    public TextUnderlineWidthMapper()
    {
        super(TextStyleKeys.TEXT_UNDERLINE_WIDTH);
        addMapping("auto", CSSAutoValue.getInstance());
        addMapping("normal", BorderWidth.MEDIUM);
        addMapping("bold", TextDecorationWidth.BOLD);
        addMapping("thin", BorderWidth.THIN);
        addMapping("dash", TextDecorationWidth.DASH);
        addMapping("medium", BorderWidth.MEDIUM);
        addMapping("thick", BorderWidth.THICK);
    }

    public void updateStyle(final String uri,
            final String attrName,
            final String attrValue,
            final CSSDeclarationRule targetRule)
    {
        if (attrName == null)
        {
            throw new NullPointerException();
        }

        final CSSValue value = lookupMapping(attrValue);
        if (value != null)
        {
            targetRule.setPropertyValue(getStyleKey(), value);
        }
        else
        // percent
        // positive integer
        // positive length
        {
            targetRule.setPropertyValueAsString(getStyleKey(), attrValue);
        }
    }
}
