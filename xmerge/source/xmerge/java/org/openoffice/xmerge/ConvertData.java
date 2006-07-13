/************************************************************************
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

