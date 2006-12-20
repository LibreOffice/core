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
