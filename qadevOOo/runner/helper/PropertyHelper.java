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



package helper;

import java.util.ArrayList;

import com.sun.star.beans.PropertyValue;

public class PropertyHelper
{
    /**
       Create a PropertyValue[] from a ArrayList
       @param _aPropertyList
       @return a PropertyValue[]
    */
    public static PropertyValue[] createPropertyValueArrayFormArrayList(ArrayList _aPropertyList)
        {
            // copy the whole PropertyValue List to an PropertyValue Array
            PropertyValue[] aSaveProperties = null;

            if (_aPropertyList == null)
            {
                aSaveProperties = new PropertyValue[0];
            }
            else
            {
                aSaveProperties = (PropertyValue[])_aPropertyList.toArray(new PropertyValue[_aPropertyList.size()]);
                // old java 1.4
//                if (_aPropertyList.size() > 0)
//                {
//                    aSaveProperties = new PropertyValue[_aPropertyList.size()];
//                    for (int i = 0;i<_aPropertyList.size(); i++)
//                    {
//                        aSaveProperties[i] = (PropertyValue) _aPropertyList.get(i);
//                    }
//                }
//                else
//                {
//                    aSaveProperties = new PropertyValue[0];
//                }

// show properties?
//                 if (_aPropertyList.size() > 0)
//                 {
//                     // aSaveProperties = new PropertyValue[_aPropertyList.size()];
//                     for (int i = 0;i<_aPropertyList.size(); i++)
//                     {
//                         PropertyValue aProp = (PropertyValue) _aPropertyList.get(i);
//                         showProperty(aProp);
//                     }
//                 }

            }
            return aSaveProperties;
        }
}
