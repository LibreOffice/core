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


public class XSpellAlternatives_impl implements
    com.sun.star.linguistic2.XSpellAlternatives
{
    String      aWord;
    Locale      aLanguage;
    String[]    aAlt;           // list of alternatives, may be empty.
    short       nType;          // type of failure

    public XSpellAlternatives_impl(
            String      aWord,
            Locale      aLanguage,
            short       nFailureType,
            String[]    aAlt )
    {
        this.aWord      = aWord;
        this.aLanguage  = aLanguage;
        this.aAlt       = aAlt;
        this.nType      = nFailureType;

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = new String();
        if (this.aLanguage == null)
            this.aLanguage = new Locale();

        // having no alternatives is OK though.
        // still for safety an empty existing array has to be provided.
        if (this.aAlt == null)
            this.aAlt = new String[]{};
    }

    // XSpellAlternatives
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }
    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLanguage;
    }
    public short getFailureType() throws com.sun.star.uno.RuntimeException
    {
        return nType;
    }
    public short getAlternativesCount() throws com.sun.star.uno.RuntimeException
    {
        return (short) aAlt.length;
    }
    public String[] getAlternatives() throws com.sun.star.uno.RuntimeException
    {
        return aAlt;
    }
};

