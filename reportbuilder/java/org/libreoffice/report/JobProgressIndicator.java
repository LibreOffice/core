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
package org.libreoffice.report;

/**
 * Based on
 * http://api.libreoffice.org/docs/common/ref/com/sun/star/task/XStatusIndicator.html
 *
 */
public interface JobProgressIndicator
{

    /**
     * Updates the progress description.
     *
     * @param string the new description
     */
    void setText(String string);

    /**
     * Initializes the progress indicator and sets the progress description to
     * the text given in the parameter <code>text</code>. The progress values
     * passed to this indicator will not exceed the given <code>maxValue</code>.
     * The value range of this indicator is set to 0 to <code>maxValue</code>
     * Stopped indicators must ignore that call.
     *
     * @param string the progress description
     * @param text
     * @param maxValue the maximum value
     * @see JobProgressIndicator#setText(String)
     */
    void start(String text, int maxValue);

    /**
     * Updates the value to the specified value. Invalid values must be ignored.
     * Stopped indicators must ignore that call.
     *
     * @param value the new value that should be shown now. This must fit the
     * range [0..maxValue] as defined in {@link JobProgressIndicator#start(String, int)}.
     */
    void setValue(int value);

    /**
     * Stop the progress. A progress indicator cannot be destructed before end()
     * has been called.
     */
    void end();

    /**
     * Clear progress value and description. Calling of setValue(0) and
     * setText("") should do the same. Stopped indicators must ignore this call.
     */
    void reset();
}
