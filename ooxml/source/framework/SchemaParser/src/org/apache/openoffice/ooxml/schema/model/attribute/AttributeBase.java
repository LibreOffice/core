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

package org.apache.openoffice.ooxml.schema.model.attribute;

import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Base class for both Attribute and AttributeReference classes.
 */
public abstract class AttributeBase
    extends Node
{
    public enum Use
    {
        Optional,
        Required
    }



    public AttributeBase (
        final QualifiedName aName,
        final String sUse,
        final String sDefault,
        final String sFixed,
        final Location aLocation)
    {
        super(null, aName, aLocation);

        switch(sUse)
        {
            case "optional":
                meUse = Use.Optional;
                break;
            case "required":
                meUse = Use.Required;
                break;
            default:
                throw new RuntimeException("value of 'use' attribute is neither 'optional' nor 'required' but "+sUse);
        }
        msDefault = sDefault;
        msFixed = sFixed;
    }




    @Override
    public String toString ()
    {
        String sText = "use "+meUse.toString();
        if (msDefault != null)
            sText += ", default "+msDefault;
        else
            sText += ", no default";
        if (msFixed != null)
            sText += ", fixed to "+msFixed;
        else
            sText += ", not fixed";
        return sText;
    }




    private final Use meUse;
    private final String msDefault;
    private final String msFixed;
}
