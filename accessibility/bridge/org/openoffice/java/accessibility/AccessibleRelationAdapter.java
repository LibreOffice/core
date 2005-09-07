/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleRelationAdapter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:33:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
