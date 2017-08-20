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

package com.sun.star.sdbcx.comp.postgresql.sdbcx;

import java.util.List;

import org.apache.commons.lang3.mutable.MutableObject;
import org.apache.commons.lang3.tuple.Pair;

import com.sun.star.lang.NullPointerException;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.uno.XComponentContext;

/**
 * helper class for accessing resources shared by different libraries
 * in the connectivity module.
 */
public class SharedResources {
    private static SharedResources instance;
    private static int referenceCount = 0;

    private OfficeResourceBundle resourceBundle;

    // FIXME: the C++ implementation gets the XComponentContext using ::comphelper::getProcessServiceFactory(), we don't.
    public synchronized static void registerClient(XComponentContext context) {
        if (instance == null) {
            instance = new SharedResources(context);
        }
        ++referenceCount;
    }

    public synchronized static void revokeClient() {
        if (--referenceCount == 0) {
            CompHelper.disposeComponent(instance);
            instance = null;
        }
    }

    public synchronized static SharedResources getInstance() {
        return instance;
    }

    private SharedResources(XComponentContext context) {
        try {
            resourceBundle = new OfficeResourceBundle(context, "cnr");
        } catch (NullPointerException nullPointerException) {
        }
    }

    private int substitute( MutableObject<String> _inout_rString,
            String sPattern, String _rReplace ) {
        int nOccurences = 0;
        String string = _inout_rString.getValue();
        int nIndex = 0;
        while ( ( nIndex = string.indexOf( sPattern ) ) > -1 )
        {
            ++nOccurences;
            string = string.substring(0, nIndex) +
                    _rReplace + string.substring(nIndex + sPattern.length());
        }
        _inout_rString.setValue(string);
        return nOccurences;
    }


    /** loads a string from the shared resource file
        @param  _nResId
            the resource ID of the string
        @return
            the string from the resource file
     */
    public String
    getResourceString(
        int _nResId
    ) {
        if (resourceBundle == null) {
            return "";
        }
        return resourceBundle.loadString(_nResId);
    }

    /** loads a string from the shared resource file, and replaces
        a given ASCII pattern with a given string

        @param  _nResId
            the resource ID of the string to load
        @param  _pAsciiPatternToReplace
            the ASCII string which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute
            the String which should substitute the ASCII pattern.

        @return
            the string from the resource file, with applied string substitution
     */
    public String
    getResourceStringWithSubstitution(
        int _nResId,
        String _pAsciiPatternToReplace,
        String _rStringToSubstitute
    ) {
        MutableObject<String> string = new MutableObject<>(getResourceString(_nResId));
        substitute(string, _pAsciiPatternToReplace, _rStringToSubstitute);
        return string.getValue();
    }

    /** loads a string from the shared resource file, and replaces
        a given ASCII pattern with a given string

        @param  _nResId
            the resource ID of the string to load
        @param  _pAsciiPatternToReplace1
            the ASCII string (1) which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute1
            the String which should substitute the ASCII pattern (1)
        @param  _pAsciiPatternToReplace2
            the ASCII string (2) which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute2
            the String which should substitute the ASCII pattern (2)

        @return
            the string from the resource file, with applied string substitution
     */
    public String
    getResourceStringWithSubstitution(
        int _nResId,
        String _pAsciiPatternToReplace1,
        String _rStringToSubstitute1,
        String _pAsciiPatternToReplace2,
        String _rStringToSubstitute2
    ) {
        MutableObject<String> string = new MutableObject<>(getResourceString(_nResId));
        substitute(string, _pAsciiPatternToReplace1, _rStringToSubstitute1);
        substitute(string, _pAsciiPatternToReplace2, _rStringToSubstitute2);
        return string.getValue();
    }

    /** loads a string from the shared resource file, and replaces
        a given ASCII pattern with a given string

        @param  _nResId
            the resource ID of the string to load
        @param  _pAsciiPatternToReplace1
            the ASCII string (1) which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute1
            the String which should substitute the ASCII pattern (1)
        @param  _pAsciiPatternToReplace2
            the ASCII string (2) which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute2
            the String which should substitute the ASCII pattern (2)
        @param  _pAsciiPatternToReplace3
            the ASCII string (3) which is to search in the string. Must not be <NULL/>.
        @param  _rStringToSubstitute3
            the String which should substitute the ASCII pattern (3)

        @return
            the string from the resource file, with applied string substitution
     */
    public String
    getResourceStringWithSubstitution(
        int _nResId,
        String _pAsciiPatternToReplace1,
        String _rStringToSubstitute1,
        String _pAsciiPatternToReplace2,
        String _rStringToSubstitute2,
        String _pAsciiPatternToReplace3,
        String _rStringToSubstitute3
    ) {
        MutableObject<String> string = new MutableObject<>(getResourceString(_nResId));
        substitute(string, _pAsciiPatternToReplace1, _rStringToSubstitute1);
        substitute(string, _pAsciiPatternToReplace2, _rStringToSubstitute2);
        substitute(string, _pAsciiPatternToReplace3, _rStringToSubstitute3);
        return string.getValue();
    }

    /** loads a string from the shared resource file, and replaces a given ASCII pattern with a given string

        @param  _nResId
            the resource ID of the string to load
        @param  _aStringToSubstitutes
            A list of substitutions.

        @return
            the string from the resource file, with applied string substitution
     */
    public String
    getResourceStringWithSubstitution( int _nResId,
            List<Pair<String,String>> patternsAndSubstitutes) {
        MutableObject<String> string = new MutableObject<>(getResourceString(_nResId));
        for (Pair<String,String> pair : patternsAndSubstitutes) {
            substitute(string, pair.getLeft(), pair.getRight());
        }
        return string.getValue();
    }
}
