/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package ifc.i18n;

import java.util.ArrayList;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.i18n.Boundary;
import com.sun.star.i18n.LineBreakHyphenationOptions;
import com.sun.star.i18n.LineBreakResults;
import com.sun.star.i18n.LineBreakUserOptions;
import com.sun.star.i18n.ScriptType;
import com.sun.star.i18n.WordType;
import com.sun.star.i18n.XBreakIterator;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XBreakIterator</code>
* interface methods :
* <ul>
*  <li><code> nextCharacters()</code></li>
*  <li><code> previousCharacters()</code></li>
*  <li><code> nextWord()</code></li>
*  <li><code> previousWord()</code></li>
*  <li><code> getWordBoundary()</code></li>
*  <li><code> getWordType()</code></li>
*  <li><code> isBeginWord()</code></li>
*  <li><code> isEndWord()</code></li>
*  <li><code> beginOfSentence()</code></li>
*  <li><code> endOfSentence()</code></li>
*  <li><code> getLineBreak()</code></li>
*  <li><code> beginOfScript()</code></li>
*  <li><code> endOfScript()</code></li>
*  <li><code> nextScript()</code></li>
*  <li><code> previousScript()</code></li>
*  <li><code> getScriptType()</code></li>
*  <li><code> beginOfCharBlock()</code></li>
*  <li><code> endOfCharBlock()</code></li>
*  <li><code> nextCharBlock()</code></li>
*  <li><code> previousCharBlock()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Locale'</code>
*   (of type <code>com.sun.star.lang.Locale</code>):
*   this locale is used as locale argument for tested methods.
*  </li>
*  <li> <code>'UnicodeString'</code>
*   (of type <code>String</code>): Unicode string which is passed
*   to methods except 'CharacterBlock' methods.
*  </li>
* <ul> <p>
* @see com.sun.star.i18n.XBreakIterator
*/
public class _XBreakIterator extends MultiMethodTest {

    public XBreakIterator oObj = null;

    Locale locale = null;
    String UnicodeString = null;

    short wordType = WordType.ANYWORD_IGNOREWHITESPACES;

    /**
     * Retrieves object relations.
     * @throws StatusException If one of relations not found.
     */
    @Override
    protected void before() {
        locale = (Locale)tEnv.getObjRelation("Locale");
        if (locale == null) {
            throw new StatusException
                (Status.failed("Relation 'Locale' not found")) ;
        }

        UnicodeString = (String)tEnv.getObjRelation("UnicodeString");
        if (UnicodeString == null) {
            throw new StatusException(Status.failed
                ("Relation 'UnicodeString' not found")) ;
        }
    }

    /**
     * Compares returned next character positions with expected values. <p>
     *
     * Has <b>OK</b> status if position after travel and traveled length
     * has expected values.
     */
    public void _nextCharacters() {
        short nCharacterIteratorMode =
            com.sun.star.i18n.CharacterIteratorMode.SKIPCHARACTER;

        int strLength = UnicodeString.length();

        //Start from position : Travel ... chars :
        // Actual position after : How many chars traveled
        int[][] nextCharacters = {
            { 1, 5000, strLength , strLength - 1 },
            { 10, 6, 16, 6}};

        boolean bRes = true;

        for(int i = 0; i < nextCharacters.length; i++) {
            int[] lDone = new int[1];
            long lRes = oObj.nextCharacters(UnicodeString, nextCharacters[i][0],
                locale, nCharacterIteratorMode, nextCharacters[i][1], lDone);
            log.println("Expected result is: lRes = " + nextCharacters[i][2] +
                        "; lDone = " + nextCharacters[i][3] );
            log.println("Actual result is: lRes = " + lRes +
                        "; lDone = " + lDone[0] );

            bRes = bRes && lRes == nextCharacters[i][2];
            bRes = bRes && lDone[0] == nextCharacters[i][3];
        }

        tRes.tested("nextCharacters()", bRes);
    }

    /**
     * Compares returned previous character positions with expected values. <p>
     *
     * Has <b>OK</b> status if position after travel and traveled length
     * has expected values.
     */
    public void _previousCharacters() {
        short nCharacterIteratorMode =
            com.sun.star.i18n.CharacterIteratorMode.SKIPCHARACTER;


        //Start from position : Travel ... chars : Actual position after :
        //How many chars traveled
        int[][] previousCharacters = {
            {5, 5000, 0, 5},
            {10, 6, 4, 6}};

        boolean bRes = true;
        for(int i = 0; i < previousCharacters.length; i++) {
            int[] lDone = new int[1];
            int lRes = oObj.previousCharacters(UnicodeString,
                previousCharacters[i][0],
                locale, nCharacterIteratorMode,
                previousCharacters[i][1], lDone);
            log.println("Expected result is: lRes = " + previousCharacters[i][2]
                + "; lDone = " + previousCharacters[i][3] );
            log.println("Actual result is: lRes = " + lRes
                + "; lDone = " + lDone[0]);

            bRes = bRes && lRes == previousCharacters[i][2];
            bRes = bRes && lDone[0] == previousCharacters[i][3];
        }

        tRes.tested("previousCharacters()", bRes);
    }

