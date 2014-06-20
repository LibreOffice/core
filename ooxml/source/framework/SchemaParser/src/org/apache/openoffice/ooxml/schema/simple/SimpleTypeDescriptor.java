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

package org.apache.openoffice.ooxml.schema.simple;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

public class SimpleTypeDescriptor
{
    public SimpleTypeDescriptor (
        final QualifiedName aName)
    {
        maName = aName;
        maSubTypes = null;
    }




    public QualifiedName GetName()
    {
        return maName;
    }




    public void SetSubTypes (final ISimpleTypeNode[] aSubTypes)
    {
        maSubTypes = aSubTypes;
    }




    public ISimpleTypeNode[] GetSubType ()
    {
        return maSubTypes;
    }




    @Override
    public String toString ()
    {
        return "simple type "+maName;
    }




    public void Print (final Log aLog)
    {
        aLog.printf("%s\n", toString());
        aLog.StartBlock();
        for (final ISimpleTypeNode aSubType : maSubTypes)
            aSubType.Print(aLog);
        aLog.EndBlock();
    }




    private final QualifiedName maName;
    private ISimpleTypeNode[] maSubTypes;
}
