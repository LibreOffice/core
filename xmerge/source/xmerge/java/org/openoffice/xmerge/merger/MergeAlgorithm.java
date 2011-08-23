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
     *  @param  objSeq       The original sequence which the difference
     *                       will be applied.  It will be modified.
     *  @param  modSeq       The modified sequence where the difference
     *                       content will be extracted.
     *  @param  differences  The <code>Difference</code> array.
     *
     *  @return  An <code>Iterator</code> which is the modified original
     *           <code>Iterator</code> Sequence.  Same as the first parameter.
     *
     *  @throws  MergeException  If an error occurs during the merge.
     */
    public void applyDifference(Iterator orgSeq, Iterator modSeq,
                                Difference[] differences) throws MergeException;
}