    ArrayList<Boundary> vBounds = new ArrayList<Boundary>();

    /**
    * Saves bounds of all returned words for the future tests. <p>
    * Has <b>OK</b> status.
    */
    public void _nextWord() {
        int i = 0;

        while( i < UnicodeString.length() - 1 ) {
            Boundary bounds = oObj.nextWord
                (UnicodeString, i, locale, wordType);
            if (bounds.endPos - bounds.startPos > 3) {
                vBounds.add( bounds );
                log.println("Word " + vBounds.size() + "("
                    + bounds.startPos + "," + bounds.endPos + "): '" +
                    UnicodeString.substring(bounds.startPos,
                                            bounds.endPos) + "'");
            }
            i = bounds.endPos - 1;
        }
        log.println("In text there are " + vBounds.size()
            + " words, if count from left to right");
        tRes.tested("nextWord()", true);
    }

    /**
    * Compares number of word bounds with number of word bounds saved
    * by the method _nextWord().<p>
    * Has <b>OK</b> status if number of word bounds are equal.
    */
    public void _previousWord() {
        requiredMethod("nextWord()");

        int i = UnicodeString.length() - 1;
        ArrayList<Boundary> vPrevBounds = new ArrayList<Boundary>();
        while( i > 0  ) {
            Boundary bounds =
                oObj.previousWord(UnicodeString, i, locale, wordType);
            if (bounds.endPos - bounds.startPos > 3) {
                vPrevBounds.add( bounds );
                log.println("Word " + vPrevBounds.size() + "("
                    + bounds.startPos + "," + bounds.endPos + "): '"
                    + UnicodeString.substring(bounds.startPos, bounds.endPos)
                    + "'");
            }
            i = bounds.startPos;
        }
        log.println("In text there are " + vPrevBounds.size()
            + " words, if count from right to left");
        tRes.tested("previousWord()", vPrevBounds.size() == vBounds.size() );
    }

    /**
     * For every word in array obtained by <code>nextWord</code> method test
     * computes bounds of the word, passing its internal character position.<p>
     *
     * Has <b>OK</b> status if bounds calculated by <code>getWordBoundary()</code>
     * method are the same as bounds obtained by <code>nextWord</code> method.
     */
    public void _getWordBoundary() {
        requiredMethod("nextWord()");

        boolean bRes = true;

        for(int i = 0; i < vBounds.size(); i++) {
            // calculate middle of the word
            Boundary iBounds = vBounds.get(i);
            int iPos = (iBounds.endPos - iBounds.startPos) / 2
                        + iBounds.startPos;
            Boundary bounds = oObj.getWordBoundary(UnicodeString, iPos,
                locale, wordType, true);
            log.println("Expected result is: startPos = " + iBounds.startPos +
                                 "; endPos = " + iBounds.endPos);
            log.println("Actual result is: startPos = " + bounds.startPos
                + "; endPos = " + bounds.endPos + " Word is: '"
                + UnicodeString.substring(bounds.startPos, bounds.endPos) + "'");

            bRes = bRes && iBounds.startPos == bounds.startPos;
            bRes = bRes && iBounds.endPos == bounds.endPos;
        }

        tRes.tested("getWordBoundary()", bRes);
    }

    /**
     * For every word in array obtained by <code>nextWord</code> method test
     * get its type, passing its internal character position.<p>
     *
     * Has <b>OK</b> status if every word has type <code>WordType.ANY_WORD</code>
     */
    public void _getWordType() {
        requiredMethod("nextWord()");

        boolean bRes = true;

        for(int i = 0; i < vBounds.size(); i++) {
            // calculate middle of the word
            Boundary iBounds = vBounds.get(i);
            int iPos = (iBounds.endPos - iBounds.startPos) / 2
                        + iBounds.startPos;

            short type = oObj.getWordType(UnicodeString, iPos, locale);

            bRes = bRes && type == WordType.ANY_WORD;
        }

        tRes.tested("getWordType()", bRes);
    }

