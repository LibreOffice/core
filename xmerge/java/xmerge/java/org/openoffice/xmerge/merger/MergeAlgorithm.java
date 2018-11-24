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



package org.openoffice.xmerge.merger;

import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.Difference;

/**
 *  This is the <code>MergeAlgorithm</code> interface. It is an
 *  interface so that different merge algorithms may be plugged-in
 *  to actually merge the diffs back to an original document.
 *
 *  @author smak
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

