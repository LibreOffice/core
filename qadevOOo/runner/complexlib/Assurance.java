/*
 * ************************************************************************
 * 
 *  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *  
 *  Copyright 2008 by Sun Microsystems, Inc.
 * 
 *  OpenOffice.org - a multi-platform office productivity suite
 * 
 *  $RCSfile: Assurance.java,v $
 *  $Revision: 1.1.2.1 $
 * 
 *  This file is part of OpenOffice.org.
 * 
 *  OpenOffice.org is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3
 *  only, as published by the Free Software Foundation.
 * 
 *  OpenOffice.org is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License version 3 for more details
 *  (a copy is included in the LICENSE file that accompanied this code).
 * 
 *  You should have received a copy of the GNU Lesser General Public License
 *  version 3 along with OpenOffice.org.  If not, see
 *  <http://www.openoffice.org/license.html>
 *  for a copy of the LGPLv3 License.
 * 
 * ***********************************************************************
 */

package complexlib;

/**
 *
 * @author ll93751
 * 
 * I have removed the assure(...) functions from ComplexTestCase due to the fact now I can
 * use the functions every where and don't need to be a ComplexTestCase any longer.
 */
public class Assurance
{
    /** State of the current test method **/
    protected boolean state = true;

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
     * Assure that two boolean values are equal
     * @param expected specifies the expected boolean value
     * @param actual specifies the actual boolean value
     */
    protected void assureEquals( boolean expected, boolean actual ) {
        assureEquals( "Equality test failed", new Boolean( expected ), new Boolean( actual ), false );
    }

    /**
     * Assure that two boolean values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected boolean value
     * @param actual specifies the actual boolean value
     */
    protected void assureEquals( String message, boolean expected, boolean actual ) {
        assureEquals( message, new Boolean( expected ), new Boolean( actual ), false );
    }

    /**
     * Assure that two byte values are equal
     * @param expected specifies the expected byte value
     * @param actual specifies the actual byte value
     */
    protected void assureEquals( byte expected, byte actual ) {
        assureEquals( "Equality test failed", new Byte( expected ), new Byte( actual ), false );
    }

    /**
     * Assure that two byte values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected byte value
     * @param actual specifies the actual byte value
     */
    protected void assureEquals( String message, byte expected, byte actual ) {
        assureEquals( message, new Byte( expected ), new Byte( actual ), false );
    }

    /**
     * Assure that two double values are equal
     * @param expected specifies the expected double value
     * @param actual specifies the actual double value
     */
    protected void assureEquals( double expected, double actual ) {
        assureEquals( "Equality test failed", new Double( expected ), new Double( actual ), false );
    }

    /**
     * Assure that two double values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected double value
     * @param actual specifies the actual double value
     */
    protected void assureEquals( String message, double expected, double actual ) {
        assureEquals( message, new Double( expected ), new Double( actual ), false );
    }

    /**
     * Assure that two float values are equal
     * @param expected specifies the expected float value
     * @param actual specifies the actual float value
     */
    protected void assureEquals( float expected, float actual ) {
        assureEquals( "Equality test failed", new Float( expected ), new Float( actual ), false );
    }

    /**
     * Assure that two float values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected float value
     * @param actual specifies the actual float value
     */
    protected void assureEquals( String message, float expected, float actual ) {
        assureEquals( message, new Float( expected ), new Float( actual ), false );
    }

    /**
     * Assure that two short values are equal
     * @param expected specifies the expected short value
     * @param actual specifies the actual short value
     */
    protected void assureEquals( short expected, short actual ) {
        assureEquals( "Equality test failed", new Short( expected ), new Short( actual ), false );
    }

    /**
     * Assure that two short values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected short value
     * @param actual specifies the actual short value
     */
    protected void assureEquals( String message, short expected, short actual ) {
        assureEquals( message, new Short( expected ), new Short( actual ), false );
    }

    /**
     * Assure that two int values are equal
     * @param expected specifies the expected int value
     * @param actual specifies the actual int value
     */
    protected void assureEquals( int expected, int actual ) {
        assureEquals( "Equality test failed", new Integer( expected ), new Integer( actual ), false );
    }

    /**
     * Assure that two int values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected int value
     * @param actual specifies the actual int value
     */
    protected void assureEquals( String message, int expected, int actual ) {
        assureEquals( message, new Integer( expected ), new Integer( actual ), false );
    }

    /**
     * Assure that two long values are equal
     * @param expected specifies the expected long value
     * @param actual specifies the actual long value
     */
    protected void assureEquals( long expected, long actual ) {
        assureEquals( "Equality test failed", new Long( expected ), new Long( actual ), false );
    }

    /**
     * Assure that two long values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected long value
     * @param actual specifies the actual long value
     */
    protected void assureEquals( String message, long expected, long actual ) {
        assureEquals( message, new Long( expected ), new Long( actual ), false );
    }

    /**
     * Assure that two string values are equal
     * @param expected specifies the expected string value
     * @param actual specifies the actual string value
     */
    protected void assureEquals( String expected, String actual ) {
        assureEquals( "Equality test failed", expected, actual, false );
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
     * Assure that two object are equal
     * @param expected specifies the expected object value
     * @param actual specifies the actual object value
     */
    protected void assureEquals( Object expected, Object actual ) {
        assureEquals( "Equality test failed", expected, actual, false );
    }

    /**
     * Assure that two objects are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected object value
     * @param actual specifies the actual object value
     */
    protected void assureEquals( String message, Object expected, Object actual ) {
        assureEquals( message, expected, actual, false );
    }

    /**
     * Mark the currently executed method as failed.
     * This function generates "Test did fail." as standard message.
     */
    protected void failed() {
        assure("Test did fail.", false, false);
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
        state &= s;
        if (!s) {
            message += msg + "\r\n";
            // log.println(msg);
            if (!cont) {
                throw new AssureException(msg);
            }
        }
    }

    protected void assureEquals( String message, Object expected, Object actual, boolean cont ) {
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

    /**
     * @deprecated
     */
//    protected void addResult(String message, boolean state) {
//        String msg = message + " - " + state;
//        this.state &= state;
//        this.message += msg + "\r\n";
//        log.println(msg);
//    }

    public class AssureException extends RuntimeException {

        public AssureException(String msg) {
            super(msg);
        }
    }
}