    /**
     * For every word in array obtained by <code>nextWord</code> method test
     * tries to determine if the character at a position starts a word.
     * First word starting position is passed, then internal character
     * position is passed. <p>
     * Has <b>OK</b> status if in the first case <code>true</code>
     * returned and in the second - <code>false</code> for every word.
     */
    public void _isBeginWord() {
        requiredMethod("nextWord()");

        boolean bRes = true;

        for(int i = 0; i < vBounds.size(); i++) {
            Boundary iBounds = vBounds.get(i);
            boolean isBegin = oObj.isBeginWord(UnicodeString, iBounds.startPos,
                                               locale, WordType.ANY_WORD);
            bRes = bRes && isBegin;
            boolean isNotBegin = !oObj.isBeginWord(UnicodeString,
                    iBounds.startPos + 1, locale, WordType.ANY_WORD);
            bRes = bRes && isNotBegin;

            log.println("At position + " + iBounds.startPos
                + " isBeginWord? " + isBegin);
            log.println("At position + " + (iBounds.startPos + 1)
                + " isBeginWord? " + !isNotBegin);
        }

        tRes.tested("isBeginWord()", bRes);
    }

    /**
     * For every word in array obtained by <code>nextWord</code> method test
     * tries to determine if the character at a position ends a word.
     * First word ending position is passed, then internal character
     * position is passed. <p>
     *
     * Has <b>OK</b> status if in the first case <code>true</code>
     * returned and in the second - <code>false</code> for every word.
     */
    public void _isEndWord() {
        requiredMethod("nextWord()");

        boolean bRes = true;

        for(int i = 0; i < vBounds.size(); i++) {
            Boundary iBounds = vBounds.get(i);
            boolean isEnd = oObj.isEndWord(UnicodeString, iBounds.endPos,
                locale, WordType.ANY_WORD);
            bRes = bRes && isEnd;
            boolean isNotEnd = !oObj.isEndWord(UnicodeString,
                iBounds.endPos - 1, locale, WordType.ANY_WORD);
            bRes = bRes && isNotEnd;

            log.println("At position + " + iBounds.endPos
                + " isEndWord? " + isEnd);
            log.println("At position + " + (iBounds.endPos - 1)
                + " isEndWord? " + !isNotEnd);
        }

        tRes.tested("isEndWord()", bRes);
    }

    ArrayList<Integer> vSentenceStart = new ArrayList<Integer>();
    /**
     * Tries to find all sentences starting positions passing every character
     * as position parameter and stores them. Then tries to pass invalid
     * position parameters.
     *
     * Has <b>OK</b> status if -1 is returned for wrong position arguments.
     */
    public void _beginOfSentence() {
        int iPos = 0;
        while( iPos < UnicodeString.length() ) {
            Integer start = Integer.valueOf( oObj.beginOfSentence(UnicodeString,
                iPos, locale) );
            if (start.intValue() >= 0 && !vSentenceStart.contains(start) ) {
                vSentenceStart.add( start );
                log.println("Sentence " + vSentenceStart.size()
                    + " : start from position " + start);
            }
            iPos++;
        }

        //test for invalid nStartPosition
        boolean bRes = oObj.beginOfSentence(UnicodeString, -10, locale) == -1;
        bRes &= oObj.beginOfSentence(UnicodeString,
            UnicodeString.length() + 1, locale) == -1;

        if (!bRes) {
            log.println("When invalid position, returned value isn't equal to -1");
        }

        tRes.tested("beginOfSentence()", bRes);
    }

    /**
     * For every sentence starting position found in
     * <code>beginOfSentence()</code> test tries to compute end
     * position of a sentence and checks that the end position is
     * greater than starting.
     * Then wrong position arguments are passed.
     *
     * Has <b>OK</b> status if the end position of every sentence
     * greater than starting and -1 returned for invalid arguments.
     */
    public void _endOfSentence() {
        boolean bRes = true;
        for(int i = 0; i < vSentenceStart.size(); i++) {
            int start = vSentenceStart.get(i).intValue();
            int end = oObj.endOfSentence(UnicodeString, start, locale);
            bRes &= end > start;
            log.println("Sentence " + i + " range is [" + start + ", "
                + end + "]");
        }

        //test for invalid nStartPosition
        boolean bInvRes = oObj.endOfSentence(UnicodeString, -10, locale) == -1;
        bInvRes &= oObj.endOfSentence(UnicodeString,
            UnicodeString.length() + 1, locale) == -1;

        if (!bInvRes) {
            log.println("When invalid position, returned value isn't equal to -1");
        }

        tRes.tested("endOfSentence()", bRes && bInvRes);
    }

