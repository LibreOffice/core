/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge;

import java.util.ArrayList;
import java.util.Iterator;

/**
 *  <p><code>ConvertData</code> is used as a container for passing
 *  <code>Document</code> objects in and out of the <code>Convert</code>
 *  class.  The <code>ConvertData</code> contains a <code>String</code>
 *  name and a <code>Vector</code> of <code>Document</code> objects.</p>
 *
 */
public class ConvertData {

    /**
     *  Vector of <code>Document</code> objects.
     */
    private ArrayList<Object> v = new ArrayList<Object>();

    /**
     *  Name of the <code>ConvertData</code> object.
     */
    private String name;


    /**
     *  Resets ConvertData.  This empties all <code>Document</code>
     *  objects from this class.  This allows reuse of a
     *  <code>ConvertData</code>.
     */
    public void reset() {
        name = null;
        v.clear();
    }

    /**
     *  Returns the <code>Document</code> name.
     *
     *  @return  The <code>Document</code> name.
     */
    public String getName() {
        return name;
    }


    /**
     *  Sets the <code>Document</code> name.
     *
     *  @param  docName  The name of the <code>Document</code>.
     */
    public void setName(String docName) {
        name = docName;
    }


    /**
     *  Adds a <code>Document</code> to the vector.
     *
     *  @param  doc  The <code>Document</code> to add.
     */
    public void addDocument(Document doc) {
        v.add(doc);
    }


    /**
     *  Gets an <code>Enumeration</code> to access the <code>Vector</code>
     *  of <code>Document</code> objects.
     *
     *  @return  The <code>Enumeration</code> to access the
     *           <code>Vector</code> of <code>Document</code> objects.
     */
    public Iterator<Object> getDocumentEnumeration() {
        Iterator<Object> enumerate = v.iterator();
        return (enumerate);
    }


    /**
     *  Gets the number of <code>Document</code> objects currently stored
     *
     *  @return  The number of <code>Document</code> objects currently
     *           stored.
     */
    public int getNumDocuments() {
        return (v.size());
    }
}

