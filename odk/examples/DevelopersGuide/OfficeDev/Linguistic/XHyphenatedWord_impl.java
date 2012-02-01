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

public class XHyphenatedWord_impl implements
    com.sun.star.linguistic2.XHyphenatedWord
{
    String     aWord;
    String     aHyphenatedWord;
    short      nHyphenPos;
    short      nHyphenationPos;
    Locale     aLang;
    boolean    bIsAltSpelling;

    public XHyphenatedWord_impl(
            String      aWord,
            Locale      aLang,
            short       nHyphenationPos,
            String      aHyphenatedWord,
            short       nHyphenPos )
    {
        this.aWord = aWord;
        this.aLang = aLang;
        this.nHyphenationPos = nHyphenationPos;
        this.aHyphenatedWord = aHyphenatedWord;
        this.nHyphenPos = nHyphenPos;
        this.bIsAltSpelling = (aWord != aHyphenatedWord);

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = new String();
        if (this.aLang == null)
            this.aLang = new Locale();
        if (this.aHyphenatedWord == null)
            this.aHyphenatedWord = new String();
    }


    // XHyphenatedWord
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }
    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLang;
    }
    public short getHyphenationPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenationPos;
    }
    public String getHyphenatedWord() throws com.sun.star.uno.RuntimeException
    {
        return aHyphenatedWord;
    }
    public short getHyphenPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenPos;
    }
    public boolean isAlternativeSpelling() throws com.sun.star.uno.RuntimeException
    {
        return bIsAltSpelling;
    }
};