    /**
    * Tries to break a string in position other than 0 iterating characters
    * from the string beginning (Hyphenation is not used for a while). <p>
    *
    * Has <b>OK</b> status if non-zero break position was found and it is
    * less or equal than position we trying to break.
    */
    public void _getLineBreak() {
        boolean bRes = true;
        LineBreakResults lineBreakResults;
        LineBreakHyphenationOptions lineBreakHyphenationOptions =
            new LineBreakHyphenationOptions();
        LineBreakUserOptions lineBreakUserOptions = new LineBreakUserOptions();

        lineBreakUserOptions.applyForbiddenRules = false;
        lineBreakUserOptions.allowHyphenateEnglish = false;

        int breakPos = 0;
        int pos = 0;

        while(breakPos == 0 && pos < UnicodeString.length() ) {
            lineBreakResults = oObj.getLineBreak(UnicodeString, pos,
                locale, 0, lineBreakHyphenationOptions, lineBreakUserOptions);
            breakPos = lineBreakResults.breakIndex;
            pos++;
        }

        // finally the position of break must be found in the middle and
        // it must be before the break position specified
        bRes = breakPos <= pos && breakPos > 0;

        if (!bRes) {
            log.println("The last position was: " + pos
                + ", and the break position was: " + breakPos);
        }

        tRes.tested("getLineBreak()", bRes);
    }

    // Asian type script
    private static String katakana = new String(new char[] {0x30A1, 0x30A2}) ;
    // Weak type script
    private static String arrows = new String(new char[] {0x2190, 0x2191}) ;
    // Complex type script
    private static String arabic = new String(new char[] {0x0641, 0x0642}) ;

    /**
    * Tries to find the beginning of the nearest script specified
    * relatively to position passed. <p>
    * Has <b>OK</b> status if the starting position of script is returned.
    */
    public void _beginOfScript() {
        String multiScript = "ab" + katakana  ;

        int pos = oObj.beginOfScript(multiScript, 3, ScriptType.ASIAN) ;

        log.println("Position = " + pos) ;

        tRes.tested("beginOfScript()", pos == 2) ;
    }

    /**
    * Tries to find the end of the nearest script specified
    * relatively to position passed. <p>
    * Has <b>OK</b> status if the end position of script is returned.
    */
    public void _endOfScript() {
        String multiScript = "ab" + katakana + "cd" ;

        int pos = oObj.endOfScript(multiScript, 2, ScriptType.ASIAN) ;

        log.println("Position = " + pos) ;

        tRes.tested("endOfScript()", pos == 4) ;
    }

    /**
    * Tries to find the next script starting position specified
    * relatively to position passed. <p>
    * Has <b>OK</b> status if the appropriate position is returned.
    */
    public void _nextScript() {
        String multiScript = "ab" + katakana + "cd"  ;

        int pos = oObj.nextScript(multiScript, 0, ScriptType.LATIN) ;

        log.println("Position = " + pos) ;

        tRes.tested("nextScript()", pos == 4) ;
    }

    /**
    * Tries to find the previous script starting position specified
    * relatively to position passed. <p>
    * Has <b>OK</b> status if the appropriate position is returned.
    */
    public void _previousScript() {
        String multiScript = "ab" + katakana + "cd"  ;

        int pos = oObj.previousScript(multiScript, 5, ScriptType.ASIAN) ;

        log.println("Position = " + pos) ;

        tRes.tested("previousScript()", pos == 2) ;
    }

    /**
    * Tries to determine script type (of all four types). <p>
    * Has <b>OK</b> status if <code>LATIN</code> type returned
    * for ASCII character, <code>ASIAN</code> for Katakana Unicode
    * codepoints, <code>COMPLEX</code> for Arabic Unicode
    * codepoints and <code>WEAK</code> for codepoints from Arrows
    * Unicode block.
    */
    public void _getScriptType() {
        boolean res = true ;

        res &= oObj.getScriptType("abcd", 0) == ScriptType.LATIN ;
        res &= oObj.getScriptType(katakana, 0) == ScriptType.ASIAN;
        res &= oObj.getScriptType(arabic, 0) == ScriptType.COMPLEX ;
        res &= oObj.getScriptType(arrows, 0) == ScriptType.WEAK ;

        tRes.tested("getScriptType()", res) ;
    }

    boolean bCharBlockRes = true;

    protected short getCharBlockType(int pos) {
        short i = 1;
        short cType = 0;
        while (i < 31) {
            if (oObj.beginOfCharBlock(UnicodeString, pos, locale, i) != -1) {
                cType = i;
                i = 100;
            }
            i++;
        }

        return cType;
    }

