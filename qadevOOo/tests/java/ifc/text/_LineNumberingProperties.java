/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _LineNumberingProperties.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:13:50 $
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

package ifc.text;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.text.LineNumberingProperties</code>
* service properties :
* <ul>
*  <li><code> IsOn</code></li>
*  <li><code> CharStyleName</code></li>
*  <li><code> CountEmptyLines</code></li>
*  <li><code> CountLinesInFrames</code></li>
*  <li><code> Distance</code></li>
*  <li><code> Interval</code></li>
*  <li><code> SeparatorText</code></li>
*  <li><code> SeparatorInterval</code></li>
*  <li><code> NumberPosition</code></li>
*  <li><code> NumberingType</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.LineNumberingProperties
*/
public class _LineNumberingProperties extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _CharStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("CharStyleName", "Endnote Symbol", "Endnote anchor") ;
    }

}


