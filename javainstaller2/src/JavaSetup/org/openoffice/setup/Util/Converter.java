/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Converter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:00:29 $
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

package org.openoffice.setup.Util;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

public class Converter {

    private Converter() {
    }

    static public String[] convertHashmapToStringArray(HashMap map) {

        int size = map.size();
        String[] myStringArray = new String[size];

        Iterator m = map.entrySet().iterator();
        int counter = 0;

        while ( m.hasNext() ) {
            Map.Entry entry = (Map.Entry) m.next();
            String env = entry.getKey() + "=" + entry.getValue();
            myStringArray[counter] = env;
            counter = counter + 1;
        }

        return myStringArray;
    }

    static public HashMap convertVectorToHashmap(Vector vec) {
        HashMap map = new HashMap();

        for (int i = 0; i < vec.size(); i++) {
            String key = null;
            String value = null;

            String line = (String)vec.get(i);
            int position = line.indexOf("=");
            if ( position > -1 ) {
                key = line.substring(0, position);
                value = line.substring(position + 1, line.length());
            } else {
                key = line;
                value = null;
            }

            map.put(key, value);
        }

        return map;
    }

    static public Vector convertHashMapToVector(HashMap hash) {
        Vector vec = new Vector();

        Iterator m = hash.entrySet().iterator();

        while ( m.hasNext() ) {
            Map.Entry entry = (Map.Entry) m.next();
            String line = entry.getKey() + "=" + entry.getValue();
            vec.add(line);
        }

        return vec;
    }

}