    ArrayList<Boundary> vCharBlockBounds = new ArrayList<Boundary>();
    ArrayList<Short> vCharBlockTypes = new ArrayList<Short>();

    /**
     * Creates array of all char blocks with their boundaries and
     * types using <code>beginOfCharBlock()</code> and
     * <code>endOfCharBlock()</code> methods. <p>
     *
     * Has <b>OK</b> status if the end of each boundary is the same
     * as start of the next one and if the start of the first block
     * has position 0 and the end of the last block is at the end
     * of the whole string.
     */
    public void _beginOfCharBlock() {
        int iPos = 0;

        while( iPos < UnicodeString.length() && iPos > -1) {
            short charType = getCharBlockType(iPos);
            int startPos = oObj.beginOfCharBlock(UnicodeString, iPos,
                locale, charType);
            int endPos = oObj.endOfCharBlock(UnicodeString, iPos,
                locale, charType);
            iPos = endPos;
            vCharBlockBounds.add(new Boundary(startPos, endPos));
            log.println(vCharBlockBounds.size() + "). Bounds: ["
                + startPos + "," + endPos + "]; Type = " + charType);
            vCharBlockTypes.add(Short.valueOf(charType));
        }

        for(int i = 0; i < vCharBlockBounds.size() - 1; i++) {
            int endPos = vCharBlockBounds.get(i).endPos;
            int startPos = vCharBlockBounds.get(i + 1).startPos;
            if (endPos != startPos) {
                bCharBlockRes = false;
            }
        }

        log.println("Testing for no intersections : " + bCharBlockRes);
        int startPos = vCharBlockBounds.get(0).startPos;
        if (startPos != 0) {
            bCharBlockRes = false;
        }
        int endPos = vCharBlockBounds.get
            (vCharBlockBounds.size() - 1).endPos;
        if (endPos != UnicodeString.length()) {
            bCharBlockRes = false;
        }
        log.println("Regions should starts with 0 and ends with "
            + UnicodeString.length());

        tRes.tested("beginOfCharBlock()", bCharBlockRes);
    }

    /**
     * Testing of this method is performed in <code>beginOfCharBlock()</code>
     * method test. <p>
     *
     * Has the status same as <code>beginOfCharBlock()</code> method status.
     */
    public void _endOfCharBlock() {
        requiredMethod("beginOfCharBlock()");
        tRes.tested("endOfCharBlock()", bCharBlockRes);
    }

    /**
     * For every character block obtained in <code>beginOfCharBlock()</code>
     * method test (except the first) tries to find its starting position
     * by mean of <code>nextCharBlock()</code> method passing as position
     * argument the position before the start of a block. <p>
     *
     * Has <b>OK</b> status if the start of every block was found and it's
     * equal to this block boundary start.
     */
    public void _nextCharBlock() {
        requiredMethod("beginOfCharBlock()");

        boolean bRes = true;
        for(int i = 0; i < vCharBlockBounds.size(); i++) {
            Boundary bounds = vCharBlockBounds.get(i);
            Short type = vCharBlockTypes.get(i);
            if (bounds.startPos - 1 < 0) continue;
            int iPos = oObj.nextCharBlock(UnicodeString, bounds.startPos - 1,
                locale, type.shortValue());
            if (iPos != bounds.startPos) {
                bRes = false;
                log.println("nextCharBlock(UnicodeString, "
                    + (bounds.startPos - 1) + ", locale, " + type
                    + ") should return " + bounds.startPos);
                log.println("... and actual value is " + iPos);
            }
        }

        tRes.tested("nextCharBlock()", bRes);
    }

    /**
     * For every character block obtained in <code>beginOfCharBlock()</code>
     * method test (except the first) tries to find its starting position
     * by mean of <code>previousCharBlock()</code> method passing as position
     * argument the position after the end of a block. <p>
     *
     * Has <b>OK</b> status if the start of every block was found and it's
     * equal to this block boundary start.
     */
    public void _previousCharBlock() {
        requiredMethod("beginOfCharBlock()");

        boolean bRes = true;
        for(int i = 0; i < vCharBlockBounds.size(); i++) {
            Boundary bounds = vCharBlockBounds.get(i);
            Short type = vCharBlockTypes.get(i);
            int iPos = oObj.previousCharBlock(UnicodeString,
                bounds.endPos + 1, locale, type.shortValue());
            if (iPos != bounds.startPos) {
                bRes = false;
                log.println("previousCharBlock(UnicodeString, "
                    + (bounds.endPos + 1) + ", locale, " + type
                    + ") should return " + bounds.startPos);
                log.println("... and actual value is " + iPos);
            }
        }

        tRes.tested("previousCharBlock()", bRes);
    }

}

