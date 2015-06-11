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

public class Part
    implements IReferenceProvider
{
    public Part (
        final ContentType eType,
        final PartManager aPartManager,
        final PartName aPartName)
    {
        meContentType = eType;
        maPartManager = aPartManager;
        maPartName = aPartName;
        maRelatedParts = null;
    }




    public Part getPartById (final String sId)
    {
        final PartName aName = getRelatedParts().GetTargetForId(sId);
        return new Part(
            maPartManager.getContentTypes().getTypeForPartName(aName),
            maPartManager,
            aName);
    }




    public Part getPartByRelationshipType (final RelationshipType eType)
    {
        final PartName aName = getRelatedParts().GetSingleTargetForType(eType);
        return new Part(
            maPartManager.getContentTypes().getTypeForPartName(aName),
            maPartManager,
            aName);
    }




    public PartName getPartName ()
    {
        return maPartName;
    }




    public ContentType getContentType ()
    {
        return meContentType;
    }




    public InputStream getStream()
    {
        return maPartManager.getStreamForPartName(maPartName);
    }




    @Override
    public RelatedParts getRelatedParts ()
    {
        if (maRelatedParts == null)
        {
            maRelatedParts = new RelatedParts(
                maPartName,
                maPartManager);
        }
        return maRelatedParts;
    }




    private final ContentType meContentType;
    private final PartManager maPartManager;
    private final PartName maPartName;
    private RelatedParts maRelatedParts;
}
