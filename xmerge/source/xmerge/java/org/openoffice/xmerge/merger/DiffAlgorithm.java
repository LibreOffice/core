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

/**
 * This is the difference algorithm interface.
 *
 * <p>It is an interface so that different algorithms may be plugged-in to
 * actually compute the differences.</p>
 *
 * <p>NOTE: this code may not be thread safe.</p>
 */
public interface DiffAlgorithm {

    /**
     * Returns a {@code Difference} array.
     *
     * <p>This method finds out the difference between two sequences.</p>
     *
     * @param   orgSeq  The original sequence of object.
     * @param   modSeq  The modified (or changed) sequence to compare against
     *                  with the original.
     *
     * @return  A {@code Difference} array.
     */
    Difference[] computeDiffs(Iterator orgSeq, Iterator modSeq);
}