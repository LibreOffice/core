/**
 * Description: operations for the a text string.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Feb 22, 2001
 * Xiaofeng Xie    May 12, 2004
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

import java.util.*;

public class GlobalString {

    /**
     * Tokenize a String with given key.
     *
     * @param input the String to be tokenized.
     * @param tokenKey the delimiters.
     * @return a String array that include the elements of input string that
     *         divided by the tokenKey.
     */
    public static String[] tokenize(String input, String tokenKey) {
        ArrayList<String> v = new ArrayList<String>();
        StringTokenizer t = new StringTokenizer(input, tokenKey);
        while (t.hasMoreTokens()) {
            v.add(t.nextToken());
        }
        return v.toArray(new String[v.size()]);
    }

}