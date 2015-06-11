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

package org.apache.openoffice.ooxml.framework.part;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.parser.ElementContext;
import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;

public class RelatedParts
{
    RelatedParts (
        final PartName aPartName,
        final PartManager aPartManager)
    {
        maIdToTargetMap = new HashMap<>();
        maTypeToTargetsMap = new HashMap<>();

        final InputStream aStream = aPartManager.getStreamForPartName(aPartName.getRelationshipsPartName());
        if (aStream != null)
        {
            final Parser aParser = ParserFactory.getParser(
                ContentType.Relationships,
                aStream,
                null);
            aParser.GetActionManager().AddElementStartAction(
                "A_CT_Relationship",
                new IAction()
                {
                    @Override public void Run (
                        final ActionTrigger eTrigger,
                        final ElementContext aContext,
                        final String sText,
                        final Location aStartLocation,
                        final Location aEndLocation)
                    {
                        final String sId = aContext.GetAttributes().GetRawAttributeValue("A_Id");
                        final String sType = aContext.GetAttributes().GetRawAttributeValue("A_Type");
                        final String sTarget = aContext.GetAttributes().GetRawAttributeValue("A_Target");
                        String sTargetMode = aContext.GetAttributes().GetRawAttributeValue("A_TargetMode");
                        if (sTargetMode == null)
                            sTargetMode = "Internal";

                        AddRelationship(
                            sId,
                            RelationshipType.CreateFromString(sType),
                            new PartName(sTarget, aPartName, sTargetMode));
                    }
                }
            );
            aParser.Parse();
        }
    }




    private void AddRelationship (
        final String sId,
        final RelationshipType eType,
        final PartName aTarget)
    {
        maIdToTargetMap.put(sId, aTarget);

        Vector<PartName> aTargets = maTypeToTargetsMap.get(eType);
        if (aTargets == null)
        {
            aTargets = new Vector<>();
            maTypeToTargetsMap.put(eType, aTargets);
        }
        aTargets.add(aTarget);
    }




    public PartName GetTargetForId (final String sId)
    {
        return maIdToTargetMap.get(sId);
    }




    public Iterable<PartName> GetTargetsForType (final RelationshipType eType)
    {
        return maTypeToTargetsMap.get(eType);
    }



    public Iterable<PartName> getAllTargets ()
    {
        final Set<PartName> aAllNames = new TreeSet<>();
        aAllNames.addAll(maIdToTargetMap.values());
        return aAllNames;
    }




    public PartName GetSingleTargetForType (final RelationshipType eType)
    {
        if (maTypeToTargetsMap.get(eType).size() != 1)
        {
            System.out.printf("there are %d targets for relationship type %s\n",
                maTypeToTargetsMap.get(eType).size(),
                eType.toString());
            for (final PartName aName : maTypeToTargetsMap.get(eType))
            {
                System.out.printf("%s\n", aName);
            }
            assert(false);
        }
        return maTypeToTargetsMap.get(eType).firstElement();
    }




    private final Map<String,PartName> maIdToTargetMap;
    private final Map<RelationshipType, Vector<PartName>> maTypeToTargetsMap;
}
