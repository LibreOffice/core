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



package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.SearchAlgorithms;
import com.sun.star.util.SearchFlags;
import com.sun.star.util.SearchOptions;
import com.sun.star.util.SearchResult;
import com.sun.star.util.XTextSearch;

/**
* Testing <code>com.sun.star.util.XTextSearch</code>
* interface methods :
* <ul>
*  <li><code> setOptions()</code></li>
*  <li><code> searchForward()</code></li>
*  <li><code> searchBackward()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XTextSearch
*/
public class _XTextSearch extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XTextSearch oObj = null ;

    protected final String str = "acababaabcababadcdaa" ;
    protected final int startPos = 2 , endPos = 20 ;
    protected final String searchStr = "(ab)*a(c|d)+" ;
    protected final int fStartRes = 10, fEndRes = 18 ;
    protected final int bStartRes = 18, bEndRes = 14 ;

    /**
    * Sets options for searching regular expression in a string,
    * ignoring case. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _setOptions() {

        SearchOptions opt = new SearchOptions() ;
        opt.algorithmType = SearchAlgorithms.REGEXP ;
        opt.searchFlag = SearchFlags.ALL_IGNORE_CASE ;
        opt.searchString = searchStr ;

        oObj.setOptions(opt) ;

        tRes.tested("setOptions()", true) ;
    }


    /**
    * Tries to find a substring matching regular expression. <p>
    * Has <b>OK</b> if the correct substring position returned.
    */
    public void _searchForward() {
        requiredMethod("setOptions()") ;

        SearchResult res = oObj.searchForward(str, startPos, endPos) ;

        log.println("Result of searching '" + searchStr + "' substring in \n'" +
            str + "' string (" + res.subRegExpressions + " matches):") ;

        for (int i = 0; i < res.subRegExpressions; i++)
            log.println("  (" + res.startOffset[i] + ", " + res.endOffset[i] + ")") ;

        tRes.tested("searchForward()", res.subRegExpressions > 0 &&
            res.startOffset[0] == fStartRes && res.endOffset[0] == fEndRes) ;
    }

    /**
    * Tries to find a substring matching regular expression walking
    * backward. <p>
    * Has <b>OK</b> if the correct substring position returned.
    */
    public void _searchBackward() {
        requiredMethod("setOptions()") ;

        SearchResult res = oObj.searchBackward(str, endPos, startPos) ;

        log.println("Result of searching '" + searchStr + "' substring in \n'" +
            str + "' string (" + res.subRegExpressions + " matches):") ;

        for (int i = 0; i < res.subRegExpressions; i++)
            log.println("  (" + res.startOffset[i] + ", " + res.endOffset[i] + ")") ;

        tRes.tested("searchBackward()", res.subRegExpressions > 0 &&
            res.startOffset[0] == bStartRes && res.endOffset[0] == bEndRes) ;
    }

}


