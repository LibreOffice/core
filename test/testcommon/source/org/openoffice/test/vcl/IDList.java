/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;

/**
 *
 * The class is used to read an from external files to replace the id in the code.
 */
public class IDList {

    private HashMap<String, String> map = new HashMap<String, String>();

    private File dir = null;

    public IDList(File dir) {
        this.dir = dir;
        load();
    }


    private void readFile(File file, HashMap<String, String> map) {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = reader.readLine()) != null ) {
                line = line.trim();
                if (line.length() == 0 /*|| line.startsWith("//")*/)
                    continue;
                String[] parts = line.split(" ");
                if (parts.length != 2)
                    continue;
                map.put(parts[0], parts[1]);
            }

        } catch (IOException e) {
            // for debug
            e.printStackTrace();
        } finally {
            if (reader != null)
                try {
                    reader.close();
                } catch (IOException e) {
                    // ignore
                }
        }
    }


    public void load() {
        if (dir == null)
            return;

        map.clear();
        ArrayList<File> validFiles = new ArrayList<File>();
        File[] files = dir.listFiles();
        for (File file : files) {
            if (file.isFile() && file.getName().endsWith(".lst")) {
                validFiles.add(file);
            }
        }

        // Sort by file name. Maybe the sorting is redundant!?
        Collections.sort(validFiles, new Comparator<File>() {
            public int compare(File o1, File o2) {
                return o1.getName().compareTo(o2.getName());
            }
        });

        for (File file: validFiles) {
            readFile(file, map);
        }
    }

    public String getId(String id) {
        String value = map.get(id);
        if (value == null) {
            int i = id.indexOf("_");
            if (i >= 0) {
                value = map.get(id.substring(++i));
            }
        }
        if (value != null)
            //The external definition overwrites the id.
            id = value;

        return id;
    }
}
