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



import com.sun.star.lang.Locale;

public class XPossibleHyphens_impl implements
    com.sun.star.linguistic2.XPossibleHyphens
{
    String              aWord;
    String              aHyphWord;
    short[]             aOrigHyphenPos;
    Locale              aLang;

    public XPossibleHyphens_impl(
            String      aWord,
            Locale      aLang,
            String      aHyphWord,
            short[]     aOrigHyphenPos)
    {
        this.aWord = aWord;
        this.aLang = aLang;
        this.aHyphWord = aHyphWord;
        this.aOrigHyphenPos = aOrigHyphenPos;

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = new String();
        if (this.aLang == null)
            this.aLang = new Locale();
        if (this.aHyphWord == null)
            this.aHyphWord = new String();

        // having no hyphenation positions is OK though.
        // still for safety an empty existing array has to be provided.
        if (this.aOrigHyphenPos == null)
            this.aOrigHyphenPos = new short[]{};
    }

    // XPossibleHyphens
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }

    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLang;
    }
    public String getPossibleHyphens() throws com.sun.star.uno.RuntimeException
    {
        return aHyphWord;
    }
    public short[] getHyphenationPositions() throws com.sun.star.uno.RuntimeException
    {
        return aOrigHyphenPos;
    }
};
