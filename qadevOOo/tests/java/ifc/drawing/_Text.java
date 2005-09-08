/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _Text.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:37:21 $
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

package ifc.drawing;

import lib.MultiPropertyTest;

import com.sun.star.container.XIndexReplace;

public class _Text extends MultiPropertyTest {

    protected boolean compare(Object ob1, Object ob2) {

        return super.compare(ob1, ob2);

    }

    public void _NumberingRules() {
        //This property is readOnly
        boolean bResult = false;

        try {
            Object theRules2 = oObj.getPropertyValue( "NumberingRules" );
            if (!(theRules2 instanceof com.sun.star.container.XIndexReplace)) {
                log.println("Property 'NumberingRules' is an instance of "+
                    theRules2.getClass().getName());
                log.println("Expected was an instance of "+
                    "com.sun.star.container.XIndexReplace");
            }
            log.println("Trying to cast property 'NumberingRules' "+
                "to XIndexReplace");
            XIndexReplace theRules = (XIndexReplace)
                oObj.getPropertyValue( "NumberingRules" );
            bResult = true;
        } catch( Exception e ) {
            log.println("Error occured while trying to get property"+
                " 'NumberingRules'");
            e.printStackTrace( log );
            bResult = false;
        }

        tRes.tested( "NumberingRules", bResult);
    }

}  // finish class _Text



