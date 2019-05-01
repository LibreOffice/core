/**
 * Description: For generating random numbers.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Feb 22, 2001
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Please acknowledge the author(s) if you use this code in any way.
 *
 * @version 1.0
 * @Since MAOS1.0
 */

package net.adaptivebox.global;

import java.util.Random;

public class RandomGenerator {
    /**
     * Pseudo-random number generator instance.
     */
    private static Random PRNG = new Random();

    /**
     * This function returns a random integer number between the lowLimit and
     * upLimit.
     *
     * @param lowLimit
     *            lower limits upLimit The upper limits (between which the
     *            random number is to be generated)
     * @return int return value Example: for find [0,1,2]
     */
    public static int intRangeRandom(int lowLimit, int upLimit) {
        int num = lowLimit + PRNG.nextInt(upLimit - lowLimit + 1);
        return num;
    }

    /**
     * This function returns a random float number between the lowLimit and
     * upLimit.
     *
     * @param lowLimit
     *            lower limits upLimit The upper limits (between which the
     *            random number is to be generated)
     * @return double return value
     */
    public static double doubleRangeRandom(double lowLimit, double upLimit) {
        double num = lowLimit + PRNG.nextDouble() * (upLimit - lowLimit);
        return num;
    }

    public static int[] randomSelection(int maxNum, int times) {
        if (maxNum < 0) {
            maxNum = 0;
        }

        if (times < 0) {
            times = 0;
        }

        int[] all = new int[maxNum];
        for (int i = 0; i < all.length; i++) {
            all[i] = i;
        }

        /* https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */
        int[] indices = new int[Math.min(maxNum, times)];
        for (int i = 0, j, value; i < indices.length; i++) {
            j = intRangeRandom(i, all.length - 1);

            value = all[j];
            all[j] = all[i];
            indices[i] = all[i] = value;
        }

        return indices;
    }
}
