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

package org.openoffice.idesupport;

import java.io.File;
import java.util.ArrayList;
import org.openoffice.idesupport.zip.ParcelZipper;

import com.sun.star.script.framework.container.ScriptEntry;

public class ExtensionFinder implements MethodFinder {

    private String[] extensions;
    private String language;

    public ExtensionFinder(String language, String[] extensions) {
        this.language = language;
        this.extensions = extensions;
    }

    public ScriptEntry[] findMethods(File basedir) {
        String parcelName;
        ArrayList files = new ArrayList(10);
        ScriptEntry[] empty = new ScriptEntry[0];

        if (basedir == null || basedir.exists() == false ||
            basedir.isDirectory() == false)
            return empty;

        parcelName = basedir.getName();
        if (parcelName.equals(ParcelZipper.CONTENTS_DIRNAME))
            parcelName = basedir.getParentFile().getName();

        findFiles(files, basedir, parcelName);

        if (files.size() != 0)
            return (ScriptEntry[])files.toArray(empty);
        return empty;
    }

    private void findFiles(ArrayList list, File basedir, String parcelName) {
        File[] children = basedir.listFiles();
        File f;

        for (int i = 0; i < children.length; i++) {
            f = children[i];

            if (f.isDirectory())
                findFiles(list, f, parcelName);
            else {
                for (int j = 0; j < extensions.length; j++) {
                    if (f.getName().endsWith(extensions[j])) {
                        ScriptEntry entry = new ScriptEntry(language,
                            f.getName(), f.getName(), parcelName);
                        list.add(entry);
                        break;
                    }
                }
            }
        }
    }
}
