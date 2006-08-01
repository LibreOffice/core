/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConverterCapabilitiesImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:39:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 *  <p>AportisDoc implementation of <code>ConverterCapabilities</code> for
 *  the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.aportisdoc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>Used with StarWriter XML to/from AportisDoc conversions.  The
 *  <code>ConverterCapibilies</code> specify which &quot;Office&quot;
 *  <code>Document</code> tags and attributes are supported on the
 *  &quot;Device&quot; <code>Document</code> format.</p>
 */
public final class ConverterCapabilitiesImpl
    implements ConverterCapabilities {

    public boolean canConvertTag(String tag) {

        if (OfficeConstants.TAG_OFFICE_DOCUMENT.equals(tag))
            return true;
        else if (OfficeConstants.TAG_OFFICE_DOCUMENT_CONTENT.equals(tag))
            return true;
        else if (OfficeConstants.TAG_OFFICE_BODY.equals(tag))
            return true;
        else if (OfficeConstants.TAG_PARAGRAPH.equals(tag))
            return true;
        else if (OfficeConstants.TAG_HEADING.equals(tag))
            return true;
        else if (OfficeConstants.TAG_ORDERED_LIST.equals(tag))
            return true;
        else if (OfficeConstants.TAG_UNORDERED_LIST.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LIST_ITEM.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LIST_HEADER.equals(tag))
            return true;
        else if (OfficeConstants.TAG_SPAN.equals(tag))
            return true;
        else if (OfficeConstants.TAG_HYPERLINK.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LINE_BREAK.equals(tag))
            return true;
        else if (OfficeConstants.TAG_SPACE.equals(tag))
            return true;
        else if (OfficeConstants.TAG_TAB_STOP.equals(tag))
            return true;

        return false;
    }

    public boolean canConvertAttribute(String tag,
                                       String attribute) {

        if (OfficeConstants.TAG_SPACE.equals(tag)) {

            if (OfficeConstants.ATTRIBUTE_SPACE_COUNT.equals(attribute))
                return true;
        }

        return false;
    }
}

