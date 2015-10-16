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

package complexlib;

/**
 * I have removed the assure(...) functions from ComplexTestCase due to the fact now I can
 * use the functions every where and don't need to be a ComplexTestCase any longer.
 */
public class Assurance
{
    public static final boolean CONTINUE = true;

    /** State of the current test method **/
    protected boolean bSuccessful = true;

    /** The message if the test did fail **/
    protected String message = null;


    /**
     * Assure that s is true.
     * This function generates "Assure failed." as standard message.
     * @param s The condition that should be true.
     */
    protected void assure(boolean s) {
        assure("Assure failed.", s, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     */
    protected void assure(String msg, boolean s) {
        assure(msg, s, false);
    }

    /**
     * Assure that two int values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected int value
     * @param actual specifies the actual int value
     */
    protected void assureEquals( String message, int expected, int actual ) {
        assureEquals( message, Integer.valueOf( expected ), Integer.valueOf( actual ), false );
    }

    /**
     * Assure that two string values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected string value
     * @param actual specifies the actual string value
     */
    protected void assureEquals( String message, String expected, String actual ) {
        assureEquals( message, expected, actual, false );
    }

    /**
     * assures the two given sequences are of equal length, and have equal content
     */
    public <T> void assureEquals( String i_message, T[] i_expected, T[] i_actual, boolean i_continue )
    {
        if ( i_expected.length != i_actual.length )
            failed( i_message + ": expected element count: " + i_expected.length + ", actual element count: " + i_actual.length );
        for ( int i=0; i<i_expected.length; ++i )
        {
            assureEquals( i_message + ": mismatch at element pos " + i, i_expected[i], i_actual[i], i_continue );
        }
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * @param msg The message of the failure.
     */
    protected void failed(String msg) {
        assure(msg, false, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * Normally, assure() leaves the current test method, and the next one
     * is executed. With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     * @param cont Continue with test method, even if s is false.
     */
    protected void assure(String msg, boolean s, boolean cont) {
        bSuccessful &= s;
        if (!s) {
            message += msg + "\r\n";
            if (!cont) {
                throw new AssureException(msg);
            }
        }
    }

    private void assureEquals( String message, Object expected, Object actual, boolean cont ) {
        assure( message + " (expected: " + expected.toString() + ", actual: " + actual.toString() + ")",
            expected.equals( actual ), cont );
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * The given message will be only evaluated, if s is false.
     * With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param cont Continue with test method, even if s is false.
     */
    protected void failed(String msg, boolean cont) {
        assure(msg, false, cont);
    }

    public static class AssureException extends RuntimeException {

        private AssureException(String msg) {
            super(msg);
        }
    }
}
