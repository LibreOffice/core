/*************************************************************************
 *
 *  $RCSfile: ErrorHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:11:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.web;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public interface ErrorHandler {

  /**
   * Error type for fatal errors which should abort application
   * execution. Should actually never be used :-)
   */
  public static final int ERROR_FATAL = 0;
  /**
   * An Error type for errors which should stop the current process.
   */
  public static final int ERROR_PROCESS_FATAL = 1;
  /**
   * An Error type for errors to which the user can choose, whether
   * to continue or to abort the current process.
   * default is abort.
   */
  public static final int ERROR_NORMAL_ABORT = 2;
  /**
   * An Error type for errors to which the user can choose, whether
   * to continue or to abort the current process.
   * default is continue.
   */
  public static final int ERROR_NORMAL_IGNORE = 3;
  /**
   * An error type for warnings which requires user interaction.
   * (a question :-) )
   * Default is abort (cancel).
   */
  public static final int ERROR_QUESTION_CANCEL = 4;
  /**
   * An error type for warnings which requires user interaction
   * (a question :-) )
   * Default is to continue (ok).
   */
  public static final int ERROR_QUESTION_OK = 5;
  /**
   * An error type for warnings which requires user interaction.
   * (a question :-) )
   * Default is abort (No).
   */
  public static final int ERROR_QUESTION_NO = 6;
  /**
   * An error type for warnings which requires user interaction
   * (a question :-) )
   * Default is to continue (Yes).
   */
  public static final int ERROR_QUESTION_YES = 7;
  /**
   * An error type which is just a warning...
   */
  public static final int ERROR_WARNING = 8;
  /**
   * An error type which just tells the user something
   * ( like "you look tired! you should take a bath! and so on)
   */
  public static final int ERROR_MESSAGE = 9;



  /**
   * @param ex the exception that accured
   * @param arg an object as help for recognizing the exception
   * @param ix an integer which helps for detailed recognizing of the exception
   * @param errorType one of the int constants defined by this Interface
   * @return true if the execution should continue, false if it should stop.
   */
  public boolean error(Exception ex, Object arg, int ix ,int errorType);
}
