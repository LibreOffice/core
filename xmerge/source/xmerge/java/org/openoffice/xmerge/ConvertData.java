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

package org.openoffice.xmerge;

import java.util.Vector;
import java.util.Enumeration;

/**
 *  <p><code>ConvertData</code> is used as a container for passing
 *  <code>Document</code> objects in and out of the <code>Convert</code>
 *  class.  The <code>ConvertData</code> contains a <code>String</code>
 *  name and a <code>Vector</code> of <code>Document</code> objects.</p>
 *
 *  @author  Martin Maher 
 */
public class ConvertData {

    /**
     *  Vector of <code>Document</code> objects.
     */
    private Vector v = new Vector();

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
                v.removeAllElements();
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
    public Enumeration getDocumentEnumeration() {
        Enumeration enumerate = v.elements();
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

