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

import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.Difference;

/**
 *  This is the <code>MergeAlgorithm</code> interface. It is an
 *  interface so that different merge algorithms may be plugged-in
 *  to actually merge the diffs back to an original document.
 */
public interface MergeAlgorithm {

    /**
     *  This method is to merge the difference to an <code>Iterator</code>.
     *  The original <code>Iterator</code> will be modified after the call.
     *
     *  @param  orgSeq       The original sequence which the difference
     *                       will be applied.  It will be modified.
     *  @param  modSeq       The modified sequence where the difference
     *                       content will be extracted.
     *  @param  differences  The <code>Difference</code> array.
     *
     *  @throws  MergeException  If an error occurs during the merge.
     */
    public void applyDifference(Iterator orgSeq, Iterator modSeq,
                                Difference[] differences) throws MergeException;
}

