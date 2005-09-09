/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _FootnoteSettings.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:13:24 $
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
* Testing <code>com.sun.star.text.FootnoteSettings</code>
* service properties :
* <ul>
*  <li><code> CharStyleName</code></li>
*  <li><code> NumberingType</code></li>
*  <li><code> PageStyleName</code></li>
*  <li><code> ParaStyleName</code></li>
*  <li><code> Prefix</code></li>
*  <li><code> StartAt</code></li>
*  <li><code> Suffix</code></li>
*  <li><code> BeginNotice</code></li>
*  <li><code> EndNotice</code></li>
*  <li><code> FootnoteCounting</code></li>
*  <li><code> PositionEndOfDoc</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.FootnoteSettings
*/
public class _FootnoteSettings extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _CharStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("CharStyleName", "Endnote Symbol", "Endnote anchor") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _PageStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("PageStyleName", "Standard", "Endnote") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _ParaStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("ParaStyleName", "Standard", "Endnote") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _NumberingType() {
        Short val1 = new Short(com.sun.star.text.FootnoteNumbering.PER_DOCUMENT);
        Short val2 = new Short(com.sun.star.text.FootnoteNumbering.PER_PAGE);
        log.println("Testing with custom Property tester") ;
        testProperty("NumberingType", val1, val2) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _FootnoteCounting() {
        Short val1 = new Short( (short) 1 );
        Short val2 = new Short( (short) 2 );
        log.println("Testing with custom Property tester") ;
        testProperty("FootnoteCounting", val1, val2) ;
    }

}


