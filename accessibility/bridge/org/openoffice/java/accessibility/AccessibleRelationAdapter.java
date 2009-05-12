/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleRelationAdapter.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
