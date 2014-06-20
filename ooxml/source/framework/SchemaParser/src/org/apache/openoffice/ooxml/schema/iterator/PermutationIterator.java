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

package org.apache.openoffice.ooxml.schema.iterator;


/** Enumerate all permutations of a given array with elements of type T.
 *  The permutations are created in place, i.e. the array given to the constructor
 *  is modified as side effect of calling HasMore().
 *
 *  The algorithm is taken from "The Art of Computer Programming, Volume 4,
 *  Fasicle 2, by Donald E. Knuth" from section 7.2.1.2, Algorithm P.
 */
public class PermutationIterator<T>
{
    public PermutationIterator (
        final T[] aItems)
    {
        // Count the nodes.
        mnItemCount = aItems.length;

        // Set up three arrays, one with the actual nodes, one for the inversions
        // and one for directions.
        maItems = aItems;
        maInversions = new int[mnItemCount];
        maDirections = new int[mnItemCount];
        for (int nIndex=0; nIndex<mnItemCount; ++nIndex)
        {
            maInversions[nIndex] = 0;
            maDirections[nIndex] = 1;
        }

        mbHasMorePermutations = mnItemCount>0;
        mbIsNextPermutationReady = true;
    }




    public boolean HasMore ()
    {
        if ( ! mbIsNextPermutationReady)
            ProvideNextPermutation();
        return mbHasMorePermutations;
    }




    public T[] Next()
    {
        assert(mbHasMorePermutations && mbIsNextPermutationReady);
        mbIsNextPermutationReady = false;
        return maItems;
    }




    private void ProvideNextPermutation ()
    {
        mbIsNextPermutationReady = true;

        // Create the next permutation.
        int nJ = mnItemCount;
        int nS = 0;

        while (true)
        {
            final int nQ = maInversions[nJ-1] + maDirections[nJ-1];
            if (nQ>=0 && nQ<nJ)
            {
                // Exchange j-cj+s and j-q+s
                final int nIndexA = nJ-maInversions[nJ - 1]+nS - 1;
                final int nIndexB = nJ-nQ+nS - 1;
                final T aItem = maItems[nIndexA];
                maItems[nIndexA] = maItems[nIndexB];
                maItems[nIndexB] = aItem;

                // cj=q
                maInversions[nJ - 1] = nQ;

                // Next permutation is ready.
                break;
            }
            else
            {
                if (nQ==nJ)
                {
                    // Increase s.
                    if (nJ == 1)
                    {
                        // All permutations generated.
                        mbHasMorePermutations = false;
                        break;
                    }
                    else
                    {
                        ++nS;
                    }
                }

                // Switch direction.
                maDirections[nJ - 1] = -maDirections[nJ - 1];
                --nJ;
            }
        }
    }




    private final int mnItemCount;
    private final T[] maItems;
    private final int[] maInversions;
    private final int[] maDirections;
    private boolean mbIsNextPermutationReady;
    private boolean mbHasMorePermutations;
}
