/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JobProgressIndicator.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
