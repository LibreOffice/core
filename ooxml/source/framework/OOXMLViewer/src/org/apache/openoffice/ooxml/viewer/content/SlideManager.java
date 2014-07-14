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

package org.apache.openoffice.ooxml.viewer.content;

import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.apache.openoffice.ooxml.framework.part.Part;
import org.apache.openoffice.ooxml.framework.part.PartName;

public class SlideManager
{
    SlideManager ()
    {
        maSlides = new Vector<>();
        maPartNameToSlideMap = new TreeMap<>();
    }



    public void AddSlide (
        final PartName aName,
        final Slide aSlide)
    {
        maSlides.add(aSlide);
        maPartNameToSlideMap.put(aName, aSlide);
    }




    public Iterable<Slide> GetSlides ()
    {
        return maSlides;
    }




    public Slide GetSlideForPart (final Part aPart)
    {
        Slide aSlide = maPartNameToSlideMap.get(aPart.getPartName());
        if (aSlide == null)
        {
            aSlide = new SlideParser(aPart).ParseSlide(maSlides.size());

            AddSlide(aPart.getPartName(), aSlide);
        }
        return aSlide;
    }




    private final Vector<Slide> maSlides;
    private final Map<PartName,Slide> maPartNameToSlideMap;
}
