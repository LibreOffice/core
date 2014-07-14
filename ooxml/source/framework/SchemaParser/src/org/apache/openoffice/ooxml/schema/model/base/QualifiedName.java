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

package org.apache.openoffice.ooxml.schema.model.base;

import javax.xml.namespace.QName;

/** Similar to the QName class.  A qualified name that consists of the local
 *  part and a namespace.
 *  The namespace is stored both as URI and short form (prefix).
 */
public class QualifiedName
    implements Comparable<QualifiedName>
{
    public QualifiedName (final QName aName)
    {
        msLocalPart = aName.getLocalPart();
        msNamespacePrefix = aName.getPrefix();
        msNamespaceURI = aName.getNamespaceURI();
    }




    public QualifiedName (
        final String sNamespaceURI,
        final String sNamespacePrefix,
        final String sLocalPart)
    {
        msLocalPart = sLocalPart;
        msNamespacePrefix = sNamespacePrefix;
        msNamespaceURI = sNamespaceURI;
    }




    public QualifiedName (final String sLocalPart)
    {
        this(null, null, sLocalPart);
    }




    /** Return a textual representation for informal (and informative) display.
     */
    public String GetDisplayName ()
    {
        if (msNamespacePrefix == null)
            return msLocalPart;
        else
            return msNamespacePrefix + ":" + msLocalPart;
    }




    public String GetStateName()
    {
        if (msNamespacePrefix == null)
            return msLocalPart;
        else
            return msNamespacePrefix + "_" + msLocalPart;
    }




    public String GetNamespaceURI ()
    {
        return msNamespaceURI;
    }




    public String GetNamespacePrefix ()
    {
        return msNamespacePrefix;
    }




    public String GetLocalPart ()
    {
        return msLocalPart;
    }




    /** Compare QualifiedName objects (e.g. for sorting them).
     *  Primary sort key is the local part.
     *  Secondary key is the namespace prefix.
     *  Missing prefixes come before existing prefixes.
     */
    @Override
    public int compareTo (final QualifiedName aOther)
    {
        final int nComparisonResult = msLocalPart.compareTo(aOther.msLocalPart);
        if (nComparisonResult != 0)
            return nComparisonResult;
        else
            if (msNamespacePrefix==null && aOther.msNamespacePrefix==null)
                return 0;
            else if (msNamespacePrefix!=null && aOther.msNamespacePrefix!=null)
                return msNamespacePrefix.compareTo(aOther.msNamespacePrefix);
            else if (msNamespacePrefix==null)
                return -1;
            else
                return +1;
    }




    @Override
    public String toString ()
    {
        return GetDisplayName();
    }




    private final String msLocalPart;
    private final String msNamespacePrefix;
    private final String msNamespaceURI;
}
