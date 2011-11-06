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



/*
 * XMLHelper.java
 *
 * Created on 30. September 2003, 15:38
 */
package com.sun.star.wizards.common;

import org.w3c.dom.*;

/**
 *
 * @author  rpiterman
 */
public class XMLHelper
{

    public static Node addElement(Node parent, String name, String[] attNames, String[] attValues)
    {
        Document doc = parent.getOwnerDocument();
        if (doc == null)
        {
            doc = (Document) parent;
        }
        Element e = doc.createElement(name);
        for (int i = 0; i < attNames.length; i++)
        {
            if (attValues[i] != null && (!attValues[i].equals(PropertyNames.EMPTY_STRING)))
            {
                e.setAttribute(attNames[i], attValues[i]);
            }
        }
        parent.appendChild(e);
        return e;
    }

    public static Node addElement(Node parent, String name, String attNames, String attValues)
    {
        return addElement(parent, name, new String[]
                {
                    attNames
                }, new String[]
                {
                    attValues
                });
    }
}
