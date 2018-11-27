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



package org.openoffice.java.accessibility;

import javax.accessibility.Accessible;
import com.sun.star.accessibility.AccessibleRelation;
import com.sun.star.accessibility.XAccessible;

/** 
 */
public abstract class AccessibleRelationTypeMap {
    
    final static String[] data = { 
        null,
        javax.accessibility.AccessibleRelation.CONTROLLED_BY,
        javax.accessibility.AccessibleRelation.CONTROLLED_BY,
        javax.accessibility.AccessibleRelation.CONTROLLER_FOR,
        javax.accessibility.AccessibleRelation.CONTROLLER_FOR,
        javax.accessibility.AccessibleRelation.LABEL_FOR,
        javax.accessibility.AccessibleRelation.LABEL_FOR,
        javax.accessibility.AccessibleRelation.LABELED_BY,
        javax.accessibility.AccessibleRelation.LABELED_BY,
        javax.accessibility.AccessibleRelation.MEMBER_OF,
        javax.accessibility.AccessibleRelation.MEMBER_OF
    };
    
    public static void fillAccessibleRelationSet(javax.accessibility.AccessibleRelationSet s, AccessibleRelation[] relations) {
        AccessibleObjectFactory factory = AccessibleObjectFactory.getDefault();
        for(int i=0; i<relations.length; i++) {
            if( relations[i].RelationType < data.length && data[relations[i].RelationType] != null ) {
                javax.accessibility.AccessibleRelation r = 
                    new javax.accessibility.AccessibleRelation(data[relations[i].RelationType]);
                
                r.setTarget(factory.getAccessibleObjectSet(relations[i].TargetSet));
                s.add(r);
            }
        }
    }
}
