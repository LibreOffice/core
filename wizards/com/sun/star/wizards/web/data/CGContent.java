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


package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.*;

import org.w3c.dom.*;

public class CGContent extends ConfigSetItem implements XMLProvider
{

    public String dirName;
    public String cp_Name;
    public String cp_Description;
    public ConfigSet cp_Contents = new ConfigSet(CGContent.class);
    public ConfigSet cp_Documents = new ConfigSet(CGDocument.class);

    public CGContent()
    {
        /*cp_Documents = new ConfigSet(CGDocument.class) {
        protected DefaultListModel createChildrenList() {
        return cp_Contents.getChildrenList();
        }
        };*/
    }

    public Node createDOM(Node parent)
    {

        Node myElement = XMLHelper.addElement(parent, "content",
                new String[]
                {
                    "name", "directory-name", "description", "directory"
                },
                new String[]
                {
                    cp_Name, dirName, cp_Description, dirName
                });

        cp_Documents.createDOM(myElement);

        return myElement;
    }
}
