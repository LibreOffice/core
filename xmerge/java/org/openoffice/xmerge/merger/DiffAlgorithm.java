/************************************************************************
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

/**
 * This is the difference algorithm interface.  It is an interface so
 * that different algorithms may be plugged-in to actually compute
 * the differences.
 *
 * NOTE: this code may not be thread safe.
 */
public interface DiffAlgorithm {

    /**
     *  Returns a <code>Difference</code> array.  This method finds out
     *  the difference between two sequences.
     *
     *  @param  orgSeq  The original sequence of object.
     *  @param  modSeq  The modified (or changed) sequence to
     *                  compare against with the origial.
     *
     *  @return  A <code>Difference</code> array.
     */
    public Difference[] computeDiffs(Iterator orgSeq, Iterator modSeq);
}

