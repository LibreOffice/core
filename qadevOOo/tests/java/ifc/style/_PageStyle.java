/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _PageStyle.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:02:00 $
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

package ifc.style;

import com.sun.star.container.XNameContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xml.AttributeData;
import ifc.style._PageProperties;
import lib.MultiPropertyTest;

/**
 * Test page style properties.
 * These properties are a subset of the PageProperties. So just use the
 * PageProperties test
 */
public class _PageStyle extends _PageProperties {

    /**
     * This property is system dependent and printer dependent.
     * Test is used from _PageProperties
     * @see ifc.style._PageProperties
     */
    public void _PrinterPaperTray() {
        super._PrinterPaperTray();
    }

    /**
     * Create some valid user defined attributes
     * Test is used from _PageProperties
     * @see ifc.style._PageProperties
     */
    public void _UserDefinedAttributes() {
        super._UserDefinedAttributes();
    }
}
