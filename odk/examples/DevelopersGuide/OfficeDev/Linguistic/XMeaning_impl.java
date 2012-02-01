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

public class XMeaning_impl implements
    com.sun.star.linguistic2.XMeaning
{
    String      aMeaning;
    String[]    aSynonyms;

    public XMeaning_impl ( String aMeaning, String[] aSynonyms )
    {
        this.aMeaning   = aMeaning;
        this.aSynonyms  = aSynonyms;

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aMeaning == null)
            this.aMeaning = new String();

        // a meaning without synonyms may be OK though.
        // still for safety an empty existing array has to be provided.
        if (this.aSynonyms == null)
            this.aSynonyms = new String[]{};
    }

    // XMeaning
    public String getMeaning() throws com.sun.star.uno.RuntimeException
    {
        return aMeaning;
    }
    public String[] querySynonyms() throws com.sun.star.uno.RuntimeException
    {
        return aSynonyms;
    }
};

