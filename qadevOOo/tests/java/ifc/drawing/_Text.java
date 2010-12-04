/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            log.println("Error occurred while trying to get property"+
                " 'NumberingRules'");
            e.printStackTrace( log );
            bResult = false;
        }

        tRes.tested( "NumberingRules", bResult);
    }

}  // finish class _Text



