/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.merger;

import org.w3c.dom.Node;

/**
 *  This is an interface for a {@link
 *  org.openoffice.xmerge.merger.MergeAlgorithm
 *  MergeAlgorithm} to merge two <code>Node</code> objects.  It is an
 *  interface so that  different merge algorithms may be plugged-in.
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
    public void merge(Node originalNode, Node modifyNode);
}

