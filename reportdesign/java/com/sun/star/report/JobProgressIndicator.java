/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JobProgressIndicator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:23:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
package com.sun.star.report;

/**
 * Based on
 * http://api.openoffice.org/docs/common/ref/com/sun/star/task/XStatusIndicator.html
 *
 * @author Thomas Morgner
 */
public interface JobProgressIndicator
{

    /**
     * Updates the progress description.
     *
     * @param string the new description
     */
    public void setText(String string);

    /**
     * Initializes the progress indicator and sets the progress description to
     * the text given in the parameter <code>text</code>. The progress values
     * passed to this indicator will not exceed the given <code>maxValue</code>.
     * The value range of this indicator is set to 0 to <code>maxValue</code>
     * Stopped indicators must ignore that call.
     *
     * @param string the progress description
     * @param maxValue the maximum value
     * @see JobProgressIndicator#setText(String)
     */
    public void start(String text, int maxValue);

    /**
     * Updates the value to the specified value. Invalid values must be ignored.
     * Stopped indicators must ignore that call.
     *
     * @param value the new value that should be shown now. This must fit the
     * range [0..maxValue] as defined in {@link JobProgressIndicator#start(String, int)}.
     */
    public void setValue(int value);

    /**
     * Stop the progress. A progress indicator cannot be destructed before end()
     * has been called.
     */
    public void end();

    /**
     * Clear progress value and description. Calling of setValue(0) and
     * setText("") should do the same. Stopped indicators must ignore this call.
     */
    public void reset();
}
