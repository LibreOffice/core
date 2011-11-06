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

