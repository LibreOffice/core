/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextSearch.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:45:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
    * Has <b>OK</b> status if no runtime exceptions occured.
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


