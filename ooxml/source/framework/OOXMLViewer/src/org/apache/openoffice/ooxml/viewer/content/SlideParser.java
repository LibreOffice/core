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

import java.util.Vector;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.framework.part.Part;
import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.parser.ElementContext;
import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;

public class SlideParser
{
    SlideParser (final Part aPart)
    {
        maParser = ParserFactory.getParser(
            aPart.getContentType(),
            aPart.getStream(),
            new Vector<String>());
        mnShapeCount = 0;

        maParser.GetActionManager().AddElementStartAction(
            "p06_CT_Shape",
            new IAction()
            {
                @Override public void Run(ActionTrigger eTrigger, ElementContext aContext,
                    String sText, Location aStartLocation, Location aEndLocation)
                {
                    SlideParser.this.IncreaseShapeCount();
                }
            });
    }




    Slide ParseSlide (final int nSlideIndex)
    {
        maParser.Parse();
        final Slide aSlide = new Slide("slide "+nSlideIndex, mnShapeCount);
        return aSlide;
    }




    private void IncreaseShapeCount ()
    {
        ++mnShapeCount;
    }




    private final Parser maParser;
    private int mnShapeCount;
}
