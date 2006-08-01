/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NodeMergeAlgorithm.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:34:41 $
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

package org.openoffice.xmerge.merger;

import org.w3c.dom.Node;

import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.Difference;

/**
 *  This is an interface for a {@link
 *  org.openoffice.xmerge.merger.MergeAlgorithm
 *  MergeAlgorithm} to merge two <code>Node</code> objects.  It is an
 *  interface so that  different merge algorithms may be plugged-in.
 *
 *  @author smak
 */
public interface NodeMergeAlgorithm {

    /**
     *  This method is used to merge two given <code>Node</code>
     *  objects.  Note:  the original <code>Node</code> may be modified.
     *
     *  @param  originalNode  The original <code>Node</code>.
     *  @param  modifyNode    The <code>Node</code> to be merged.  It may
     *                        be modified.
     */
    public void merge(Node orginialNode, Node modifyNode);
}

