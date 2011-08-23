/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.xmerge.merger;

import org.w3c.dom.Node;

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

